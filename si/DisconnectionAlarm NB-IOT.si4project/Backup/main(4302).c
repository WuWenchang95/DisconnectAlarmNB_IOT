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

#include "../../Library/includes/uart.h"
#include "../../Library/includes/delay.h"
#include "../NB_iot/Nb_iot.h"
#include "../Wirealarm/Wirealarm.h"

//#include "TM1640.h"
//#include "flowfunction.h"
//#include "key.h"
//#include "buzzer.h"

#include <intrins.h>

//uint8_t xdata Time_10ms;
uint16_t xdata Time_ms;
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



void main(void)
{
	uint8_t i;

	Sys_Clk_Set_PLL(PLL_Multiple);	//时钟初始化
	
	Timer1Init();					//timer1初始化
	Uart0_Initial(UART0_BAUTRATE);          //串口0初始化
	Uart1_Initial(UART1_BAUTRATE);			//串口1初始化	
	NB73Init();
	AlarmGPIOInit();
	INT0Init(1);


	
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
/*
		if((WIRE_IN == 0)&&(g_alarmfg))
		{
			Delay_ms(100);
			if(WIRE_IN == 0)
			{
				ALARM_OUT = 0;   //关闭报警器
				//IT0 = 1;		 //设置为上升沿中断
				g_alarmfg = 0;
			}
		}
*/		
	}

	return;
}	


#endif

