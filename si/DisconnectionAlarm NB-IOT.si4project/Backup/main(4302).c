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
**�������ƣ�Timer1Init
**������������ʱ��1��ʼ����           ģʽ1 ��16��������   5ms�ж�
**��ڲ�������
**���ڲ�������
***************************************************************/
void Timer1Init(void)			//T1��ʼ��	3.6864 MHZ/12*4       1������0.814us
{
 	TMOD = (TMOD&0xFF)|0x10;     //��ʱ��1 ģʽ1 16������  
 	TH1 = 0xFB;TL1 = 0x33;		//1ms
	//TH1=0xE8;TL1=0x02;	 	//     5ms     5,000 / 0.814 = 6142    65536-12285 = 59394
	//TH1=0xD0;TL1=0x03;	//10ms 	
	
	ET1=1;
	TR1=1;
	
}

/**************************************************************
**�������ƣ�Timer1Zd
**������������ʱ��1�жϡ�1ms�ж� ˢ�¹��̼�ʱTime_ms
**��ڲ�������
**���ڲ�������
***************************************************************/
void Timer1Zd(void) interrupt 3	   	//��ʱ��0�ж�
{
	TH1 = 0xFB;TL1 = 0x33;		//1ms

	//TH1=0xE8;TL1=0x02;			//5ms�ж�һ��	

	//TH0=0xD0;	 TL0=0x03;	 	//10ms�ж�һ��	
	
	Time_ms++;
	if(Time_ms == 0xffff)
	{
		Time_ms = 0;
	}
	
	NBTickClk();
}

/**************************************************************
**�������ƣ�GetProClockTick
**����������ȡ���̼�ʱTime_ms
**��ڲ�������
**���ڲ�������
***************************************************************/
uint16_t GetProClockTick(void)
{
	return Time_ms;
}



void main(void)
{
	uint8_t i;

	Sys_Clk_Set_PLL(PLL_Multiple);	//ʱ�ӳ�ʼ��
	
	Timer1Init();					//timer1��ʼ��
	Uart0_Initial(UART0_BAUTRATE);          //����0��ʼ��
	Uart1_Initial(UART1_BAUTRATE);			//����1��ʼ��	
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
				ALARM_OUT = 0;   //�رձ�����
				//IT0 = 1;		 //����Ϊ�������ж�
				g_alarmfg = 0;
			}
		}
*/		
	}

	return;
}	


#endif

