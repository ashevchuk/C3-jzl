/*************************************************

 ZEM 200

 msg.c message process and gather functions

 Copyright (C) 2003-2004, ZKSoftware Inc.

 $Log: msg.c,v $
 Revision 5.20  2006/03/04 17:30:09  david
 Add multi-language function

*************************************************/

#include <stdlib.h>
#include <string.h>
#include "arca.h"
#include "msg.h"
#include "options.h"
#include "kb.h"
#include "rs232/rs232comm.h"
#include "serial.h"
#include "net/net.h"
#include "timer.h"
#include "wiegand.h"
#include "timer.h"
#include "flashdb.h"

#include "main.h"

static U32 TimerCount=0;
static U32 EnabledMsg=0;

static PMsgProc *MessageProcs=NULL;
static int MsgProcCount=0;

static TMsg *msgQue=NULL;
static int msgQueCount=0;
static int msgQueIndex=0;


#define WiegandKey	8
#define WG26	26
#define WG34	34
#define WG50	50

static int QueryMCUID=0;

#ifdef MACHINETYPE_C4
static int MCUADDR[5]={0x03,0x7c,0x7d,0x7e,0x7f};
#else
static int MCUADDR[5]={0x03,0x7c,0x7d,0x7c,0x7d};//, 0x7e, 0x7f};
#endif

//char DoorWiegand[5][12]={"Wiegand26", "Wiegand26", "Wiegand26", "Wiegand26", "Wiegand26"};

extern PSharedStateOptions shared_stateoptions_stuff;
extern PCommData shared_Comm_stuff;



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

int GatherMsgs(PMsg msg)
{
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
			{
				TimerType = Timer_Hour;
			}

			if (TimerType == Timer_Hour)
			{
				DBPRINTF("t=%d:%d:%d, gCurTime=%d:%d:%d\n", t.tm_hour,
				           t.tm_min, t.tm_sec, gCurTime.tm_hour, gCurTime.tm_min, gCurTime.tm_sec);

			}

			//printf("get_sec = %d,gcur_sec = %d\n",t.tm_sec,gCurTime.tm_sec);
			ConstructMSG(msg, MSG_TYPE_TIMER,  0, 0);

			memcpy(&gCurTime, &t, sizeof(TTime));
			//GetTime(&gCurTime);
			Timer_count();

			return MSG_TYPE_TIMER;
		}

	}
    //232串口
	if ((gOptions.RS232On || gOptions.RS485On) && (!gOptions.IsSupportModem
			|| !gOptions.IsConnectModem) && (gOptions.CT232On != 2))
	{
		RS232Check(&st232);
	}
    //484串口
	if((gOptions.RS232On||gOptions.RS485On) && gOptions.IsSupportModem)
	{
		RS232Check(&ttl232);
	}

	EthBoradcastCheck();

	EthCommTCPCheck();

	if(1 == shared_Comm_stuff->CreateMainCmd)
	{
		ConstructMSG(msg, shared_Comm_stuff->Cmd,  shared_Comm_stuff->NewsType, (int)shared_Comm_stuff);
		shared_Comm_stuff->CreateMainCmd = 0;

		return shared_Comm_stuff->Cmd;
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
