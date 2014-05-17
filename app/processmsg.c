//by ocscarchang fo c3/c4  2010-01-15
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "flashdb.h"
#include "options.h"
#include "processmsg.h"
#include "exfun.h"
#include "c4i2c.h"
#include "pollcard_logic.h"
#include "main.h"
#include "lcm.h"
#include "kb.h"
#include "timer.h"
#include "msg.h"
#include "lcdmenu.h"

//门控产品有可能有两组反潜，所以需要两组内存
extern PAlarmRec CurAlarmRec;//反潜回数组buff
extern PAlarmRec CurAlarmRec2;//反潜回数组buff
extern PLastTimeAPB CurLastTimeAPB[4];

extern U32 forcepwdtimeout[4];
extern int CommKeepOpenSign[4];



extern TTime gCurTime;
extern Tpollcard pollcard_struct_variable;//刷卡变量结构
extern PSharedStateOptions shared_stateoptions_stuff;
#ifdef MACHINETYPE_C4
int	On_Button(int readerno,int keyvalue)
{
	printf("On_Button readerno=%d,keyvalue=%d \n ",readerno,keyvalue);

	switch (keyvalue)
	{
		case  C4_MENU:
		{
			//LCD_ClearFullBuff();
			ExEnableClock(0);//关掉显示两个点
			DoMainMenu();
		}
		break;
		case  C4_OK:
		break;
		case  C4_ESC:

		break;
		case  C4_UP:
		break;
		case  C4_DOWN:
		break;
	}

	ShowMainLCD();
	return 0;
}
#endif

int On_UserAuthorizeInfo(PC3User user,int readerno)
{
	int DoorID = 0;
	int inoutstate = 0;
	if(gOptions.MachineType == C3_400 || gOptions.MachineType == C4 || gOptions.MachineType == C4_200)
	{
		DoorID = readerno;
	}
	else if(gOptions.MachineType == C3_200 || gOptions.MachineType == C3_100
			|| gOptions.MachineType == C4_400To_200 || gOptions.MachineType == C3_400To_200)
	{
	   DoorID = (readerno+1)/2;
	   inoutstate = (readerno+1)%2;
	}
	//当前时间 是否是门激活时间段
	if(!process_door_valid_time(gOptions.DoorPollCardTimeZoneOfValidity[DoorID-1],gCurTime))
	{
		printf("process_door_valid_time error\n");
		ProcessSaveAcessEvenLog(user->cardno,user->PIN,gOptions.DoorOpenMode[DoorID-1],DoorID,EVENT_DOORSLEEP_ERROR,inoutstate,OldEncodeTime(&gCurTime));
		EX_ConctrolReaderLedorBeep(gOptions.MachineType,readerno,READERBEEP,100);//10ms为单位
		memset(&pollcard_struct_variable.c3MultiCardOpendoor[readerno-1],0x00,sizeof(TC3Multicardopendoor));

		return 0;
	}

	//有效日期，失效日期判断,用户有效期。
//	if(!process_card_valid_time(user->start_time,user->end_time,(gCurTime.tm_year+1900)*10000+ (gCurTime.tm_mon+1) *100 +gCurTime.tm_mday))
	printf("lqw starttime=%d endtime=%d curtime=%d\n",user->start_time,user->end_time,((gCurTime.tm_year+1900)%100)*100000000+ (gCurTime.tm_mon+1)*1000000 +gCurTime.tm_mday*10000+gCurTime.tm_hour*100+gCurTime.tm_min);
	if(!process_card_valid_time(user->start_time,user->end_time,((gCurTime.tm_year+1900)%100)*100000000+ (gCurTime.tm_mon+1)*1000000 +gCurTime.tm_mday*10000+gCurTime.tm_hour*100+gCurTime.tm_min))
	{

		ProcessSaveAcessEvenLog(user->cardno,user->PIN,gOptions.DoorOpenMode[DoorID-1],DoorID,EVENT_CARD_NOTVALID_TIME,inoutstate,OldEncodeTime(&gCurTime));
		memset(&pollcard_struct_variable.c3MultiCardOpendoor[readerno-1],0x00,sizeof(TC3Multicardopendoor));

		EX_ConctrolReaderLedorBeep(gOptions.MachineType,readerno,READERBEEP,100);//10ms为单位

		return 0;
	}

	//寻找卡的门权限
	if(!FDB_GetDoorAuthInfo(user->PIN,DoorID))
	{
		ProcessSaveAcessEvenLog(user->cardno,user->PIN,gOptions.DoorOpenMode[DoorID-1],DoorID,EVENT_NORIGHT_ERROR,inoutstate,OldEncodeTime(&gCurTime));
		memset(&pollcard_struct_variable.c3MultiCardOpendoor[readerno-1],0x00,sizeof(TC3Multicardopendoor));
		EX_ConctrolReaderLedorBeep(gOptions.MachineType,readerno,READEROKLED,0);//10ms为单位
		EX_ConctrolReaderLedorBeep(gOptions.MachineType,readerno,READERBEEP,100);//10ms为单位

		return 0;
	}

	//找寻卡片的时间段及门区信息并判断门区, 时间段
	if(!FDB_GetcardauthorizeInfo2(user->PIN,DoorID,gCurTime))
	{
		ProcessSaveAcessEvenLog(0,user->PIN,gOptions.DoorOpenMode[DoorID-1],DoorID,EVENT_ILLEGALTIME_ERROR,inoutstate,OldEncodeTime(&gCurTime));
		EX_ConctrolReaderLedorBeep(gOptions.MachineType,readerno,READEROKLED,0);//10ms为单位
		EX_ConctrolReaderLedorBeep(gOptions.MachineType,readerno,READERBEEP,100);//10ms为单位
		memset(&pollcard_struct_variable.c3MultiCardOpendoor[readerno-1],0x00,sizeof(TC3Multicardopendoor));
		return 0;
	}

	return 1;
}


