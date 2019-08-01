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
#include "../NB_iot/Nb_iot.h"
#include "../Wirealarm/Wirealarm.h"
#include "../../Library/includes/pwm.h"

//#include "TM1640.h"
//#include "flowfunction.h"
//#include "key.h"
//#include "buzzer.h"

#include <intrins.h>

//uint8_t xdata Time_10ms;
uint16_t xdata Time_ms;
uint8_t g_sleepFg;
uint16_t g_sleepCnt;
extern uint8_t g_alarmfg;

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

/*
	if(WIRE_IN == 0)
	{
		g_alarmfg++;
		if(g_alarmfg > 50)
		{
			ALARM_OUT = 0;	 //关闭报警器
			g_alarmfg = 0;
		}
		
	}
*/	

	if(recTime > 0)
	{
		recTime--;
		if(recTime == 0)
		{
			uart1_rx_finish_flag = 1;
		}
	}
		
	if(WIRE_IN == 1)
	{
		if(tAlarmDri.cnt < 50)
		{
			tAlarmDri.cnt++;
			if(tAlarmDri.cnt >= 49)
			{
				if(tAlarmDri.alarmFlag == 0)
				{
					tAlarmDri.alarmFlag = 1;
					SetSendAlarmDataToCloud(DISCONNECT_WIRE);
				
				}
				
			}
		}
	}
	else if(WIRE_IN == 0)
	{
		if(tAlarmDri.cnt > 0)
		{
			tAlarmDri.cnt--;
			if(tAlarmDri.cnt == 0)
			{
				if(tAlarmDri.alarmFlag == 1)
				{
					tAlarmDri.alarmFlag = 0;
				
				}
				
			}
		}		
	}

	if(tAlarmDri.alarmFlag)
	{
		//ALARM_OUT = 1;   //打开报警器
		//ALARM_ON;
	}
	else
	{
		//ALARM_OUT = 0;   //关闭报警器
		ALARM_OFF;		
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

void WakeUp(void)
{

	Sys_Clk_Set_PLL(PLL_Multiple);
	Timer1Init();							//timer1初始化
	Uart0_Initial(UART0_BAUTRATE);          //串口0初始化
	Uart1_Initial(UART1_BAUTRATE);			//串口1初始化	
	NB73Init();
	INT0Init(0);  //
	AlarmGPIOInit();
	
	BuzzerInitSet();
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
#if (POWER_SAVING_MODE == STOP_MODE)
	bit IE_EA;
	I2CCON = 0;						//¹Ø±ÕI2C£¬ÒòÎªI2CÉÏµçÄ¬ÈÏÊÇ´ò¿ªµÄ£¬±ØÐë¹Ø±Õ£¬·ñÔòÖ÷Ê±ÖÓÎÞ·¨¹Ø±Õ
	CKCON = 0;						//¹Ø±ÕËùÓÐÊ±ÖÓ
	MECON |= BIT6;				//ÉèÖÃFLASH½øÈëÉî¶ÈË¯ÃßÄ£Ê½
	PWCON = (PWCON&0xF0) | 0x04;		//ÉèÖÃÄÚ²¿LDOÎªµÍ¹¦ÂÊÄ£Ê½
	
	while(!CheckStopCondition());
	IE_EA = EA;				//±£´æµ±Ç°×ÜÖÐ¶Ï¿ª¹Ø×´Ì¬
	EA = 1;					//¹Ø×ÜÖÐ¶Ï

	StopEnter();	//½øÈëSTOPÄ£Ê½
	
	PWCON = (PWCON&0xF0) | 0x0B;	//ÍË³öSTOPÄ£Ê½ºó±ØÐëÉèÖÃLDOÎª¸ß¹¦ÂÊÄ£Ê½

	WDFLG = 0xA5;//Ë¢ÐÂ¿´ÃÅ¹·

	EA = 1;				//»Ö¸´Ö®Ç°µÄ×ÜÖÐ¶Ï×´Ì¬	

#elif (POWER_SAVING_MODE == IDLE_MODE)
	bit IE_EA;
	I2CCON = 0;						//¹Ø±ÕI2C£¬ÒòÎªI2CÉÏµçÄ¬ÈÏÊÇ´ò¿ªµÄ£¬±ØÐë¹Ø±Õ£¬·ñÔòÖ÷Ê±ÖÓÎÞ·¨¹Ø±Õ
	CKCON = 0;						//¹Ø±ÕËùÓÐÊ±ÖÓ
	MECON |= BIT6;				//ÉèÖÃFLASH½øÈëÉî¶ÈË¯ÃßÄ£Ê½
	PWCON = (PWCON&0xF0) | 0x04;		//ÉèÖÃÄÚ²¿LDOÎªµÍ¹¦ÂÊÄ£Ê½	
	
	while(!CheckIdleCondition());
	Sys_Clk_Set_XOSCL();	//ÇÐ»»Ö÷Ê±ÖÓÎªµÍËÙÊ±ÖÓ£¬ÒòÎªIDLEÄ£Ê½Ö÷Ê±ÖÓÊÇ´ò¿ªµÄ£¬ÇÐ»»Ö÷Ê±ÖÓÎªµÍËÙÊ±ÖÓ¿ÉÓÐÐ§½µµÍ¹¦ºÄ
//	Sys_Clk_Set_IRCL();
	IE_EA = EA;				//±£´æµ±Ç°×ÜÖÐ¶Ï¿ª¹Ø×´Ì¬
	EA = 0;					//¹Ø×ÜÖÐ¶Ï
	IdleEnter();					//½øÈëIDLEÄ£Ê½
	PWCON = (PWCON&0xF0) | 0x0B;	//ÍË³öIDLEÄ£Ê½ºó±ØÐëÉèÖÃLDOÎª¸ß¹¦ÂÊÄ£Ê½
	EA = IE_EA;				//»Ö¸´Ö®Ç°µÄ×ÜÖÐ¶Ï×´Ì¬
	while(1)
	{
		
	}
#elif (POWER_SAVING_MODE == LOW_SPEED_MODE)
	I2CCON = 0;						//¹Ø±ÕI2C£¬ÒòÎªI2CÉÏµçÄ¬ÈÏÊÇ´ò¿ªµÄ£¬±ØÐë¹Ø±Õ£¬·ñÔòÖ÷Ê±ÖÓÎÞ·¨¹Ø±Õ
	CKCON = 0;						//¹Ø±ÕËùÓÐÊ±ÖÓ
	MECON |= BIT6;				//ÉèÖÃFLASH½øÈëÉî¶ÈË¯ÃßÄ£Ê½
	PWCON = (PWCON&0xF0) | 0x04;		//ÉèÖÃÄÚ²¿LDOÎªµÍ¹¦ÂÊÄ£Ê½	
	
	Sys_Clk_Set_XOSCL();	//ÇÐ»»Ö÷Ê±ÖÓÎªµÍËÙÊ±ÖÓ£¬×¢Òâ£ºµ±Ö÷Ê±ÖÓ´ÓµÍËÙÊ±ÖÓÇÐ»»»Ø¸ßËÙÊ±ÖÓÊ±£¬±ØÐë°ÑÄÚ²¿LDOÉèÖÃ»Ø¸ß¹¦ÂÊÄ£Ê½
//	Sys_Clk_Set_IRCL();
	while(1)
	{
		
	}
#endif
}
	


/**************************************************************
**函数名称：EnterStopMode
**函数描述：进入低功耗前的关闭外设
**入口参数：无
**出口参数：无
***************************************************************/
void EnterStopMode(void)
{
	SW_NB = 0;

	TR2 = 0;	//关定时器2
	ES0 = 0;	//关串口0
	ES1 = 0;	//关串口1
	
	ET0=0;		//关定时器0
	TR0=0;
	ET1=0;      //关定时器1
	TR1=0;

	PWMEN=0;
	
	INT0Init(1);  //开外部中断
	
	Sys_Clk_Set_IRCL();
	PLLCON =0;	
	WDFLG = 0xA5;//喂狗
	PowerSaving_MainLoop();

}

void main(void)
{
	uint8_t i;

	Sys_Clk_Set_PLL(PLL_Multiple);	//时钟初始化
	
	Timer1Init();					//timer1初始化
	Uart0_Initial(UART0_BAUTRATE);          //串口0初始化
	Uart1_Initial(UART1_BAUTRATE);			//串口1初始化	
	NB73Init();
	AlarmGPIOInit();
	//INT0Init(1);
	
	ALARM_OUT = 1;
	Delay_ms(50);
	ALARM_OUT = 0;
	EA = 1;	
	
#ifdef PRINT_EN 
	uart_printf("main  test!\r\n");
#endif	
	BuzzerInitSet();

	while(1)
	{
		
		NB73Tsak();
		
	}

	return;
}	


#endif

