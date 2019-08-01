#ifndef _VOLTAGE_ADC_C_
#define _VOLTAGE_ADC_C_

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
#include "../../Library/includes/adc.h"
#include "../../Library/includes/uart.h"
#include "../../Library/includes/delay.h"
#include "../drive/Include/voltage_adc.h"
#include "../NB_iot/Nb_iot.h"

#include "main.h"
#include <intrins.h>

#define VOLTAGE_ADC_IO		    P10F
#define VOLTAGE_ADC     		P10

sVoltageAdc tVoltageAdc = {0};


/**************************************************************
**函数名称：VoltageAdcInit
**函数描述：电压检测初始化
**入口参数：
**出口参数：
***************************************************************/
void VoltageAdcInit(void)
{
	GPIO_Init(VOLTAGE_ADC_IO,P10_ADC0_SETTING);	
	ADC_init(ADC_REF_VDD,DISABLE,DISABLE,NO_AMP);	//电压采集
	ADC_ConfigChannel(ADC_CH0);

//	StartVoltageAdc();
}

/**************************************************************
**函数名称：StartVoltageAdc
**函数描述：启动电压检测
**入口参数：
**出口参数：
***************************************************************/
void StartVoltageAdc(void)
{
	ADC_StartConversion();
}

/**************************************************************
**函数名称：VoltageADCCleck
**函数描述：取电压ADC值
**入口参数：
**出口参数：
***************************************************************/
void VoltageADCCleck(void)
{
	unsigned int AD_Value;
	
	ADC_init(ADC_REF_VDD,DISABLE,DISABLE,NO_AMP);	//电压采集
	ADC_ConfigChannel(ADC_CH0);	
	ADC_StartConversion();
	AD_Value = GetAdcValue();

	tVoltageAdc.voltageADCTempData += AD_Value;
	tVoltageAdc.voltageADCCnt++;
	if(tVoltageAdc.voltageADCCnt >= 16)
	{
		float tVoltValue;
		
		tVoltageAdc.voltageADCCnt = 0;
		tVoltageAdc.voltageADCValue = (tVoltageAdc.voltageADCTempData>>4);	
		tVoltageAdc.voltageADCTempData = 0;

		tVoltValue = (float)(((float)((float)tVoltageAdc.voltageADCValue/4096))*3600)*3;
		tVoltageAdc.voltageValue = (uint16_t)tVoltValue;					
	}	

}

/**************************************************************
**函数名称：ADC_ISR
**函数描述：ADC检测中断
**入口参数：
**出口参数：
***************************************************************/
void ADC_ISR (void) interrupt 7
{
	unsigned int AD_Value;
	if(ADCON & ADIF)
	{
		ADCON |= ADIF; //清除 ADC 中断标志
		AD_Value = ADCDH*256 + ADCDL; //读取 ADC 值
		AD_Value >>= 4;

		tVoltageAdc.voltageADCTempData += AD_Value;
		tVoltageAdc.voltageADCCnt++;
		if(tVoltageAdc.voltageADCCnt >= 16)
		{
			float tVoltValue;
			
			tVoltageAdc.voltageADCCnt = 0;
			tVoltageAdc.voltageADCValue = (tVoltageAdc.voltageADCTempData>>4);	
			tVoltageAdc.voltageADCTempData = 0;

			tVoltValue = (float)(((float)((float)tVoltageAdc.voltageADCValue/4096))*3600)*3;
			tVoltageAdc.voltageValue = (uint16_t)tVoltValue;					
		}
		
		StartVoltageAdc();
	}
	
}



/**************************************************************
**函数名称：IsLowVoltage
**函数描述：电压是否过低
**入口参数：
**出口参数：
***************************************************************/
uint8_t IsLowVoltage(void)
{
	if(tVoltageAdc.voltageValue < VOLTAGE_LOW)
		return 1;
	else
		return 0;	
}

/**************************************************************
**函数名称：VoltageCleck
**函数描述：电压检测
**入口参数：
**出口参数：
***************************************************************/
void VoltageCleck(void)
{
	
	if(!tVoltageAdc.rtmWakeAdcFg)
	{
		return;
	}	
	
    if(IsLowVoltage())
    {
        if(tVoltageAdc.tCnt < 80)
        {
            tVoltageAdc.tCnt++;
            if(tVoltageAdc.tCnt >= 79)
            {
                if(tVoltageAdc.lowFlag == 0)
                {
                    tVoltageAdc.lowFlag = 1;
					if(!tVoltageAdc.publishFg)
					{
						tVoltageAdc.publishFg = 1;
						SetSendAlarmDataToCloud(LOW_BATTERY);
					}
                    	
                }
            }
        }
    }
    else
    {
        if(tVoltageAdc.tCnt > 0)
        {
            tVoltageAdc.tCnt--;
            if(tVoltageAdc.tCnt == 0)
            {
                if(tVoltageAdc.lowFlag == 1)
                {
                    tVoltageAdc.lowFlag = 0;
                }
            }
        }       
    }

}
#endif