int	On_PollCard(cardno,readerno)
{
	int DoorID = 0;
	int inoutstate = 0;
	PC3User user = NULL;

	if(gOptions.MachineType == C3_400 || gOptions.MachineType == C4 || gOptions.MachineType == C4_200)
	{
		DoorID = readerno;
	}
	else if(gOptions.MachineType == C3_200 || gOptions.MachineType == C3_100
			|| gOptions.MachineType == C4_400To_200 || gOptions.MachineType == C3_400To_200)
	{
	   DoorID = (readerno+1)/2;
	   inoutstate = (readerno+1)%2;
	}

	if(gOptions.DoorOpenMode[DoorID-1] == VS_PW)
	{
		return 0;
	}

	user = &pollcard_struct_variable.c3userinfo[readerno-1];
	//清缓冲
	memset(&pollcard_struct_variable.c3userinfo[readerno-1],0x00,sizeof(TC3User));
	memset(&pollcard_struct_variable.c3doorpassword[readerno-1],0x00,sizeof(TC3Inputpassword));//
	memset(&pollcard_struct_variable.c3cardandpasswordverify[readerno-1],0x00,sizeof(TC3Cardandpassword));
	//刷卡间隔判断
	if(!process_interval(&pollcard_struct_variable.time_count[readerno-1],&gOptions.DoorIntertimepreset[DoorID-1]))
	{
		printf("process_interval error: too short\n");
		FDB_GetC3UserByCard(&cardno,user);
		ProcessSaveAcessEvenLog(cardno,user->PIN,VS_RF,DoorID,
			EVENT_INTERVAL_ERROR,inoutstate,OldEncodeTime(&gCurTime));
		//EX_ConctrolReaderLedorBeep(gOptions.MachineType,readerno,READEROKLED,0);//10ms为单位
		EX_ConctrolReaderLedorBeep(gOptions.MachineType,readerno,READERBEEP,100);//10ms为单位

		return 0;
	}
	//printf("start time %d\n",GetTickCount());
	//从用户表/卡号授权表中找卡信息
	if(!FDB_GetC3UserByCard(&cardno,user))
	{
		printf("FDB_GetC3UserByCard error\n");
		ProcessSaveAcessEvenLog(cardno,0,gOptions.DoorOpenMode[DoorID-1],DoorID,EVENT_ILLEGALCARD,inoutstate,OldEncodeTime(&gCurTime));
		//EX_ConctrolReaderLedorBeep(gOptions.MachineType,readerno,READEROKLED,0);//10ms为单位
		EX_ConctrolReaderLedorBeep(gOptions.MachineType,readerno,READERBEEP,100);//10ms为单位
		memset(&pollcard_struct_variable.c3MultiCardOpendoor[readerno-1],0x00,sizeof(TC3Multicardopendoor));

		return 0;
	}
	else
	{
		printf("no=%u,Pin=%d,pwd=%s,group=%d,start=%d,end=%d\n",cardno,user->PIN,
				user->password,user->group,user->start_time,user->end_time);
	}

	U32 superAuthorize = user->superAuthorize & (1<<(DoorID-1));

	if(superAuthorize != 0)//超级用户
	{
		EX_ConctrolReaderLedorBeep(gOptions.MachineType,readerno,READEROKLED,200);//10ms为单位
		if(0 == pollcard_struct_variable.c3doorkeepopen[DoorID-1].keepOpenSign)
		{
			Ex_ConctrolLock(POLLCARD,gOptions.MachineType,DoorID,gOptions.DoorDrivertime[DoorID-1]);//驱动电锁
		}
		ProcessSaveAcessEvenLog(cardno,user->PIN,gOptions.DoorOpenMode[DoorID-1],DoorID,
				EVENT_OK,inoutstate,OldEncodeTime(&gCurTime));
		return 1;
	}

	if(!On_UserAuthorizeInfo(user,readerno))//无权限
	{
		return 0;
	}

	if(gOptions.KeepOpenDayFunOn)
	{
	//合法卡连续刷5次取消当天门常开功能,合法时间段
		if(user->PIN ==	pollcard_struct_variable.CancelDoorKeepOpen[readerno-1].PIN)
		{//同一张卡
			pollcard_struct_variable.CancelDoorKeepOpen[readerno-1].PollCount ++;
			pollcard_struct_variable.CancelDoorKeepOpen[readerno-1].PollTime = 5;
			if(pollcard_struct_variable.CancelDoorKeepOpen[readerno-1].PollCount == 5)
			{
				int CurDay = (gCurTime.tm_year+1900)*10000 + (gCurTime.tm_mon+1)*100 + gCurTime.tm_mday;
				if(pollcard_struct_variable.c3doorkeepopen[DoorID-1].keepOpenSign
						|| gOptions.CancelKeepOpenDay[DoorID-1] != CurDay)
				{
					Ex_ConctrolLock(POLLCARD,gOptions.MachineType,DoorID,0);//关闭电锁
					//memset(&pollcard_struct_variable.c3doorkeepopen[readerno-1],0x00,sizeof(TC3doorkeepOpen));//
					gOptions.CancelKeepOpenDay[DoorID-1] = CurDay;
					memset(&pollcard_struct_variable.c3doorkeepopen[DoorID-1],0x00,sizeof(TC3doorkeepOpen));//
					printf("pollcard cancel doorkeepopen and SaveOptions!!!!!!!!!!!\n");
					SaveOptions(&gOptions);
					CommKeepOpenSign[DoorID-1] = 0;
					ProcessSaveAcessEvenLog(cardno,user->PIN,gOptions.DoorOpenMode[DoorID-1],DoorID,
							EVENT_CANCEL_DOORKEEPOPEN,inoutstate,OldEncodeTime(&gCurTime));
					pollcard_struct_variable.CancelDoorKeepOpen[readerno-1].PollCount = 0;
					return 0;
				}
				else
				{
					gOptions.CancelKeepOpenDay[DoorID-1] = 0;
					SaveOptions(&gOptions);
					memset(&pollcard_struct_variable.c3doorkeepopen[DoorID-1],0x00,sizeof(TC3doorkeepOpen));
					//memset(&pollcard_struct_variable.c3doorkeepopen[readerno-1],0x00,sizeof(TC3doorkeepOpen));
					ProcessSaveAcessEvenLog(cardno,user->PIN,gOptions.DoorOpenMode[DoorID-1],DoorID,
							EVENT_START_DOORKEEPOPEN,inoutstate,OldEncodeTime(&gCurTime));
					RefleshLockKeepOpen(DoorID-1);
					pollcard_struct_variable.CancelDoorKeepOpen[readerno-1].PollCount = 0;

					return 0;
				}
			}
		}
		else
		{
			pollcard_struct_variable.CancelDoorKeepOpen[readerno-1].PIN = user->PIN;
			pollcard_struct_variable.CancelDoorKeepOpen[readerno-1].PollCount = 1;
			pollcard_struct_variable.CancelDoorKeepOpen[readerno-1].PollTime = 5;
		}
	}
	//当前时段是否为门常开时段
	if(pollcard_struct_variable.c3doorkeepopen[DoorID-1].keepOpenSign)//说明门是常开的
	{
		printf("DoorKeepOpen process over!\n");
		if(!add_Rec_CurAlarmRec(CurAlarmRec,CurAlarmRec2,user->PIN,OldEncodeTime(&gCurTime), readerno,0))
		{
			printf("add_Rec_CurAlarmRec error\n");
			return 0;
		}
		ProcessSaveAcessEvenLog(cardno,user->PIN,gOptions.DoorOpenMode[DoorID-1],DoorID,
				EVENT_DOORKEEPOPEN_OK,inoutstate,OldEncodeTime(&gCurTime));

		return 1;
	}

	//是卡+密码模式，如果是卡+密码模式,需要作密码验�
	if(gOptions.DoorOpenMode[DoorID-1] == VS_PW_AND_RF)
	{
		pollcard_struct_variable.c3cardandpasswordverify[readerno-1].cardandpasswordsign = 1;
		pollcard_struct_variable.c3cardandpasswordverify[readerno-1].cardno = cardno;
		memcpy(pollcard_struct_variable.c3cardandpasswordverify[readerno-1].password,pollcard_struct_variable.c3userinfo[readerno-1].password,8);

		printf("card and pass mode: cardno = %d,pass =%s\n",pollcard_struct_variable.c3cardandpasswordverify[readerno-1].cardno,
			pollcard_struct_variable.c3cardandpasswordverify[readerno-1].password);
		//清密码输入缓冲区
		memset(&pollcard_struct_variable.c3doorpassword[readerno-1].keybuftimeout,0x00,sizeof(TC3Inputpassword));
		pollcard_struct_variable.c3doorpassword[readerno-1].keybuftimeout = 10;
		return 1;	//不开门，也不保存记录退出，等待按键验证
	}
	//多卡开门判断
	if(gOptions.DoorMultiCardOpenDoor[DoorID-1] != 0) //启用多卡开门功能
	{

        int ret = 0;
        ret = process_multicardopendoor(DoorID,user->PIN,user->group,&pollcard_struct_variable.c3MultiCardOpendoor[readerno-1]);
		if(0 == ret)
		{
			printf("process_multicardopendoor error\n");
			EX_ConctrolReaderLedorBeep(gOptions.MachineType,readerno,READEROKLED,100);//10ms为单位
			ProcessSaveAcessEvenLog(cardno,user->PIN,gOptions.DoorOpenMode[DoorID-1],DoorID,
					EVENT_MULTICARD_ERROR,inoutstate,OldEncodeTime(&gCurTime));


			return 0;
		}
		else if(-1 == ret)
		{
			printf("process_multicardopendoor error\n");
			memset(&pollcard_struct_variable.c3MultiCardOpendoor[readerno-1],0x00,sizeof(TC3Multicardopendoor));
			EX_ConctrolReaderLedorBeep(gOptions.MachineType,readerno,READEROKLED,0);//10ms为单位
			EX_ConctrolReaderLedorBeep(gOptions.MachineType,readerno,READERBEEP,100);//10ms为单位
			ProcessSaveAcessEvenLog(cardno,user->PIN,gOptions.DoorOpenMode[DoorID-1],DoorID,EVENT_MULTICARD_ERROR,0,OldEncodeTime(&gCurTime));

			return 0;
		}
		else
		{
			memset(&pollcard_struct_variable.c3MultiCardOpendoor[readerno-1],0x00,sizeof(TC3Multicardopendoor));
		}
	}


	//时间反潜
	if(-1 == ProcessAPBLastTime(CurLastTimeAPB[DoorID-1],user->PIN,gOptions.DoorLastTimeAPB[DoorID-1],OldEncodeTime(&gCurTime))
		|| -1 == ProcessAPBLastTime(CurLastTimeAPB[readerno-1],user->PIN,gOptions.ReaderLastTimeAPB[readerno-1],OldEncodeTime(&gCurTime)))
	{
		if(0 == superAuthorize)
		{
			EX_ConctrolReaderLedorBeep(gOptions.MachineType,readerno,READEROKLED,0);//10ms为单位
			EX_ConctrolReaderLedorBeep(gOptions.MachineType,readerno,READERBEEP,100);//10ms为单位
			ProcessSaveAcessEvenLog(cardno,user->PIN,gOptions.DoorOpenMode[DoorID-1],DoorID,
					EVENT_ANTIBACK_ERROR,inoutstate,OldEncodeTime(&gCurTime));
			return 0;
		}
	}

	//返潜回判断
	if(!process_card_AntiPassback(CurAlarmRec,CurAlarmRec2,user->PIN,readerno))
	{
		printf("process_card_AntiPassback error\n");
		if(0 == superAuthorize)
		{
			EX_ConctrolReaderLedorBeep(gOptions.MachineType,readerno,READEROKLED,0);//10ms为单位
			EX_ConctrolReaderLedorBeep(gOptions.MachineType,readerno,READERBEEP,100);//10ms为单位
			ProcessSaveAcessEvenLog(cardno,user->PIN,gOptions.DoorOpenMode[DoorID-1],DoorID,
					EVENT_ANTIBACK_ERROR,inoutstate,OldEncodeTime(&gCurTime));
			return 0;
		}
	}

	//双门互锁判断
	if(!process_mutiLock_linkage(DoorID))//将readerno转化为门ID
	{
		printf("process_mutiLock_linkage error\n");
		EX_ConctrolReaderLedorBeep(gOptions.MachineType,readerno,READEROKLED,0);//10ms为单位
		EX_ConctrolReaderLedorBeep(gOptions.MachineType,readerno,READERBEEP,100);//10ms为单位
		ProcessSaveAcessEvenLog(cardno,user->PIN,gOptions.DoorOpenMode[DoorID-1],DoorID,
				EVENT_MUTILOCKLINGAGE_ERROR,inoutstate,OldEncodeTime(&gCurTime));
		return 0;
	}

	//如果当天是已经取消的常开功能的直接退出 //首卡开门常开判断
	if(gOptions.CancelKeepOpenDay[DoorID-1] != (gCurTime.tm_year+1900)*10000 + (gCurTime.tm_mon+1)*100 + gCurTime.tm_mday)
	{
		if(process_firstcardopendoor(DoorID,user->PIN,gCurTime,&pollcard_struct_variable.c3doorkeepopen[DoorID-1]))
		{
			printf("process_firstcardopendoor OK !\n");
			if(pollcard_struct_variable.c3doorkeepopen[DoorID-1].keepOpenSign)
			{
				Ex_ConctrolLock(FIRSTCARDCAUSE,gOptions.MachineType,DoorID,255);//开启电锁常开
				//读头绿灯亮2S
				EX_ConctrolReaderLedorBeep(gOptions.MachineType,readerno,READEROKLED,200);//10ms为单位
				//保存cardno
				ProcessSaveAcessEvenLog(cardno,user->PIN,gOptions.DoorOpenMode[DoorID-1],DoorID,
						EVENT_FIRSTCARD_OK,inoutstate,OldEncodeTime(&gCurTime));
				return 1;
			}
		}
	}

	Ex_ConctrolLock(POLLCARD,gOptions.MachineType,DoorID,gOptions.DoorDrivertime[DoorID-1]);//驱动电锁

	//加记录到每张卡最后一条记录中，作反潜�

	/*if(!add_Rec_CurAlarmRec(CurAlarmRec,CurAlarmRec2,user->PIN,OldEncodeTime(&gCurTime), readerno,0))
	{
		printf("add_Rec_CurAlarmRec error\n");
		return 0;
	}*/

	if(forcepwdtimeout[readerno-1])
	{
		ProcessSaveAcessEvenLog(cardno,user->PIN,gOptions.DoorOpenMode[DoorID-1],DoorID,
				EVENT_FORCEALARM,inoutstate,OldEncodeTime(&gCurTime));
		forcepwdtimeout[readerno-1] = 0;
	}
	else if(gOptions.DoorMultiCardOpenDoor[DoorID-1] == 0) //如果没启用多卡开门功能,保存一般开门记录，否则是多卡开门记录
	{
		ProcessSaveAcessEvenLog(cardno,user->PIN,gOptions.DoorOpenMode[DoorID-1],DoorID,
				EVENT_OK,inoutstate,OldEncodeTime(&gCurTime));
		memset(&pollcard_struct_variable.c3MultiCardOpendoor[readerno-1],0x00,sizeof(TC3Multicardopendoor));
	}

	else
	{
		ProcessSaveAcessEvenLog(cardno,user->PIN,gOptions.DoorOpenMode[DoorID-1],DoorID,
				EVENT_MULTICARD_OK,inoutstate,OldEncodeTime(&gCurTime));
	}

	//读头绿灯亮2S
	EX_ConctrolReaderLedorBeep(gOptions.MachineType,readerno,READEROKLED,200);//10ms为单位

	return 1;
}

