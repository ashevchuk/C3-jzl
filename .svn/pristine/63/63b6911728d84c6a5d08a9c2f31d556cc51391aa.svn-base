/*************************************************

 ZEM 200

 msg.c message process and gather functions

 Copyright (C) 2003-2004, ZKSoftware Inc.

 $Log: msg.c,v $
*************************************************/

#include <stdlib.h>
#include <string.h>
#include "arca.h"
#include "msg.h"
#include "exfun.h"
#include "options.h"
#include "kb.h"
#include "main.h"
#include "timer.h"
#include "wiegand.h"
#include "timer.h"
#include "flashdb.h"

#include "utils.h"

static U32 TimerCount=0;
static U32 EnabledMsg=0;

static PMsgProc *MessageProcs=NULL;
static int MsgProcCount=0;

static TMsg *msgQue=NULL;
static int msgQueCount=0;
static int msgQueIndex=0;

#define WiegandKey	8
#define WiegandKey2	4
#define WG26	26
#define WG34	34
#define WG50	50
#define WG36	36
#define WG66	66

static int QueryMCUID=0;

#ifdef MACHINETYPE_C4
static int MCUADDR[5]={0x03,0x7c,0x7d,0x7e,0x7f};
#else
static int MCUADDR[5]={0x03,0x7c,0x7d,0x7c,0x7d};//, 0x7e, 0x7f};
#endif

//char DoorWiegand[5][12]={"Wiegand26", "Wiegand26", "Wiegand26", "Wiegand26", "Wiegand26"};

extern PSharedStateOptions shared_stateoptions_stuff;
extern PCommData shared_Comm_stuff;
extern int ShowMainLCDDelay;

static int putQueMsg(PMsg msg)
{
	if(msgQueIndex>=msgQueCount)
	{
		if(msgQueCount==0)
			msgQue=(PMsg)malloc(sizeof(TMsg)*10);
		else
			msgQue=(PMsg)realloc(msgQue, sizeof(TMsg)*(10+msgQueCount));
		msgQueCount+=10;
	}
	if(msgQue)
	{
		msgQue[msgQueIndex++]=*msg;
		return msgQueIndex;
	}
	return 0;
}

static int getQueMsg(PMsg msg)
{
	if(msgQueIndex && msgQue)
	{
		int i;
		*msg=msgQue[0];
		for(i=1; i<msgQueIndex; i++)
			msgQue[i-1]=msgQue[i];
		msgQueIndex--;
		return 1;
	}
	return 0;
}

int PostMsg(int message, int Param1, int param2, void *object)
{
	TMsg msg;
	msg.Object=object;
	msg.Message=message;
	msg.Param1=Param1;
	msg.Param2=param2;
	return putQueMsg(&msg);
}

int SendMsg(int message, int Param1, int param2, void *object)
{
	TMsg msg;
	msg.Object=object;
	msg.Message=message;
	msg.Param1=Param1;
	msg.Param2=param2;
	return ProcessMsg(&msg);
}


int HasInputControl(void)
{
	return MsgProcCount>1;
}

int TestEnabledMsg(int MsgType)
{
	return EnabledMsg & MsgType;
}

void EnableMsgType(int MsgType, int Enabled)
{
	if(Enabled)
	{
		EnabledMsg=EnabledMsg|MsgType;
		switch(MsgType)
		{
		case MSG_TYPE_TIMER:
			TimerCount=0;
			break;
		}
	}
	else
	{
		EnabledMsg=(EnabledMsg & ~MsgType);
	}
}

int GetMsg(PMsg msg)
{
	int m;
	while((m=GatherMsgs(msg))==0);
	return m;
}

int gFPDirectProc=0;

