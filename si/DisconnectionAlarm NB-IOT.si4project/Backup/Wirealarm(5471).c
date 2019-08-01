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
#include <intrins.h>
#include "main.h"
#include "../drive/Include/key.h"
#include "../drive/Include/buzzer.h"
#include "../Wirealarm/Wirealarm.h"
#include "../../Library/includes/pwm.h"

uint8_t g_alarmfg = 0;

/**************************************************************
**�������ƣ�AlarmGPIOInit
**����������������,����� IO��ʼ��   
**��ڲ�������
**���ڲ�������
***************************************************************/
void AlarmGPIOInit(void)
{
	GPIO_Init(WIRE_IO,INPUT);
	GPIO_Init(ALARM_IO,OUTPUT|PU_EN);
	ALARM_OUT =0;
}


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
		P17F = 1;   //����P36Ϊ��������
		IT0CON = 15; //P36�жϱ��Ϊ31
		EX0 =1; 	//INT0�ж�ʹ��
		IE0 =1; 	//�ⲿ�ж�ʹ��
		IT0 = 1;		 //����Ϊ�½����ж�
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
**�����������ⲿ0�ж�  ת�ӵ��ⲿ�жϼ��� 
**��ڲ�������
**���ڲ�������
***************************************************************/
void INT0Zd(void) interrupt 0
{
	WDFLG = 0xA5;//ˢ�¿��Ź�
	
	//
	if(WIRE_IN == 1)
	{
		Delay_ms(80);
		if(WIRE_IN == 1)
		{
			//ALARM_OUT = 1;   //�򿪱�����
			ALARM_ON;
			IT0 = 0;		//����Ϊ�½����ж�
							//NB���ƶ˱���
			g_alarmfg = 1;				
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

}	

void BuzzerInitSet(void)
{
	P35F = P35_PWM2_SETTING;
	PWM_init(PWM_CH2,Center_Align,CKS_IH,Toggle_Off,0,Complementary_Off);

	PWM_CfgDivDuty(PWM_CH2,921,460);
	PWM_Update(1<<PWM_CH2);
	WaitPWMUpdateComplete();
	ALARM_ON;
}	


#endif

