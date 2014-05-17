/*
 * modulename: 	firmware pushsdk
 * filename: 	push_client_api.h
 * author:	 	yuanfat
 * copyright:	Copyright (C) 2011-2012, ZKSoftware Inc.
 */
#ifndef __PUSH_CLIENT_API_H__
#define __PUSH_CLIENT_API_H__






#ifdef _cplusplus
extern "C"
{
#endif


//for pushclient process
int PushClient_Init();
int PushClient_Run();


//for service process
int PushMain_Init();
int PushMain_Check();
int PushMain_RTWrite(const char *pszData, const int iSize);
int PushMain_RTRead(int *piCmd, char *pszData, int iSize);


#ifdef _cplusplus
}
#endif


#endif



