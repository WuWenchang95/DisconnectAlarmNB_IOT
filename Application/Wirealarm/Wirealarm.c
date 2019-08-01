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
**函数名称：AlarmGPIOInit
**函数描述：警报器,检测线 IO初始化   
**入口参数：无
**出口参数：无
***************************************************************/
void AlarmGPIOInit(void)
{
	GPIO_Init(WIRE_IO,INPUT|PU_EN);
//	GPIO_Init(ALARM_IO,OUTPUT|PU_EN);
//	ALARM_OUT =0;
}

/**************************************************************
**函数名称：AlarmWireCleck
**函数描述：检测线检测并报警  
**入口参数：无
**出口参数：无
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
		
		//ALARM_OUT = 1;   //打开报警器
		ALARM_ON;	
	}
	else
	{
		//ALARM_OUT = 0;   //关闭报警器
		ALARM_OFF;		
	}

}
#if 0

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
		P17F = INPUT;   //设置P36为输入引脚
		IT0CON = 15; //P36中断编号为31
		EX0 =1; 	//INT0中断使能
		IE0 =1; 	//外部中断使能
		//IT0 = 1;		 //设置为上降沿中断
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
**函数描述：外部0中断  
**入口参数：无
**出口参数：无
***************************************************************/
void INT0Zd(void) interrupt 0
{
	WDFLG = 0xA5;//刷新看门狗
	
	EX0 =0;     //INT0中断失能
	IE0 =0;     //外部中断失能	
	
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
			//ALARM_OUT = 1;   //打开报警器
			ALARM_ON;
			IT0 = 0;		//设置为下降沿中断
			SetSendAlarmDataToCloud(DISCONNECT_WIRE);			//NB向云端报警
			g_alarmfg = 0;				
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
#endif

}	
#endif
#if 0
/**************************************************************
**函数名称：EX_INT2_Init
**函数描述：外部2中初始化   
**入口参数：
**出口参数：无
***************************************************************/
void EX_INT2_Init(void)
{
	GPIO_Init(P17F,INPUT);
	INDEX = 0;                     //设置EPCON索引号，0~7 对应外部中断 2~9。
	EPCON = (0<<7) | P17_INDEX;    //设置为上升沿触发，设置中断引脚索引编号； 如果设置为下降沿中断：EPCON = (1<<7) | 16;
	INT2EN = 1;  
	EPIE |= 0x01; 
}

/**************************************************************
**函数名称：INT2_ISR
**函数描述：外部2中断  
**入口参数：无
**出口参数：无
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
**函数名称：EX_INT2_Init
**函数描述：外部2中初始化   
**入口参数：
**出口参数：无
***************************************************************/
void EX_INT1_Init(void)
{
	GPIO_Init(P17F,INPUT);; //设置 P20 为输入引脚
	IT1CON = P17_INDEX; //选择 P20 作为中断 1 引脚， 16 为 P20 对应的索引编号
	EX1 = 1; //INT1 中断使能
	IE1 = 1; //外部中断 1 使能
	IT1 = 0; //设置为上降沿中断
}

/**************************************************************
**函数名称：INT1_ISR
**函数描述：外部2中断  
**入口参数：无
**出口参数：无
***************************************************************/
void INT1_ISR (void) interrupt 2
{
	//外部中断 1 中断服务程序
	WDFLG = 0xA5;//刷新看门狗
	EX1 =0;     //INT1中断失能
	IE1 =0;    //外部中断失能
	RTM_Con(RTM_Off);
		
}


/**************************************************************
**函数名称：BuzzerInitSet
**函数描述：无源蜂鸣器初始化，pwm配置 
**入口参数：无
**出口参数：无
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

