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
**�������ƣ�GetProClockTick
**����������ȡ���̼�ʱTime_ms
**��ڲ�������
**���ڲ�������
***************************************************************/
uint16_t GetProClockTick(void)
{
	return Time_ms;
}

/**************************************************************
**�������ƣ�WakeUp
**�������������ѵ�Ƭ����ʼ������
**��ڲ�������
**���ڲ�������
***************************************************************/
void WakeUp(void)
{

	Sys_Clk_Set_PLL(PLL_Multiple);
	
	RTM_Con(RTM_Off);
	Timer1Init();							//timer1��ʼ��
//	NB73Init();
	
//	INT2EN =0;      //INT2�ж�ʧ��
//	EPIE &=(~0X01);    //�ⲿ2�ж�ʧ��
	EX1 =0;     //INT1�ж�ʧ��
	IE1 =0;    //�ⲿ�ж�ʧ��

	AlarmGPIOInit();
#ifdef PRINT_EN 	
	Uart0_Initial(UART0_BAUTRATE);          //����0��ʼ��
#endif	
	Uart1_Initial(UART1_BAUTRATE);			//����1��ʼ��	
	BuzzerInitSet();
	
	if(tVoltageAdc.rtmWakeAdcFg)
	{
        VoltageAdcInit();
	}
		
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
	bit IE_EA;
	I2CCON = 0;						//�ر�I2C����ΪI2C�ϵ�Ĭ���Ǵ򿪵ģ�����رգ�������ʱ���޷��ر�
	CKCON = 0;						//�ر�����ʱ��
	MECON |= BIT6;				//����FLASH�������˯��ģʽ
	PWCON = (PWCON&0xF0) | 0x04;		//�����ڲ�LDOΪ�͹���ģʽ
	
	while(!CheckStopCondition());
	IE_EA = EA;				//���浱ǰ���жϿ���״̬
	EA = 0;					//�����ж�
	StopEnter();	//����STOPģʽ
	
	PWCON = (PWCON&0xF0) | 0x0B;	//�˳�STOPģʽ���������LDOΪ�߹���ģʽ
	EA = IE_EA;				//�ָ�֮ǰ�����ж�״̬

}

	


/**************************************************************
**�������ƣ�EnterStopMode
**��������������͹���ǰ�Ĺر�����
**��ڲ�������
**���ڲ�������
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
	EX_INT1_Init(); //���ⲿ�ж�

	TR2 = 0;	//�ض�ʱ��2
	ES0 = 0;	//�ش���0
	ES1 = 0;	//�ش���1
	
//	ET0=0;		//�ض�ʱ��0
//	TR0=0;

	ET1=0;		//�ض�ʱ��1
	TR1=0;
	
	INT2EN =0;      //INT2�ж�ʧ��
	EPIE &=(~0X01);    //�ⲿ2�ж�ʧ��
	
	ADCON |= ADIF;
	ADCON = 0;
//	ADCFGL = 0x10;
//	ADCFGH = 0x23;
	
	PWMEN=0;
			
	Sys_Clk_Set_IRCL();
	PLLCON =0;	
	//WDFLG = 0xA5;//ι��
	
	PowerSaving_MainLoop();

}


void main(void)
{

	Sys_Clk_Set_PLL(PLL_Multiple);			//ʱ�ӳ�ʼ��
	
#ifdef PRINT_EN 	
	Uart0_Initial(UART0_BAUTRATE);          //����0��ʼ��
#endif	

#if 1	
	Timer1Init();							//timer1��ʼ��
	Uart1_Initial(UART1_BAUTRATE);			//����1��ʼ��	
	NB73Init();								//NBģ��io�ĳ�ʼ��
	AlarmGPIOInit();						//����� IO��ʼ��   
	
	GPIO_Init(P32F,P32_XOSCL_IN_SETTING);	//rtc ��ʼ��	
	GPIO_Init(P33F,P33_XOSCL_OUT_SETTING);			
	RTC_init(RTCE_On,MSE_Off,HSE_Off);
	RTM_init(0,10,40);
	RTM_Con(RTM_Off);
	
	BuzzerInitSet();						//��������ʼ��		
	
	VoltageAdcInit();						//ADC��ʼ��
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

