#ifndef VOLTAGE_ADC_H
#define VOLTAGE_ADC_H

#include "../../includes/system.h"

#define VOLTAGE_LOW    6000

typedef struct 
{
	uint16_t	 voltageValue;
	uint16_t	 voltageADCValue;
	uint16_t 	 voltageADCTempData;	
	uint8_t 	 voltageADCCnt;		
	//uint8_t 	 timeTemp;	
    uint16_t     tCnt;						//消抖计数
    uint8_t      lowFlag;					//低电压标记
	uint8_t      rtmWakeAdcFg;				//rtc唤醒检测ADC
	uint8_t 	 publishFg;					//发布标记
	uint8_t 	 cleckCnt;					//低电压多次计数
}sVoltageAdc;


extern sVoltageAdc tVoltageAdc;

void VoltageAdcInit(void);
void StartVoltageAdc(void);
void VoltageADCCleck(void);

uint16_t GetvoltageValue(void);
void VoltageCleck(void);

#endif