int GatherMsgs(PMsg msg)
{
	unsigned char buffer[10];
	int  wiegandkey=0;
	int  wiegandkey2=0;
	U32 manufactureCode=0, facilityCode=0, siteCode=0, cardNumber=0;
//	if(getQueMsg(msg)) return msg->Message;
	mmsleep(10);
	if (EnabledMsg & MSG_TYPE_TIMER)
	{
		TTime t;
		BYTE TimerType;
		GetTime(&t);
		if(t.tm_sec!=gCurTime.tm_sec)
		{
			TimerType = (gCurTime.tm_min == t.tm_min ? Timer_Second : Timer_Minute);
			if ((TimerType == Timer_Minute) && (gCurTime.tm_hour != t.tm_hour))
				TimerType = Timer_Hour;

			if (TimerType == Timer_Hour)
				DBPRINTF("t=%d:%d:%d, gCurTime=%d:%d:%d\n", t.tm_hour,
				           t.tm_min, t.tm_sec, gCurTime.tm_hour, gCurTime.tm_min, gCurTime.tm_sec);
			ConstructMSG(msg, MSG_TYPE_TIMER,  0, 0);

			memcpy(&gCurTime, &t, sizeof(TTime));
			//GetTime(&gCurTime);
			Timer_count();
			return MSG_TYPE_TIMER;
		}

	}

//add by yuanfat
#ifdef PUSHSDK
	CheckPush();
#endif

	/*
	if(CheckSDCard())
	{
		DBPRINTF("CheckSDCard ... OK\n");
	//	SDcardOK =  true;
	}else
	{
		DBPRINTF("CheckSDCard ... error\n");
	}*/
	//int tmpret = 0;
	//tmpret = get_sd_status();

	//printf("----------------- tmpret: %d\n",tmpret);

	if(1 == shared_Comm_stuff->CreateCommCmd)
	{
		ConstructMSG(msg, shared_Comm_stuff->Cmd,  shared_Comm_stuff->NewsType, (int)shared_Comm_stuff);
		shared_Comm_stuff->CreateCommCmd = 0;

		return shared_Comm_stuff->Cmd;
	}


	QueryMCUID++;
	if(gOptions.MachineType == C4 || gOptions.MachineType == C4_400To_200)	//	//5个cpu轮询
	{
		if(QueryMCUID >=5)
			QueryMCUID =0;
		if (QueryMCUID<=0)
			QueryMCUID = 0;
//		printf("Query , MCUADDR[%d]=%x\n",QueryMCUID,MCUADDR[QueryMCUID]);

	}
	else if(gOptions.MachineType == C4_200)	//	//5个cpu轮询
	{
		if(QueryMCUID >=3)
			QueryMCUID =0;
		if (QueryMCUID<=0)
			QueryMCUID = 0;
//		printf("Query , MCUADDR[%d]=%x\n",QueryMCUID,MCUADDR[QueryMCUID]);
	}
	else if(gOptions.MachineType == C3_200 ||gOptions.MachineType == C3_400 || gOptions.MachineType == C3_400To_200)	//	//2个cpu轮询
	{
		if(QueryMCUID >=5)
			QueryMCUID =1;
		if (QueryMCUID<=0)
			QueryMCUID = 1;
	}
	else if(gOptions.MachineType == C3_100)	//	//1个cpu轮询
	{
			QueryMCUID =1;
	}

	memset(buffer, 0, 10);
	if (!CheckI2CState(MCUADDR[QueryMCUID], buffer))//CheckC4State
	{
		int readerID;

//if (buffer[0] !=Evt_None)
		printf("I2C state addr=0x%x,  buffer: %02x, %02x, %02x, %02x, %02x, %02x, %02x, %02x.\n", MCUADDR[QueryMCUID],
			buffer[0], buffer[1],buffer[2], buffer[3],buffer[4], buffer[5],buffer[6], buffer[7]);

		if (buffer[0]==Evt_None)
		{
			return 0;
		}
		else if (buffer[0]==Evt_C3_Weigend)	//c3 wg事件
		{// 	事件参数：DATA[1]=维根读头编号（从1�?��编号）；DATA[2]=维根位数；DATA[3]- DATA[N]为wiegand数据，最先收到的第一位数据存放在DATA[3]的最高位，依此类推，即数据的存放是高位在前低位在后�?
			readerID = (QueryMCUID%2 == 1)?  buffer[1] :(buffer[1] + 2);//将读头编号为1~4

			if (buffer[2]==WiegandKey)// wg按键
			{
				wiegandKeyDecode(buffer+3, &wiegandkey);
				if (wiegandkey <0) return 0;
				ConstructMSG(msg, MSG_WIEGAND_KEY, wiegandkey, readerID);//QueryMCUID);
				Req_QueryOnoff(MCUADDR[QueryMCUID], Evt_C3_Weigend, buffer[1]&0xFF);
				return MSG_WIEGAND_KEY;
			}
			else if (buffer[2]==WiegandKey2)// wg按键
			{
				wiegandKeyDecode2(buffer+3, &wiegandkey2);
				if (wiegandkey <0) return 0;
				ConstructMSG(msg, MSG_WIEGAND_KEY, wiegandkey2, readerID);//QueryMCUID);
				Req_QueryOnoff(MCUADDR[QueryMCUID], Evt_C3_Weigend, buffer[1]&0xFF);
				return MSG_WIEGAND_KEY;
			}
			else  if (buffer[2] == 32 || buffer[2]==WG26 || buffer[2]== WG34 || buffer[2]== WG36)//wg读卡
			{
				if(buffer[2]==WG26)
				{
					wiegandDecode("Wiegand26", buffer+2, &manufactureCode, &facilityCode, &siteCode, &cardNumber);
				}
				else if(buffer[2]==WG34)
				{
					wiegandDecode("Wiegand34", buffer+2, &manufactureCode, &facilityCode, &siteCode, &cardNumber);
				}
				else if(buffer[2]==32)
				{
					wiegandDecode("Wiegand35", buffer+2, &manufactureCode, &facilityCode, &siteCode, &cardNumber);
				}
				else if(buffer[2]==WG36)
				{
					wiegandDecode("Wiegand36", buffer+2, &manufactureCode, &facilityCode, &siteCode, &cardNumber);
				}
				else if(buffer[2]==WG66)
				{
					wiegandDecode("Wiegand66", buffer+2, &manufactureCode, &facilityCode, &siteCode, &cardNumber);
				}
				printf("API cardNumber=%u\n", cardNumber);
				ConstructMSG(msg, MSG_TYPE_HID, cardNumber, readerID);
				//Req_Query(MCUADDR[QueryMCUID], Evt_Weigend);
				Req_QueryOnoff(MCUADDR[QueryMCUID], Evt_C3_Weigend, buffer[1]&0xFF);
				return MSG_TYPE_HID;
			}
			else
			{
				printf("NOT stardand reader bitlen = %d\n", buffer[2]);
				Req_QueryOnoff(MCUADDR[QueryMCUID], Evt_C3_Weigend, buffer[1]&0xFF);
			}

		}
		else if (buffer[0]==Evt_Weigend)	// c4 reader card
		{
			if (buffer[1]==WiegandKey)// wg按键
			{
				wiegandKeyDecode(buffer+2, &wiegandkey);
				if (wiegandkey <0) return 0;
				ConstructMSG(msg, MSG_WIEGAND_KEY, wiegandkey, QueryMCUID);
				Req_Query(MCUADDR[QueryMCUID], Evt_Weigend);
				return MSG_WIEGAND_KEY;
			}
			else if (buffer[1]==WiegandKey2)// wg按键
			{
				wiegandKeyDecode2(buffer+2, &wiegandkey2);
				if (wiegandkey2 <0) return 0;
				ConstructMSG(msg, MSG_WIEGAND_KEY, wiegandkey2, QueryMCUID);
				Req_Query(MCUADDR[QueryMCUID], Evt_Weigend);
				return MSG_WIEGAND_KEY;
			}
			else   if (buffer[1]==WG26 || buffer[1]== WG34 || buffer[2]== WG36)//wg读卡
			{
				if(buffer[1]==WG26)
				{
					wiegandDecode("Wiegand26", buffer+1, &manufactureCode, &facilityCode, &siteCode, &cardNumber);
				}
				else if(buffer[1]==WG34)
				{
					wiegandDecode("Wiegand34", buffer+1, &manufactureCode, &facilityCode, &siteCode, &cardNumber);
				}
				else if(buffer[2]==WG36)
				{
					wiegandDecode("Wiegand36", buffer+1, &manufactureCode, &facilityCode, &siteCode, &cardNumber);
				}
//				printf("API cardNumber=%u\n", cardNumber);
				ConstructMSG(msg, MSG_TYPE_HID, cardNumber, QueryMCUID);
				Req_Query(MCUADDR[QueryMCUID], Evt_Weigend);
				return MSG_TYPE_HID;
			}
			else
				Req_Query(MCUADDR[QueryMCUID], Evt_Weigend);
		}
		//其它事件
		else if (buffer[0]==Evt_KeyPress)	//键盘按键事件
		{
			ShowMainLCDDelay = 10;
			printf("Evt_KeyPress  buffer[1]=%d  QueryMCUID %d\n", buffer[1],QueryMCUID);
			//参数2 单片机ID,  参数1 按键�?
			ConstructMSG(msg, MSG_TYPE_BUTTON,buffer[1],QueryMCUID);
			Req_Query(MCUADDR[QueryMCUID], Evt_KeyPress);
			return MSG_TYPE_BUTTON;
		}
		else if (buffer[0]==Evt_OnOff) //输入io口状态变�?
		{
			////取门的状�?并�?�?comm
			{
				int DoorOpenstatus =  GetDoorState();//表示门的�?��状�?�?

				 if(gStateOptions.DoorOpenstatus != DoorOpenstatus)
				 {
					 gStateOptions.DoorOpenstatus = DoorOpenstatus;
					 memcpy(shared_stateoptions_stuff,&gStateOptions,sizeof(TStateOptions));
					printf("main have changed goptions. ---  GetDoorState \n");
				 }
			}

			printf("Evt_OnOff addr=%d, state=%d,MCUADDR = 0x%x\n",
				buffer[1]&0xFF, buffer[2]&0xFF,MCUADDR[QueryMCUID]);
			U32 seq = ((buffer[1]<<8)&0xFF00) + (buffer[2] & 0x00FF);
			if(gOptions.MachineType == C4 || gOptions.MachineType == C4_200 || gOptions.MachineType == C4_400To_200)	//
				ConstructMSG(msg, MSG_TYPE_RELAY, seq, QueryMCUID);
			else if(gOptions.MachineType == C3_100 ||gOptions.MachineType == C3_200
					|| gOptions.MachineType == C3_400 || gOptions.MachineType == C3_400To_200)
				ConstructMSG(msg, MSG_TYPE_RELAY, seq, ((QueryMCUID>2)?(QueryMCUID-2):QueryMCUID));
			Req_QueryOnoff(MCUADDR[QueryMCUID], Evt_OnOff, buffer[1]&0xFF);
			return MSG_TYPE_RELAY;
		}
		else if (buffer[0]==Evt_McuReset)	//	MCU复位事件
		{
			printf("resetcode=%d\n", buffer[1]);
//			ConstructMSG(msg, MSG_TYPE_CMD, Evt_McuReset, QueryMCUID);
			Req_Query(MCUADDR[QueryMCUID], Evt_McuReset);
			return MSG_TYPE_CMD;
		}
		else if (buffer[0]==Evt_Change485Addr)	//485地址变了
		{
			printf("Evt_Change485Addr= %d\n", buffer[1]);

			if(buffer[1] !=0)
			{
				gOptions.DeviceID = buffer[1];
				//memcpy(&(shared_option_stuff->Options),&gOptions,sizeof(TOptions));
				//shared_option_stuff->main_change_mark = 1;
				printf("main have changed goptions.\n");
			}
//			ConstructMSG(msg, MSG_TYPE_CMD, Evt_McuReset, QueryMCUID);
			Req_QueryOnoff(MCUADDR[QueryMCUID], Evt_Change485Addr,buffer[1]&0xFF);
			return MSG_TYPE_CMD;
		}
		else if (buffer[0]==Evt_ReadStrip)//	读头安裑或拆卸时产生的事�?
		{
		//c3 buf[1] 读头号，buf[2]安裑或拆�?00:拆卸 ff:安裑
			printf("Evt_ReadStrip  ID = %d,state=%d\n", readerID,buffer[2]&0xFF);
//			ConstructMSG(msg, MSG_TYPE_CMD, Evt_ReadStrip, QueryMCUID);
			Req_QueryOnoff(MCUADDR[QueryMCUID], Evt_ReadStrip,buffer[1]&0xFF);
			return MSG_TYPE_CMD;
		}
		else if (buffer[0]==Evt_MachineMal)	//当MCU自棌时，�?��到故障，产生�?��故障事件告知主节�?
		{
			printf("Evt_MachineMal state=%d\n", buffer[1]&0xFF);
//			ConstructMSG(msg, MSG_TYPE_CMD, Evt_MachineMal, QueryMCUID);
			Req_Query(MCUADDR[QueryMCUID], Evt_MachineMal);
			return MSG_TYPE_CMD;
		}
		else if (buffer[0] == Evt_DogOver)	//	软件狗溢出事�?
		{
//			ConstructMSG(msg, MSG_TYPE_CMD, Evt_DogOver, QueryMCUID);
			Req_Query(MCUADDR[QueryMCUID], Evt_DogOver);
			return MSG_TYPE_CMD;
		}
		else if (buffer[0] == Evt_ACPower)	//	AC电源事件
		{
//			ConstructMSG(msg, MSG_TYPE_CMD, Evt_ACPower, QueryMCUID);
			Req_Query(MCUADDR[QueryMCUID], Evt_ACPower);
			return MSG_TYPE_CMD;
		}
		else if (buffer[0] == Evt_BatteryLower)		//电池电压事件
		{
//			ConstructMSG(msg, MSG_TYPE_CMD, Evt_BatteryLower, QueryMCUID);
			Req_Query(MCUADDR[QueryMCUID], Evt_BatteryLower);
			return MSG_TYPE_CMD;
		}
	}
	return 0;
}

