#ifndef _WIRE_ALARM_C_
#define _WIRE_ALARM_C_

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
#include "../../Library/includes/uart.h"
#include "../../Library/includes/delay.h"
#include "../../Library/includes/exint.h"
#include "../../Library/includes/rtc.h"

#include <intrins.h>
#include "main.h"
//#include "../drive/Include/key.h"
//#include "../drive/Include/buzzer.h"
#include "../Wirealarm/Wirealarm.h"
#include "../../Library/includes/pwm.h"
#include "../NB_iot/Nb_iot.h"


sAlarmDri tAlarmDri = {0,0};


/**************************************************************
**�������ƣ�AlarmGPIOInit
**����������������,����� IO��ʼ��   
**��ڲ�������
**���ڲ�������
***************************************************************/
void AlarmGPIOInit(void)
{
	GPIO_Init(WIRE_IO,INPUT|PU_EN);
//	GPIO_Init(ALARM_IO,OUTPUT|PU_EN);
//	ALARM_OUT =0;
}

/**************************************************************
**�������ƣ�AlarmWireCleck
**��������������߼�Ⲣ����  
**��ڲ�������
**���ڲ�������
***************************************************************/
void AlarmWireCleck(void)
{
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

		//ALARM_OFF;

		tAlarmDri.tcnt ++;
		#if 1
		if(tAlarmDri.tcnt%10 == 0)
		{
			if(tAlarmDri.forwardFg)
			{
				tAlarmDri.frp += 10;
				if(tAlarmDri.frp >= 15000)
				{
					tAlarmDri.forwardFg = 0;
					tAlarmDri.tcnt = 0;
				}
			}
			else
			{
				tAlarmDri.frp -= 10;
				if(tAlarmDri.frp <= 5000)
				{
					tAlarmDri.forwardFg = 1;
					tAlarmDri.tcnt = 0;
				}
			}

			PWM_CfgDivDuty(PWM_CH2,FOSC/tAlarmDri.frp,FOSC/(tAlarmDri.frp*2-3500));
			PWM_Update(1<<PWM_CH2);
			WaitPWMUpdateComplete();

		}

		#else
		
		if(tAlarmDri.tcnt == 1000) 
		{
			PWM_CfgDivDuty(PWM_CH2,FOSC/5000,FOSC/8000);
			PWM_Update(1<<PWM_CH2);
			WaitPWMUpdateComplete();
		}
		else if(tAlarmDri.tcnt == 2000)
		{
			tAlarmDri.tcnt = 0;
			PWM_CfgDivDuty(PWM_CH2,1084,542);
			PWM_Update(1<<PWM_CH2);
			WaitPWMUpdateComplete();
		}
		#endif
		
		//ALARM_OUT = 1;   //�򿪱�����
		ALARM_ON;	
	}
	else
	{
		//ALARM_OUT = 0;   //�رձ�����
		ALARM_OFF;		
	}

}
#if 0

/**************************************************************
**�������ƣ�INT0Init
**�����������ⲿ0�г�ʼ��   
**��ڲ�����isStart      �Ƿ����ж�          1����   0�ر�
**���ڲ�������
***************************************************************/
void INT0Init(uint8_t     isStart)	//����
{

	if(isStart)
	{
		P17F = INPUT;   //����P36Ϊ��������
		IT0CON = 15; //P36�жϱ��Ϊ31
		EX0 =1; 	//INT0�ж�ʹ��
		IE0 =1; 	//�ⲿ�ж�ʹ��
		//IT0 = 1;		 //����Ϊ�Ͻ����ж�
	}
	else
	{
		EX0 =0; 	//INT0�ж�ʧ��
		IE0 =0; 	//�ⲿ�ж�ʧ��
	}

	IT0 =1;
	PX0 =1;	

}



