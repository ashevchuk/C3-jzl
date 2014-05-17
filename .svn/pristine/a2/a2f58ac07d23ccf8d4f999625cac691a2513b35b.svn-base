/************************************************
 ZEM 200
 main.c Main source file

 Copyright (C) 2003-2005, ZKSoftware Inc.

 $Log: main.c,v $
 Revision 5.2  2005/04/05 21:38:11  david
 Add Support to update firmware by USB Flash Disk

 Revision 5.1  2005/03/17 18:48:00  david
 Add CVS Log KeyWord
*************************************************/
#include <stdlib.h>
#include <sys/types.h>
#include<string.h>

//add by oscar for c3 为共享内存用
#include <sys/ipc.h>
#include <sys/shm.h>
//end

#include "../platform/serial.h"
#include "flashdb.h"
#include "msg.h"

#include "net/net.h"
#include "netspeed.h"
#include "rs232/rs232comm.h"

#include "utils.h"
#include "options.h"
#include "timer.h"

#include "main.h"

int gMachineState=STA_IDLE;

#define TIMEOUT_INPUT_PIN		(gOptions.TimeOutPin)	//等待用户输入验证考勤号码的时长
//#define TIMEOUT_WAIT_MAINLCD		3	//显示验证成功或失败信息后延迟显示主见面的时间
#define TIMEOUT_SHOWSTATE		(gOptions.TimeOutState)	//显示临时考勤状态时长
#define MaxAdminVerify 			10
#define DELAYNEXTUSER     		30 	//多人验证时等待下一个用户验证的时间


int MainProcMsg(PMsg msg);

int gEthOpened=FALSE;

BOOL RTCTimeValidSign=TRUE;

#define CHECK_C2_INTERVAL       10    //10秒查一次C2

PRTLogNode rtloglist;
PRTLogNode rtlogheader;
#define MAXRTLOGSCACHE 32	//max count for real time logs

int gSetGatewayWaitCount=0;

int dev_handle_timer = 0;

//add by ocar for c3
Tpollcard pollcard_struct_variable;//刷卡变量结构
//TOverseeRTLog Oversee_RTLog;

void *shared_stateoptions = (void *)0;
PSharedStateOptions shared_stateoptions_stuff;
int shmid;

void *shared_RTLog = (void *)0;
POverseeRTLog shared_RTLog_stuff;
int shmid2;

void *shared_Comm = (void *)0;
PCommData shared_Comm_stuff;
int g_comm_shmid = 0;

