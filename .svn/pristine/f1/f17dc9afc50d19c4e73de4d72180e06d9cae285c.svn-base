/*************************************************
                                           
 ZEM 200                                          
                                                    
 rtc.c Setup system time and RTC clock 
                                                      
 Copyright (C) 2003-2005, ZKSoftware Inc.      		
                                                      
*************************************************/

#include <stdio.h>
#include <linux/rtc.h>
#include <sys/ioctl.h>
#include <sys/time.h>
#include <sys/types.h>
#include <fcntl.h>
#include "arca.h"
#include "exfun.h"
#ifdef ZEM500
#include "c4i2c.h"
#endif

BOOL SetRTCClock(TTime *tm)
{
	SetExternelRTC(tm);
	return TRUE;
}

BOOL ReadRTCClockToSyncSys(TTime *tm)
{
    int fd, retval=0;
    time_t newtime;
    struct timeval tv;
    struct timezone tz;	

    DBPRINTF("before GetExternelRTC  weekday = %d,gCurTime=%d-%d-%d,%d:%d:%d\n",tm->tm_wday,tm->tm_year,tm->tm_mon,tm->tm_mday,tm->tm_hour,tm->tm_min,tm->tm_sec);
	GetExternelRTC(tm);

    DBPRINTF("after GetExternelRTC weekday = %d,gCurTime= %d-%d-%d,%d:%d:%d\n",tm->tm_wday,tm->tm_year,tm->tm_mon,tm->tm_mday,tm->tm_hour,tm->tm_min,tm->tm_sec);
/*	 tm->tm_year= 110;
	tm->tm_mon = 1;
	tm->tm_mday= 6;
	tm->tm_hour = 13;
	tm->tm_min = 45;
	tm->tm_sec = 20;    
*/

    DBPRINTF("RetVal=%d Year=%d Month=%d Day=%d Hour=%d Min=%d Sec=%d\n", 
	     retval, tm->tm_year, tm->tm_mon, tm->tm_mday, tm->tm_hour, tm->tm_min, tm->tm_sec);    
    if(((tm->tm_year<=70)||(tm->tm_year>=137))||
       ((tm->tm_mon>=12)||(tm->tm_mon<0))||
       ((tm->tm_mday<=0)||(tm->tm_mday>=32))||
       ((tm->tm_hour<0)||(tm->tm_hour>=24))||
       ((tm->tm_min<0)||(tm->tm_min>=60))||
       ((tm->tm_sec<0)||(tm->tm_sec>=60)))
	    retval=-1;
    //Fixed RTC time 
    if((tm->tm_year<=70)||(tm->tm_year>=137)) tm->tm_year=100;
    if((tm->tm_mon>=12)||(tm->tm_mon<0)) tm->tm_mon=0;
    if((tm->tm_mday<=0)||(tm->tm_mday>=32)) tm->tm_mday=1;
    if((tm->tm_hour<0)||(tm->tm_hour>=24)) tm->tm_hour=0;
    if((tm->tm_min<0)||(tm->tm_min>=60)) tm->tm_min=0;
    if((tm->tm_sec<0)||(tm->tm_sec>=60)) tm->tm_sec=0;
#ifndef ZEM500
    if(retval==-1)
    {
	retval=ioctl(fd, RTC_SET_TIME, tm);
    }
    close(fd);
#endif

    tm->tm_isdst=-1;          //don't know whether it's dst   夏令时
    //EncodeTime标准函数mktime
    //mktime可以修正tm_wday, tm_yday的值位于正确范围
    newtime=EncodeTime(tm);
    
    gettimeofday(&tv, &tz);
    
    tv.tv_sec = newtime+1;
    tv.tv_usec = 0;
    
    settimeofday(&tv, &tz);
//    settimeofday(&tv, &tz);
  
  return (retval==0);

}
