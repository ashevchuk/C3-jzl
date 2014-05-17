/*************************************************

 ZEM 200

 exfun.c time and voice and access control function

 Copyright (C) 2003-2005, ZKSoftware Inc.

 $Log: exfun.c,v $
 Revision 5.14  2006/03/04 17:30:09  david
 Add multi-language function

 Revision 5.13  2005/12/22 08:54:23  david
 Add workcode and PIN2 support

 Revision 5.12  2005/08/15 13:00:22  david
 Fixed some Minor Bugs

 Revision 5.11  2005/08/13 13:26:14  david
 Fixed some minor bugs and Modify schedule bell

 Revision 5.10  2005/08/07 08:13:15  david
 Modfiy Red&Green LED and Beep

 Revision 5.9  2005/08/04 15:42:53  david
 Add Wiegand 26 Output&Fixed some minor bug

 Revision 5.8  2005/08/02 16:07:51  david
 Add Mifare function&Duress function

 Revision 5.7  2005/07/14 16:59:53  david
 Add update firmware by SDK and U-disk

 Revision 5.6  2005/06/10 17:11:01  david
 support tcp connection

 Revision 5.5  2005/05/13 23:19:32  david
 Fixed some minor bugs

 Revision 5.4  2005/04/27 00:15:37  david
 Fixed Some Bugs

 Revision 5.3  2005/04/24 11:11:26  david
 Add advanced access control function

*************************************************/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <sys/time.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <wait.h>
#include "arca.h"
//#include "lcm.h"
#include "exfun.h"
#include "wavmain.h"
#include "options.h"
#include "msg.h"
#include "main.h"
#include "rtc.h"
#include "sensor.h"
#include "usb_helper.h"
//#include "wiegand.h"
#include "serial.h"
//dsl 2008.4.21
#include "flashdb.h"

static TMyBuf *buff_in=NULL, *buff_out1=NULL, *buff_out2=NULL;
static char WavFilePath[80]="NONE";

int fd_wiegand=-1;
static int fd_dummy=-1;
int gAlarmDelay=0,gAlarmDelayIndex=0,gBellDelay=0;

int PlayWavFileAsync(int argc, char **command)
{
	return 1;
}

time_t EncodeTime(TTime *t)
{
	time_t tt;

	//夏令时 = 没有信息
	t->tm_isdst = -1;
	tt = mktime_1(t);
	return tt;
}

time_t OldEncodeTime(TTime *t)
{
	time_t tt;

	tt=((t->tm_year-100)*12*31+((t->tm_mon)*31)+t->tm_mday-1)*(24*60*60)+
	   (t->tm_hour*60+t->tm_min)*60+t->tm_sec;
	return tt;
}

TTime * OldDecodeTime(time_t t, TTime *ts)
{

	ts->tm_sec=t % 60;
	t/=60;
	ts->tm_min=t % 60;
	t/=60;
	ts->tm_hour=t % 24;
	t/=24;
	ts->tm_mday=t % 31+1;
	t/=31;
	ts->tm_mon=t % 12;
	t/=12;
	ts->tm_year=t+100;
	return ts;
}

TTime * DecodeTime(time_t t, TTime *ts)
{
	memcpy(ts, localtime(&t), sizeof(TTime));
	return ts;
}

int TimeDiffSec(TTime t1, TTime t2)
{
	return (EncodeTime(&t1) - EncodeTime(&t2));
}

//获取一个月的天数
int GetLastDayofmonth(int y,int m)
{
        int f,n;
        n=0;
        f=(y%4==0&&y%100!=0)||(y%400==0);
        if (m>7 && m%2==0)
                n=31;
        else if (m>7)
                n=30;
        else if (m%2==0 && m!=2)
                n=30;
        else if (m!=2)
                n=31;
        else
         n=28+f;
        return n;
}

void GetTime(TTime *t)
{
	time_t tt;

	tt = time(NULL);
	memcpy(t, localtime(&tt), sizeof(TTime));
}

