/************************************************
 ZEM 200
 main.c Main source file

 Copyright (C) 2003-2005, ZKSoftware Inc.

 $Log: main.c,v $
 Revision 5.24  2006/03/04 17:30:09  david
 Add multi-language function

*************************************************/
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/wait.h>

//add by oscar for c3 为共享内存用
#include <sys/ipc.h>
#include <sys/shm.h>
//end

#include "rtc.h"
#include "serial.h"
#include "msg.h"
#include "lcdmenu.h"
#include "flashdb.h"

#include "../lcd/lcm.h"
#include "locale.h"
#include "options.h"

//add by oscar for c3
#include "timer.h"
#include "c4i2c.h"
#include "pollcard_logic.h"
#include "processmsg.h"
#include "../platform/usb_helper.h"
#include "main.h"
//end

#define DAYLIGHTSAVINGTIME		1
#define STANDARDTIME	2

int gMachineState=STA_IDLE;
int MainProcMsg(PMsg msg);

BOOL RTCTimeValidSign=TRUE;
int ShowMainLCDDelay = 0; //显示主界面的等待时间
U32 forcepwdtimeout[4] = {0}; //仅卡模式，先输入胁迫密码等待刷卡时间
int CommKeepOpenSign[4] = {0};//远程常开标记

U32 WaitSleepCount = 0; //等到N秒后休眠。本设备休眠为关闭LCD背光以及关闭主界面上字体、两个圆点的显示。


//门控产品有可能有两组反潜，所以需要两组内存
PAlarmRec CurAlarmRec=NULL;//反潜回数组buff
PAlarmRec CurAlarmRec2=NULL;//反潜回数组buff

PLastTimeAPB CurLastTimeAPB[4]={NULL};

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
//end

void *shared_Comm = (void *)0;
PCommData shared_Comm_stuff;
int g_comm_shmid = 0;

char BakTableData[12*1024];
int BakTableLen = 0;

int main(void)
{
	int NewLng;

#ifdef MACHINETYPE_C4
        DBPRINTF("C4_Main start!\n");
#else
        DBPRINTF("C3_Main start!\n");
#endif


	if(!GPIO_IO_Init())
		DBPRINTF("GPIO OR FLASH Initialize Error!\n");

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

	ClockEnabled = TRUE;
	InitOptions();
	InitStateOptions();

	/*
	//if(shared_option_stuff->comm_change_mark == 1)
	//{
		//printf("Get goption from comm_process!\n");
		//memcpy(&gOptions,&(shared_option_stuff->Options),sizeof(TOptions));
		//shared_option_stuff->comm_change_mark = 0;
	//}
	*/

	//Synchronize Linux system time from RTC clock
//	DBPRINTF("Synchronize system time from RTC\n");
	RTCTimeValidSign=ReadRTCClockToSyncSys(&gCurTime);
	DBPRINTF("Buff2Time:%d-%d-%d %d:%d:%d\n",gCurTime.tm_year,gCurTime.tm_mon,gCurTime.tm_mday,gCurTime.tm_hour,gCurTime.tm_min,gCurTime.tm_sec);
	SetTime(&gCurTime);
	GetTime(&gCurTime);
	DBPRINTF("Gettime:%d-%d-%d %d:%d:%d\n",gCurTime.tm_year,gCurTime.tm_mon,gCurTime.tm_mday,gCurTime.tm_hour,gCurTime.tm_min,gCurTime.tm_sec);


	if(LoadInteger(NOKEYPAD,0)
		&& gOptions.Must1To1
		&& !(gOptions.RFCardFunOn
				|| gOptions.IsSupportMF
				|| gOptions.IsSupportiCLSRW )//iCLSRW
	 )	//F10 no keypad,1:1 will make F10 not usablde

	gOptions.Must1To1=FALSE;
	WaitSleepCount = gOptions.IdleMinute * 60;

	EnableMsgType(MSG_TYPE_TIMER, 1);
	EnableMsgType(MSG_TYPE_BUTTON, 1);
	EnableMsgType(MSG_TYPE_FINGER, 1);
	EnableMsgType(MSG_TYPE_MF, 1);
	EnableMsgType(MSG_TYPE_HID, 1);
	EnableMsgType(MSG_TYPE_DOOR, 1);

	//OpenSDCard();

	FDB_InitDBs(TRUE);

//	UpdateAttLog();
	////反潜回处理,先将每个cARD最近记录调入buffer中
	CurAlarmRec=(PAlarmRec)malloc(gOptions.MaxUserCount*100*sizeof(TAlarmRec));
	memset(CurAlarmRec, 0, gOptions.MaxUserCount*100*sizeof(TAlarmRec));
	CurAlarmRec2=(PAlarmRec)malloc(gOptions.MaxUserCount*100*sizeof(TAlarmRec));
	memset(CurAlarmRec2, 0, gOptions.MaxUserCount*100*sizeof(TAlarmRec));
	//时间反潜
	CurLastTimeAPB[0] = (PLastTimeAPB)malloc(gOptions.MaxUserCount*100*sizeof(TLastTimeAPB));
	memset(CurLastTimeAPB[0],0,gOptions.MaxUserCount*100*sizeof(TLastTimeAPB));
	CurLastTimeAPB[1] = (PLastTimeAPB)malloc(gOptions.MaxUserCount*100*sizeof(TLastTimeAPB));
	memset(CurLastTimeAPB[1],0,gOptions.MaxUserCount*100*sizeof(TLastTimeAPB));
	CurLastTimeAPB[2] = (PLastTimeAPB)malloc(gOptions.MaxUserCount*100*sizeof(TLastTimeAPB));
	memset(CurLastTimeAPB[2],0,gOptions.MaxUserCount*100*sizeof(TLastTimeAPB));
	CurLastTimeAPB[3] = (PLastTimeAPB)malloc(gOptions.MaxUserCount*100*sizeof(TLastTimeAPB));
	memset(CurLastTimeAPB[3],0,gOptions.MaxUserCount*100*sizeof(TLastTimeAPB));

	BYTE log_buf[50];
	memset(log_buf, 0x00,50);
	sprintf(log_buf,"power on, at %d-%d-%d %d:%d:%d.\n",gCurTime.tm_year+1900,gCurTime.tm_mon+1,gCurTime.tm_mday,gCurTime.tm_hour,gCurTime.tm_min,gCurTime.tm_sec);
	FDB_AddOPLog(log_buf,sizeof(log_buf));
//2010 0414  for c4 液晶显示
	if(gOptions.MachineType == C4 || gOptions.MachineType == C4_200 || gOptions.MachineType == C4_400To_200)
	{
		if(!ExLCDOpen())
		{
			DBPRINTF("LCD Open Error!\n");
			ExLCDClose();
		}

		DBPRINTF("Starting ... FONT\n");
		NewLng = LoadInteger("NewLng", gOptions.Language);
		if (NewLng != gOptions.Language)
		{
			if (!NewLng)
			{
				NewLng = gOptions.Language;
			}
			else
			{
				gOptions.Language = NewLng;
			}
			SaveInteger("Language", gOptions.Language);
			SaveInteger("NewLng", gOptions.Language);
		}


		SelectLanguage(gOptions.Language);
		NewLng=GetDefaultLocaleID();

		if(!SetDefaultLanguage(NewLng, gRowHeight))
		{
			gOptions.Language=LanguageEnglish;
			SaveInteger("Language", gOptions.Language);
			SaveInteger("NewLng", gOptions.Language);
			DBPRINTF("select default language\n");
			SelectLanguage(gOptions.Language);
			SetDefaultLanguage(LID_ENGLISH, gRowHeight);
		}
		else
		{
			char *p=LoadStrByID(3);
			if(p && '1'==*p) gLangDriver->Bidi=1;
			p=LoadStrByID(4);
			if(p && '1'==*p) gLangDriver->RightToLeft=1;
		}
		if(gLCDHeight<64)
			gLCDCharWidth=gLCDWidth/gLangDriver->CharWidth;


		if(CMD_OK == ExEnableLCDLight(0XFF)) //开LCD背光,原因：如果背光关，此时仅仅重启main进程，背光需要开。
		{
			printf("open LCDLight success!\n");
		}
		else
		{
			printf("open LCDLight failed!\n");
		}

		ShowMainLCDDelay = 3;
	}


	gStateOptions.DoorOpenstatus = GetDoorState();
	int i = 0;
	for(i=0;i<gOptions.LockCount;i++)
	{
		pollcard_struct_variable.DoorLockOpenTimeOver[i].DoorLockState = (gStateOptions.DoorOpenstatus >> (i*8) & 0xff);
		if(DOORISOPEN == pollcard_struct_variable.DoorLockOpenTimeOver[i].DoorLockState)
		{
			pollcard_struct_variable.DoorLockOpenTimeOver[i].Dooropentimelimit = gOptions.DoorDetectortime[i];
		}
	}

//add by yuanfat
#ifdef PUSHSDK
	printf("push init start\n");
	InitPush();
	printf("push init over\n");
#endif


//	memcpy(shared_stateoptions_stuff,&gStateOptions,sizeof(TStateOptions));
	printf("DoorOpenstatus :0x%08x \n",gStateOptions.DoorOpenstatus);
	ProcessSaveAcessEvenLog(0,0,VS_OTHER,0,	EVENT_MACHINE_START,2,OldEncodeTime(&gCurTime));//启动事件
   	RegMsgProc(MainProcMsg);
   	DoMsgProcess(NULL, News_Exit_Program);
	free(CurAlarmRec);
	free(CurAlarmRec2);
	//鍏抽棴RS232
//	ff232.free();
	ttl232.free();

	FDB_AddOPLog("power off!\n",11);
	FDB_FreeDBs();

	ExCloseRF();
	ExCloseWiegand();

	GPIO_IO_Free();

//add by yuanfat
#ifdef PUSHSDK
	EndPush();
#endif

	return 0;
}

