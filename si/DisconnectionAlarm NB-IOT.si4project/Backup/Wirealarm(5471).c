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
**函数名称：AlarmGPIOInit
**函数描述：警报器,检测线 IO初始化   
**入口参数：无
**出口参数：无
***************************************************************/
void AlarmGPIOInit(void)
{
	GPIO_Init(WIRE_IO,INPUT);
	GPIO_Init(ALARM_IO,OUTPUT|PU_EN);
	ALARM_OUT =0;
}


/**************************************************************
**函数名称：INT0Init
**函数描述：外部0中初始化   
**入口参数：isStart      是否开启中断          1开启   0关闭
**出口参数：无
***************************************************************/
void INT0Init(uint8_t     isStart)	//开机
{

	if(isStart)
	{
		P17F = 1;   //设置P36为输入引脚
		IT0CON = 15; //P36中断编号为31
		EX0 =1; 	//INT0中断使能
		IE0 =1; 	//外部中断使能
		IT0 = 1;		 //设置为下降沿中断
	}
	else
	{
		EX0 =0; 	//INT0中断失能
		IE0 =0; 	//外部中断失能
	}

	IT0 =1;
	PX0 =1;	

}

/**************************************************************
**函数名称：INT0Zd
**函数描述：外部0中断  转子的外部中断计数 
**入口参数：无
**出口参数：无
***************************************************************/
void INT0Zd(void) interrupt 0
{
	WDFLG = 0xA5;//刷新看门狗
	
	//
	if(WIRE_IN == 1)
	{
		Delay_ms(80);
		if(WIRE_IN == 1)
		{
			//ALARM_OUT = 1;   //打开报警器
			ALARM_ON;
			IT0 = 0;		//设置为下降沿中断
							//NB向云端报警
			g_alarmfg = 1;				
		}
		
	}

	else 
	{
		Delay_ms(80);
		if(WIRE_IN == 0)
		{
			//ALARM_OUT = 0;   //关闭报警器
			ALARM_OFF;
			IT0 = 1;		 //设置为上升沿中断
			

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