int TranslateMsg(int MsgType, PMsg msg)
{
	return 1;
}

U32 SelectNewMsgMask(U32 newmsk)
{
	U32 oldmsk=EnabledMsg;
	EnabledMsg=newmsk;
	return oldmsk;
}

int ProcessMsg(PMsg msg)
{
	int i=MsgProcCount;
	while(i>0)
	{
		i--;
		if (MessageProcs[i](msg)) break;
	}
	return i>=0;
}

int DoMsgProcess(void *Obj, int ExitCommand)
{
	TMsg msg;
	int i;
	msg.Object=Obj;
	do
	{
		msg.Message=0;
		i=GetMsg(&msg);
		if(TranslateMsg(i, &msg))
		{
			while(msg.Message && !((msg.Message==MSG_TYPE_CMD) && (msg.Param1==ExitCommand)))
			{
				if(!ProcessMsg(&msg)) break;
			}
		}
	}while(!((msg.Message==MSG_TYPE_CMD) && (msg.Param1==ExitCommand)));
	return msg.Param2;
}

int RegMsgProc(PMsgProc MsgProc)
{
	if(MessageProcs==NULL)
		MessageProcs=(PMsgProc*)malloc(sizeof(PMsgProc)*200);
	MessageProcs[MsgProcCount++]=MsgProc;
	return MsgProcCount-1;
}