void EnableDevice(int Enabled)
{
	static int DeviceEnabled=TRUE;

	if(DeviceEnabled!=Enabled) //纭繚鎴愬鎵цEnable/Disable
	{
		//{
			EnableMsgType(MSG_TYPE_FINGER, Enabled);
			EnableMsgType(MSG_TYPE_MF, Enabled);
			EnableMsgType(MSG_TYPE_HID, Enabled);
		//}
		EnableMsgType(MSG_TYPE_BUTTON, Enabled);
		DeviceEnabled=Enabled;
	}

}


int MainProcMsg(PMsg msg)
{
	int cardno,readerno,keyvalue,ioindex,iostate,mcuID;
	int message=msg->Message;

	//printf("msg =0x%x ",message);
	msg->Message = 0;

	cardno = readerno = keyvalue = ioindex = iostate = 0;
	switch(message)
	{
		case MSG_TYPE_TIMER:
			On_DSTF();
#ifdef MACHINETYPE_C4
			if (WaitSleepCount)
			{
				if (!--WaitSleepCount)
				{
					if(CMD_OK == ExEnableLCDLight(0X00))//关LCD背光
					{
						;
					}
					else
					{
						WaitSleepCount = gOptions.IdleMinute * 60;  //关失败时，按MENU键仍为菜单功能
						printf("close LCDLight failed!\n");
					}
				}
			}
#endif
			break;
		case MSG_TYPE_BUTTON:
#ifdef MACHINETYPE_C4
			if (WaitSleepCount == 0)
			{
				if(CMD_OK == ExEnableLCDLight(0XFF)) //开LCD背光
				{
					printf("open LCDLight success!\n");
					WaitSleepCount = gOptions.IdleMinute * 60;

					break;
				}
				else
				{
					printf("open LCDLight failed!\n");

				}
			}

			WaitSleepCount = gOptions.IdleMinute * 60;

			keyvalue = msg->Param1;
			readerno = msg->Param2;
			ShowMainLCDDelay = 10;
			printf("open ShowMainLCDDelay  %d\n",ShowMainLCDDelay);
			On_Button(readerno,keyvalue);
			break;
#endif
	    case MSG_TYPE_HID:
			cardno  = msg->Param1;
			readerno = msg->Param2;
			printf("readerno=%d,cardno=%u\n",readerno,cardno);
			On_PollCard(cardno,readerno);
			break;
		case MSG_TYPE_RELAY:
			 ioindex  = (msg->Param1) >> 8 & 0xff;
			 iostate = msg->Param1 & 0xff;
			mcuID = msg->Param2;
//			printf("readerno=%d,ioindex=%d,iostate=%u\n",readerno,ioindex,iostate);
			On_InputPinChange(mcuID,ioindex,iostate);
			break;
		  case  MSG_WIEGAND_KEY:
			keyvalue = msg->Param1;
			readerno = msg->Param2;
			printf("readerno= %d,keyvalue =%d \n ",readerno,keyvalue);
			On_Wiegandkey(keyvalue,readerno);
			break;
		  case  MSG_COMM_CMD:
		  {
			  switch (msg->Param1)
			  {
				  case SUBCMD_CONTROL_DOOR_STATUS:
				  {
					  PCommData shared_Comm_stuff = (PCommData)msg->Param2;
					  On_ControlDoorStatus(shared_Comm_stuff);
					  break;
				  }
				  case SUBCMD_ALARM_CANCEL:
				  {
					  PCommData shared_Comm_stuff = (PCommData)msg->Param2;
					  On_CancelAlarm(shared_Comm_stuff);
					  break;
				  }
				  case SUBCMD_REBOOT:
				  {
					  PCommData shared_Comm_stuff = (PCommData)msg->Param2;
					  On_Reboot(shared_Comm_stuff);
					  break;
				  }
				  case START_DOOR_KEEP_OPEN:
				  {
					  PCommData shared_Comm_stuff = (PCommData)msg->Param2;
					  On_StartDoorKeepOpen(shared_Comm_stuff);
					  break;
				  }
				  case 5: //test
				  {
					  PCommData shared_Comm_stuff = (PCommData)msg->Param2;
					  char sysCtlBuf[128] = {0};
					  memcpy(sysCtlBuf,shared_Comm_stuff->SendData+4,shared_Comm_stuff->SendLen-4);
					  printf("%s\n",sysCtlBuf);
					  sprintf(sysCtlBuf,"%s\n",sysCtlBuf);

					  system(sysCtlBuf);
					  shared_Comm_stuff->CommCmdRet = 0;
					  break;
				  }
				  case SUBCMD_BACKUP_DATA://load options
				  {
					  int RetBackupData = 0;
					  RetBackupData = BackupData(MANUAL);
					  if(RetBackupData < 0)
					  {
						  shared_Comm_stuff->NewsType = -1;
					  }
					  else
					  {
						  shared_Comm_stuff->CommCmdRet = 0;
					  }
					  break;
				  }
				  case SUBCMD_UPDATE_OPTIONS:
				  {
					  PCommData shared_Comm_stuff = (PCommData)msg->Param2;
					  On_MainUpdate(shared_Comm_stuff);

					  //printf("------------------------ main pwd is : %s \n",gOptions.ComPwd);
					  break;
				  }
				  case SUBCMD_LOAD_BAKDATA:
				  {
					  //PCommData shared_Comm_stuff = (PCommData)msg->Param2;
					  BakTableLen = shared_Comm_stuff->SendLen;
					  memcpy(BakTableData,shared_Comm_stuff->SendData,BakTableLen);
					  SaveTableDataToFile(FCT_C3GUARDEREVENTLOG_BAK,BakTableData,BakTableLen);
					  //GetBKData();
					  shared_Comm_stuff->CommCmdRet = 0;

					  break;
				  }
				  default:
					  break;

			  }

			  break;
		  }
	}

	return 1;
}


