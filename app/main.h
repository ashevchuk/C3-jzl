#ifndef	_MAIN_H_
#define	_MAIN_H_
//#define jz_7131 1
#include "arca.h"
#include "exfun.h"
#include "ccc.h"
//#include "Options.h"


TTime gCurTime;
int ShowMainLCDDelay;
typedef struct _VSStatus_{
	BOOL PIN;
	BOOL FP;
	BOOL Password;
	BOOL Card;
}GCC_PACKED TVSStatus, *PVSStatus;

#define MANUAL    1
#define AUTOMATIC 0

void ShowMainLCD(void);
void EnableDevice(int Enabled);

void OnTimer(unsigned char cID);
int ProcessSaveAcessEvenLog(U32 cardno,U32 pin,BYTE verified,BYTE ReaderID,BYTE EventType,BYTE inoutstate,
		time_t time_second);
void AppendRTLogBuff(int cardno,int pin,BYTE verified,BYTE ReaderID,BYTE EventType,BYTE inoutstate,
		U32 time_second);
void Ex_ConctrolLock(int opencause,int MachineType,int index,int time);


#endif
