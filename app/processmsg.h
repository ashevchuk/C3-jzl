/*************************************************

 ZEM 510

processmsg.h message process  functions

 Copyright (C) 2010-2015, ZKSoftware Inc.

*************************************************/


#ifndef _PROCESSMSG_H_
#define	_PROCESSMSG_H_

#include "flashdb.h"

int	On_Button(int readerno,int keyvalue);
int	On_PollCard(int cardno,int readerno);
int	On_Wiegandkey(int keyvalue,int readerno);
int On_InputPinChange(int mcuID,int ioindex,int iostate);
int On_ControlDoorStatus(PCommData shared_Comm_stuff);
int On_CancelAlarm(PCommData shared_Comm_stuff);
int On_Reboot(PCommData shared_Comm_stuff);
int On_MainUpdate(PCommData shared_Comm_stuff);
int On_StartDoorKeepOpen(PCommData shared_Comm_stuff);


#endif