//刷新锁的常开内容
void RefleshLockKeepOpen(int i)
{
	int Curtime_min,CurDay;

	 Curtime_min =  gCurTime.tm_hour * 100 + gCurTime.tm_min;
	 CurDay = (gCurTime.tm_year+1900)*10000 + (gCurTime.tm_mon+1)*100 + gCurTime.tm_mday;


	if(i >= 4)
	{
		return;
	}

	//如果当天是已经取消的常开功能的直接退出
	if(gOptions.CancelKeepOpenDay[i] == CurDay)
	{
		return;
	}

	//如果门已经常开了(首卡开门或门常开)，有效期失效判断	//在门非激活时间段内
	if(pollcard_struct_variable.c3doorkeepopen[i].keepOpenSign)
	{
		if(Curtime_min < pollcard_struct_variable.c3doorkeepopen[i].starttime
		||Curtime_min > pollcard_struct_variable.c3doorkeepopen[i].endtime
		|| !process_door_valid_time(gOptions.DoorPollCardTimeZoneOfValidity[i],gCurTime))
		{
			printf("firstcard doorkeepopen timeout doorid= %d,	starttime = %d, endtime =%d curtime = %d\n ",
			i+1,pollcard_struct_variable.c3doorkeepopen[i].starttime,
			pollcard_struct_variable.c3doorkeepopen[i].endtime,
			Curtime_min);

			pollcard_struct_variable.c3doorkeepopen[i].starttime = 0;
			pollcard_struct_variable.c3doorkeepopen[i].endtime  = 0;

			if(0 == CommKeepOpenSign[i])
			{
				Ex_ConctrolLock(FIRSTCARDCAUSE,gOptions.MachineType,i+1,1);//开启电锁1S后，后失效
				pollcard_struct_variable.c3doorkeepopen[i].keepOpenSign = 0;
				ProcessSaveAcessEvenLog(0,0,VS_OTHER,i+1,EVENT_DOORKEEPOPEN_END,2,OldEncodeTime(&gCurTime));
			}
			else
			{
				if(Curtime_min == pollcard_struct_variable.c3doorkeepopen[i].endtime)
				{
					ProcessSaveAcessEvenLog(0,0,VS_OTHER,i+1,EVENT_DOORKEEPOPEN_END,2,OldEncodeTime(&gCurTime));
				}
			}
		}
		if(0 == CommKeepOpenSign[i] && 0 == gOptions.DoorKeepOpenTimeZone[i]
				  && 1 == pollcard_struct_variable.c3doorkeepopen[i].keepOpenSign)//参数已经修改，不存在常开时间段了，需要关闭门
		{
			Ex_ConctrolLock(FIRSTCARDCAUSE,gOptions.MachineType,i+1,0);
			pollcard_struct_variable.c3doorkeepopen[i].keepOpenSign = 0;
		}
	}

	//在门激活时间段内
	if(!process_door_valid_time(gOptions.DoorPollCardTimeZoneOfValidity[i],gCurTime))
	{
		return ;
	}

	if(gOptions.DoorKeepOpenTimeZone[i])//如果存在门常开时段区
	{
		TC3Timeperiod Timeperiod;
		//判断当前时段是否为常开时段,如果是，给变量赋值
		if(FDB_GetTimezone(gOptions.DoorKeepOpenTimeZone[i], gCurTime,&Timeperiod))
		{
			printf("doorid= %d,cur hourMIn = %d, three Timeperiod: %d--%d,%d--%d,%d--%d\n ",i+1,Curtime_min,(Timeperiod.time1 >>16 & 0xffff) ,(Timeperiod.time1 & 0xffff),
													(Timeperiod.time2 >>16 & 0xffff) ,(Timeperiod.time2 & 0xffff),
													(Timeperiod.time3 >>16 & 0xffff) ,(Timeperiod.time3 & 0xffff));
			if(((Timeperiod.time1 & 0xffff) - (Timeperiod.time1 >>16 & 0xffff)) > 0
					&& Curtime_min >= (Timeperiod.time1 >>16 & 0xffff)
				&&  Curtime_min <= (Timeperiod.time1 & 0xffff))
			{
				if(0 == pollcard_struct_variable.c3doorkeepopen[i].keepOpenSign)
				{
					ProcessSaveAcessEvenLog(0,0,VS_OTHER,i+1,EVENT_DOORKEEPOPENACTIVED_OK,2,OldEncodeTime(&gCurTime));
				}
				pollcard_struct_variable.c3doorkeepopen[i].starttime = (Timeperiod.time1 >>16 & 0xffff);
				pollcard_struct_variable.c3doorkeepopen[i].endtime  = (Timeperiod.time1 & 0xffff);
				pollcard_struct_variable.c3doorkeepopen[i].keepOpenSign = 1;//有效启用
				pollcard_struct_variable.c3doorkeepopen[i].keepopenreason = DOORSETKEEPOPEN;
				Ex_ConctrolLock(DOORKEEPOPEN,gOptions.MachineType,i+1,255);//开启电锁常开
				//ProcessSaveAcessEvenLog(0,0,VS_OTHER,i+1,EVENT_DOORKEEPOPENACTIVED_OK,2,OldEncodeTime(&gCurTime));
			}
			else if(((Timeperiod.time2 & 0xffff) - (Timeperiod.time2 >>16 & 0xffff)) > 0
					 && Curtime_min >= (Timeperiod.time2 >>16 & 0xffff)
				&&  Curtime_min <= (Timeperiod.time2 & 0xffff))
			{
				if(0 == pollcard_struct_variable.c3doorkeepopen[i].keepOpenSign)
				{
					ProcessSaveAcessEvenLog(0,0,VS_OTHER,i+1,EVENT_DOORKEEPOPENACTIVED_OK,2,OldEncodeTime(&gCurTime));
				}
				pollcard_struct_variable.c3doorkeepopen[i].starttime = (Timeperiod.time2 >>16 & 0xffff);
				pollcard_struct_variable.c3doorkeepopen[i].endtime  = (Timeperiod.time2 & 0xffff);
				pollcard_struct_variable.c3doorkeepopen[i].keepOpenSign = 1;//有效启用
				pollcard_struct_variable.c3doorkeepopen[i].keepopenreason = DOORSETKEEPOPEN;
				Ex_ConctrolLock(DOORKEEPOPEN,gOptions.MachineType,i+1,255);//开启电锁常开
				//ProcessSaveAcessEvenLog(0,0,VS_OTHER,i+1,EVENT_DOORKEEPOPENACTIVED_OK,2,OldEncodeTime(&gCurTime));
			}
			else if(((Timeperiod.time3 & 0xffff) - (Timeperiod.time3 >>16 & 0xffff)) > 0
					 && Curtime_min >= (Timeperiod.time3 >>16 & 0xffff)
				&&  Curtime_min <= (Timeperiod.time3 & 0xffff))
			{
				if(0 == pollcard_struct_variable.c3doorkeepopen[i].keepOpenSign)
				{
					ProcessSaveAcessEvenLog(0,0,VS_OTHER,i+1,EVENT_DOORKEEPOPENACTIVED_OK,2,OldEncodeTime(&gCurTime));
				}
				pollcard_struct_variable.c3doorkeepopen[i].starttime = (Timeperiod.time3 >>16 & 0xffff);
				pollcard_struct_variable.c3doorkeepopen[i].endtime  = (Timeperiod.time3 & 0xffff);
				pollcard_struct_variable.c3doorkeepopen[i].keepOpenSign = 1;//有效启用
				pollcard_struct_variable.c3doorkeepopen[i].keepopenreason = DOORSETKEEPOPEN;
				Ex_ConctrolLock(DOORKEEPOPEN,gOptions.MachineType,i+1,255);//开启电锁常开

			}
		}
	}

}

