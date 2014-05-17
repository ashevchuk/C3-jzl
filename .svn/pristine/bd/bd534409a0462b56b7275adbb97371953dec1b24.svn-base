#ifndef	_MAIN_H_
#define	_MAIN_H_
//#define jz_7131 1
#include "arca.h"
#include "exfun.h"
#include "ccc.h"
#include "flashdb.h"
//#include "Options.h"

#define SUBCMD_UPDATE_OPTIONS  					10  //comm进程发送的重启指令

char *gImageBuffer;
TTime gCurTime;

typedef struct _VSStatus_{
	BOOL PIN;
	BOOL FP;
	BOOL Password;
	BOOL Card;
}GCC_PACKED TVSStatus, *PVSStatus;

void ShowMainLCD(void);
void EnableDevice(int Enabled);
void ExSetPowerSleepTime(int IdleMinute);
int ShowFPAnimate(int x, int y);
//void OutputPrinterFmt2(int pin);
extern int  WaitInitSensorCount;
int Filter_Group_Run(int TID);
int Switch24HTo12H(int Hour, int Min, int *pHour, int *pMin);//modify by dsl 2007.6.27
int IsDaylightSavingTime(void);//2007.7.14
int IsDaylightSavintTimeMenu(void);
int GetNoNcState(void);//dsl 2007.8.8
//BYTE GetUserVSFormAuthServer(U32 pin, int msgtype);//dsl 2007.8.14
int Change_LockForceAction(int state);//dsl 2007.9.4
int SetNoNcStateBySDK(void);//dsl 2007.9.8 the lock state take effact


void OnTimer(unsigned char cID);

int ProcessSaveAcessEvenLog(U32 cardno,U32 pin,BYTE verified,BYTE ReaderID,
	BYTE EventType,BYTE inoutstate,time_t time_second);

void AppendRTLogBuff(int cardno,int pin,BYTE verified,BYTE ReaderID,BYTE EventType,BYTE inoutstate,U32 time_second);
void GetDoorAlarmStatus(BYTE EventType,BYTE ReaderID);
void Ex_ConctrolLock(int opencause,int MachineType,int index,int time);
int On_CommUpdate(PCommData shared_Comm_stuff);

#endif