void SetTime(TTime *t)
{
	time_t tt;
	//fix tm_wday tm_yday
	tt = EncodeTime(t);
	//setup RTC CLOCK
	SetRTCClock(t);

	DelayUS(100*1000);
	//synochronize system time from RTC
	ReadRTCClockToSyncSys(t);
}

/*
 * Very simple buffer management.
 *
 * io = 0 : request for input buffer
 * io = 1 : request for output buffer
 *
 * set buffer length to 0 to free the buffer.
 *
 */
PMyBuf bget(int io)
{
	if(buff_in==NULL)
	{
		buff_out2=(TMyBuf*)malloc(sizeof(TMyBuf));
		buff_out2->len=0;
		buff_out1=(TMyBuf*)malloc(sizeof(TMyBuf));
		buff_out1->len=0;
		buff_in=(TMyBuf*)malloc(sizeof(TMyBuf));
		buff_in->len=0;
	}
	if( io == 1) {
		if(buff_out1->len == 0) return buff_out1;
		else return buff_out2;
	}else return buff_in;

	//      DBPRINTF("%s:can't get buffer\n",__FUNCTION__);
	return 0;
}

void FreeCommuCache(void)
{
	if(buff_in != NULL) free(buff_in);
	if(buff_out1 != NULL) free(buff_out1);
	if(buff_out2 != NULL) free(buff_out2);
}

unsigned short in_chksum(unsigned char *p, int len)
{
	unsigned long sum=0;
	printf("p[0]=%d,p[1]=%d,p[2]=%d\n",p[0],p[1],p[2]);
	while(len > 1) {
		sum += *((unsigned short*)p); p+=2;
		if( sum & 0x80000000 )
		{	
			sum = (sum & 0xFFFF) + (sum >> 16);
			printf("sum=%lu\n");
		}
		len -= 2;
	}
	printf("sum=%lu,len=%d\n",sum,len);
	if(len)
		sum += (unsigned short) *(unsigned char*) p;

	while(sum >> 16)
		sum = (sum & 0xFFFF) + (sum >> 16);
	printf("sum=%lu,len=%d\n",sum,len);
	printf("~sum=%lu\n",~sum);
	return ~sum;
}

void ExCloseRF(void)
{
	if (fd_wiegand>=0) close(fd_wiegand);
}

BOOL Check232IDCard(serial_driver_t *rs,int * pcardno)
{
	int chars;
	char buf[4]={0};
	if((chars=rs->poll())==0) return 0;
	chars=rs->read_buf(buf,chars);
	if(chars>=3)
	{
		memcpy(pcardno,buf,3);
		return TRUE;
	}else
		return FALSE;
}

void ExOpenWiegand(void)
{
}

void ExCloseWiegand(void)
{
//	if (fd_dummy>=0) close(fd_dummy);
}
//cxy
BOOL WiegandSendString(U8 * CommandStr,int Strlen)
{

	return FALSE;
}

BOOL WiegandSend(U32 deviceID, U32 CardNum, U32 DuressID)
{
	return FALSE;
}

void Switch_mode(U32 RS232Mode)
{
	//Output a LOW pulse
	//Switch RJ45/RS232
	GPIOSetLevel(IO_FUN_BUTTON, RS232Mode);
}

int WeekOfMonth(TTime t)
{
	int iAddNum = 0, iWeek1 = 0, iWeek2 = 0, iWeek3 = 0, iWeek4 = 0, iWeek5 = 0, iWeek6 = 0, iWeek = 1;
	int iDay = t.tm_mday;
	TTime t1 = t;
	t1.tm_mday = 1;
	time_t t2 = EncodeTime(&t1);
	DecodeTime(t2, &t1);

	iAddNum = 7 - t1.tm_wday;
	iWeek1 = iAddNum;
	iWeek2 = iWeek1 + 7;
	iWeek3 = iWeek2 + 7;
	iWeek4 = iWeek3 + 7;
	iWeek5 = iWeek4 + 7;
	iWeek6 = iWeek5 + 7;

	if (iDay <= iWeek6) iWeek = 6;
	if (iDay <= iWeek5) iWeek = 5;
	if (iDay <= iWeek4) iWeek = 4;
	if (iDay <= iWeek3) iWeek = 3;
	if (iDay <= iWeek2) iWeek = 2;
	if (iDay <= iWeek1) iWeek = 1;
	return iWeek;
}

