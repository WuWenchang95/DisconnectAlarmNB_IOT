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
    uint16_t     tCnt;						//��������
    uint8_t      lowFlag;					//�͵�ѹ���
	uint8_t      rtmWakeAdcFg;				//rtc���Ѽ��ADC
	uint8_t 	 publishFg;					//�������
	uint8_t 	 cleckCnt;					//�͵�ѹ��μ���
}sVoltageAdc;


extern sVoltageAdc tVoltageAdc;

void VoltageAdcInit(void);
void StartVoltageAdc(void);
void VoltageADCCleck(void);

uint16_t GetvoltageValue(void);
void VoltageCleck(void);

#endif

