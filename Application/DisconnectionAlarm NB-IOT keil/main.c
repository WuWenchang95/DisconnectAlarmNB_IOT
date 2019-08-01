#ifndef _MAIN_C_
#define _MAIN_C_

/*********************************************************************************************************************/
#include "ca51f_config.h"
#if (IC_TYPE == IC_TYPE_CA51F2X)
#include "../../includes/ca51f2sfr.h"
#include "../../includes/ca51f2xsfr.h"
#include "../../includes/gpiodef_f2.h"
#elif (IC_TYPE == IC_TYPE_CA51F3X)
#include "../../includes/ca51f3sfr.h"
#include "../../includes/ca51f3xsfr.h"
#include "../../includes/gpiodef_f3.h"
#endif
#include "../../includes/system.h"
#include "../../Library/includes/system_clock.h"
#include "../../Library/includes/stop_idle.h"
#include "../../Library/includes/uart.h"
#include "../../Library/includes/delay.h"
#include "../../Library/includes/pwm.h"
#include "../../Library/includes/rtc.h"
#include "../../Library/includes/adc.h"

#include "../NB_iot/Nb_iot.h"
#include "../Wirealarm/Wirealarm.h"
#include "../drive/Include/voltage_adc.h"
#include <intrins.h>

//uint8_t xdata Time_10ms;
uint16_t xdata Time_ms;
uint8_t g_sleepFg;
uint16_t g_sleepCnt;


/**************************************************************
**函数名称：Timer1Init
**函数描述：定时器1初始化、           模式1 、16计数器、   5ms中断
**入口参数：无
**出口参数：无
***************************************************************/
void Timer1Init(void)			//T1初始化	3.6864 MHZ/12*4       1个计数0.814us
{
 	TMOD = (TMOD&0xFF)|0x10;     //定时器1 模式1 16计数器  
 	TH1 = 0xFB;TL1 = 0x33;		//1ms
	//TH1=0xE8;TL1=0x02;	 	//     5ms     5,000 / 0.814 = 6142    65536-12285 = 59394
	//TH1=0xD0;TL1=0x03;	//10ms 	
	
	ET1=1;
	TR1=1;
	
}

/**************************************************************
**函数名称：Timer1Zd
**函数描述：定时器1中断、1ms中断 刷新工程计时Time_ms
**入口参数：无
**出口参数：无
***************************************************************/
void Timer1Zd(void) interrupt 3	   	//定时器0中断
{
	TH1 = 0xFB;TL1 = 0x33;		//1ms

	//TH1=0xE8;TL1=0x02;			//5ms中断一次	

	//TH0=0xD0;	 TL0=0x03;	 	//10ms中断一次	
	
	Time_ms++;
	if(Time_ms == 0xffff)
	{
		Time_ms = 0;
	}
	
	NBTickClk();
    AlarmWireCleck();
    
	
	if(recTime > 0)
	{
		recTime--;
		if(recTime == 0)
		{
			uart1_rx_finish_flag = 1;
		}
	}
	
	if(g_sleepFg == 0)
	{
		g_sleepCnt++;
		if(g_sleepCnt > 1000)
		{
			g_sleepFg = 1;
		}
	}
}

/**************************************************************
**函数名称：GetProClockTick
**函数描述：取工程计时Time_ms
**入口参数：无
**出口参数：无
***************************************************************/
uint16_t GetProClockTick(void)
{
	return Time_ms;
}

