/*************************************************

 ZEM 200

 msg.h

 Copyright (C) 2003-2004, ZKSoftware Inc.

*************************************************/

#ifndef _MSG_H_
#define _MSG_H_

#include <stdlib.h>
#include <stdio.h>
#include "arca.h"

#define MSG_TYPE_CMD		1
#define MSG_WIEGAND_KEY	(1 << 1)
#define MSG_TYPE_RELAY 		(1 << 2)
#define MSG_TYPE_FINGER 	(1 << 3)
#define MSG_TYPE_USB 		(1 << 4)
#define MSG_TYPE_ETHERNET 	(1 << 5)
#define MSG_TYPE_TIMER 		(1 << 6)
#define MSG_TYPE_BUTTON	(1 << 7)  //0x40
#define MSG_TYPE_MF		(1 << 8)
#define MSG_TYPE_HID		(1 << 9)
#define MSG_TYPE_DOOR		(1 << 10)
#define MSG_COMM_CMD		(1 << 11) //从comm过来的指令。
#define MSG_MAIN_CMD		(1 << 12) //向comm发送指令。

#define News_Exit_Program 		1
#define News_Exit_Input 		3

#define News_VerifiedByPwd		10
#define News_VerifiedByFP		11
#define News_VerifiedByIDCard		12
#define News_VerifiedByPIN		13
#define News_VerifiedByMFCard		14
#define News_VerifiedByIDCardRIS	15
#define News_VerifiedByFPRIS		16
#define News_VerifiedByiCLASSCard  17 //iCLSRW

#define News_FailedByPwd		20
#define News_FailedByFP			21
#define News_FailedByIDCard		22
#define News_FailedByPIN		23
#define News_FailedByMFCard		24
#define News_FailedByiCLASSCard 25 //iCLSRW

#define News_Door_Button 	53
#define News_Door_Sensor 	54
#define News_Alarm_Strip 	55
#define News_Reset_Options	56
#define News_Alarm_ErrTimes     58

#define News_CancelInput 	100
#define News_CommitInput 	101
#define News_ErrorInput		102
#define News_NextInput 		103
#define News_PrevInput 		104
#define News_TimeOut		105


#define SUBCMD_CONTROL_DOOR_STATUS		1  //comm进程发送的控制门状态的指令
#define SUBCMD_ALARM_CANCEL				2  //comm进程发送的取消报警
#define SUBCMD_REBOOT  					3  //comm进程发送的重启指令
#define START_DOOR_KEEP_OPEN            4  //comm进程发送的启动常开指令
#define SUBCMD_BACKUP_DATA            6    //comm进程发送的备份指令
#define SUBCMD_UPDATE_OPTIONS  					10  //comm进程发送的更新Options参数的指令
#define SUBCMD_LOAD_BAKDATA             16  //comm进程发送备份数据表结构信息

#define Timer_None	0
#define Timer_Second	1
#define Timer_Minute	2
#define Timer_Hour	3


//与从单片机通迅事件定义
#define Evt_None		0x0

// 	事件参数：DATA[1]=维根读头编号（从1开始编号）；DATA[2]=维根位数；DATA[3]- DATA[N]为wiegand数据，最先收到的第一位数据存放在DATA[3]的最高位，依此类推，即数据的存放是高位在前低位在后。
#define Evt_C3_Weigend	0x10	//c3维根事件
#define Evt_Weigend		0x11
#define Evt_OnOff		0x12
#define Evt_ReadStrip	0x13
#define Evt_MachineMal	0x14
#define Evt_DogOver		0x15
#define Evt_KeyPress		0x16
#define Evt_McuReset		0x17
#define Evt_ACPower		0x18
#define Evt_BatteryLower	0x19
#define Evt_Change485Addr	0x1b

#define	SEND_MAINCMD_OK	0	//向comm进程发送指令，comm进程处理完成后，在设定时间内返回。
#define	ERR_SEND_MAINCMD_TIMEOUT	-1   //comm进程返回给comm进程超时。
#define	ERR_SEND_MAINCMD_DATAERR	-2   //main进程发送了错误的数据给comm进程。
#define	ERR_SEND_MAINCMD_OVERMAXLEN -3   //main发送的数据超过了最大值。
#define	ERR_SEND_MAINCMD_MAINING	-4   //main进程检测到comm进程在发送。

typedef struct _TAG_MSG
{
	void *Object;
	int Message;
	int Param1,Param2;
} TMsg, *PMsg;

//a message process handle will process a interested event and
//return 1, if the msg isn't interested, return is 0
typedef int (*PMsgProc)(PMsg msg);

void EnableMsgType(int MsgType, int Enabled);

//wait a event and get it, return the message
int GetMsg(PMsg msg);
//test if have a event, return the message or 0
int GatherMsgs(PMsg msg);
//call all registered message handle, if there is no handle
//process it return 0, else return 1. Only the top handle can
//process the interested messages.
int ProcessMsg(PMsg msg);
//translate a message
int TranslateMsg(int MsgType, PMsg msg);
//waiting and process events util ExitCommand occurred
int DoMsgProcess(void *Obj, int ExitCommand);

U32 SelectNewMsgMask(U32 newmsk);

//Register a custom message process function
int RegMsgProc(PMsgProc MsgProc);
int RegMsgProcBottom(PMsgProc MsgProc);
int UnRegMsgProc(int index);
int TestEnabledMsg(int MsgType);
int HasInputControl(void);

#define ConstructMSG(msg,MSG,P1,P2) \
do\
{\
	msg->Message=MSG;\
	msg->Param1=P1;\
	msg->Param2=P2;\
}while(0)

#endif