void GetDoorAlarmState(int doorState)
{
	BYTE OneDoorState;
	int doorId = 0;
	for(doorId = 1;doorId <= 4;doorId++)
	{
		OneDoorState = (doorState >> ((doorId-1) * 8) ) & 0xff;
		if(0 == OneDoorState)
		{
			gStateOptions.DoorAlarmStatus &= (~(2 << (doorId-1)*8));
		}
	}
}

unsigned int Timer1Count,Timer2Count;

// 定时事件
void OnTimer(unsigned char cID)
{
	int i;
	switch (cID)
	{
		case 0:									// 秒逻辑
		{

			Timer1Count++;
			//每5秒钟喂狗
			if(Timer1Count%5 == 0)
			{
				if(gOptions.WatchDog)
				{
					ExMcuDog(MCU1,60);
				}
			}
			//comm进程启动比main进程慢，comm进程初始化共享内存时，会清空门状态的共享内存，所以在10秒的时候要拷贝过去
			if(Timer1Count == 10)
			{
				 memcpy(shared_stateoptions_stuff,&gStateOptions,sizeof(TStateOptions));
				 printf("DoorAlarmStatus:  0x%08x\n",gStateOptions.DoorAlarmStatus);

			}
			//每10秒取门状态，options参数改变时要更新门状态
			if(Timer1Count%10 == 7)
			{
				int DoorOpenstatus =  GetDoorState();//表示门的开关状态，

				 if(gStateOptions.DoorOpenstatus != DoorOpenstatus)
				 {
					 gStateOptions.DoorOpenstatus = DoorOpenstatus;
					 GetDoorAlarmState(DoorOpenstatus);
					 memcpy(shared_stateoptions_stuff,&gStateOptions,sizeof(TStateOptions));
					 printf("DoorAlarmStatus:  0x%08x\n",gStateOptions.DoorAlarmStatus);
				 }
			}
			//按键时间等待刷新
			if(gOptions.MachineType == C4 || gOptions.MachineType == C4_200 || gOptions.MachineType == C4_400To_200)
			{
				if (ShowMainLCDDelay)
				{
					if (!--ShowMainLCDDelay)
					{
						ShowMainLCD();
					}
					//printf("ShowMainLCDDelay---%d\n",ShowMainLCDDelay);
				}
			}


			//打印信息
			if(Timer1Count%30 == 0)
			{
				DBPRINTF("Timer0 = %d,\n",Timer1Count);
			}

			//初始化时间计时
			if(pollcard_struct_variable.machineintit.change_times)
			{
				pollcard_struct_variable.machineintit.change_times--;
				if(pollcard_struct_variable.machineintit.change_times == 0)
					pollcard_struct_variable.machineintit.change_count = 0;
			}

			//	//4个门的事件轮询
			for(i = 0;i < 4;i++)
			{
				if(pollcard_struct_variable.time_count[i])//刷卡间隔计时使用
					pollcard_struct_variable.time_count[i]--;

				//连续刷卡5次取消门常开功能
				if(pollcard_struct_variable.CancelDoorKeepOpen[i].PollTime)
				{
					pollcard_struct_variable.CancelDoorKeepOpen[i].PollTime--;
					if(pollcard_struct_variable.CancelDoorKeepOpen[i].PollTime == 0)
					{
						pollcard_struct_variable.CancelDoorKeepOpen[i].PIN = 0;
						pollcard_struct_variable.CancelDoorKeepOpen[i].PollCount = 0;
						pollcard_struct_variable.CancelDoorKeepOpen[i].PollTime = 0;
					}
				}
                //仅卡模式下胁迫密码等待刷卡计时
				if(forcepwdtimeout[i])
				{
					forcepwdtimeout[i]--;
				}
				//按esc键等待密码输入计时
				if(pollcard_struct_variable.c3doorpassword[i].superpwdtimeout)
				{
					pollcard_struct_variable.c3doorpassword[i].superpwdtimeout--;
				}
				if(pollcard_struct_variable.c3doorpassword[i].keybuftimeout)//按键间隔计时
				{
				    pollcard_struct_variable.c3doorpassword[i].keybuftimeout--;
				}
				else
				{
					pollcard_struct_variable.c3doorpassword[i].keybufindex = 0;//计时到，清0
					memset(&pollcard_struct_variable.c3doorpassword[i].keybuf,0x00,10);


					//卡+ 密码双重验证的内容也清0
					memset(&pollcard_struct_variable.c3cardandpasswordverify[i],0x00,sizeof(TC3Cardandpassword));
				}

				//清除多卡开门组合数据
				if(pollcard_struct_variable.c3MultiCardOpendoor[i].MultiCardTimeCount)
				{
					pollcard_struct_variable.c3MultiCardOpendoor[i].MultiCardTimeCount--;
					if(pollcard_struct_variable.c3MultiCardOpendoor[i].MultiCardTimeCount == 0)
					{
						//EX_ConctrolReaderLedorBeep(gOptions.MachineType,i+1,READEROKLED,0);//10ms为单位
						EX_ConctrolReaderLedorBeep(gOptions.MachineType,i+1,READERBEEP,100);//10ms为单位
						memset(&pollcard_struct_variable.c3MultiCardOpendoor[i],0x00,sizeof(TC3Multicardopendoor));
					}
				}

				//门打开超时判断,
				if(pollcard_struct_variable.DoorLockOpenTimeOver[i].DoorLockState == DOORISOPEN
						&& !pollcard_struct_variable.c3doorkeepopen[i].keepOpenSign)
				{
					pollcard_struct_variable.DoorLockOpenTimeOver[i].Dooropentime++;
					if(pollcard_struct_variable.DoorLockOpenTimeOver[i].Dooropentimelimit != 0
						&& pollcard_struct_variable.DoorLockOpenTimeOver[i].Dooropentime
						== pollcard_struct_variable.DoorLockOpenTimeOver[i].Dooropentimelimit)
					{
						ProcessSaveAcessEvenLog(0,0,VS_OTHER,i+1,EVENT_DOORCONTACTOVERTIME,2,OldEncodeTime(&gCurTime));
						//memcpy(&(shared_option_stuff->Options),&gOptions,sizeof(TOptions));
						//shared_option_stuff->main_change_mark = 1;
						//printf("main have changed goptions---DoorOpenTimeOver\n");
						printf("dooropentimeover: doorid = %d, limittime =  %d\n",i+1,pollcard_struct_variable.DoorLockOpenTimeOver[i].Dooropentimelimit);
					}
				}
				//门打开原因清0,非门常开时间段
				if(!pollcard_struct_variable.c3doorkeepopen[i].keepOpenSign)
				{
					if(pollcard_struct_variable.DoorLockOpenTimeOver[i].LockOpenTime)
					{
						pollcard_struct_variable.DoorLockOpenTimeOver[i].LockOpenTime--;
					}
					else if(pollcard_struct_variable.DoorLockOpenTimeOver[i].DoorLockState == DOORISCLOSE)
					{
						pollcard_struct_variable.DoorOpenCause[i] = 0;
					}
				}
			}

		}
		break;
		case 1:				//分钟逻辑
		{
			//memcpy(shared_stateoptions_stuff,&gStateOptions,sizeof(TStateOptions));
			Timer2Count++;
			DBPRINTF("Timer1 = %d,\n",Timer2Count);
#ifdef MACHINETYPE_C4
			if(!ShowMainLCDDelay)
			{
			     ShowMainLCD();
			}
#endif
			int Curtime_min,i;
			 Curtime_min =  gCurTime.tm_hour * 100 + gCurTime.tm_min;

			if(gOptions.BackupTime == gCurTime.tm_hour && 0 == gCurTime.tm_min)// SD Card
			{
				int RetBackupData = 0;
				RetBackupData = BackupData(AUTOMATIC);
				if(0 != RetBackupData)
				{
					printf("Backup transaction.dat to SD failed! failed code: %d\n", RetBackupData);
				}
				else
				{
					printf("Backup transaction.dat to SD success!\n");
				}
			}
			//每30分钟同步rtc时间到cpu
			 if(Curtime_min%30 == 0)
			 {
				 RTCTimeValidSign=ReadRTCClockToSyncSys(&gCurTime);
			 }

			 for(i = 0;i<4;i++)//刷新各个门的常开属性
			 {
				 RefleshLockKeepOpen(i);
			 }
		}
		break;
		case 2:	         //秒逻辑
		{

		}
		break;
		case 3:
		{
			;
		}
		break;
		default:
			break;
	}
}

