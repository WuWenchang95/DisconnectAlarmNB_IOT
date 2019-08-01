#ifndef NB_IOT_DEF
#define NB_IOT_DEF

#include "../../includes/system.h"

#define NB_RESET_IO P13F   
#define NB_RESET P13   
#define SW_NB_IO P15F   
#define SW_NB P15   


typedef enum
{
	SUCCESS_REC= 0,
	ERR_REC, 
	TIME_OUT,
	NO_REC,
}reATStatus;

typedef enum
{
	NB_DEV_RESET = 0,
	NB_AT,			
	NB_AT_CMEE_1,   //AT+CMEE=1  //������ʾ
	NB_AT_NBAND,   //����bands
	NB_AT_CIMI,    //��ȡSIM����
	NB_AT_CGSN_1,  //��ȡIMEI��
	NB_AT_SN,      //��ȡSN��
	NB_AT_CFUN_1,  //����ģ���rf����
	NB_AT_CSQ,     //��ѯ��ǰ���ź�ǿ��
	NB_AT_CGDCONT,  //ָ��APN
	NB_AT_CGATT_1,  //����ģ����������
	NB_AT_CGATT,    //��ѯģ������ +CGATT:1
	NB_AT_CGPADDR,  //��ѯ�ն�ģ���IP��ַ
	NB_AT_NSOCR_STREAM,//����socket
	NB_AT_NSOCO,		//����TCPģʽ
	NB_AT_NSOSD,    //����ָ��
	NB_NOR_COMM,
	
}nb_step;

typedef enum
{
//	NONO,
	DISCONNECT_WIRE,
	LOW_BATTERY,
}alarm_tpye;

typedef struct
{
 	alarm_tpye alarmTpye;  //��������
	uint8_t   isPublish;
	uint8_t   msCnt;	
}sPublishSt;


typedef struct
{
	nb_step step;
	uint8_t connectFg;
	uint8_t cmdTxStep;  //ָ��͵Ĳ���
	uint8_t cmdErrCnt;  //ָ��������
	uint16_t NBTimerClk; //ʱ�Ӽ���
	uint8_t eSIM[16];   //SIM����
	uint8_t iMEI[16];   //IMEI����
	uint8_t sN[16];		//SN��
 	uint8_t protId;     //socket�˿ں�
 	
}sNbiotDev;




void NB73Init(void);
void NBTickClk(void);
void ResetNBTickClk(void);
void NB73Tsak(void);
void SetSendAlarmDataToCloud(alarm_tpye alarm_type);




#endif