int main(void)
{
	sleep(1);
#ifdef MACHINETYPE_C4
	DBPRINTF("C4_comm start!\n");
#else
	DBPRINTF("C3_comm start!\n");
#endif


	if(!GPIO_IO_Init())
	{
		DBPRINTF("GPIO OR FLASH Initialize Error!\n");
	}
	else
	{
		DBPRINTF("GPIO OR FLASH Initialize OK!\n");
	}

	dev_handle_timer = open_timer();
	if(dev_handle_timer)
	{
		SetTimer(0, 1);//
		SetTimer(1, 60);//
		SetTimer(3,1 );//  3自动调用2   2  用作gui刷屏用

		DBPRINTF("open_timer OK\n");
	}
	else
		DBPRINTF("open_timer error\n");

//共享内存思路:
//共两个内存块，一个为goption,一个为实时坚控
//在comm中，如果修改了goption,则同时改变标志，main中就会捕捉到
//共享内存准备
//共享内存思路:
//共两个内存块，一个为goption,一个为实时坚控
//在comm中，如果修改了goption,则同时改变标志，main中就会捕捉到
//共享内存准备
	shmid = shmget((key_t)12345,sizeof(TSharedStateOptions),0666|IPC_CREAT);
	if(shmid == -1)
	{
		printf("shmget failed\n");
		exit(EXIT_FAILURE);
	}
	shared_stateoptions = shmat(shmid,(void *)0,0);
	if(shared_stateoptions == (void *)-1)
	{
		printf("shmat failed\n");
		exit(EXIT_FAILURE);
	}
	shared_stateoptions_stuff = (PSharedStateOptions)shared_stateoptions;
	printf("shared_gOptions attached at %X\n",(int)shared_stateoptions);
	memset(shared_stateoptions_stuff,0x00,sizeof(TSharedStateOptions));

	shmid2 = shmget((key_t)1234,sizeof(TOverseeRTLog),0666|IPC_CREAT);
	if(shmid2 == -1)
	{
		printf("shmget2 failed\n");
		exit(EXIT_FAILURE);
	}
	shared_RTLog = shmat(shmid2,(void *)0,0);
	if(shared_RTLog == (void *)-1)
	{
		printf("shmat2 failed\n");
		exit(EXIT_FAILURE);
	}
	shared_RTLog_stuff = (POverseeRTLog )shared_RTLog;
	printf("shared_gOptions attached at %X\n",(int)shared_RTLog);
	memset(shared_RTLog_stuff,0x00,sizeof(TOverseeRTLog));

	//为进程间通信申请共享内存
	g_comm_shmid = shmget((key_t)123,sizeof(TCommData),0666|IPC_CREAT);
	if(g_comm_shmid == -1)
	{
		printf("g_comm_shmid failed\n");
		exit(EXIT_FAILURE);
	}
	shared_Comm = shmat(g_comm_shmid,(void *)0,0);
	if(shared_Comm == (void *)-1)
	{
		printf("g_comm_shmid failed\n");
		exit(EXIT_FAILURE);
	}
	shared_Comm_stuff = (PCommData )shared_Comm;
	printf("shared_Comm attached at %X\n",(int)shared_Comm);
	memset(shared_Comm_stuff,0x00,sizeof(TCommData));

	//为进程间通信申请共享内存结束

	InitOptions();
	InitStateOptions();

	memcpy(&gStateOptions, shared_stateoptions_stuff, sizeof(TStateOptions));
	//shared_option_stuff->comm_change_mark = 1;

	//准备网络参数
	gEthOpened=FALSE;
	if(gOptions.NetworkFunOn)
	{
		//SET IP ADDRESS, NETMASK , SPEED
		SetNetworkIP_MASK(gOptions.IPAddress, gOptions.NetMask);
		set_network_speed(NET_DEVICE_NAME, gOptions.HiSpeedNet);
//为方便udp广播发送成功，如果无网关，则将自己的ip设置为网关
		if(gOptions.GATEIPAddress[0] == 0)
		{
			if(!SetGateway("add", gOptions.IPAddress))
				DBPRINTF("add gate way: %d.%d.%d.%d \n",gOptions.IPAddress[0],gOptions.IPAddress[1],gOptions.IPAddress

[2],gOptions.IPAddress[3]);
		}
		else
		{
			if(!SetGateway("add", gOptions.GATEIPAddress))
				gSetGatewayWaitCount=gOptions.SetGatewayWaitCount;
		}

		ExportProxySetting();
		gEthOpened=(EthInit()==0);
		DBPRINTF("EthInit Result =  %d\n",gEthOpened);
	}
	//Synchronize Linux system time from RTC clock
//	DBPRINTF("Synchronize system time from RTC\n");
	//RTCTimeValidSign=ReadRTCClockToSyncSys(&gCurTime);

	DBPRINTF("Buff2Time:%d-%d-%d %d:%d:%d\n",gCurTime.tm_year,gCurTime.tm_mon,gCurTime.tm_mday,gCurTime.tm_hour,gCurTime.tm_min,gCurTime.tm_sec);
	GetTime(&gCurTime);
	DBPRINTF("Gettime:%d-%d-%d %d:%d:%d\n",gCurTime.tm_year,gCurTime.tm_mon,gCurTime.tm_mday,gCurTime.tm_hour,gCurTime.tm_min,gCurTime.tm_sec);

	//SetTime(&gCurTime);


//如果不是通迅进程，不需要初始化comm
	printf("comm init......\n");
	if (gOptions.IsSupportModem != 0)
	{
		if (ttl232.init(gOptions.RS232BaudRate, V10_8BIT, V10_NONE, 0) ==0)
		{
			printf("rs232 init baudrate = %d, gOptions.RS485On: %d\n",gOptions.RS232BaudRate, gOptions.RS485On);
			if (gOptions.RS485On)
			{
				RS485_setmode(FALSE);
				printf("Rs232/485  init  baudrate = %d,...........................OK\n",gOptions.RS232BaudRate);
			}
		}
	}

	if (gOptions.IsSupportModem == 0)
	{
		if (st232.init(gOptions.RS232BaudRate, V10_8BIT, V10_NONE, 0) != 0)
		{
			printf("rs232 init failed baudrate = %d, gOptions.RS485On: %d\n",gOptions.RS232BaudRate, gOptions.RS485On);
			//gOptions.RS232On = 0;
			//gOptions.RS485On = 0;
		}
		printf("st232 init baudrate = %d, gOptions.RS485On: %d\n",gOptions.RS232BaudRate, gOptions.RS485On);
		if (gOptions.RS485On)
		{
			RS485_setmode(FALSE);
		}
	}

	//main 进程不负责保存
	SaveOptions(&gOptions);

	shared_Comm_stuff->SendLen = SaveTableContent(FCT_C3GUARDEREVENTLOG,shared_Comm_stuff->SendData);

	SendCommCmd(MSG_COMM_CMD,SUBCMD_LOAD_BAKDATA, shared_Comm_stuff->SendData, shared_Comm_stuff->SendLen,40000);

	EnableMsgType(MSG_TYPE_TIMER, 1);

	FDB_InitDBs(TRUE);


	RegMsgProc(MainProcMsg);
	DoMsgProcess(NULL, News_Exit_Program);

	ttl232.free();

	EthFree();
	//关闭LCD
	FDB_FreeDBs();

	return 0;
}