/**************************************************************
**函数名称：WakeUp
**函数描述：唤醒单片机初始化外设
**入口参数：无
**出口参数：无
***************************************************************/
void WakeUp(void)
{

	Sys_Clk_Set_PLL(PLL_Multiple);
	
	RTM_Con(RTM_Off);
	Timer1Init();							//timer1初始化
//	NB73Init();
	
//	INT2EN =0;      //INT2中断失能
//	EPIE &=(~0X01);    //外部2中断失能
	EX1 =0;     //INT1中断失能
	IE1 =0;    //外部中断失能

	AlarmGPIOInit();
#ifdef PRINT_EN 	
	Uart0_Initial(UART0_BAUTRATE);          //串口0初始化
#endif	
	Uart1_Initial(UART1_BAUTRATE);			//串口1初始化	
	BuzzerInitSet();
	
	if(tVoltageAdc.rtmWakeAdcFg)
	{
        VoltageAdcInit();
	}
		
	g_sleepFg = 0;	
	g_sleepCnt = 0;
}	


/**************************************************************
**函数名称：PowerSaving_MainLoop
**函数描述：进入低功耗
**入口参数：无
**出口参数：无
***************************************************************/
void PowerSaving_MainLoop(void)
{
	bit IE_EA;
	I2CCON = 0;						//关闭I2C，因为I2C上电默认是打开的，必须关闭，否则主时钟无法关闭
	CKCON = 0;						//关闭所有时钟
	MECON |= BIT6;				//设置FLASH进入深度睡眠模式
	PWCON = (PWCON&0xF0) | 0x04;		//设置内部LDO为低功率模式
	
	while(!CheckStopCondition());
	IE_EA = EA;				//保存当前总中断开关状态
	EA = 0;					//关总中断
	StopEnter();	//进入STOP模式
	
	PWCON = (PWCON&0xF0) | 0x0B;	//退出STOP模式后必须设置LDO为高功率模式
	EA = IE_EA;				//恢复之前的总中断状态

}

	


/**************************************************************
**函数名称：EnterStopMode
**函数描述：进入低功耗前的关闭外设
**入口参数：无
**出口参数：无
***************************************************************/
void EnterStopMode(void)
{
	tVoltageAdc.rtmWakeAdcFg = 0;
//	RTC_init(RTCE_On,MSE_Off,HSE_Off);
//	RTM_init(0,10,40);	
	RTM_Con(RTM_On);
	
	GPIO_Init(SW_NB_IO,OUTPUT);
	SW_NB = 0;

	GPIO_Init(ALARM_IO,OUTPUT);
	ALARM_OUT = 0;
	
	GPIO_Init(NB_RESET_IO,OUTPUT);
	NB_RESET = 0;
	
	PWMEN &= (~((1<<PWM_CH2)|(1<<PWM_CH3)));
	GPIO_Init(P35F,OUTPUT);
	P35 = 0;

	GPIO_Init(P10F,HIGH_Z);				//VOLTAGE_ADC_IO
	
	GPIO_Init(P11F,HIGH_Z);
	GPIO_Init(P12F,HIGH_Z);
	//GPIO_Init(P13F,HIGH_Z);
	GPIO_Init(P14F,HIGH_Z);
	GPIO_Init(P15F,HIGH_Z);
	
	//GPIO_Init(P16F,OUTPUT);
	
	GPIO_Init(P31F,HIGH_Z);
	GPIO_Init(P30F,HIGH_Z);
	
//	GPIO_Init(P32F,HIGH_Z); 	
//	GPIO_Init(P33F,HIGH_Z); 
	

#if 0
	GPIO_Init(P34F,HIGH_Z);
	GPIO_Init(P36F,HIGH_Z);
	GPIO_Init(P37F,HIGH_Z);
	GPIO_Init(P00F,HIGH_Z);
	GPIO_Init(P01F,HIGH_Z);
	GPIO_Init(P02F,HIGH_Z);
	GPIO_Init(P03F,HIGH_Z);
	GPIO_Init(P04F,HIGH_Z);
	GPIO_Init(P05F,HIGH_Z);
	GPIO_Init(P06F,HIGH_Z);
	GPIO_Init(P07F,HIGH_Z);
	GPIO_Init(P20F,HIGH_Z);
	GPIO_Init(P21F,HIGH_Z);

#else	
	GPIO_Init(P34F,OUTPUT);
	GPIO_Init(P36F,OUTPUT);
	GPIO_Init(P37F,OUTPUT);

	GPIO_Init(P00F,OUTPUT);
	GPIO_Init(P01F,OUTPUT);
	GPIO_Init(P02F,OUTPUT);
	GPIO_Init(P03F,OUTPUT);
	GPIO_Init(P04F,OUTPUT);
	GPIO_Init(P05F,OUTPUT);
	GPIO_Init(P06F,OUTPUT);
	GPIO_Init(P07F,OUTPUT);
	GPIO_Init(P20F,OUTPUT);
	GPIO_Init(P21F,OUTPUT);
	P34 = 0;
	P36 = 0;
	P37 = 0;
	P00 = 0;
	P01 = 0;
	P02 = 0;
	P03 = 0;
	P04 = 0;
	P05 = 0;
	P06 = 0;
	P07 = 0;
	P20 = 0;	
	P21 = 0;
#endif	
	EX_INT1_Init(); //开外部中断

	TR2 = 0;	//关定时器2
	ES0 = 0;	//关串口0
	ES1 = 0;	//关串口1
	
//	ET0=0;		//关定时器0
//	TR0=0;

	ET1=0;		//关定时器1
	TR1=0;
	
	INT2EN =0;      //INT2中断失能
	EPIE &=(~0X01);    //外部2中断失能
	
	ADCON |= ADIF;
	ADCON = 0;
//	ADCFGL = 0x10;
//	ADCFGH = 0x23;
	
	PWMEN=0;
			
	Sys_Clk_Set_IRCL();
	PLLCON =0;	
	//WDFLG = 0xA5;//喂狗
	
	PowerSaving_MainLoop();

}