int	On_Wiegandkey(int keyvalue,int readerno)
{
	int DoorID = 0;
	int inoutstate = 0;
	PC3User user = NULL;
	if(gOptions.MachineType == C3_400 || gOptions.MachineType == C4 || gOptions.MachineType == C4_200)
	{
	   DoorID = readerno;
	}
	else if(gOptions.MachineType == C3_200 || gOptions.MachineType == C3_100
			|| gOptions.MachineType == C4_400To_200 || gOptions.MachineType == C3_400To_200)
	{
	   DoorID = (readerno+1)/2;
	   inoutstate = (readerno+1)%2;
	}
	user = &pollcard_struct_variable.c3userinfo[readerno-1];
	//接收输入密码
	if(keyvalue == 10)//10为esc 键
	{
		//清缓冲
		if((gOptions.DoorOpenMode[DoorID-1] == VS_PW_OR_RF || gOptions.DoorOpenMode[DoorID-1] == VS_PW))
		{
			pollcard_struct_variable.c3cardandpasswordverify[readerno-1].cardandpasswordsign = 0;
		}
		memset(&pollcard_struct_variable.c3doorpassword[readerno-1],0x00,sizeof(TC3Inputpassword));//
		pollcard_struct_variable.c3doorpassword[readerno-1].keybuftimeout = 10;
		if(pollcard_struct_variable.c3cardandpasswordverify[readerno-1].cardandpasswordsign != 1)
		{
			pollcard_struct_variable.c3doorpassword[readerno-1].superpwdtimeout = 10;
		}

	}
	else if(keyvalue == 11)//11为ent 键
	{
		int keybufindex;//密码位索引

		keybufindex = pollcard_struct_variable.c3doorpassword[readerno-1].keybufindex;//取上次索引位

		if(keybufindex == 0)//没有按数字键盘，直接按enter键
		{
			return 0;
		}
		/*if((gOptions.DoorOpenMode[DoorID-1] == VS_PW_OR_RF || gOptions.DoorOpenMode[DoorID-1] == VS_PW)
				&& pollcard_struct_variable.c3cardandpasswordverify[readerno-1].cardandpasswordsign == 0)
		{
			pollcard_struct_variable.c3userinfo[readerno-1].PIN = atoi(pollcard_struct_variable.c3doorpassword[readerno-1].keybuf);

			if(!FDB_GetC3UserByPin(pollcard_struct_variable.c3userinfo[readerno-1].PIN,&pollcard_struct_variable.c3userinfo[readerno-1]))
			{
				EX_ConctrolReaderLedorBeep(gOptions.MachineType,readerno,READERBEEP,100);//10ms为单位
				ProcessSaveAcessEvenLog(0,pollcard_struct_variable.c3userinfo[readerno-1].PIN,gOptions.DoorOpenMode[DoorID-1],DoorID,EVENT_NOUSER,inoutstate,OldEncodeTime(&gCurTime));
				memset(&pollcard_struct_variable.c3doorpassword[readerno-1],0x00,sizeof(TC3Inputpassword));
				return 0;
			}
			if(!On_UserAuthorizeInfo(&pollcard_struct_variable.c3userinfo[readerno-1],readerno))
			{
				memset(&pollcard_struct_variable.c3doorpassword[readerno-1],0x00,sizeof(TC3Inputpassword));
				return 0;
			}
			pollcard_struct_variable.c3cardandpasswordverify[readerno-1].cardandpasswordsign = 1;
			memcpy(pollcard_struct_variable.c3cardandpasswordverify[readerno-1].password,pollcard_struct_variable.c3userinfo[readerno-1].password,8);
			memset(&pollcard_struct_variable.c3doorpassword[readerno-1],0x00,sizeof(TC3Inputpassword));
			pollcard_struct_variable.c3doorpassword[readerno-1].keybuftimeout = 10;
			return 0;
		}*/
		printf("keybufindex = %d   keyvalue = %u\n",keybufindex,pollcard_struct_variable.c3doorpassword[readerno-1].keyvalue);
		if(gOptions.DoorOpenMode[DoorID-1] == VS_PW_OR_RF || gOptions.DoorOpenMode[DoorID-1] == VS_PW)
		{
			if(FDB_GetC3UserByPwd(pollcard_struct_variable.c3doorpassword[readerno-1].keybuf,&pollcard_struct_variable.c3userinfo[readerno-1]))
			{
				if(!On_UserAuthorizeInfo(&pollcard_struct_variable.c3userinfo[readerno-1],readerno))
				{
					memset(&pollcard_struct_variable.c3doorpassword[readerno-1],0x00,sizeof(TC3Inputpassword));
					return 0;
				}
				pollcard_struct_variable.c3cardandpasswordverify[readerno-1].cardandpasswordsign = 1;
				memcpy(pollcard_struct_variable.c3cardandpasswordverify[readerno-1].password,pollcard_struct_variable.c3userinfo[readerno-1].password,8);
			}

		}
		if(pollcard_struct_variable.c3cardandpasswordverify[readerno-1].cardandpasswordsign == 1)//个人密码且 需要双重验证，
		{
			int password_input;
			int password_person;
			BYTE ForcePwd_input[14]={0};
			BYTE DoorForcePwd[10]={0};
			memcpy(ForcePwd_input,&pollcard_struct_variable.c3doorpassword[readerno-1].keybuf, 12);
			pollcard_struct_variable.c3cardandpasswordverify[readerno-1].cardandpasswordsign = 0;
			if(DoorID == 1)
			{
				memcpy(DoorForcePwd, &gOptions.Door1ForcePassWord, 8);
			}
			else if(DoorID == 2)
			{
				memcpy(DoorForcePwd, &gOptions.Door2ForcePassWord, 8);
			}
			else if(DoorID == 3)
			{
				memcpy(DoorForcePwd, &gOptions.Door3ForcePassWord, 8);
			}
			else if(DoorID == 4)
			{
				memcpy(DoorForcePwd, &gOptions.Door4ForcePassWord, 8);
			}
			printf("cardandpasswordverify cardpassword = %s, && inputPassWord = %s,  \n",
					ForcePwd_input,	pollcard_struct_variable.c3cardandpasswordverify[readerno-1].password);

			if(strcmp(ForcePwd_input,DoorForcePwd) == 0)//协逼迫密码相同
			{
				printf("DoorForcePassWord OK!\n");
				Ex_ConctrolLock(POLLCARD,gOptions.MachineType,DoorID,gOptions.DoorDrivertime[DoorID-1]);//驱动电锁
				//保存事件记录
				ProcessSaveAcessEvenLog(pollcard_struct_variable.c3userinfo[readerno-1].cardno,
						pollcard_struct_variable.c3userinfo[readerno-1].PIN,gOptions.DoorOpenMode[DoorID-1],DoorID,
						EVENT_FORCEALARM,inoutstate,OldEncodeTime(&gCurTime));

			}
			else if(0 == strcmp(ForcePwd_input,pollcard_struct_variable.c3cardandpasswordverify[readerno-1].password))//个人密码比对
			{

				if(gOptions.DoorMultiCardOpenDoor[DoorID-1] != 0) //启用多卡开门功能
				{

			        int ret = 0;
			        ret = process_multicardopendoor(DoorID,pollcard_struct_variable.c3userinfo[readerno-1].PIN,
			    			pollcard_struct_variable.c3userinfo[readerno-1].group,&pollcard_struct_variable.c3MultiCardOpendoor[readerno-1]);
					if(0 == ret)
					{
						printf("process_multicardopendoor error\n");
						EX_ConctrolReaderLedorBeep(gOptions.MachineType,readerno,READEROKLED,100);//10ms为单位
						ProcessSaveAcessEvenLog(pollcard_struct_variable.c3userinfo[readerno-1].cardno,
								pollcard_struct_variable.c3userinfo[readerno-1].PIN,gOptions.DoorOpenMode[DoorID-1],DoorID,
								EVENT_MULTICARD_ERROR,inoutstate,OldEncodeTime(&gCurTime));
						return 0;
					}
					else if(-1 == ret)
					{
						printf("process_multicardopendoor error\n");
						memset(&pollcard_struct_variable.c3MultiCardOpendoor[readerno-1],0x00,sizeof(TC3Multicardopendoor));
						EX_ConctrolReaderLedorBeep(gOptions.MachineType,readerno,READEROKLED,0);//10ms为单位
						EX_ConctrolReaderLedorBeep(gOptions.MachineType,readerno,READERBEEP,100);//10ms为单位
						ProcessSaveAcessEvenLog(pollcard_struct_variable.c3userinfo[readerno-1].cardno,pollcard_struct_variable.c3userinfo[readerno-1].PIN,VS_PW_AND_RF,DoorID,EVENT_MULTICARD_ERROR,0,OldEncodeTime(&gCurTime));

						return 0;
					}
					else
					{
						memset(&pollcard_struct_variable.c3MultiCardOpendoor[readerno-1],0x00,sizeof(TC3Multicardopendoor));
					}
				}
				//时间反潜
				if(-1 == ProcessAPBLastTime(CurLastTimeAPB[DoorID-1],user->PIN,gOptions.DoorLastTimeAPB[DoorID-1],OldEncodeTime(&gCurTime))
					|| -1 == ProcessAPBLastTime(CurLastTimeAPB[readerno-1],user->PIN,gOptions.ReaderLastTimeAPB[readerno-1],OldEncodeTime(&gCurTime)))
				{
					EX_ConctrolReaderLedorBeep(gOptions.MachineType,readerno,READEROKLED,0);//10ms为单位
					EX_ConctrolReaderLedorBeep(gOptions.MachineType,readerno,READERBEEP,100);//10ms为单位
					ProcessSaveAcessEvenLog(user->cardno,user->PIN,gOptions.DoorOpenMode[DoorID-1],DoorID,
							EVENT_ANTIBACK_ERROR,inoutstate,OldEncodeTime(&gCurTime));
					return 0;

				}
				//返潜回判断
				if(!process_card_AntiPassback(CurAlarmRec,CurAlarmRec2,pollcard_struct_variable.c3userinfo[readerno-1].PIN,readerno))
				{
					if(pollcard_struct_variable.c3userinfo[readerno-1].superAuthorize & (1<<(readerno-1)) >= 0)
					{
						EX_ConctrolReaderLedorBeep(gOptions.MachineType,readerno,READEROKLED,0);//10ms为单位
						EX_ConctrolReaderLedorBeep(gOptions.MachineType,readerno,READERBEEP,100);//10ms为单位
						ProcessSaveAcessEvenLog(pollcard_struct_variable.c3userinfo[readerno-1].cardno,pollcard_struct_variable.c3userinfo[readerno-1].PIN
								,gOptions.DoorOpenMode[DoorID-1],DoorID,EVENT_ANTIBACK_ERROR,inoutstate,OldEncodeTime(&gCurTime));
						return 0;
					}
				}

				//双门互锁判断
				if(!process_mutiLock_linkage(DoorID))//将readerno转化为门ID
				{
					printf("process_mutiLock_linkage error\n");
					EX_ConctrolReaderLedorBeep(gOptions.MachineType,readerno,READEROKLED,0);//10ms为单位
					EX_ConctrolReaderLedorBeep(gOptions.MachineType,readerno,READERBEEP,100);//10ms为单位
					ProcessSaveAcessEvenLog(pollcard_struct_variable.c3userinfo[readerno-1].cardno,
							pollcard_struct_variable.c3userinfo[readerno-1].PIN,VS_PW_AND_RF,DoorID,
							EVENT_MUTILOCKLINGAGE_ERROR,inoutstate,OldEncodeTime(&gCurTime));
					return 0;
				}
				//如果当天是已经取消的常开功能的直接退出 //首卡开门常开判断
				if(gOptions.CancelKeepOpenDay[DoorID-1] != (gCurTime.tm_year+1900)*10000 + (gCurTime.tm_mon+1)*100 + gCurTime.tm_mday)
				{
					if(process_firstcardopendoor(DoorID,pollcard_struct_variable.c3userinfo[readerno-1].PIN,
						gCurTime,&pollcard_struct_variable.c3doorkeepopen[DoorID-1]))
					{
						printf("process_firstcardopendoor OK !\n");
						if(pollcard_struct_variable.c3doorkeepopen[DoorID-1].keepOpenSign)
						{
							Ex_ConctrolLock(FIRSTCARDCAUSE,gOptions.MachineType,DoorID,255);//开启电锁常开
							//读头绿灯亮2S
							EX_ConctrolReaderLedorBeep(gOptions.MachineType,readerno,READEROKLED,200);//10ms为单位
							//保存cardno
							ProcessSaveAcessEvenLog(pollcard_struct_variable.c3userinfo[readerno-1].cardno,
									pollcard_struct_variable.c3userinfo[readerno-1].PIN,gOptions.DoorOpenMode[DoorID-1],DoorID,
									EVENT_FIRSTCARD_OK,inoutstate,OldEncodeTime(&gCurTime));
							if(strcmp(ForcePwd_input,DoorForcePwd) == 0)//协逼迫密码相同
							{
								//保存事件记录
								ProcessSaveAcessEvenLog(pollcard_struct_variable.c3userinfo[readerno-1].cardno,
										pollcard_struct_variable.c3userinfo[readerno-1].PIN,VS_PW_AND_RF,DoorID,
										EVENT_FORCEALARM,inoutstate,OldEncodeTime(&gCurTime));
							}
							return 1;
						}
					}
				}
				//加记录到每张卡最后一条记录中，作反潜�
				if(!add_Rec_CurAlarmRec(CurAlarmRec,CurAlarmRec2,pollcard_struct_variable.c3userinfo[readerno-1].PIN,OldEncodeTime(&gCurTime),readerno,0))
				{
					printf("add_Rec_CurAlarmRec error\n");
					return 0;
				}
				//清缓冲
				memset(&pollcard_struct_variable.c3doorpassword[readerno-1],0x00,sizeof(TC3Inputpassword));//
				Ex_ConctrolLock(POLLCARD,gOptions.MachineType,DoorID,gOptions.DoorDrivertime[DoorID-1]);//驱动电锁
				EX_ConctrolReaderLedorBeep(gOptions.MachineType,readerno,READEROKLED,200);//10ms为单位
				if(gOptions.DoorMultiCardOpenDoor[DoorID-1] == 0) //如果没启用多卡开门功能,保存一般开门记录，否则是多卡开门记录
				{
					BYTE EventType = EVENT_OK;
					if(gOptions.DoorOpenMode[DoorID-1] == VS_PW_OR_RF || gOptions.DoorOpenMode[DoorID-1] == VS_PW)
					{
						EventType = ECENT_PW_OPENDOOR;
					}
					ProcessSaveAcessEvenLog(pollcard_struct_variable.c3userinfo[readerno-1].cardno,
							pollcard_struct_variable.c3userinfo[readerno-1].PIN,gOptions.DoorOpenMode[DoorID-1],DoorID,
							EventType,inoutstate,OldEncodeTime(&gCurTime));
					memset(&pollcard_struct_variable.c3MultiCardOpendoor[readerno-1],0x00,sizeof(TC3Multicardopendoor));
					return 1;
				}

				else
				{
					ProcessSaveAcessEvenLog(pollcard_struct_variable.c3userinfo[readerno-1].cardno,
							pollcard_struct_variable.c3userinfo[readerno-1].PIN,gOptions.DoorOpenMode[DoorID-1],DoorID,
							EVENT_MULTICARD_OK,inoutstate,OldEncodeTime(&gCurTime));
					return 1;
				}

			}
			else     //密码错误
			{
				ProcessSaveAcessEvenLog(pollcard_struct_variable.c3userinfo[readerno-1].cardno,
						pollcard_struct_variable.c3userinfo[readerno-1].PIN,gOptions.DoorOpenMode[DoorID-1],DoorID,
						EVENT_PASSWORD_ERROR,inoutstate,OldEncodeTime(&gCurTime));
				EX_ConctrolReaderLedorBeep(gOptions.MachineType,readerno,READERBEEP,100);//蜂鸣提示比对失败
			}
			//清缓冲
			memset(&pollcard_struct_variable.c3doorpassword[readerno-1],0x00,sizeof(TC3Inputpassword));//
		}
		else if(pollcard_struct_variable.c3doorpassword[readerno-1].superpwdtimeout)  //超级密码判断
		{
			BYTE password_input[14]={0};
			BYTE DoorForcePwd[10]={0};
			BYTE DoorSuperPwd[10]={0};

			memcpy(password_input,&pollcard_struct_variable.c3doorpassword[readerno-1].keybuf, 12);
			if(DoorID == 1)
			{
				memcpy(DoorForcePwd, &gOptions.Door1ForcePassWord, 8);
				memcpy(DoorSuperPwd,&gOptions.Door1SupperPassWord, 8);
			}
			else if(DoorID == 2)
			{
				memcpy(DoorForcePwd, &gOptions.Door2ForcePassWord, 8);
				memcpy(DoorSuperPwd,&gOptions.Door2SupperPassWord, 8);
			}
			else if(DoorID == 3)
			{
				memcpy(DoorForcePwd, &gOptions.Door3ForcePassWord, 8);
				memcpy(DoorSuperPwd,&gOptions.Door3SupperPassWord, 8);
			}
			else if(DoorID == 4)
			{
				memcpy(DoorForcePwd, &gOptions.Door4ForcePassWord, 8);
				memcpy(DoorSuperPwd,&gOptions.Door4SupperPassWord, 8);
			}

			printf("DoorSuperPassWord = %s, DoorForcePwd = %s, && inputPassWord = %s\n",
					DoorSuperPwd, DoorForcePwd, password_input);

			//清0，准备下一个输入
			memset(&pollcard_struct_variable.c3doorpassword[readerno-1].keybuftimeout,0x00,sizeof(TC3Inputpassword));

			if(strcmp(password_input,DoorForcePwd) == 0)//协逼迫密码相同
			{
				forcepwdtimeout[readerno-1] = 10;
				printf("--forcepwdtimeout %d = %d\n",DoorID,forcepwdtimeout[readerno-1]);
				ProcessSaveAcessEvenLog(pollcard_struct_variable.c3userinfo[readerno-1].cardno,
						pollcard_struct_variable.c3userinfo[readerno-1].PIN,VS_PW_AND_RF,DoorID,
						EVENT_FORCEALARM,inoutstate,OldEncodeTime(&gCurTime));
			}
			if(strcmp(password_input,DoorSuperPwd) == 0)//超级开门密码相同
			{
				printf("DoorSuperPassWord OK!\n");
				if(!process_mutiLock_linkage(DoorID))//将readerno转化为门ID---互锁
				{
					printf("process_mutiLock_linkage error\n");
					EX_ConctrolReaderLedorBeep(gOptions.MachineType,readerno,READERBEEP,100);//10ms为单位
					ProcessSaveAcessEvenLog(0,0,VS_PW,DoorID,EVENT_MUTILOCKLINGAGE_ERROR,inoutstate,OldEncodeTime(&gCurTime));
					pollcard_struct_variable.c3doorpassword[readerno-1].superpwdtimeout = 0;
					return 0;
				}
				//读头绿灯亮2S
				EX_ConctrolReaderLedorBeep(gOptions.MachineType,readerno,READEROKLED,200);//10ms为单位
				if(0 == pollcard_struct_variable.c3doorkeepopen[DoorID-1].keepOpenSign)
				{
					Ex_ConctrolLock(POLLCARD,gOptions.MachineType,DoorID,gOptions.DoorDrivertime[DoorID-1]);//驱动电锁
				}
				//保存事件记录
				ProcessSaveAcessEvenLog(0,0,VS_PW,DoorID,
						EVENT_SUPPERPASSWORD_OK,inoutstate,OldEncodeTime(&gCurTime));
			}
			if(strcmp(password_input,DoorForcePwd) != 0 && strcmp(password_input,DoorSuperPwd) != 0)
			{
				ProcessSaveAcessEvenLog(0,0,VS_PW,DoorID,
						EVENT_PASSWORD_ERROR,inoutstate,OldEncodeTime(&gCurTime));
				EX_ConctrolReaderLedorBeep(gOptions.MachineType,readerno,READERBEEP,100);//蜂鸣提示比对失败
			}
			pollcard_struct_variable.c3doorpassword[readerno-1].superpwdtimeout = 0;
		}
		if(gOptions.DoorOpenMode[DoorID-1] == VS_PW_OR_RF || gOptions.DoorOpenMode[DoorID-1] == VS_PW)
		{
			memset(&pollcard_struct_variable.c3doorpassword[readerno-1],0x00,sizeof(TC3Inputpassword));
		}
	}
	else if(keyvalue >=0 && keyvalue <=9)
	{
		int keybufindex;//密码位索引

		keybufindex = pollcard_struct_variable.c3doorpassword[readerno-1].keybufindex;//取上次索引位
		pollcard_struct_variable.c3doorpassword[readerno-1].keybuftimeout = 10;	//10秒
        if(pollcard_struct_variable.c3doorpassword[readerno-1].superpwdtimeout)
        {
        	pollcard_struct_variable.c3doorpassword[readerno-1].superpwdtimeout = 10;
        	printf("----------\n");
        }
		if(keybufindex > 8)//超过8位
		{
			printf("keybufinput error   :%d\n",keybufindex);
			return 0;
		}
		pollcard_struct_variable.c3doorpassword[readerno-1].keybuf[keybufindex]= keyvalue + '0';//
		pollcard_struct_variable.c3doorpassword[readerno-1].keyvalue = ( pollcard_struct_variable.c3doorpassword[readerno-1].keyvalue) * 10 +keyvalue;
		pollcard_struct_variable.c3doorpassword[readerno-1].keybufindex++ ;
		keybufindex = pollcard_struct_variable.c3doorpassword[readerno-1].keybufindex;//再取索引位

		printf("keybufindex = %d,inputpassword: %s\n",keybufindex,&pollcard_struct_variable.c3doorpassword[readerno-1].keybuf[0]);
	}
	return 1;
}



