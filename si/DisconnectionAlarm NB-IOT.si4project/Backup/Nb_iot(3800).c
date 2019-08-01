#ifndef _NB_IOT_C_
#define _NB_IOT_C_

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
#include "../NB_iot/Nb_iot.h"

#include <string.h>
#include <stdlib.h>
#include <stdio.h>


#define NB_RESET_IO P13F   
#define NB_RESET P13   
#define SW_NB_IO P15F   
#define SW_NB P15   


sNbiotDev NbiotDev = {NB_DEV_RESET,0,0,0,0,{0},{0},{0},0};


/**************************************************************
**函数名称：NB73Init
**函数描述：NB模块io的初始化
**入口参数：
**出口参数：
***************************************************************/
void NB73Init(void)
{
	GPIO_Init(NB_RESET_IO,OUTPUT);
	GPIO_Init(SW_NB_IO,OUTPUT);
	SW_NB = 1;
}


/**************************************************************
**函数名称：AsciiToHex
**函数描述：Ascii字符串转HEX字符串
**入口参数：pnum：源数据
			size：大小
			des_str转换后的数据
**出口参数：
***************************************************************/
void AsciiToHex(uint8_t *sou_str,uint8_t size,uint8_t *des_str)	
{
	/*索引表*/
	const char index[]="0123456789ABCDEF";
	uint8_t i;
	for(i=0;i<size;i++)
	{
		des_str[2*i]   = index[(sou_str[i] >>4)];
		des_str[2*i+1] = index[(sou_str[i] &0x0f)];
	}
}


/**************************************************************
**函数名称：SendDataToNB
**函数描述：向NB发送指令
**入口参数：Sendstr：要发送的字符串
**出口参数：
***************************************************************/
void SendDataToNB(uint8_t *Sendstr)
{
	while(*Sendstr)
	{	
 		Uart1_PutChar(*Sendstr++);
	}

}

/**************************************************************
**函数名称：NBTickClk
**函数描述：定时器1中断中执行、1ms中断 刷新NBiot工程计时
**入口参数：无
**出口参数：无
***************************************************************/
void NBTickClk(void)
{
	NbiotDev.NBTimerClk++;
}
/**************************************************************
**函数名称：ResetNBTickClk
**函数描述：nb 时钟清零
**入口参数：无
**出口参数：无
***************************************************************/
void ResetNBTickClk(void)
{
	NbiotDev.NBTimerClk = 0;
}

/**************************************************************
**函数名称：NB_ATRecPro
**函数描述：处理 NB AT指令回复的结果
**入口参数：Recstr：要接受的字符串
**出口参数：reATStatus
***************************************************************/
reATStatus NB_ATRecPro(uint8_t *Recstr)
{
	reATStatus rATStatus = NO_REC;
		
	if(!uart1_rx_finish_flag)
		return  rATStatus;


	if(strstr((const char*)uart1_rx_buf,Recstr) != NULL)
	{
		rATStatus = SUCCESS_REC;
	}
	else
	{
		rATStatus = ERR_REC;
	}
#ifdef PRINT_EN 
	//uart_printf("%s\r\n",uart1_rx_buf);
#endif	
	ResetUART1Rec();
	
	return rATStatus;
}



#define NB_AT_DELAY    50
/**************************************************************
**函数名称：NB73Tsak
**函数描述：NB任务的处理
**入口参数：
**出口参数：
***************************************************************/