TTime GetDateByWeek(int Year, int Month, int Weeks, int Week, int Hour, int Min)
{
        //dsl 2007.9.27
        TTime tt1 = {0,0,0,1,1,2004,4,0,0};
        TTime tt2 = {0,0,0,1,1,2004,4,0,0};
        int iDay = 0,tmpiDay = 0, i = 1;
        int iAddNum = 0, iWeek1 = 0, iWeek2 = 0, iWeek3 = 0, iWeek4 = 0, iWeek5 = 0, iWeeks = 1;
        time_t t1, t2;
        //first day of month
        tt1.tm_year = Year;
        tt1.tm_mon = Month;
        tt1.tm_hour = Hour;
        tt1.tm_min = Min;
        t1 = EncodeTime(&tt1);
        DecodeTime(t1, &tt1);
        //DBPRINTF("dsl_tt1 %d-%d-%d %d:%d:%d\n", tt1.tm_year+1900,tt1.tm_mon+1,tt1.tm_mday,tt1.tm_hour,tt1.tm_min,tt1.tm_sec);
        //last day of month
        tt2.tm_year = Year;
        tt2.tm_mon = Month+1;
        t2 = EncodeTime(&tt2);
        DecodeTime(t2, &tt2);
        tt2.tm_mday = tt2.tm_mday - 1;
        tt2.tm_hour = Hour;
        tt2.tm_min = Min;
        t2 = EncodeTime(&tt2);
        DecodeTime(t2, &tt2);
        //DBPRINTF("dsl_tt2 %d-%d-%d %d:%d:%d\n", tt2.tm_year+1900,tt2.tm_mon+1,tt2.tm_mday,tt2.tm_hour,tt2.tm_min,tt2.tm_sec);
        if(tt1.tm_wday <= Week)
        {
			while(i <= Weeks)
			{
				//printf("zhc no1 ------- Weeks: %d, i: %d\n",Weeks,i);
				tmpiDay = tt1.tm_mday + (Week - tt1.tm_wday) + (i-1)*7;
				if(tmpiDay <= 31)
				{
						iDay = tmpiDay;
				}
				i++;
			}
        }
        else
        {
            while(i <= Weeks)
            {
				//printf("zhc no2 ------- Weeks: %d, i: %d\n",Weeks,i);
				tmpiDay = (8 - tt1.tm_wday) + Week + (i-1)*7;
				if(tmpiDay <= 31)
				{
						iDay = tmpiDay;
				}
				i++;
            }
    }

    tt1.tm_mday = iDay;
    t1 = EncodeTime(&tt1);
    DecodeTime(t1, &tt1);
    //DBPRINTF("tt1:%d-%d-%d %d:%d:%d\n", tt1.tm_year+1900, tt1.tm_mon+1,tt1.tm_mday, tt1.tm_hour, tt1.tm_min, tt1.tm_sec);

    return tt1;
}

char *FindFileName(const char *name)
{
        char *head = name;
        char *name_start = NULL;
        char sep = '/';
        if(NULL == name)
        {
                return NULL;
        }
        name_start = strrchr(name,sep);

        printf("name_start: %s\n",name_start);

        name = head;

        return (NULL == name_start)?name:(name_start+1);
}

char *FindPathName(const char *name)
{
        char *head = name;
        char *name_start = NULL;
        char TmpName[100] = {0};
        char sep = '/';
        if(NULL == name)
        {
                return NULL;
        }

        memcpy(TmpName, name, strlen(name));
        name_start = strrchr(TmpName,sep);
        if(NULL == name_start)
        {
                return NULL;
        }

        printf("name_start: %s (name_end-name): %d\n",name_start, (name_start - TmpName));

        strncpy(TmpName, TmpName, (name_start - TmpName + 1));
        memset(TmpName + (name_start - TmpName + 1), 0, 1);

        name = head;

        return TmpName;
}