int MainProcMsg(PMsg msg)
{

	int message=msg->Message;

	//printf("msg =0x%x ",message);
	msg->Message = 0;

	switch(message)
	{
		case MSG_MAIN_CMD:
			  switch (msg->Param1)
			  {
				  case SUBCMD_UPDATE_OPTIONS:
				  {
					  PCommData shared_Comm_stuff = (PCommData)msg->Param2;
					  On_CommUpdate(shared_Comm_stuff);
					  break;
				  }
				  default:
					  break;

			  }
			break;
		default:
			break;
	}

	return 1;
}


unsigned int Timer1Count,Timer2Count;
const U16 mii_regs_val = 0x3100;
int linkOK = 0;
// 定时事件
void OnTimer(unsigned char cID)
{
	switch (cID)
	{
		case 0:									// 秒逻辑
		{
			linkOK = get_netlink_status_by_mii();
			if(linkOK >= 0)
			{
				GPIO_netled_link_Status(linkOK);
			}
		}
		break;
		case 1:
		{
			//读取以太网的在线状态
			if(linkOK && mii_regs_val != ReadNetState())
			{
				printf("----------Network error\n");
				RebootNetwork(mii_regs_val);
			}
			//GetOptions_DeviceID();//防止485地址读取错误，每分钟读取一次。

		}
		break;
	}

}



void Ex_ConctrolLock(int opencause,int MachineType,int index,int time)
{
	pollcard_struct_variable.DoorOpenCause[index-1]  = opencause;//门是正常打开的
	IIC_ConctrolLock(MachineType, index, time);
}

int On_CommUpdate(PCommData shared_Comm_stuff)
{
	shared_Comm_stuff->MainCmdRet = 0;	 //main进程执行开门命令成功。
	memcpy(&gOptions, shared_Comm_stuff->SendData, sizeof(TOptions));

	return 0;
}