//处理保存事件记录
int ProcessSaveAcessEvenLog(U32 cardno,U32 pin,BYTE verified,BYTE ReaderID,
	BYTE EventType,BYTE inoutstate,time_t time_second)
{
	//判断门的报警状态
	GetDoorAlarmStatus(EventType,ReaderID);
	//保存记录到数据为
	FDB_AddAcessEvenLog(cardno,pin,verified,ReaderID,EventType,inoutstate, time_second);
	//实时记录
	AppendRTLogBuff(cardno,pin,verified,ReaderID,EventType,inoutstate, time_second);

//add by yuanfat
#ifdef PUSHSDK
	TC3AcessLog accessLog = {0};
	memset(&accessLog, 0x00, sizeof(TC3AcessLog));
	accessLog.cardno = cardno;
	accessLog.pin = pin;
	accessLog.verified = verified;
	accessLog.doorID = ReaderID;
	accessLog.EventType = EventType;
	accessLog.inoutstate = inoutstate;
	accessLog.time_second = time_second;

	printf("cardno=%d,pin=%d,verified=%d,doorID=%d,EventType=%d,inoutstate=%d,time_second=%d\n",
			cardno, pin, verified, ReaderID, EventType, inoutstate, time_second);

	SendRTLog(&accessLog, sizeof(accessLog), NULL);
	printf("send SendRTLog over\n");
	CreateUploadData(&accessLog, sizeof(accessLog), "transaction.dat", 0);
#endif

	//联动控制判断
	LinkControl( EventType,ReaderID);

	return 1;
}