//输入io口变化事件处理s
int 	On_InputPinChange(int mcuID,int ioindex,int iostate)
{
	int DoorID;
	BYTE DoorState = 0;
	BYTE DoorOpenORCloseEven = 0;

	printf("On_InputPinChange  mcuID=%d,ioindex=%d,iostate=%d\n",mcuID,ioindex,iostate);

	if(gOptions.MachineType ==  C4 || gOptions.MachineType == C4_200)
	{
		DoorID = mcuID;
		if(ioindex == 1 && mcuID != 0)	//按键开门
		{
			if(0 != iostate)
			{
				if(!process_door_valid_time(gOptions.DoorPollCardTimeZoneOfValidity[DoorID-1],gCurTime))
				{
					ProcessSaveAcessEvenLog(0,0,VS_OTHER,DoorID,EVENT_BUTTONOPEN_ERROR,2,OldEncodeTime(&gCurTime));

					return 0;
				}
				if(!process_mutiLock_linkage(DoorID))//将readerno转化为门ID---互锁
				{
					printf("process_mutiLock_linkage error\n");
					//EX_ConctrolReaderLedorBeep(gOptions.MachineType,readerno,READERBEEP,100);//10ms为单位
					ProcessSaveAcessEvenLog(0,0,VS_OTHER,DoorID,EVENT_MUTILOCKLINGAGE_ERROR,2,OldEncodeTime(&gCurTime));

					return 0;
				}
				if(pollcard_struct_variable.c3doorkeepopen[DoorID-1].keepOpenSign == 0)
				{
					Ex_ConctrolLock(POLLCARD,gOptions.MachineType,DoorID,gOptions.DoorDrivertime[DoorID-1]);//驱动电锁
				}
				ProcessSaveAcessEvenLog(0,0,VS_OTHER,DoorID,EVENT_BUTTONOPENDOOR,2,OldEncodeTime(&gCurTime));
			}
		}
		else if(ioindex == 2 && mcuID != 0)//门磁
		{
			if(gOptions.DoorDetectortype[DoorID-1]==2)//门磁为常闭型，检测门的开关为相反
			{
				DoorOpenORCloseEven = (iostate)? EVENT_DOORCLOSE : EVENT_DOOROPEN;
			}
			else if(gOptions.DoorDetectortype[DoorID-1]==1)
			{
				DoorOpenORCloseEven = (iostate)? EVENT_DOOROPEN : EVENT_DOORCLOSE;
			}
			if(GetRelayState(DoorID) == 0 && DoorOpenORCloseEven == EVENT_DOOROPEN)//门意外打开
			{
				ProcessSaveAcessEvenLog(0,0,VS_OTHER,DoorID,
					EVENT_UNEXPECTED_DOOROPEN,2,OldEncodeTime(&gCurTime));
			}
			else if(DoorOpenORCloseEven)
			{
				if(DoorOpenORCloseEven == EVENT_DOORCLOSE && gStateOptions.DoorAlarmStatus) //清除门开超时状态
				{
					gStateOptions.DoorAlarmStatus &= ((0xfffffffd >> (32-(DoorID-1)*8)) | (0xfffffffd << (DoorID-1)*8));
					printf("DoorAlarmStatus: 0x%08x  ,doorid = %d\n",gStateOptions.DoorAlarmStatus,DoorID);
					memcpy(shared_stateoptions_stuff,&gStateOptions,sizeof(TStateOptions));

				}
				if(DoorOpenORCloseEven == EVENT_DOORCLOSE && gOptions.CloseDoorAndLock[DoorID-1]
					&& !pollcard_struct_variable.c3doorkeepopen[DoorID-1].keepOpenSign)  //闭门回锁
				{
					Ex_ConctrolLock(POLLCARD,gOptions.MachineType,DoorID,0);
				}
				ProcessSaveAcessEvenLog(0,0,VS_OTHER,DoorID,DoorOpenORCloseEven,2,OldEncodeTime(&gCurTime));
			}

			//0为无门状态，1为关，2为开
			if(gOptions.DoorDetectortype[DoorID-1]!=0)
			{
			    DoorState = GetOneDoorState(DoorID);

			    if(DoorState == DOORISOPEN)//门打开了，开始计时
				{
					pollcard_struct_variable.DoorLockOpenTimeOver[DoorID-1].DoorLockState = DoorState;
					pollcard_struct_variable.DoorLockOpenTimeOver[DoorID-1].Dooropentimelimit = gOptions.DoorDetectortime[DoorID-1];
					pollcard_struct_variable.DoorLockOpenTimeOver[DoorID-1].Dooropentime = 0;
				}
				else
				{
					pollcard_struct_variable.DoorLockOpenTimeOver[DoorID-1].DoorLockState = DoorState;
					pollcard_struct_variable.DoorLockOpenTimeOver[DoorID-1].Dooropentimelimit = 0;
					pollcard_struct_variable.DoorLockOpenTimeOver[DoorID-1].Dooropentime = 0;
				}
			}
		}
		else if(mcuID == 0)//扩展输入口电平变化   扩展in口在mcu0上
		{
			DoorID = ioindex+8;
//			EX_ConctrolAuxAlarm(ioindex/2,20);////4 in 转到 2out上
			ProcessSaveAcessEvenLog(0,0,VS_OTHER,DoorID,
				 ((iostate)? EVENT_AUXIN_CLOSE:EVENT_AUXIN_OPEN),2,OldEncodeTime(&gCurTime));    //高电平表示扩展输入口为闭合
		}
	}

	else if(gOptions.MachineType == C4_400To_200)
		{
		    DoorID = mcuID/2+1;
		    if(ioindex == 1 && mcuID!=0 && mcuID!=2 && mcuID!=4)// || ioindex == 2)	//按键开门
			{
		    	if(0 != iostate)
				{
					if(!process_door_valid_time(gOptions.DoorPollCardTimeZoneOfValidity[DoorID-1],gCurTime))
					{
						ProcessSaveAcessEvenLog(0,0,VS_OTHER,DoorID,EVENT_BUTTONOPEN_ERROR,2,OldEncodeTime(&gCurTime));

						return 0;
					}
					if(!process_mutiLock_linkage(DoorID))
					{
						printf("process_mutiLock_linkage error\n");
						//EX_ConctrolReaderLedorBeep(gOptions.MachineType,readerno,READERBEEP,100);//10ms为单位
						ProcessSaveAcessEvenLog(0,0,VS_OTHER,DoorID,EVENT_MUTILOCKLINGAGE_ERROR,2,OldEncodeTime(&gCurTime));

						return 0;
					}
					if(pollcard_struct_variable.c3doorkeepopen[DoorID-1].keepOpenSign == 0)
					{
						Ex_ConctrolLock(POLLCARD,gOptions.MachineType,DoorID,gOptions.DoorDrivertime[DoorID-1]);//驱动电锁
					}
					ProcessSaveAcessEvenLog(0,0,VS_OTHER,DoorID,EVENT_BUTTONOPENDOOR,2,OldEncodeTime(&gCurTime));
				}
			}
			else if(ioindex == 2 && mcuID!=4 && mcuID!=2 && mcuID!=0)// || ioindex == 4 )//门磁
			{

				if(gOptions.DoorDetectortype[DoorID-1]==2)//门磁为常闭型，检测门的开关为相反
				{
					DoorOpenORCloseEven = (iostate)? EVENT_DOORCLOSE : EVENT_DOOROPEN;
				}
				else if(gOptions.DoorDetectortype[DoorID-1]==1)
				{
					DoorOpenORCloseEven = (iostate)? EVENT_DOOROPEN : EVENT_DOORCLOSE;
				}
				if(GetRelayState(DoorID) == 0 && DoorOpenORCloseEven == EVENT_DOOROPEN)//门意外打开
				{
					ProcessSaveAcessEvenLog(0,0,VS_OTHER,DoorID,
						EVENT_UNEXPECTED_DOOROPEN,2,OldEncodeTime(&gCurTime));
				}
				else if(DoorOpenORCloseEven)
				{
					if(DoorOpenORCloseEven == EVENT_DOORCLOSE && gStateOptions.DoorAlarmStatus) //清除门开超时状态
					{
						gStateOptions.DoorAlarmStatus &= ((0xfffffffd >> (32-(DoorID-1)*8)) | (0xfffffffd << (DoorID-1)*8));
						printf("DoorAlarmStatus: 0x%08x  ,doorid = %d\n",gStateOptions.DoorAlarmStatus,DoorID);
						memcpy(shared_stateoptions_stuff,&gStateOptions,sizeof(TStateOptions));

					}
					if(DoorOpenORCloseEven == EVENT_DOORCLOSE && gOptions.CloseDoorAndLock[DoorID-1]
						&& !pollcard_struct_variable.c3doorkeepopen[DoorID-1].keepOpenSign)  //闭门回锁
					{
						Ex_ConctrolLock(POLLCARD,gOptions.MachineType,DoorID,0);
					}
					ProcessSaveAcessEvenLog(0,0,VS_OTHER,DoorID,DoorOpenORCloseEven,2,OldEncodeTime(&gCurTime));
				}

				//0为无门状态，1为关，2为开
				if(gOptions.DoorDetectortype[DoorID-1]!=0)
				{
					DoorState = GetOneDoorState(DoorID);

					if(DoorState == DOORISOPEN)//门打开了，开始计时
					{
						pollcard_struct_variable.DoorLockOpenTimeOver[DoorID-1].DoorLockState = DoorState;
						pollcard_struct_variable.DoorLockOpenTimeOver[DoorID-1].Dooropentimelimit = gOptions.DoorDetectortime[DoorID-1];
						pollcard_struct_variable.DoorLockOpenTimeOver[DoorID-1].Dooropentime = 0;
					}
					else
					{
						pollcard_struct_variable.DoorLockOpenTimeOver[DoorID-1].DoorLockState = DoorState;
						pollcard_struct_variable.DoorLockOpenTimeOver[DoorID-1].Dooropentimelimit = 0;
						pollcard_struct_variable.DoorLockOpenTimeOver[DoorID-1].Dooropentime = 0;
					}
				}
			}
			else if(mcuID == 0)//扩展输入口电平变化   扩展in口在mcu0上
			{
				DoorID = ioindex+8;
	//			EX_ConctrolAuxAlarm(ioindex/2,20);////4 in 转到 2out上
				ProcessSaveAcessEvenLog(0,0,VS_OTHER,DoorID,
					 ((iostate)? EVENT_AUXIN_CLOSE:EVENT_AUXIN_OPEN),2,OldEncodeTime(&gCurTime));
			}
		}

	else if(gOptions.MachineType ==  C3_200 || gOptions.MachineType ==  C3_100
				|| gOptions.MachineType == C3_400To_200 || gOptions.MachineType == C3_400)
	{
		if(ioindex == 1 || ioindex == 2)	//按键开门
		{
			if(gOptions.MachineType == C3_400)
			{
				DoorID = (mcuID -1)* 2 + ioindex;
			}
			else
			{
				DoorID = mcuID;
			}
	    	if(0 != iostate)
			{
				if(!process_door_valid_time(gOptions.DoorPollCardTimeZoneOfValidity[DoorID-1],gCurTime))
				{
					ProcessSaveAcessEvenLog(0,0,VS_OTHER,DoorID,EVENT_BUTTONOPEN_ERROR,2,OldEncodeTime(&gCurTime));

					return 0;
				}
				if(!process_mutiLock_linkage(DoorID))//将readerno转化为门ID---互锁
				{
					printf("process_mutiLock_linkage error\n");
					//EX_ConctrolReaderLedorBeep(gOptions.MachineType,readerno,READERBEEP,100);//10ms为单位
					ProcessSaveAcessEvenLog(0,0,VS_OTHER,DoorID,EVENT_MUTILOCKLINGAGE_ERROR,2,OldEncodeTime(&gCurTime));

					return 0;
				}
				if(pollcard_struct_variable.c3doorkeepopen[DoorID-1].keepOpenSign == 0)
				{
					Ex_ConctrolLock(POLLCARD,gOptions.MachineType,DoorID,gOptions.DoorDrivertime[DoorID-1]);//驱动电锁
				}
				ProcessSaveAcessEvenLog(0,0,VS_OTHER,DoorID,EVENT_BUTTONOPENDOOR,2,OldEncodeTime(&gCurTime));
			}
		}
		else if(ioindex == 3 || ioindex == 4 )//门磁
		{
			if(gOptions.MachineType == C3_400)
			{
				DoorID = (mcuID -1)* 2 + ioindex - 2;
			}
			else
			{
				DoorID = mcuID;
			}

			if(gOptions.DoorDetectortype[DoorID-1]==2)//门磁为常闭型，检测门的开关为相反
			{
				DoorOpenORCloseEven = (iostate)? EVENT_DOORCLOSE : EVENT_DOOROPEN;
			}
			else if(gOptions.DoorDetectortype[DoorID-1]==1)
			{
				DoorOpenORCloseEven = (iostate)? EVENT_DOOROPEN : EVENT_DOORCLOSE;
			}
			if(GetRelayState(DoorID) == 0 && DoorOpenORCloseEven == EVENT_DOOROPEN)//门意外打开
			{
				ProcessSaveAcessEvenLog(0,0,VS_OTHER,DoorID,
					EVENT_UNEXPECTED_DOOROPEN,2,OldEncodeTime(&gCurTime));
			}
			else if(DoorOpenORCloseEven)
			{
				if(DoorOpenORCloseEven == EVENT_DOORCLOSE && gStateOptions.DoorAlarmStatus) //清除门开超时状态
				{
					gStateOptions.DoorAlarmStatus &= ((0xfffffffd >> (32-(DoorID-1)*8)) | (0xfffffffd << (DoorID-1)*8));
					printf("DoorAlarmStatus: 0x%08x  ,doorid = %d\n",gStateOptions.DoorAlarmStatus,DoorID);
					memcpy(shared_stateoptions_stuff,&gStateOptions,sizeof(TStateOptions));

				}
				if(DoorOpenORCloseEven == EVENT_DOORCLOSE && gOptions.CloseDoorAndLock[DoorID-1]
					&& !pollcard_struct_variable.c3doorkeepopen[DoorID-1].keepOpenSign)  //闭门回锁
				{
					Ex_ConctrolLock(POLLCARD,gOptions.MachineType,DoorID,0);
				}
				ProcessSaveAcessEvenLog(0,0,VS_OTHER,DoorID,DoorOpenORCloseEven,2,OldEncodeTime(&gCurTime));
			}

			//0为无门状态，1为关，2为开
			if(gOptions.DoorDetectortype[DoorID-1]!=0)
			{
				DoorState = GetOneDoorState(DoorID);

				if(DoorState == DOORISOPEN)//门打开了，开始计时
				{
					pollcard_struct_variable.DoorLockOpenTimeOver[DoorID-1].DoorLockState = DoorState;
					pollcard_struct_variable.DoorLockOpenTimeOver[DoorID-1].Dooropentimelimit = gOptions.DoorDetectortime[DoorID-1];
					pollcard_struct_variable.DoorLockOpenTimeOver[DoorID-1].Dooropentime = 0;
				}
				else
				{
					pollcard_struct_variable.DoorLockOpenTimeOver[DoorID-1].DoorLockState = DoorState;
					pollcard_struct_variable.DoorLockOpenTimeOver[DoorID-1].Dooropentimelimit = 0;
					pollcard_struct_variable.DoorLockOpenTimeOver[DoorID-1].Dooropentime = 0;
				}
			}
		}
		else if(ioindex == 5 || ioindex == 6 )//扩展 in
		{
			if(gOptions.MachineType == C3_400 || C3_400To_200 == gOptions.MachineType)
			{
				DoorID = (mcuID -1)* 2 + ioindex - 4;
			}
			else
			{
				DoorID = mcuID;
			}
			ProcessSaveAcessEvenLog(0,0,VS_OTHER,DoorID,
				((iostate)? EVENT_AUXIN_CLOSE:EVENT_AUXIN_OPEN),2,OldEncodeTime(&gCurTime));
		}
		else if(ioindex == 7)//初始化
		{
			pollcard_struct_variable.machineintit.change_count++;
			pollcard_struct_variable.machineintit.change_times = 10;
			int max_count = 0;
			if(gOptions.MachineType == C3_400 || gOptions.MachineType == C3_200)
			{
				max_count = 12;
			}
			else
			{
				max_count = 6;
			}
			printf("ioindex = 7, change_count =  %d !\n",pollcard_struct_variable.machineintit.change_count);

			if(pollcard_struct_variable.machineintit.change_count >= max_count)
			{
				printf("start machineinit !\n");
				pollcard_struct_variable.machineintit.change_count = 0;
				DoRestoreDefaultOptions();
				mmsleep(1000);
				system("reboot");
			}

		}
	}

	return 1;
}