/**************************************************************
**�������ƣ�INT0Zd
**�����������ⲿ0�ж�  
**��ڲ�������
**���ڲ�������
***************************************************************/
void INT0Zd(void) interrupt 0
{
	WDFLG = 0xA5;//ˢ�¿��Ź�
	
	EX0 =0;     //INT0�ж�ʧ��
	IE0 =0;     //�ⲿ�ж�ʧ��	
	
	RTM_Con(RTM_Off);	
	INT_Flag = 1;
#ifdef PRINT_EN
	uart_printf("INT_Flag\r\n");
#endif		
#if 0

	if(WIRE_IN == 1)
	{
		Delay_ms(80);
		if(WIRE_IN == 1)
		{
			//ALARM_OUT = 1;   //�򿪱�����
			ALARM_ON;
			IT0 = 0;		//����Ϊ�½����ж�
			SetSendAlarmDataToCloud(DISCONNECT_WIRE);			//NB���ƶ˱���
			g_alarmfg = 0;				
		}
		
	}

	else 
	{
		Delay_ms(80);
		if(WIRE_IN == 0)
		{
			//ALARM_OUT = 0;   //�رձ�����
			ALARM_OFF;
			IT0 = 1;		 //����Ϊ�������ж�
			

		}
	}
#endif

}	
#endif
#if 0
/**************************************************************
**�������ƣ�EX_INT2_Init
**�����������ⲿ2�г�ʼ��   
**��ڲ�����
**���ڲ�������
***************************************************************/
void EX_INT2_Init(void)
{
	GPIO_Init(P17F,INPUT);
	INDEX = 0;                     //����EPCON�����ţ�0~7 ��Ӧ�ⲿ�ж� 2~9��
	EPCON = (0<<7) | P17_INDEX;    //����Ϊ�����ش����������ж�����������ţ� �������Ϊ�½����жϣ�EPCON = (1<<7) | 16;
	INT2EN = 1;  
	EPIE |= 0x01; 
}

/**************************************************************
**�������ƣ�INT2_ISR
**�����������ⲿ2�ж�  
**��ڲ�������
**���ڲ�������
***************************************************************/
void INT2_ISR (void) interrupt 7
{
	if(EPIF & 0x01) 
	{
		EPIF = 0x01; 
		RTM_Con(RTM_Off);

	}
}
#endif

/**************************************************************
**�������ƣ�EX_INT2_Init
**�����������ⲿ2�г�ʼ��   
**��ڲ�����
**���ڲ�������
***************************************************************/
void EX_INT1_Init(void)
{
	GPIO_Init(P17F,INPUT);; //���� P20 Ϊ��������
	IT1CON = P17_INDEX; //ѡ�� P20 ��Ϊ�ж� 1 ���ţ� 16 Ϊ P20 ��Ӧ���������
	EX1 = 1; //INT1 �ж�ʹ��
	IE1 = 1; //�ⲿ�ж� 1 ʹ��
	IT1 = 0; //����Ϊ�Ͻ����ж�
}

/**************************************************************
**�������ƣ�INT1_ISR
**�����������ⲿ2�ж�  
**��ڲ�������
**���ڲ�������
***************************************************************/
void INT1_ISR (void) interrupt 2
{
	//�ⲿ�ж� 1 �жϷ������
	WDFLG = 0xA5;//ˢ�¿��Ź�
	EX1 =0;     //INT1�ж�ʧ��
	IE1 =0;    //�ⲿ�ж�ʧ��
	RTM_Con(RTM_Off);
		
}


/**************************************************************
**�������ƣ�BuzzerInitSet
**������������Դ��������ʼ����pwm���� 
**��ڲ�������
**���ڲ�������
***************************************************************/
void BuzzerInitSet(void)
{
	P35F = P35_PWM2_SETTING;
	PWM_init(PWM_CH2,Center_Align,CKS_IH,Toggle_Off,0,Complementary_Off);

	PWM_CfgDivDuty(PWM_CH2,1084,542);
	PWM_Update(1<<PWM_CH2);
	WaitPWMUpdateComplete();
	//ALARM_ON;
}	


#endif