int RegMsgProcBottom(PMsgProc MsgProc)
{
	int i=MsgProcCount-1;
	if(MessageProcs==NULL)
		MessageProcs=(PMsgProc*)malloc(sizeof(PMsgProc)*200);
	while(i)
		MessageProcs[i+1]=MessageProcs[i];
	MessageProcs[0]=MsgProc;
	MsgProcCount++;
	return 0;
}

int UnRegMsgProc(int index)
{
	int i=index;
	while(i<MsgProcCount-1)
	{
		MessageProcs[i]=MessageProcs[i+1];
	}
	MsgProcCount--;
	return MsgProcCount;
}

extern PCommData shared_Comm_stuff;
#define MAX_SEND_DATA_LEN    1024*8

int SendMainCmd(int Cmd, int NewsType, char *SendData, int SendDataLen, int TimeOutMS)
{
	int ret = 0;
	U32 ms = 0;
	int WaitCommTimeOutMS = TimeOutMS;
	shared_Comm_stuff->MainCmdRet = 1;
	shared_Comm_stuff->CreateMainCmd = 0;
	shared_Comm_stuff->MainSendMark = 0;


	if(SendDataLen > MAX_SEND_DATA_LEN)
	{
		return ERR_SEND_MAINCMD_OVERMAXLEN;
	}

	//如果main、comm同时发送，main进程等待TimeOutMS/2，让comm先发。确保comm发送成功。
	ms = GetTickCount();
	while(1)
	{
		if(0 == shared_Comm_stuff->CommSendMark)
		{
			break;
		}

		if((GetTickCount()-ms) > WaitCommTimeOutMS)
		{
			break;
		}

		mmsleep(50);
	}

	printf("------------- CommSendMark: %d\n",shared_Comm_stuff->CommSendMark);
	if(1 == shared_Comm_stuff->CommSendMark)
	{
		shared_Comm_stuff->MainSendMark = 0;
		return ERR_SEND_MAINCMD_MAINING;
	}

	shared_Comm_stuff->MainSendMark = 1;
	shared_Comm_stuff->CreateMainCmd = 1;
	shared_Comm_stuff->Cmd = Cmd;
	shared_Comm_stuff->NewsType = NewsType;

	if(NULL == SendData)
	{
		memset(shared_Comm_stuff->SendData, 0, MAX_SEND_DATA_LEN);
	}
	else
	{
		memcpy(shared_Comm_stuff->SendData, SendData, SendDataLen);
	}

	ms = GetTickCount();
	while(1)
	{
		shared_Comm_stuff->MainSendMark = 1;
		if((GetTickCount()-ms) > WaitCommTimeOutMS)
		{
			ret = ERR_SEND_MAINCMD_TIMEOUT; //comm 进程响应超时，发送命令退出。
			break;
		}

		//printf("comm SendCommCmd shared_Comm_stuff->NewsType: %d\n", shared_Comm_stuff->NewsType);
		if(0 == shared_Comm_stuff->MainCmdRet)
		{
			ret = SEND_MAINCMD_OK;        //main进程处理NewsType指令完毕。
			break;
		}
		else if(-1 == shared_Comm_stuff->NewsType)
		{
			ret = ERR_SEND_MAINCMD_DATAERR;  //comm进程发送了错误的数据给main进程。比如DoorID > 4  或  SignalID > 2
			break;
		}

		mmsleep(50);
	}

	shared_Comm_stuff->MainSendMark = 0;
	return ret;
}