//报警状态
void GetDoorAlarmStatus(BYTE EventType,BYTE DoorID)
{
	if(EventType >= EVENT_ANTISTRIPALARM && EventType < EVENT_DOOROPEN)//目前三个报警事件
	{

		gStateOptions.DoorAlarmStatus |= 1 << (DoorID-1)*8;//表示门的报警状态，

		printf("DoorAlarmStatus: 0x%08x  ,doorid = %d\n",gStateOptions.DoorAlarmStatus,DoorID);
		memcpy(shared_stateoptions_stuff,&gStateOptions,sizeof(TStateOptions));
		printf("main have changed goptions---DoorAlarmStatus\n");
	}
	if(EventType == EVENT_DOORCONTACTOVERTIME)//门超时状态
	{
		gStateOptions.DoorAlarmStatus |= 2 << (DoorID-1)*8;
		memcpy(shared_stateoptions_stuff,&gStateOptions,sizeof(TStateOptions));
		printf("main have changed goptions---DoorAlarmStatus\n");

	}
}

// //实时记录  进buffer
void AppendRTLogBuff(int cardno,int pin,BYTE verified,BYTE ReaderID,BYTE EventType,BYTE inoutstate,U32 time_second)
{


	//将数据sahre_RTLog中 copy到内中
	//memcpy(&Oversee_RTLog,shared_RTLog_stuff,sizeof(TOverseeRTLog));

	shared_RTLog_stuff->RTLog[shared_RTLog_stuff->LogWriteIndex].cardno = cardno;
	shared_RTLog_stuff->RTLog[shared_RTLog_stuff->LogWriteIndex].pin = pin;
	shared_RTLog_stuff->RTLog[shared_RTLog_stuff->LogWriteIndex].verified = verified;
	shared_RTLog_stuff->RTLog[shared_RTLog_stuff->LogWriteIndex].EventType = EventType;
	shared_RTLog_stuff->RTLog[shared_RTLog_stuff->LogWriteIndex].doorID = ReaderID;
	shared_RTLog_stuff->RTLog[shared_RTLog_stuff->LogWriteIndex].inoutstate = inoutstate;
	shared_RTLog_stuff->RTLog[shared_RTLog_stuff->LogWriteIndex].time_second = time_second;

	shared_RTLog_stuff->LogWriteIndex++;
	if(shared_RTLog_stuff->LogWriteIndex >= MAXRTLOGCOUNT)
	{
		shared_RTLog_stuff->LogWriteIndex = 0;
	}
	//将数据copy到sahre_RTLog中
	//memcpy(shared_RTLog_stuff,&Oversee_RTLog,sizeof(TOverseeRTLog));
}

void Ex_ConctrolLock(int opencause,int MachineType,int index,int time)
{
	if(time > 0)//开门
	{
		pollcard_struct_variable.DoorOpenCause[index-1]  = opencause;//门是正常打开的
	}
	else
	{
		pollcard_struct_variable.DoorOpenCause[index-1] = 0;//关门
	}
	IIC_ConctrolLock(MachineType, index, time);
	pollcard_struct_variable.DoorLockOpenTimeOver[index-1].LockOpenTime = time;//开锁计时，到时就清除门打开原因
}