int On_ControlDoorStatus(PCommData shared_Comm_stuff)
{
	  //命令字 + 门号 + 锁号/报警号(1:锁号 2:报警号)+ 时间 (暂定义，稍后修改,先以下面定义为准)
	  int DoorID = shared_Comm_stuff->SendData[0] & 0xff;
	  int SignalID = shared_Comm_stuff->SendData[1] & 0xff;
	  int Time = shared_Comm_stuff->SendData[2] & 0xff;

	  printf("\nmain rec data News_CONTROL_DEVICE:  DoorID =%d,SignalID = %d,Time=%d\n",DoorID,SignalID,Time);
	  if(DoorID > 4  ||  SignalID > 2)
	  {
		  shared_Comm_stuff->NewsType = -1;  //comm进程发的数据错误。
		  printf("main News_CONTROL_DEVICE shared_Comm_stuff->NewsType: %d\n",shared_Comm_stuff->NewsType);
		  return -1;
	  }

	  if(SignalID == 1 )
	  {

		  if(Time == 0)//关门
		  {
			   //CommKeepOpenSign[DoorID-1] = 0;
			   if(pollcard_struct_variable.c3doorkeepopen[DoorID-1].keepOpenSign == 0)
			   {
				   Ex_ConctrolLock(COMMCONTROL, gOptions.MachineType,DoorID,Time);
				   ProcessSaveAcessEvenLog(0,0,VS_OTHER,DoorID,EVENT_COMMCLOSEDOOR,2,OldEncodeTime(&gCurTime));
			   }
			   else
			   {
				   ProcessSaveAcessEvenLog(0,0,VS_OTHER,DoorID,EVENT_DOORKEEPOPEN_ERROR,2,OldEncodeTime(&gCurTime));
			   }

		  }
		  else if(255 == Time)//远程常开
		  {
			   ProcessSaveAcessEvenLog(0,0,VS_OTHER,DoorID,EVENT_COMM_KEEPOPENDOOR,2,OldEncodeTime(&gCurTime));
			   pollcard_struct_variable.c3doorkeepopen[DoorID-1].keepOpenSign = 1;
			   CommKeepOpenSign[DoorID-1] = 1;
			   gOptions.CancelKeepOpenDay[DoorID-1] = 0;
			   Ex_ConctrolLock(COMMCONTROL, gOptions.MachineType,DoorID,Time);
		  }
		  else
		  {
			   ProcessSaveAcessEvenLog(0,0,VS_OTHER,DoorID,EVENT_COMMOPENDOOR,2,OldEncodeTime(&gCurTime));
			   if(pollcard_struct_variable.c3doorkeepopen[DoorID-1].keepOpenSign == 0)
			   {
				   Ex_ConctrolLock(COMMCONTROL, gOptions.MachineType,DoorID,Time);
			   }
		  }

	  }
	  else if(SignalID == 2)
	  {
		  Ex_ConctrolAlarm(gOptions.MachineType,DoorID,Time);
		  if(Time == 0)
		  {
			  ProcessSaveAcessEvenLog(0,0,VS_OTHER,DoorID,EVENT_COMMCLOSEALARM,2,OldEncodeTime(&gCurTime));
		  }
		  else
		  {
			  ProcessSaveAcessEvenLog(0,0,VS_OTHER,DoorID,EVENT_COMMOPENALARM,2,OldEncodeTime(&gCurTime));
		  }
	  }
	  shared_Comm_stuff->CommCmdRet = 0;  //main进程执行命令成功。
	  return 0;
}

