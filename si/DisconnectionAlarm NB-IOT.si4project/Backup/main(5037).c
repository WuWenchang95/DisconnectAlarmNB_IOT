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

/*
	if(WIRE_IN == 0)
	{
		g_alarmfg++;
		if(g_alarmfg > 50)
		{
			ALARM_OUT = 0;	 //�رձ�����
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
		if(tAlarmDri.cnt < 100)
		{
			tAlarmDri.cnt++;
			if(tAlarmDri.cnt >= 99)
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
		//ALARM_OUT = 1;   //�򿪱�����
		ALARM_ON;
	}
	else
	{
		//ALARM_OUT = 0;   //�رձ�����
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
**�������ƣ�GetProClockTick
**����������ȡ���̼�ʱTime_ms
**��ڲ�������
**���ڲ�������
***************************************************************/
uint16_t GetProClockTick(void)
{
	return Time_ms;
}

void WakeUp(void)
{

	Sys_Clk_Set_PLL(PLL_Multiple);
	Timer1Init();							//timer1��ʼ��
	Uart0_Initial(UART0_BAUTRATE);          //����0��ʼ��
	Uart1_Initial(UART1_BAUTRATE);			//����1��ʼ��	
	NB73Init();
	INT0Init(0);  //
	AlarmGPIOInit();
	
	BuzzerInitSet();
	g_sleepFg = 0;	
	g_sleepCnt = 0;
}	


/**************************************************************
**�������ƣ�PowerSaving_MainLoop
**��������������͹���
**��ڲ�������
**���ڲ�������
***************************************************************/
	void PowerSaving_MainLoop(void)
	{
#if (POWER_SAVING_MODE == STOP_MODE)
		bit IE_EA;
		I2CCON = 0; 					//�ر�I2C����ΪI2C�ϵ�Ĭ���Ǵ򿪵ģ�����رգ�������ʱ���޷��ر�
		CKCON = 0;						//�ر�����ʱ��
		MECON |= BIT6;				//����FLASH�������˯��ģʽ
		PWCON = (PWCON&0xF0) | 0x04;		//�����ڲ�LDOΪ�͹���ģʽ
		
		while(!CheckStopCondition());
		IE_EA = EA; 			//���浱ǰ���жϿ���״̬
		EA = 1; 				//�����ж�

		StopEnter();	//����STOPģʽ
		
		PWCON = (PWCON&0xF0) | 0x0B;	//�˳�STOPģʽ���������LDOΪ�߹���ģʽ
	
		WDFLG = 0xA5;//ˢ�¿��Ź�
	
		EA = 1; 			//�ָ�֮ǰ�����ж�״̬	
	
#elif (POWER_SAVING_MODE == IDLE_MODE)
		bit IE_EA;
		I2CCON = 0; 					//�ر�I2C����ΪI2C�ϵ�Ĭ���Ǵ򿪵ģ�����رգ�������ʱ���޷��ر�
		CKCON = 0;						//�ر�����ʱ��
		MECON |= BIT6;				//����FLASH�������˯��ģʽ
		PWCON = (PWCON&0xF0) | 0x04;		//�����ڲ�LDOΪ�͹���ģʽ 
		
		while(!CheckIdleCondition());
		Sys_Clk_Set_XOSCL();	//�л���ʱ��Ϊ����ʱ�ӣ���ΪIDLEģʽ��ʱ���Ǵ򿪵ģ��л���ʱ��Ϊ����ʱ�ӿ���Ч���͹���
	//	Sys_Clk_Set_IRCL();
		IE_EA = EA; 			//���浱ǰ���жϿ���״̬
		EA = 0; 				//�����ж�
		IdleEnter();					//����IDLEģʽ
		PWCON = (PWCON&0xF0) | 0x0B;	//�˳�IDLEģʽ���������LDOΪ�߹���ģʽ
		EA = IE_EA; 			//�ָ�֮ǰ�����ж�״̬
		while(1)
		{
			
		}
#elif (POWER_SAVING_MODE == LOW_SPEED_MODE)
		I2CCON = 0; 					//�ر�I2C����ΪI2C�ϵ�Ĭ���Ǵ򿪵ģ�����رգ�������ʱ���޷��ر�
		CKCON = 0;						//�ر�����ʱ��
		MECON |= BIT6;				//����FLASH�������˯��ģʽ
		PWCON = (PWCON&0xF0) | 0x04;		//�����ڲ�LDOΪ�͹���ģʽ 
		
		Sys_Clk_Set_XOSCL();	//�л���ʱ��Ϊ����ʱ�ӣ�ע�⣺����ʱ�Ӵӵ���ʱ���л��ظ���ʱ��ʱ��������ڲ�LDO���ûظ߹���ģʽ
	//	Sys_Clk_Set_IRCL();
		while(1)
		{
			
		}
#endif
	
		
	
	}

	


/**************************************************************
**�������ƣ�EnterStopMode
**��������������͹���ǰ�Ĺر�����
**��ڲ�������
**���ڲ�������
***************************************************************/
void EnterStopMode(void)
{
	SW_NB = 0;

	TR2 = 0;	//�ض�ʱ��2
	ES0 = 0;	//�ش���0
	ES1 = 0;	//�ش���1
	
	ET0=0;		//�ض�ʱ��0
	TR0=0;
	ET1=0;      //�ض�ʱ��1
	TR1=0;

	PWMEN=0;
	
	INT0Init(1);  //���ⲿ�ж�
	
	Sys_Clk_Set_IRCL();
	PLLCON =0;	
	WDFLG = 0xA5;//ι��
	PowerSaving_MainLoop();

}

void main(void)
{

	Sys_Clk_Set_PLL(PLL_Multiple);	//ʱ�ӳ�ʼ��
	
	Timer1Init();					//timer1��ʼ��
	Uart0_Initial(UART0_BAUTRATE);          //����0��ʼ��
	Uart1_Initial(UART1_BAUTRATE);			//����1��ʼ��	
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