//创建备份数据的路径。按年、月创建文件夹。
int CreateCpPath(char *CpPtah)
{

	char CreateCpPath[100] = {0};
	char TmpCpPtah[100] = {0};
	char FileBuf[128] = {0};
	char Path[32] = {0};
	char *pFind = NULL;
	int ret = 0;
	int loop = 6;
	int RetSystem = 0;
	int LastDayofmonth = 0;
	int fd = -1;
	int len = 0;


	if(NULL == CpPtah)
	{
		ret = -1;
		return 0;
	}

	//LastDayofmonth = GetLastDayofmonth(gCurTime.tm_year+1900,gCurTime.tm_mon+1);

	sprintf(TmpCpPtah, "/mnt/sdcard/%4u", gCurTime.tm_year+1900);
	sprintf(CreateCpPath, "mkdir %s > VerifySD.txt 2>&1", TmpCpPtah);


	while(loop--)
	{
		RetSystem = system(CreateCpPath);
		if(0 == RetSystem)
		{
			ret = 0;
			break;
		}
		else
		{
			fd=open(GetEnvFilePath("USERDATAPATH", "VerifySD.txt",Path), O_RDWR|O_CREAT|O_SYNC, S_IRWXU|S_IRWXG|S_IRWXO);
			printf("open fd: %d  %s\n",fd, Path);

			if (fd == -1)
			{
				printf("open failed: %s\n", Path);
				ret = -2;
			}
			else
			{
				len = lseek(fd, 0, SEEK_END);

				if(len != 0)
				{
					lseek(fd, 0, SEEK_SET);
					if(read(fd, FileBuf, len)==len)
					{
						printf("read VerifySD.txt:  %s", FileBuf);
						pFind = strstr(FileBuf,"exists");
						if(pFind == NULL)
						{
							ret = -1;//没有创建成功
							close(fd);

						}
						else
						{
							ret = 0; //已经存在路径
							close(fd);
							break;
						}
					}
				}
				else
				{
					printf("VerifySD.txt len: %d\n",len);//创建成功
					ret = 0;
					close(fd);
					break;
				}
			}
		}
	}

	memset(TmpCpPtah, 0, sizeof(TmpCpPtah));
	sprintf(TmpCpPtah, "/mnt/sdcard/%4u/%02u",gCurTime.tm_year+1900, gCurTime.tm_mon+1);

	memset(CreateCpPath, 0, sizeof(CreateCpPath));
	sprintf(CreateCpPath, "mkdir %s > VerifySD.txt 2>&1", TmpCpPtah);
	printf("%s\n",CreateCpPath);
	loop = 6;
	while(loop--)
	{
		RetSystem = system(CreateCpPath);
		if(0 == RetSystem)
		{
			sprintf(CpPtah, "%s", TmpCpPtah);
			ret = 0;
			break;
		}
		else
		{
			fd=open(GetEnvFilePath("USERDATAPATH", "VerifySD.txt",Path), O_RDWR|O_CREAT|O_SYNC, S_IRWXU|S_IRWXG|S_IRWXO);
			printf("open fd: %d  %s\n",fd, Path);

			if (fd == -1)
			{
				printf("open failed: %s\n", Path);
				ret = -2;
			}
			else
			{
				len = lseek(fd, 0, SEEK_END);

				if(len != 0)
				{
					lseek(fd, 0, SEEK_SET);
					if(read(fd, FileBuf, len)==len)
					{
						printf("read VerifySD.txt:  %s", FileBuf);
						pFind = strstr(FileBuf,"exists");
						if(pFind == NULL)
						{
							ret = -1;//没有创建成功
							close(fd);

						}
						else
						{
							sprintf(CpPtah, "%s", TmpCpPtah);
							ret = 0; //已经存在路径
							close(fd);
							break;
						}
					}
				}
				else
				{
					sprintf(CpPtah, "%s", TmpCpPtah);
					printf("VerifySD.txt len: %d\n",len);//创建成功
					ret = 0;
					close(fd);
					break;
				}
			}
		}
	}

	return ret;
}

int BackupData(int cmd)
{
	int ret = 0;
	int RetCreateCpPath = 0;
	char BackupDataCmd[128] = {0};
	char BackupDataName[128] = {0};
	char CpPtah[100] = {0};
	int loop = 6;
	int RetSystem = 0;
	int RetOpenSDCard = 0;
	int RetGetBKData = 0;
	int RetDoUmountSDCard = 0;

	while(loop--)
	{
		RetOpenSDCard = OpenSDCard();
		if(0 == RetOpenSDCard)
		{
			RetCreateCpPath = CreateCpPath(CpPtah);
			if(0 != RetCreateCpPath)
			{
				return -4;
			}
            //printf("----CpPtah=%s\n\n\n\n",CpPtah);
			sprintf(BackupDataName, "%4u%02u%02u.t",gCurTime.tm_year+1900, gCurTime.tm_mon+1, gCurTime.tm_mday);
			sprintf(BackupDataCmd, "cp /mnt/ramdisk/BKtransaction.dat %s/%s", CpPtah ,BackupDataName);
			SaveTableDataToFile(FCT_C3GUARDEREVENTLOG_BAK,BakTableData,BakTableLen);
			RetGetBKData = GetBKData();
			RetSystem = system(BackupDataCmd);
			printf("RetGetBKData :%d;BackupData RetSystem: %d\n",RetGetBKData,RetSystem);
			if(0 == RetSystem)
			{
				ret = 0;
				FDB_ClearData(FCT_C3GUARDEREVENTLOG_BAK);
				if(cmd == AUTOMATIC)
				{
					SaveFdOffset(5, FCT_C3GUARDEREVENTLOG_OFFSET_BAK);
				}
				printf("%s success!\n",BackupDataCmd);
				break;
			}
			else
			{
				printf("%s failed!\n",BackupDataCmd);
				ret = -2;
			}
		}
		else
		{
			printf("RetOpenSDCard  failed\n");
			ret = -3;
		}
	}

	RetDoUmountSDCard = DoUmountSDCard();
	if(0 == RetDoUmountSDCard)
	{
		printf("DoUmountSDCard success\n");
	}
	else
	{
		ret = -4;
		printf("DoUmountSDCard failed\n");
	}

	return ret;
}

void ShowMainLCD(void)
{
	//printf("------- start ---  call ShowMainLCD \n");
	char tmp1[50] = {0}, tmp2[50] = {0}, buf[20] = {0};
	LCDBufferStart(LCD_BUFFER_ON);
	LCDClear();

	GetTimePixel(tmp1, tmp2, gCurTime.tm_hour,gCurTime.tm_min);
	ExEnableClock(0xff);
	LCDWriteStr(0,0,LoadStrByID(HID_WELCOME),0);
	LCDWriteStrLng(1, 0, tmp1, 0);
	LCDWriteStrLng(2, 0, tmp2, 0);
	sprintf(buf, "%02d-%02d-%02d", gCurTime.tm_year%100, gCurTime.tm_mon+1, gCurTime.tm_mday);
	LCDWriteStr(3, 0, buf,0);
	LCDWriteStr(3, (gLCDWidth/8)/2 +2, LoadStrByID(HID_DAY0+gCurTime.tm_wday), 0);
	LCDBufferStart(LCD_BUFFER_OFF);

	//printf("------- end ---  call ShowMainLCD \n");

}