void NB73Tsak(void)
{
	reATStatus rATStatus = ERR_REC;
	static uint8_t sunn;
	static uint8_t sunnt;
			
#if 1
	switch(NbiotDev.step)
	{
		case NB_DEV_RESET:
			NB_RESET = 1;
			Delay_ms(5);
			NB_RESET = 0;
#ifdef PRINT_EN 
			uart_printf("NB_RESET1\r\n");
#endif

			//ResetNBTickClk();
			Time_ms = 0;
			while(1)
			{		
				rATStatus = NB_ATRecPro("OK");
				if(rATStatus == SUCCESS_REC)
				{
					NbiotDev.step = NB_AT;
					NbiotDev.cmdTxStep = 0;
#ifdef PRINT_EN 
					uart_printf("NB_OK\r\n");
#endif					
				}
			//	else if(NbiotDev.NBTimerClk	> 5000)
				else if(Time_ms	> 10000)
				{
				
#ifdef PRINT_EN 
					uart_printf("break\r\n");
#endif					
					break;
					
				}				
			}	
#ifdef PRINT_EN 
					uart_printf("NB_DEV_RESET\r\n");
#endif				
			break;
		case NB_AT:
			{
				if(NbiotDev.cmdTxStep == 0)
				{
					SendDataToNB("AT\r\n\0");
					NbiotDev.cmdTxStep = 1;
					ResetNBTickClk();
#ifdef PRINT_EN 
					uart_printf("NB_AT\r\n");
#endif					
				}
				else if(NbiotDev.cmdTxStep == 1)
				{
					rATStatus = NB_ATRecPro("OK");
					if(rATStatus == SUCCESS_REC)
					{
						//NbiotDev.step = NB_AT_CIMI;
						NbiotDev.step = NB_AT_CMEE_1;
						NbiotDev.cmdTxStep = 0;
						NbiotDev.cmdErrCnt = 0;
#ifdef PRINT_EN 
					uart_printf("OK\r\n");
#endif						
					Delay_ms(NB_AT_DELAY);

					}
					else
					{
						if(NbiotDev.NBTimerClk	> 1000)
						{
							NbiotDev.cmdErrCnt++;
							NbiotDev.cmdTxStep = 0;
							if(NbiotDev.cmdErrCnt > 3)
							{
								NbiotDev.step = NB_DEV_RESET;
							}
							break;
						}
					}				
				}
			}
			
			break;
		case NB_AT_CMEE_1:
			{
				if(NbiotDev.cmdTxStep == 0)
				{
					SendDataToNB("AT+CMEE=1\r\n\0");
					NbiotDev.cmdTxStep = 1;
					ResetNBTickClk();
#ifdef PRINT_EN 
					uart_printf("AT+CMEE=1\r\n");
#endif					
				}
				else if(NbiotDev.cmdTxStep == 1)
				{
					rATStatus = NB_ATRecPro("OK");
					if(rATStatus == SUCCESS_REC)
					{
						//NbiotDev.step = NB_AT_CIMI;
						NbiotDev.step = NB_AT_CIMI;
						NbiotDev.cmdTxStep = 0;
						NbiotDev.cmdErrCnt = 0;
#ifdef PRINT_EN 
					uart_printf("OK\r\n");
#endif						
					Delay_ms(NB_AT_DELAY);

					}
					else
					{
						if(NbiotDev.NBTimerClk	> 1000)
						{
							NbiotDev.cmdErrCnt++;
							NbiotDev.cmdTxStep = 0;
							if(NbiotDev.cmdErrCnt > 3)
							{
								NbiotDev.step = NB_DEV_RESET;
							}
							break;
						}
					}				
				}
			}	
			break;

		case NB_AT_NBAND:
			
			break;			
		case NB_AT_CIMI:     //获取SIM卡号
			{
				if(NbiotDev.cmdTxStep == 0)
				{
					SendDataToNB("AT+CIMI\r\n\0");   
					NbiotDev.cmdTxStep = 1;
					ResetNBTickClk();
#ifdef PRINT_EN 
					uart_printf("AT+CIMI\r\n");
#endif						
				}
				else if(NbiotDev.cmdTxStep == 1)
				{
					if(uart1_rx_finish_flag)
					{
						uint8_t i;

						for(i = 0; i < 20; i++)
						{
							if((uart1_rx_buf[i] >= '0') && (uart1_rx_buf[i] <= '9'))
							{
								NbiotDev.eSIM[i] = uart1_rx_buf[i];
							}
							else
							{
								break;
							}
						}
						
						if(i == 15)
						{
							
							rATStatus = SUCCESS_REC;
						}
						
						ResetUART1Rec();
						
					}
					//rATStatus = NB_ATRecPro("OK");
					if(rATStatus == SUCCESS_REC)
					{
						NbiotDev.cmdTxStep = 2;

					}
					else
					{
						if(NbiotDev.NBTimerClk	> 1000)
						{
							NbiotDev.cmdErrCnt++;
							NbiotDev.cmdTxStep = 0;
							if(NbiotDev.cmdErrCnt > 3)
							{
								NbiotDev.step = NB_DEV_RESET;
							}
							break;
						}
					}				
				}
				else if(NbiotDev.cmdTxStep == 2)
				{
					rATStatus = NB_ATRecPro("OK");
					if(rATStatus == SUCCESS_REC)
					{
						NbiotDev.step = NB_AT_CGSN_1;
						NbiotDev.cmdTxStep = 0;
						NbiotDev.cmdErrCnt = 0;
#ifdef PRINT_EN 
						uart_printf("eSIM:%15s\r\n",NbiotDev.eSIM);
#endif							
						Delay_ms(NB_AT_DELAY);

					}
					else
					{
						if(NbiotDev.NBTimerClk	> 1000)
						{
							NbiotDev.cmdTxStep = 0;
							NbiotDev.cmdErrCnt = 0;
							NbiotDev.step = NB_AT_CGSN_1;
							
							break;
						}
					}									
				}
			}
			break;	
		case NB_AT_CGSN_1:    //获取IMEI号
			{  
				if(NbiotDev.cmdTxStep == 0)
				{
					SendDataToNB("AT+CGSN=1\r\n\0");   
					NbiotDev.cmdTxStep = 1;
					ResetNBTickClk();
#ifdef PRINT_EN 
					uart_printf("AT+CGSN=1\r\n");
#endif					
				}
				else if(NbiotDev.cmdTxStep == 1)
				{
					if(uart1_rx_finish_flag)
					{
						uint8_t i;
						uint8_t *pt;
						
						pt = strstr(uart1_rx_buf,"+CGSN:");
						//if(!pt)   break;

						pt = pt+strlen("+CGSN:");
						
						for(i = 0; i < 20; i++)
						{
							if((pt[i] >= '0') && (pt[i] <= '9'))
							{
								NbiotDev.iMEI[i] = pt[i];
							}
							else
							{
								break;
							}
						}

						if(i == 15)
						{
							
							rATStatus = SUCCESS_REC;
						}
						
						ResetUART1Rec();


					}
					//rATStatus = NB_ATRecPro("OK");
					if(rATStatus == SUCCESS_REC)
					{
						NbiotDev.cmdTxStep = 2;
					}
					else
					{
						if(NbiotDev.NBTimerClk	> 1000)
						{
							NbiotDev.cmdErrCnt++;
							NbiotDev.cmdTxStep = 0;
							if(NbiotDev.cmdErrCnt > 3)
							{
								NbiotDev.step = NB_DEV_RESET;
							}
							break;
						}
					}				
				}
				else if(NbiotDev.cmdTxStep == 2)
				{
					rATStatus = NB_ATRecPro("OK");
					if(rATStatus == SUCCESS_REC)
					{
						NbiotDev.step = NB_AT_SN;
						NbiotDev.cmdTxStep = 0;
						NbiotDev.cmdErrCnt = 0;
#ifdef PRINT_EN 
						uart_printf("iMEI:%15s\r\n",NbiotDev.iMEI);
#endif							
						Delay_ms(NB_AT_DELAY);						
					}
					else
					{
						if(NbiotDev.NBTimerClk	> 1000)
						{
							NbiotDev.cmdTxStep = 0;
							NbiotDev.cmdErrCnt = 0;
							NbiotDev.step = NB_AT_SN;
							break;
						}
					}									
				}
			}			
			break;	
		case NB_AT_SN:
			{  
				if(NbiotDev.cmdTxStep == 0)
				{
					SendDataToNB("AT+SN\r\n\0");   
					NbiotDev.cmdTxStep = 1;
					ResetNBTickClk();
#ifdef PRINT_EN 
					uart_printf("AT+SN\r\n");
#endif						
				}
				else if(NbiotDev.cmdTxStep == 1)
				{
					if(uart1_rx_finish_flag)
					{
						uint8_t i;
						uint8_t *pt;
						
						pt = strstr(uart1_rx_buf,"+SN:SN");
						//if(!pt)   break;

						pt = pt+strlen("+SN:SN");
						
						for(i = 0; i < 20; i++)
						{
							if((pt[i] >= '0') && (pt[i] <= '9'))
							{
								NbiotDev.sN[i] = pt[i];
							}
							else
							{
								break;
							}
						}

						if(i == 15)
						{
							
							rATStatus = SUCCESS_REC;
						}
						
						ResetUART1Rec();

					}
					//rATStatus = NB_ATRecPro("OK");
					if(rATStatus == SUCCESS_REC)
					{
						NbiotDev.cmdTxStep = 2;
					}
					else
					{
						if(NbiotDev.NBTimerClk	> 1000)
						{
							NbiotDev.cmdErrCnt++;
							NbiotDev.cmdTxStep = 0;
							if(NbiotDev.cmdErrCnt > 3)
							{
								NbiotDev.step = NB_DEV_RESET;
							}
							break;
						}
					}				
				}
				else if(NbiotDev.cmdTxStep == 2)
				{
					rATStatus = NB_ATRecPro("OK");
					if(rATStatus == SUCCESS_REC)
					{
						NbiotDev.step = NB_AT_CFUN_1;
						NbiotDev.cmdTxStep = 0;
						NbiotDev.cmdErrCnt = 0;
#ifdef PRINT_EN 
						uart_printf("sN:%15s\r\n",NbiotDev.sN);
#endif							
						Delay_ms(NB_AT_DELAY);						
					}
					else
					{
						if(NbiotDev.NBTimerClk	> 1000)
						{
							NbiotDev.cmdTxStep = 0;
							NbiotDev.cmdErrCnt = 0;
							NbiotDev.step = NB_AT_CFUN_1;
							break;
						}
					}									
				}
			}
			break;

		case NB_AT_CFUN_1:
			{
				if(NbiotDev.cmdTxStep == 0)
				{
					SendDataToNB("AT+CFUN=1\r\n\0");
					NbiotDev.cmdTxStep = 1;
					
					ResetNBTickClk();
#ifdef PRINT_EN 
					uart_printf("AT+CFUN=1\r\n");
#endif						
				}
				else if(NbiotDev.cmdTxStep == 1)
				{
					rATStatus = NB_ATRecPro("OK");
					if(rATStatus == SUCCESS_REC)
					{
						NbiotDev.step = NB_AT_CGATT_1;
						NbiotDev.cmdTxStep = 0;
						NbiotDev.cmdErrCnt = 0;
#ifdef PRINT_EN 
					uart_printf("OK\r\n");
#endif							
					Delay_ms(NB_AT_DELAY);
					}
					else
					{
						if(NbiotDev.NBTimerClk	> 1000)
						{
							NbiotDev.cmdErrCnt++;
							NbiotDev.cmdTxStep = 0;
							if(NbiotDev.cmdErrCnt > 3)
							{
								NbiotDev.step = NB_DEV_RESET;
							}
							break;
						}
					}				
				}
			}
		break;
		
		case NB_AT_CSQ:
			
			break;
		case NB_AT_CGATT_1:
			{
				if(NbiotDev.cmdTxStep == 0)
				{
					SendDataToNB("AT+CGATT=1\r\n\0");
					NbiotDev.cmdTxStep = 1;
					
					ResetNBTickClk();
#ifdef PRINT_EN 
					uart_printf("AT+CGATT=1\r\n");
#endif						
				}
				else if(NbiotDev.cmdTxStep == 1)
				{
					rATStatus = NB_ATRecPro("OK");
					if(rATStatus == SUCCESS_REC)
					{
						NbiotDev.step = NB_AT_CGDCONT;
						NbiotDev.cmdTxStep = 0;
						NbiotDev.cmdErrCnt = 0;
#ifdef PRINT_EN 
						uart_printf("OK\r\n");
#endif		
						Delay_ms(NB_AT_DELAY);
					}
					else
					{
						if(NbiotDev.NBTimerClk	> 1000)
						{
							NbiotDev.cmdErrCnt++;
							NbiotDev.cmdTxStep = 0;
							if(NbiotDev.cmdErrCnt > 3)
							{
								NbiotDev.step = NB_DEV_RESET;
							}
							break;
						}
					}				
				}
			}	
			break;			
		case NB_AT_CGDCONT:
			{
				if(NbiotDev.cmdTxStep == 0)
				{
					SendDataToNB("AT+CGDCONT=1,\"IP\",\"CMCC\"\r\n\0");
					NbiotDev.cmdTxStep = 1;
					
					ResetNBTickClk();
#ifdef PRINT_EN 
					uart_printf("AT+CGDCONT=1,\"IP\",\"CMCC\"\r\n\0");
#endif						
				}
				else if(NbiotDev.cmdTxStep == 1)
				{
					rATStatus = NB_ATRecPro("OK");
					if(rATStatus == SUCCESS_REC)
					{
						NbiotDev.step = NB_AT_NSOCR_STREAM;
						NbiotDev.cmdTxStep = 0;
						NbiotDev.cmdErrCnt = 0;
#ifdef PRINT_EN 
						uart_printf("OK\r\n");
#endif		
						Delay_ms(NB_AT_DELAY);
					}
					else
					{
						if(NbiotDev.NBTimerClk	> 1000)
						{
							NbiotDev.cmdErrCnt++;
							NbiotDev.cmdTxStep = 0;
							if(NbiotDev.cmdErrCnt > 3)
							{
								NbiotDev.step = NB_DEV_RESET;
							}
							break;
						}
					}				
				}
			}
			break;

		case NB_AT_NSOCR_STREAM:
			{  
				if(NbiotDev.cmdTxStep == 0)
				{
					SendDataToNB("AT+NSOCR=STREAM,6,65000,1\r\n\0");   
					NbiotDev.cmdTxStep = 1;
					ResetNBTickClk();
#ifdef PRINT_EN 
					uart_printf("AT+NSOCR=STREAM,6,65000,1\r\n");
#endif					
				}
				else if(NbiotDev.cmdTxStep == 1)
				{
					if(uart1_rx_finish_flag)
					{
						uint8_t i;
												
						for(i = 0; i < 20; i++)
						{
							if((uart1_rx_buf[i] >= '0') && (uart1_rx_buf[i] <= '9'))
							{
								NbiotDev.protId = uart1_rx_buf[i];
							}
							else
							{
								break;
							}
						}

						if(i == 1)
						{
							
							rATStatus = SUCCESS_REC;
						}
						
						ResetUART1Rec();
						
					}
					//rATStatus = NB_ATRecPro("OK");
					if(rATStatus == SUCCESS_REC)
					{
						NbiotDev.cmdTxStep = 2;
					}
					else
					{
						if(NbiotDev.NBTimerClk	> 1000)
						{
							NbiotDev.cmdErrCnt++;
							NbiotDev.cmdTxStep = 0;
							if(NbiotDev.cmdErrCnt > 3)
							{
								NbiotDev.step = NB_DEV_RESET;
							}
							break;
						}
					}				
				}
				else if(NbiotDev.cmdTxStep == 2)
				{
					rATStatus = NB_ATRecPro("OK");
					if(rATStatus == SUCCESS_REC)
					{
						NbiotDev.step = NB_AT_NSOCO;
						NbiotDev.cmdTxStep = 0;
						NbiotDev.cmdErrCnt = 0;
#ifdef PRINT_EN 
					uart_printf("OK\r\n");
#endif							
					Delay_ms(NB_AT_DELAY*2);

					}
					else
					{
						if(NbiotDev.NBTimerClk	> 1000)
						{
							NbiotDev.cmdTxStep = 0;
							NbiotDev.step = NB_AT_NSOCO;
							break;
						}
					}									
				}
			}
			break;
		case NB_AT_NSOCO:
			{
				if(NbiotDev.cmdTxStep == 0)
				{
					//SendDataToNB("AT+NSOCO=1,47.107.236.178,8000\r\n\0");
					SendDataToNB("AT+NSOCO=1,112.74.59.250,10002\r\n\0");
					NbiotDev.cmdTxStep = 1;
					
					ResetNBTickClk();
#ifdef PRINT_EN
					//uart_printf("AT+NSOCO=1,47.107.236.178,8000\r\n");
					uart_printf("AT+NSOCO=1,112.74.59.250,10002\r\n");

#endif
				}
				else if(NbiotDev.cmdTxStep == 1)
				{
					rATStatus = NB_ATRecPro("OK");
					if(rATStatus == SUCCESS_REC)
					{
						NbiotDev.step = NB_AT_NSOSD;
						NbiotDev.cmdTxStep = 0;
						NbiotDev.cmdErrCnt = 0;	
#ifdef PRINT_EN 
					uart_printf("OK\r\n");
#endif							
					Delay_ms(NB_AT_DELAY);

					}
					else
					{
						if(NbiotDev.NBTimerClk	> 1000)
						{
							NbiotDev.cmdErrCnt++;
							NbiotDev.cmdTxStep = 0;
							if(NbiotDev.cmdErrCnt > 3)
							{
								NbiotDev.step = NB_DEV_RESET;
							}
							break;
						}
					}	
				}
				
			}
			break;
		case NB_AT_NSOSD:
			{
				if(NbiotDev.cmdTxStep == 0)
				{
					uint8_t tbuff[100] = {0};
					sprintf(tbuff,"AT+NSOSD=1,6,313637303933\r\n\0");
					
					SendDataToNB(tbuff);
					NbiotDev.cmdTxStep = 1;
					
					ResetNBTickClk();
				}
				else if(NbiotDev.cmdTxStep == 2)
				{
					rATStatus = NB_ATRecPro("1,6");
					if(rATStatus == SUCCESS_REC)
					{
						NbiotDev.step = NB_AT_NSOCR_STREAM;
						NbiotDev.cmdTxStep = 0;
						NbiotDev.cmdErrCnt = 0;
					}
					else
					{
						if(NbiotDev.NBTimerClk	> 1000)
						{
							NbiotDev.cmdErrCnt++;
							NbiotDev.cmdTxStep = 0;
							if(NbiotDev.cmdErrCnt > 3)
							{
								NbiotDev.step = NB_DEV_RESET;
							}
							break;
						}
					}				
				}
				else if(NbiotDev.cmdTxStep == 2)
				{
					rATStatus = NB_ATRecPro("OK");
					if(rATStatus == SUCCESS_REC)
					{
						NbiotDev.step = NB_AT_SN;
						NbiotDev.cmdTxStep = 0;
						NbiotDev.cmdErrCnt = 0;
					}
					else
					{
						if(NbiotDev.NBTimerClk	> 1000)
						{
							NbiotDev.cmdTxStep = 0;
							NbiotDev.step = NB_DEV_RESET;
							break;
						}
					}									
				}
			}
			break;
		
	}
#endif
}

#endif

