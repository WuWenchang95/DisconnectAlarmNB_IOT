#ifndef WIRE_ALARM_H
#define WIRE_ALARM_H

#include "../../includes/system.h"

#define ALARM_IO		P16F
#define ALARM_OUT 		P16
#define WIRE_IO		    P17F
#define WIRE_IN     	P17

#define  ALARM_ON   PWMEN |= (1<<PWM_CH2)
#define  ALARM_OFF   PWMEN &= ~(1<<PWM_CH2)

typedef struct
{
	uint8_t alarmFlag;   //报警标记
	uint8_t cnt;		 //消抖计数
	uint8_t forwardFg;   //正向
	uint16_t tcnt;		 //pwm计数	
	uint16_t frp;		 //频率	
	
}sAlarmDri;

extern sAlarmDri tAlarmDri;

void AlarmGPIOInit(void);
void AlarmWireCleck(void);

void INT0Init(uint8_t     isStart);
void BuzzerInitSet(void);
void EX_INT2_Init(void);
void EX_INT1_Init(void);

#endif