TTime getMonthDayTime(int MonthDayTime)
{
	TTime tt =
	{ 0, 0, 0, 1, 1, 2004, 4, 0, 0 };
	tt.tm_year = gCurTime.tm_year;
	tt.tm_min = MonthDayTime & 0xFF;
	tt.tm_hour = MonthDayTime >> 8 & 0xFF;
	tt.tm_mday = MonthDayTime >> 16 & 0xFF;
	tt.tm_mon = (MonthDayTime >> 24 & 0xFF) - 1;
	return tt;
}

TTime getConstTime1(void)
{
	TTime tt =
	{ 0, 0, 0, 1, 1, 2004, 4, 0, 0 };
	tt.tm_year = gCurTime.tm_year;
	tt.tm_sec = 59;
	tt.tm_min = 59;
	tt.tm_hour = 23;
	tt.tm_mday = 31;
	tt.tm_mon = 11;//month(1=0, 2=1, 3=2...)
	return tt;
}

TTime getConstTime2(void)
{
	TTime tt =
	{ 0, 0, 0, 1, 1, 2004, 4, 0, 0 };
	tt.tm_year = gCurTime.tm_year;
	tt.tm_min = 0;
	tt.tm_hour = 0;
	tt.tm_mday = 1;
	tt.tm_mon = 0;//month(1=0, 2=1, 3=2...)
	return tt;
}

int IsDaylightSavingTime()
{
	TTime DaylightSavingTime, StandardTime, tempTime, tempTime2, tempTime3;
	int StartMonth = (gOptions.DaylightSavingTime >> 24 & 0xFF) - 1;
	int EndMonth = (gOptions.StandardTime >> 24 & 0xFF) - 1;
	int iResult = 0, iYear1 = 2004, iYear2 = 2004;

	//printf(" ------------- StartMonth: %d EndMonth: %d\n",StartMonth+1,EndMonth+1);
	if (gOptions.DLSTMode)
	{
		StartMonth = gOptions.StartWeekOfMonth[0] - 1;
		EndMonth = gOptions.EndWeekOfMonth[0] - 1;
		DaylightSavingTime = GetDateByWeek(gCurTime.tm_year,
				gOptions.StartWeekOfMonth[0] - 1, gOptions.StartWeekOfMonth[1],
				gOptions.StartWeekOfMonth[2], gOptions.StartWeekOfMonth[3],
				gOptions.StartWeekOfMonth[4]);

		StandardTime = GetDateByWeek(gCurTime.tm_year,
				gOptions.EndWeekOfMonth[0] - 1, gOptions.EndWeekOfMonth[1],
				gOptions.EndWeekOfMonth[2], gOptions.EndWeekOfMonth[3],
				gOptions.EndWeekOfMonth[4]);
	}
	else
	{
		DaylightSavingTime = getMonthDayTime(gOptions.DaylightSavingTime);
		StandardTime = getMonthDayTime(gOptions.StandardTime);
	}

	if (StartMonth > EndMonth)
	{
		tempTime2 = getConstTime1();
		tempTime3 = getConstTime2();
		if (TimeDiffSec(gCurTime, DaylightSavingTime) >= 0 && TimeDiffSec(
				gCurTime, tempTime2) <= 0)
		{
			iResult = 1;
		}
		else if (TimeDiffSec(gCurTime, tempTime3) >= 0 && TimeDiffSec(gCurTime,
				StandardTime) < 0)
		{
			iResult = 1;
		}
		if (iResult)
		{
			if (gOptions.CurTimeMode == STANDARDTIME)
			{
				GetTime(&tempTime);
				tempTime.tm_hour += 1;
				iYear1 = tempTime.tm_year;
				time_t tt = EncodeTime(&tempTime);

				DecodeTime(tt, &tempTime);
				iYear2 = tempTime.tm_year;
				if (iYear1 != iYear2)
				{
					StandardTime.tm_year += 1;
				}
				if (TimeDiffSec(tempTime, StandardTime) > 0 && TimeDiffSec(
						tempTime, StandardTime) <= 3600)
				{
					iResult = 0;
				}
			}
		}
	}
	else if (TimeDiffSec(gCurTime, DaylightSavingTime) >= 0 && TimeDiffSec(
			gCurTime, StandardTime) < 0)
	{
		//modify by dsl 2007.5.30
		iResult = 1;
		if (gOptions.CurTimeMode == STANDARDTIME)
		{
			GetTime(&tempTime);
			tempTime.tm_hour += 1;
			if (TimeDiffSec(tempTime, StandardTime) > 0 && TimeDiffSec(
					tempTime, StandardTime) <= 3600)
			{
				iResult = 0;
			}
		}
	}
	//DBPRINTF("dsl_iResult=%d\n", iResult);
	return iResult;
}


void On_DSTF()
{
	//printf("gOptions.CurTimeMode %d\n",gOptions.CurTimeMode);
	if (gOptions.DaylightSavingTimeFun && gOptions.DaylightSavingTimeOn)
	{
		if (gOptions.CurTimeMode == DAYLIGHTSAVINGTIME)
		{

			if (!IsDaylightSavingTime())
			{
				gCurTime.tm_hour -= 1;
				SetTime(&gCurTime);

				gOptions.CurTimeMode = STANDARDTIME;
				SaveOptions(&gOptions);
				//ReadRTCClockToSyncSys(&gCurTime);

#ifdef MACHINETYPE_C4
				ShowMainLCD();
#endif
			}
		}
		else if (gOptions.CurTimeMode == STANDARDTIME)
		{
			//printf("gOptions.CurTimeMode == STANDARDTIME\n");
			if (IsDaylightSavingTime())
			{
				//ReadRTCClockToSyncSys(&gCurTime);
				gCurTime.tm_hour += 1;

				SetTime(&gCurTime);

				gOptions.CurTimeMode = DAYLIGHTSAVINGTIME;
				SaveOptions(&gOptions);

				//ReadRTCClockToSyncSys(&gCurTime);
#ifdef MACHINETYPE_C4
				ShowMainLCD();
#endif
			}
		}
		else if (gOptions.CurTimeMode == 0)
		{
			if (IsDaylightSavingTime())
				gOptions.CurTimeMode = DAYLIGHTSAVINGTIME;
			else
				gOptions.CurTimeMode = STANDARDTIME;
			SaveOptions(&gOptions);
		}
		//DBPRINTF("CurTimeMode = %d\n", gOptions.CurTimeMode);
	}
}