int On_CancelAlarm(PCommData shared_Comm_stuff)
{
	int DoorID = shared_Comm_stuff->SendData[0] & 0xff;
	ProcessSaveAcessEvenLog(0,0,VS_OTHER,DoorID,EVENT_COMMCONTROL,2,OldEncodeTime(&gCurTime));
	gStateOptions.DoorAlarmStatus &= 0xfefefefe;//表示门的报警状态，
	memcpy(shared_stateoptions_stuff,&gStateOptions,sizeof(TStateOptions));
	printf("DoorAlarmStatus: 0x%08x \n",gStateOptions.DoorAlarmStatus);
	shared_Comm_stuff->CommCmdRet = 0;  //main进程执行命令成功。
	return 0;
}

int On_Reboot(PCommData shared_Comm_stuff)
{
	shared_Comm_stuff->CommCmdRet = 0;  //main进程执行命令成功。

	mmsleep(1000);

	system("reboot");

	return 0;
}

int On_MainUpdate(PCommData shared_Comm_stuff)
{
	shared_Comm_stuff->CommCmdRet = 0;	 //main进程执行命令成功。
	LoadOptions(&gOptions);
	ReadRTCClockToSyncSys(&gCurTime);
	return 0;
}

int On_StartDoorKeepOpen(PCommData shared_Comm_stuff)
{
	int DoorID = shared_Comm_stuff->SendData[0] & 0xff;
	int SignalID = shared_Comm_stuff->SendData[1] & 0xff;
	int CurDay = (gCurTime.tm_year+1900)*10000 + (gCurTime.tm_mon+1)*100 + gCurTime.tm_mday;
	if(0 == SignalID)
	{
		memset(&pollcard_struct_variable.c3doorkeepopen[DoorID-1],0x00,sizeof(TC3doorkeepOpen));
		CommKeepOpenSign[DoorID-1] = 0;
		if(pollcard_struct_variable.c3doorkeepopen[DoorID-1].keepOpenSign == 1)
		{
			Ex_ConctrolLock(COMMCONTROL, gOptions.MachineType,DoorID,0);
		}
		printf("comm cancel doorkeepopen and SaveOptions!!!!!!!!!!!\n");
		gOptions.CancelKeepOpenDay[DoorID-1] = CurDay;
		SaveOptions(&gOptions);
		ProcessSaveAcessEvenLog(0,0,VS_OTHER,DoorID,EVENT_CANCEL_DOORKEEPOPEN,2,OldEncodeTime(&gCurTime));
	}
	else if(1 == SignalID)
	{
		gOptions.CancelKeepOpenDay[DoorID-1] = 0;
		SaveOptions(&gOptions);
		ProcessSaveAcessEvenLog(0,0,VS_OTHER,DoorID,EVENT_START_DOORKEEPOPEN,2,OldEncodeTime(&gCurTime));
		RefleshLockKeepOpen(DoorID-1);
	}
	shared_Comm_stuff->CommCmdRet = 0;	 //main进程执行命令成功。
	return 0;
}
