#ifndef NB_IOT_DEF
#define NB_IOT_DEF

#include "../../includes/system.h"

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
	NB_AT_CMEE_1,   //AT+CMEE=1  //错误提示
	NB_AT_NBAND,   //设置bands
	NB_AT_CIMI,    //获取SIM卡号
	NB_AT_CGSN_1,  //获取IMEI号
	NB_AT_SN,      //获取SN号
	NB_AT_CFUN_1,  //启动模块的rf功能
	NB_AT_CSQ,     //查询当前的信号强度
	NB_AT_CGDCONT,  //指定APN
	NB_AT_CGATT_1,  //启动模块联网功能
	NB_AT_CGATT,    //查询模块联网 +CGATT:1
	NB_AT_CGPADDR,  //查询终端模块的IP地址
	NB_AT_NSOCR_STREAM,//建立socket
	NB_AT_NSOCO,		//建立TCP模式
	NB_AT_NSOSD,    //发送指令
	
}nb_step;


typedef struct
{
	nb_step step;
	uint8_t connectFg;
	uint8_t cmdTxStep;  //指令发送的步骤
	uint8_t cmdErrCnt;  //指令错误计数
	uint16_t NBTimerClk; //时钟计数
	uint8_t eSIM[15];   //SIM卡号
	uint8_t iMEI[15];   //IMEI卡号
	uint8_t sN[15];		//SN号
 	uint8_t protId;     //socket端口号
}sNbiotDev;




void NB73Init(void);
void NBTickClk(void);
void ResetNBTickClk(void);
void NB73Tsak(void);




#endif