void main(void)
{

	Sys_Clk_Set_PLL(PLL_Multiple);			//时钟初始化
	
#ifdef PRINT_EN 	
	Uart0_Initial(UART0_BAUTRATE);          //串口0初始化
#endif	

#if 1	
	Timer1Init();							//timer1初始化
	Uart1_Initial(UART1_BAUTRATE);			//串口1初始化	
	NB73Init();								//NB模块io的初始化
	AlarmGPIOInit();						//检测线 IO初始化   
	
	GPIO_Init(P32F,P32_XOSCL_IN_SETTING);	//rtc 初始化	
	GPIO_Init(P33F,P33_XOSCL_OUT_SETTING);			
	RTC_init(RTCE_On,MSE_Off,HSE_Off);
	RTM_init(0,10,40);
	RTM_Con(RTM_Off);
	
	BuzzerInitSet();						//蜂鸣器初始化		
	
	VoltageAdcInit();						//ADC初始化
	tVoltageAdc.voltageValue = 8000;
	tVoltageAdc.rtmWakeAdcFg = 0;

	tAlarmDri.forwardFg = 1;
	tAlarmDri.frp = 5000;
	
#endif

#ifdef PRINT_EN 
	uart_printf("main  test!\r\n");
#endif	

	EA = 1; 

	while(1)
	{
#if 1	
		NB73Tsak();

#ifdef PRINT_EN		
		if(Time_ms%200 == 0)
		{	
			uart_printf("V: %d\r\n",tVoltageAdc.voltageValue);
		}
#endif

#else		
		if(g_sleepFg == 1)
		{
#ifdef PRINT_EN
			uart_printf("V: %d\r\n",tVoltageAdc.voltageValue);
			uart_printf("Stop\r\n");
#endif			
			Delay_ms(2);
			
			EnterStopMode();	
			
			WakeUp();
			
#ifdef PRINT_EN				
			if(tVoltageAdc.rtmWakeAdcFg)
			{
				uart_printf("rtc wake\r\n");
			}
			else
			{
			    uart_printf("int wake\r\n");
			}
#endif					
		}

#endif
	}

	return;
}	


#endif

