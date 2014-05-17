/*************************************************

 ZEM 200

 options.c all function for options

 Copyright (C) 2003-2005, ZKSoftware Inc.

 Author: Richard Chen

 Modified by David Lee for JFFS2 FS 2004.12.12

 $Log: options.c,v $
 *************************************************/

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <dirent.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <linux/sockios.h>
#include <netinet/in.h>
#include <net/if.h>
#include <fcntl.h>
#include <unistd.h>
#include "arca.h"
#include "options.h"
#include "utils.h"
#include "sensor.h"
#include "finger.h"
#include "ccc.h"
//#include "mainmenu.h"
#include "lcm.h"
#include "kb.h"
#include "../platform/c4i2c.h"
#include "netspeed.h"

#include "serial.h"
#include "exfun.h"
//#include "exvoice.h"
#include <sys/vfs.h>

//options file handle
static int fdOptions = -1;
static int fdLanguage = -1;
static char CurLanguage = ' ';

char *DateFormats[]={"YY-MM-DD","YY/MM/DD","YY.MM.DD", "MM-DD-YY","MM/DD/YY","MM.DD.YY","DD-MM-YY","DD/MM/YY","DD.MM.YY","YYYYMMDD"};

int FormatDate(char *buf, int index, int y, int m, int d)
{
	index=index%10;
	if(index==9)
		sprintf(buf, "%04d%02d%02d", y,m,d);
	else
	{
		char ss;
		if(index<3) sprintf(buf, "%02d-%02d-%02d", y%100,m,d);
		else if(index<6) sprintf(buf, "%02d-%02d-%02d", m,d,y%100);
		else sprintf(buf, "%02d-%02d-%02d", d,m,y%100);
		if(index%3==0) ss='-';
		else if(index%3==1) ss='/';
		else ss='.';
		buf[2]=ss;buf[5]=ss;
	}

	return 8;
}

int FormatDate2(char *buf, int index, int m, int d)
{
	index=index%10;
	if(index==9)
		sprintf(buf, "%02d%02d", m,d);
	else
	{
		char ss;
		if(index<3) sprintf(buf, "%02d-%02d", m,d);
		else if(index<6) sprintf(buf, "%02d-%02d", m,d);
		else sprintf(buf, "%02d-%02d", d,m);
		if(index%3==0) ss='-';
		else if(index%3==1) ss='/';
		else ss='.';
		buf[2]=ss;
	}
	return 5;
}
//this function is only used for options.cfg, it is different with ReadOneLine
U32 PackStrBuffer(char *Buffer, const char *name, int size)
{
	char c, *cp, *namep, *valuep,*TheName;
	int i, isname, OriSize;
	char tmp[VALUE_BUFFERLEN];
	int offset=0;

	OriSize=size;
	TheName=(char*)malloc(size);
	namep=Buffer;
	valuep=namep;
	cp=Buffer;

	while(cp<(Buffer+size))
	{
		if(('='==*cp) && (valuep<=namep))
		{
			valuep=cp++;
			offset++;
		}
		else if((('\n'==*cp) || ('\r'==*cp)) && (cp>namep))
		{
			cp++;offset++;
			if (('\n'==*cp) || ('\r'==*cp)){cp++; offset++;}
			i=0;isname=1;
			while(1)
			{
				c=namep[i];
				if(c=='=')
				{
					TheName[i]=0;
					if(isname && name[i]) isname=0;
					break;
				}
				else
				{
					TheName[i]=c;
					if(c!=name[i]) isname=0;
				}
				i++;
			}
			if (isname || (LoadStrFromFile(fdOptions, TheName, tmp, TRUE, offset)!=-1))
			{ 	//delete this name and value
				memmove(namep,cp,size-(cp-Buffer));
				size-=cp-namep;
				memset(Buffer+size, 0, OriSize-size);
				cp=namep;
			}
			namep=cp;
		}
		else
		{
			cp++;
			offset++;
		}
		if('\0'==*cp) break;
	}
	free(TheName);
	return cp-Buffer;
}

//The strings are of the form name = value.
void CombineNameAndValue(const char *name, const char *value, int SaveTrue, char *processedStr)
{
	sprintf(processedStr, "%s=%s\n", name, value);
}

//support two format XXX=YYY OR "XXX=YYY" for compatible with zem100 language file format.
BOOL ReadOneLine(int fd, char *dest, int *size)
{
       char c;

       *size=0;
       while(TRUE)
       {
	       if (read(fd, &c, 1) == 1)
	       {
		       if((c == '\n') || (c == '\r') || (c == '"'))
		       {
			       if(*size==0)
				       continue;
			       else
				       break;
		       }
		       dest[*size] = c;
		       *size = *size + 1;
	       }
	       else
		       break;
       }
       if (*size > 0)
       {
	       dest[*size] = '\0';
       }
       return(*size > 0);
}

int GetFileCurPos(int fd)
{
       return lseek(fd, 0, SEEK_CUR);
}

void SplitByChar(char *buffer, char *name, char * value, char DeliChar)
{
       int cnt;
       char *p;

       p=buffer;
       cnt=0;
       while(*p)
       {
	       if (*p==DeliChar) break;
	       cnt++;
	       p++;
       }
       memcpy(name, buffer, cnt);
       name[cnt]='\0';
       if ((cnt+1)<strlen(buffer))
	       memcpy(value, buffer+cnt+1, strlen(buffer)-cnt-1);
       value[strlen(buffer)-cnt-1]='\0';
}

//支持一次取多个option
int GetOptionNameAndValue(char *p, int size,char *outbuf)
{
	char value[1024]={0};
	int l,vl;
       char pOptionName[30][40];
     	int i,j,len,m;

	i=j=len=m=0;

	memset(pOptionName,0x00,1200);
//	printf("GetOptionNameAndValue  size= %d\n",size);

	while(i < size)
	{
		if(p[i] != ',')//如果不等于，表明为同一个option
		{
			pOptionName[j][m++] = p[i];
		}
		else
		{
			//printf("curr get option is %s\n",pOptionName[j]);
			j++;
			m=0;
		}
		i++;
		if(m>40 || j >30)//option 名称长度超过40,本次option 个数超过30 表示错误
		return 0;
	}
	if(p[size-1] != ',')//最后一个optioon没有','
	{
		//printf("curr get option is %s\n",pOptionName[j]);
		j++;
	}
	printf("optionNum= %d,%s\n",j,p);

	for(i = 0;i<j;i++)//共有j个option
		printf("No %d   -----is %s\n",i,pOptionName[i]);


	for(i = 0,len= 0;i<j;i++)//共有j个option
	{

		if(!LoadStr(pOptionName[i], value))	//????没?懈? Option,??取??缺省值
		{
			GetDefaultOption(pOptionName[i],value);
		}

		//printf("value: %s strlen(value): %d pOptionName[%d]: %s\n",value, strlen(value), i ,pOptionName[i]);
		if(value)
		{
			l=strlen(pOptionName[i]);
			vl=strlen(value);
			if(value && vl<1024)
			{
				strcpy(&outbuf[len],pOptionName[i]);

				printf("outbuf: %s\n",outbuf);
				outbuf[l+len] = '=';
				strcpy(outbuf +l +1+len,value);

				len =  len+ l+vl+1;
				if(i<(j-1))			//不是最后一个option后加','
				{
					outbuf[len++] = ',';
				}
			}
			else
				len = len;
		}
		else
		{
			len = len;
		}
	}

	return len;
}


 //支持一次设置多个option
int SetOptionNameAndValue(char *p, int size)//,char inbuf)
{
//	char value[1024]={0};
	int l = 0, vl = 0;
	char pOptionName[30][50] = {0};
	int i = 0, j = 0, len = 0, m = 0;
	int ret = TRUE;

	i=j=len=m=0;
	memset(pOptionName,0x00,1500);

	while(i < size)
	{
		if(p[i] != ',')//如果不等于，表明为同一个option
		{
			pOptionName[j][m++] = p[i];
		}
		else
		{
			if(0 != m) //过滤名称长度为0的参数
			{
				j++;
			}
			m = 0;
		}
		i++;
		if(m>50 || j >30)//option 名称长度超过40,本次option 个数超过30 表示错误
		{
			ret = FALSE;
			return ret;
		}
	}
	if(p[size-1] != ',')//最后一个optioon没有','
		j++;
	printf("optionNum= %d,%s\n",j,p);

	for(i = 0;i<j;i++)//共有j个option
		printf("No %d   -----is %s\n",i,pOptionName[i]);

	char *value = NULL;//=pOptionName[i];
	int namel = 0, optionlen = 0;
	for(i = 0,len= 0;i<j;i++)//共有j个option
	{
		value=pOptionName[i];
		namel =0;
		optionlen = strlen(pOptionName[i]);

		value[optionlen]=0;
		printf("len = %d,set para  is : %s\n",optionlen,value);
		while(*value)
		{
			if('='==*value++)
				break;
			if(namel++>optionlen) break;
		}
		pOptionName[i][namel]=0;
		if(0 == strcmp(pOptionName[i],"GATEIPAddress") || 0 == strcmp(pOptionName[i],"IPAddress"))
		{
			if('0' == *value)//IP或网关的前一位不能为0。
			{
				return FALSE;
			}
		}
		ret = RemoteSaveStr(pOptionName[i],value,TRUE);
		if(FALSE == ret)  //如果options.cfg文件长度超过MAX_OPTION_SIZE。即本次定义8K时，返回错误。
		{
			ret = FALSE;
			break;
		}
	}
	return ret;
}

//广播设置参数，支持一次设置多个option。 广播只能设置IP地址、子网掩码、网关、带宽。
int BroadSetOptionNameAndValue(char *p, int size)
{
	int l = 0, vl = 0;
	char pOptionName[30][50] = {0};
	int i = 0, j = 0, len = 0, m = 0;
	int ret = TRUE;

	i=j=len=m=0;
	memset(pOptionName,0x00,1500);

	while(i < size)
	{
		if(p[i] != ',')//如果不等于，表明为同一个option
		{
			pOptionName[j][m++] = p[i];
		}
		else
		{
			if(0 != m) //过滤名称长度为0的参数
			{
				j++;
			}
			m = 0;
		}
		i++;
		if(m>50 || j >30)//option 名称长度超过40,本次option 个数超过30 表示错误
		{
			ret = FALSE;
			return ret;
		}
	}
	if(p[size-1] != ',')//最后一个optioon没有','
		j++;
	printf("optionNum= %d,%s\n",j,p);

	char *value = NULL;//=pOptionName[i];
	int namel = 0, optionlen = 0;
	for(i = 0,len= 0;i<j;i++)//共有j个option
	{
		printf("set NO %d   -----is %s\n",i,pOptionName[i]);
		value=pOptionName[i];
		namel =0;
		optionlen = strlen(pOptionName[i]);

		value[optionlen]=0;
		printf("len = %d,set para  is : %s\n",optionlen,value);
		while(*value)
		{
			if('='==*value++)
				break;
			if(namel++>optionlen) break;
		}

		pOptionName[i][namel]=0;
		if(strcmp(pOptionName[i],"IPAddress") == 0 )
		{
			if('0'==*value)
			{
				return FALSE;
			}
			printf("No %d   -----is %s\n",i,pOptionName[i]); //默认成功
			RemoteSaveStr(pOptionName[i],value,TRUE);
		}
		else if(strcmp(pOptionName[i],"GATEIPAddress") == 0 )
		{
			if('0'==*value)
			{
				return FALSE;
			}
			printf("No %d   -----is %s\n",i,pOptionName[i]);
			RemoteSaveStr(pOptionName[i],value,TRUE);
		}
		else if(strcmp(pOptionName[i],"NetMask") == 0 )
		{
			printf("No %d   -----is %s\n",i,pOptionName[i]);
			RemoteSaveStr(pOptionName[i],value,TRUE);
		}
		else if(strcmp(pOptionName[i],"eth0") == 0 )
		{
			printf("No %d   -----is %s\n",i,pOptionName[i]);
			RemoteSaveStr(pOptionName[i],value,TRUE);
		}
	}
	return ret;
}

//return -1 mean can not find string by name
int LoadStrFromFile(int fd, const char *name, char *value, BOOL ExitSign, int offset)
{
       char name1[128], value1[VALUE_BUFFERLEN];
       char buffer[VALUE_BUFFERLEN];
       int size;
       int position;

       position=-1;
       lseek(fd, offset, SEEK_SET);
       while(TRUE){
	   if(ReadOneLine(fd, buffer, &size)){
	       SplitByChar(buffer, name1, value1, '=');
	       if(strcmp(name1, name)==0){
		   strcpy(value, value1);
		   position = GetFileCurPos(fd);
		   if (ExitSign) break;
	       }
	   }else
	       break;
       }
       return position;
}

BOOL LoadStr(const char *name, char *value)
{
	if (strcmp(name, "DateTime")==0)
	{
		TTime t;
		U32 tt;

		GetTime(&t);

		tt = OldEncodeTime(&t);
//		sprintf(value,"DateTime=%d", tt);
		sprintf(value,"%d", tt);

		DBPRINTF("LoadStr Gettime:%d-%d-%d %d:%d:%d  ---%s\n",
			t.tm_year,t.tm_mon,t.tm_mday,t.tm_hour,t.tm_min,t.tm_sec,value);

		return TRUE;
	}
	else if (strcmp(name, "MAC")==0)
	{
		if(GetMAC(value))
			return TRUE;

	}
	else if (strcmp(name, "DeviceInState")==0)
	{
		//GetDeviceInState(value);

		DBPRINTF("DeviceInState = %s\n",value);
		return TRUE;
	}
	else
	{
		return (LoadStrFromFile(fdOptions, name, value, FALSE, 0)!=-1?TRUE:FALSE);
	}
}


void SetTime2(char* value)
{
	TTime t;
	time_t t2;

	t2 = atoi(value);
	OldDecodeTime(t2, &t);
	DBPRINTF("SetTime2 linux:t2 = %d,   %d-%d-%d %d:%d:%d\n",t2,t.tm_year,t.tm_mon,t.tm_mday,t.tm_hour,t.tm_min,t.tm_sec);
	SetTime(&t);
}

void SaveMACToFlash_ZEM510(const char *MAC)
{
	char macdata[10];
	#define CFG_MAC_LEN             6

   	if(str2mac((char *)MAC, macdata)==0)
	{
		printf("------------------------------change mac  = %d .....  \n",
		SaveBufferTONandFlashAddr(macdata, 513*2048,513*2048+6, CFG_MAC_LEN,1));
	}
}

void ExecuteActionForOption(const char *name, const char *value)
{
	DBPRINTF("options:%s\n",name);
	if (strcmp(name, "IPAddress")==0)
		SetIPAddress("IP", gOptions.IPAddress);
	else if (strcmp(name, "NetMask")==0)
		SetIPAddress("NETMASK", gOptions.NetMask);
	else if (strcmp(name, "GATEIPAddress")==0)
		SetGateway("add", gOptions.GATEIPAddress);
	else if (strcmp(name, "HiSpeedNet")==0)
		set_network_speed(NET_DEVICE_NAME, gOptions.HiSpeedNet);
	else if (strcmp(name, "RS485On")==0)
	{
		if (gOptions.RS485On) RS485_setmode(FALSE);
	}
	else if (strcmp(name, "COMKey")==0)
		SetRootPwd(gOptions.ComKey);
	else if (strcmp(name, "VOLUME")==0)
		SetAudioVol(gOptions.AudioVol);
//#ifdef MACHINETYPE_C4
        else if (strcmp(name, "MAC")==0)//本系统使用zem510
                SaveMACToFlash_ZEM510(value);
//#else
//	else if (strcmp(name, "MAC")==0)
//                SaveMACToFlash(value);
//#endif

       else if (strcmp(name, "DateTime")==0)
                SetTime2(value);
        else if (strcmp(name, "Reboot")==0)
            system("reboot");

}

//设置确定的几个参数到系统中
int ExecuteActionForFixOption(const char *name, const char *value)
{
	DBPRINTF("Execute Action For Fix Options\n");

	//目的：name为NULL时，和IP相关的参数才设置到系统中，为了延时设置。
	if(NULL == name)
	{
		DBPRINTF("Execute Action For IP Options\n");
		printf("IPAddress: %d.%d.%d.%d\n", gOptions.IPAddress[0],gOptions.IPAddress[1],gOptions.IPAddress[2],gOptions.IPAddress[3]);
		printf("NetMask: %d.%d.%d.%d\n", gOptions.NetMask[0], gOptions.NetMask[1], gOptions.NetMask[2], gOptions.NetMask[3]);
		printf("GATEIPAddress: %d.%d.%d.%d\n", gOptions.GATEIPAddress[0],gOptions.GATEIPAddress[1],gOptions.GATEIPAddress[2],gOptions.GATEIPAddress[3]);
		printf("eth0: %d\n",gOptions.HiSpeedNet);

		SetIPAddress("IP", gOptions.IPAddress);
		SetIPAddress("NETMASK", gOptions.NetMask);
		//SetGateway("add", gOptions.GATEIPAddress);
		set_network_speed(NET_DEVICE_NAME, gOptions.HiSpeedNet);

		if(gOptions.GATEIPAddress[0] == 0)
		{
			if(SetGateway("add", gOptions.IPAddress))
			{
				DBPRINTF("add gate way: %d.%d.%d.%d \n",gOptions.IPAddress[0],gOptions.IPAddress[1],gOptions.IPAddress[2],gOptions.IPAddress[3]);
			}
			else
			{
				DBPRINTF("add gate way failed: %d.%d.%d.%d \n",gOptions.IPAddress[0],gOptions.IPAddress[1],gOptions.IPAddress[2],gOptions.IPAddress[3]);
			}
		}
		else
		{
			if(SetGateway("add", gOptions.GATEIPAddress))
			{
				DBPRINTF("add gate way: %d.%d.%d.%d \n",gOptions.GATEIPAddress[0],gOptions.GATEIPAddress[1],gOptions.GATEIPAddress[2],gOptions.GATEIPAddress[3]);
			}
			else
			{
				DBPRINTF("add gate way failed: %d.%d.%d.%d \n",gOptions.GATEIPAddress[0],gOptions.GATEIPAddress[1],gOptions.GATEIPAddress[2],gOptions.GATEIPAddress[3]);
			}
		}

		return 0;
	}

	if (strcmp(name, "RS485On")==0)
	{
		if (gOptions.RS485On) RS485_setmode(FALSE);
	}
	else if (strcmp(name, "COMKey")==0)
	{
		SetRootPwd(gOptions.ComKey);
	}
	else if (strcmp(name, "VOLUME")==0)
	{
		SetAudioVol(gOptions.AudioVol);
	}
    else if (strcmp(name, "DateTime")==0)
    {
		SetTime2(value);
    }
    else if (strcmp(name, "MAC")==0)
    {
    	SaveMACToFlash_ZEM510(value);//本系统使用zem510
    }
	else if (strcmp(name, "Reboot")==0)
	{
		system("reboot");
	}

}

BOOL SaveStr(const char *name, const char *value, int SaveTrue)
{
	char buffer[VALUE_BUFFERLEN] = {0};
	int len = 0;

	len=strlen(value);
	if (LoadStr(name, buffer))
	{
		//the value is the same as old value, then return.
		if(strcmp(name, "GATEIPAddress") == 0 && strcmp(value, "") == 0)
		{
			sprintf(value,"%d.%d.%d.%d",gOptions.IPAddress[0],gOptions.IPAddress[1],gOptions.IPAddress[2],gOptions.IPAddress[3]);
		}
		printf("value=%s,buffer=%s  \n",value,buffer);
		if (0==strcmp(value, buffer))
		{
			return TRUE;
		}
	}
	//check language item whether can be setup or not
	if(!gOptions.MultiLanguage)
	{
		if(strcmp(name, "Language")==0) return TRUE;
	}
	//部分参数不需要保存到option.cfg中
	if (strcmp(name, "DateTime") !=0
		&& strcmp(name, "Reboot") != 0
		&& strcmp(name, "MachineType") != 0	//MachineType只供内部使用，不保存到option.cfg 中
		&& strcmp(name, "MAC") != 0)
	{
		CombineNameAndValue(name, value, SaveTrue, buffer);
		len=lseek(fdOptions, 0, SEEK_END);
		if (len>=MAX_OPTION_SIZE)
		{
		    ClearOptionItem("NONE");
		    len=lseek(fdOptions, 0, SEEK_END);
		}
		if (len<MAX_OPTION_SIZE)
		    write(fdOptions, buffer, strlen(buffer));
	}

	LoadOptions(&gOptions);

	ExecuteActionForOption(name, value);

	return ((len<MAX_OPTION_SIZE)?TRUE:FALSE);
}

//通过SDK设置设备的参数，与SaveStr不同之处在于去掉ExecuteActionForOption(name, value);
//原因：例如即时把IP地址设置到系统中，设备与SDK的通信中断，SDK将获不了返回值，只能超时等待。
BOOL RemoteSaveStr(const char *name, const char *value, int SaveTrue)
{
	char buffer[VALUE_BUFFERLEN] = {0};
	int len = 0;

	len=strlen(value);
	if (LoadStr(name, buffer))
	{
		//the value is the same as old value, then return.
		if(strcmp(name, "GATEIPAddress") == 0 && strcmp(value, "") == 0)
		{
				sprintf(value,"%d.%d.%d.%d",gOptions.IPAddress[0],gOptions.IPAddress[1],gOptions.IPAddress[2],gOptions.IPAddress[3]);
		}

		printf("value=%s,buffer=%s  \n",value,buffer);
		if (0==strcmp(value, buffer))
		{
			return TRUE;
		}
	}
	//check language item whether can be setup or not
	if(!gOptions.MultiLanguage)
	{
		if(strcmp(name, "Language")==0) return TRUE;
	}
	//部分参数不需要保存到option.cfg中
	if (strcmp(name, "DateTime") !=0
		&& strcmp(name, "Reboot") != 0
		&& strcmp(name, "MachineType") != 0	//MachineType只供内部使用，不保存到option.cfg 中
		&& strcmp(name, "MAC") != 0
		&& strcmp(name, "FirmVer") != 0
		&& strcmp(name, "MainMCUVer") != 0
		&& strcmp(name, "SubMCUVer1") != 0
		&& strcmp(name, "SubMCUVer2") != 0
		&& strcmp(name, "SubMCUVer3") != 0
		&& strcmp(name, "SubMCUVer4") != 0)
	{
		CombineNameAndValue(name, value, SaveTrue, buffer);
		len=lseek(fdOptions, 0, SEEK_END);
		if (len>=MAX_OPTION_SIZE)
		{
		    ClearOptionItem("NONE");
		    len=lseek(fdOptions, 0, SEEK_END);
		}
		if (len<MAX_OPTION_SIZE)
		    write(fdOptions, buffer, strlen(buffer));
	}

	LoadOptions(&gOptions);

	ExecuteActionForFixOption(name, value);

	return ((len<MAX_OPTION_SIZE)?TRUE:FALSE);
}

char * keycardformat(char *str, BYTE *value)
{
	sprintf(str,"%02X-%02X-%02X-%02X-%02X-%02X", value[0],value[1],value[2],value[3],value[4],value[5]);
	return str;
}


char *GetCardKeyStr(char *Buffer, BYTE *Key)
{
	int i;
	if(gOptions.CardkeyKeypad)
    return keycardformat(Buffer,Key);
	BYTE *tmp=(BYTE *)Buffer;
        memcpy(tmp,Key,6);
        tmp[6]=0;
        for(i=5;i>=0;i--)
                if(tmp[i]==0xff) tmp[i]=0;
        return Buffer;
}

static char ln[40];

int LoadInteger(const char *Name, int DefaultValue)
{
	char tmp[VALUE_BUFFERLEN];
	char *buf;
	int v,n=1,d,c;

	buf=tmp;
	if(LoadStr(Name, buf))
	{
		if(*buf)
		{
			if('-'==*buf)
			{
				n=-1;
				buf++;
			}
			v=0;c=0;
			do{
				d=buf[c];
				if(d==0) break;
				if(d<'0' || d>'9')
				{
					return DefaultValue;
				}
				v=v*10+(d-'0');
				c++;
			}while(1);
			if(c)
				return n*v;
		}
	}
	return DefaultValue;
}

int SaveInteger(const char *Name, int Value)
{
	char Buf[20];
	sprintf(Buf,"%d",Value);
	if (SaveStr(Name, Buf, FALSE))
		return 0;
	else
		return 1;
}

TOptions gOptions;

char* SaveOptionItem(char *buf, const char *name, const char *value)
{
	char *p=buf;
	while(*name) *p++=*name++;
	*p++='=';
	while(*value) *p++=*value++;
	*p++=0;
	return p;
}

int SaveDefaultOptions(char *buffer)
{
	char *p=buffer;
	return p-buffer;
}

BYTE ByteInvert( BYTE chSrc )
{
	BYTE i, chDst;

	chDst = chSrc&1;

	for( i=0; i<7; i++)
	chDst<<=1, chSrc>>=1, chDst|=chSrc&1;



	return chDst;
}

void GetOptions_DeviceID(void)
{
	BYTE Addr = 0;
	BYTE TmpAddr = 0;

	Get485HardAddr(&Addr);

	TmpAddr = ByteInvert(Addr);

	gOptions.DeviceID = TmpAddr >> 2; //485地址，转高位的前6位，所以右移2位
}

void GetOption_DoorNum(void)
{
#ifndef MACHINETYPE_C4

	int Num = 0;

	if(GetDoorNumByMcu(&Num))
	{
		gOptions.LockCount = Num;
	}
	else
	{
		gOptions.LockCount = 0;
	}

#endif
}

#ifdef MACHINETYPE_C4
int machineisC4()
{
// 借用取单片机版本号来判断有几个单片机，以此识别c4 与c4_200

	char buf[20] = {0};
	int MCUVersion;

	MCUVersion=GetMCUVersion(MCU4,buf);
	DBPRINTF("MCU4 Version %d----:%d-%d-%d-%d\n",MCUVersion,buf[0],buf[1],buf[2],buf[3]);
	return MCUVersion;
}
#endif		//如果无法猎取ver，返回0

int SaveFirmVer()
{
	int ret = 0;

	sprintf(gOptions.FirmVer, "%s", MAINVERSION);

	printf("FirmVer: %s\n", gOptions.FirmVer);
	ret = SaveStr("FirmVer", gOptions.FirmVer, 1);

	return ret;
}

#define MaxDataLen 24
int SaveMCUVer()
{
	int ret = 0;

	char data[MaxDataLen] = {0};
#ifdef MACHINETYPE_C4
	memset(data, 0, MaxDataLen);
	GetMCUVersion(MAINMCU, data);
	sprintf(gOptions.MainMCUVer,"V%d.%d.%d.%d",data[0],data[1],data[2],data[3]);
	SaveStr("MainMCUVer", gOptions.MainMCUVer, 1);
	printf("MAINMCU:  %s\n",gOptions.MainMCUVer);

	if(C4_200 != gOptions.MachineType)
	{
		memset(data, 0, MaxDataLen);
		GetMCUVersion(MCU3, data);
		sprintf(gOptions.SubMCUVer3,"V%d.%d.%d.%d",data[0],data[1],data[2],data[3]);
		SaveStr("SubMCUVer3", gOptions.SubMCUVer3, 1);
		printf("SubMCUVer3:  %s\n",gOptions.SubMCUVer3);

		memset(data, 0, MaxDataLen);
		GetMCUVersion(MCU4, data);
		sprintf(gOptions.SubMCUVer4,"V%d.%d.%d.%d",data[0],data[1],data[2],data[3]);
		SaveStr("SubMCUVer4", gOptions.SubMCUVer4, 1);
		printf("SubMCUVer4:  %s\n",gOptions.SubMCUVer4);
	}
#endif

	memset(data, 0, MaxDataLen);
	GetMCUVersion(MCU1, data);
	sprintf(gOptions.SubMCUVer1,"V%d.%d.%d.%d",data[0],data[1],data[2],data[3]);
	SaveStr("SubMCUVer1", gOptions.SubMCUVer1, 1);
	printf("SubMCUVer1:  %s\n",gOptions.SubMCUVer1);
	if(gOptions.LockCount > 1)
	{
		memset(data, 0, MaxDataLen);
		GetMCUVersion(MCU2, data);
		sprintf(gOptions.SubMCUVer2,"V%d.%d.%d.%d",data[0],data[1],data[2],data[3]);
		SaveStr("SubMCUVer2", gOptions.SubMCUVer2, 1);
		printf("SubMCUVer2:  %s\n",gOptions.SubMCUVer2);
	}

	return ret;
}

int InitOptions(void)
{
	char Buffer[80];
//	struct statfs s;

	DBPRINTF("InitOptions!\n");

	GetEnvFilePath("USERDATAPATH", "options.cfg", Buffer);

	fdOptions=open(Buffer, O_RDWR|O_CREAT|O_SYNC, S_IRWXU|S_IRWXG|S_IRWXO);

	SaveFirmVer();

	LoadOptions(&gOptions);

	if(LoadStr("BCIIKeyLayouts", Buffer))
	{
		SetKeyLayouts(Buffer);
	}
	if(gOptions.IsOnlyRFMachine)
	{
		if(gOptions.MaxUserCount>300) //3//3W人
			gOptions.MaxUserCount=300;
		if(gOptions.MaxAttLogCount > 10)//10W条记录
			gOptions.MaxAttLogCount = 10;
	}
	else
	{
		if(gOptions.MaxUserCount>30) //3000人
			gOptions.MaxUserCount=30;
		if(gOptions.MaxAttLogCount > 3)//3//3W条记录
			gOptions.MaxAttLogCount = 3;
	}

	printf("before Option_cfg DoorNum= %d\n",gOptions.LockCount);
	GetOption_DoorNum();//通过硬件获取lock数据
	printf("ByMcu Get DoorNum= %d\n",gOptions.LockCount);

	//GetMachineType(&gOptions.MachineType);根据参数获取机器机型
	if(gOptions.ACPanelFunOn==2)//机器类别 c3: 1 c4:2
	{
#ifdef MACHINETYPE_C4
		if(machineisC4())
		{
			if(gOptions.Door4ToDoor2 == 0)
			{
				gOptions.MachineType = C4;
				printf("MachineType is C4.\n");
				gOptions.LockCount = 4;
				gOptions.ReaderCount = 4;
				gOptions.AuxInCount = 4;//自定义输入数量
				gOptions.AuxOutCount = 6;//自定义输出数量
			}
			else
			{
				gOptions.MachineType = C4_400To_200;
				printf("MachineType is C4_400To_200.\n");
				gOptions.LockCount = 2;
				gOptions.ReaderCount = 4;
				gOptions.AuxInCount = 4;//自定义输入数量
				gOptions.AuxOutCount = 6;//自定义输出数量
			}
		}
		else
		{
			gOptions.MachineType = C4_200;
			printf("MachineType is C4_200.\n");
			gOptions.LockCount = 2;
			gOptions.ReaderCount = 2;
			gOptions.AuxInCount = 4;//自定义输入数量
			gOptions.AuxOutCount = 4;//自定义输出数量
		}
#endif
	}
	else if(gOptions.ACPanelFunOn== 1)
	{
		if(gOptions.LockCount == 1)//门数量  n为门数量为n,一般n为1,2或4
		{
			gOptions.MachineType = C3_100;
			printf("MachineType is C3_100.\n");
			gOptions.ReaderCount = 2;
			gOptions.AuxInCount = 0;//自定义输入数量
			gOptions.AuxOutCount = 1;//自定义输出数量
		}
		else if(gOptions.LockCount == 2)//门数量  n为门数量为n,一般n为2或4
		{
			gOptions.MachineType = C3_200;
			printf("MachineType is C3_200.\n");
			gOptions.ReaderCount = 4;
			gOptions.AuxInCount = 2;//自定义输入数量
			gOptions.AuxOutCount = 2;//自定义输出数量
		}
		else  if(gOptions.LockCount == 4 && gOptions.Door4ToDoor2 != 1)
		{
			gOptions.MachineType = C3_400;
			printf("MachineType is C3_400.\n");
			gOptions.LockCount = 4;
			gOptions.ReaderCount = 4;
			gOptions.AuxInCount = 4;//自定义输入数量
			gOptions.AuxOutCount = 4;//自定义输出数量
		}
		else  if(gOptions.LockCount == 4 && gOptions.Door4ToDoor2 == 1)//c3_400用作c3_200双门双向应用
		{
			gOptions.MachineType = C3_400To_200;
			printf("MachineType is C3_400 to C3_200.\n");
			gOptions.LockCount = 2;
			gOptions.ReaderCount = 4;
			gOptions.AuxInCount = 4;//自定义输入数量
			gOptions.AuxOutCount = 4;//自定义输出数量
		}
		else
		{
			gOptions.MachineType = UNKNOWNMACHINE;
			printf("MachineType is UNKNOWNMACHINE.\n");
		}
	}

	SaveMCUVer();
#ifndef MACHINETYPE_C4
	printf("Option_cfg DeviceID= %d\n",gOptions.DeviceID);
	GetOptions_DeviceID();
	printf("ByMcu Get DeviceID= %d\n",gOptions.DeviceID);
#endif
	return 1;
}

void TruncOptionAndSaveAs(char *buffer, int size)
{
	char tmp[80];

	GetEnvFilePath("USERDATAPATH", "options.cfg", tmp);

	if (fdOptions > 0) close(fdOptions);
    	fdOptions = open(tmp, O_RDWR|O_TRUNC|O_SYNC, S_IRWXU|S_IRWXG|S_IRWXO);
	if (buffer!=NULL)
	    write(fdOptions, buffer, size);
	close(fdOptions);
	fdOptions = open(tmp, O_RDWR|O_CREAT|O_SYNC, S_IRWXU|S_IRWXG|S_IRWXO);
	//flush the cached data to disk
	sync();
}

static char gBuffer[VALUE_BUFFERCACHE+1];
static int gPosition=0;

char *strCache(char *value)
{
       char *p;
       int len;

       len=strlen(value);
       p=gBuffer;
       if ((gPosition+len)>=VALUE_BUFFERCACHE) gPosition=0;
       p+=gPosition;
       memcpy(p, value, len+1);
       gPosition+=len+1;
       return p;
}

char *LoadStrOld(const char *name)
{
       char tmp[VALUE_BUFFERLEN];

       return (LoadStr(name, tmp)?strCache(tmp):NULL);
}

char* LoadStrByIDDef(int ID, const char *DefStr)
{
	char tmp[VALUE_BUFFERLEN];
	char *p;

       	ln[0]=CurLanguage;
       	sprintf(ln+1,"/_%d_",ID);

       	if (LoadStrFromFile(fdLanguage, ln, tmp, FALSE, 0) != -1)
		p = strCache(tmp);
	else
		p = NULL;
	if (p==NULL)
	{
                if(ln[1]=='/')
                {
        	       SelectLanguage(LanguageEnglish);
                        ln[0]='E';
       			if (LoadStrFromFile(fdLanguage, ln, tmp, FALSE, 0) != -1)
				p = strCache(tmp);
			else
				p = NULL;
        		SelectLanguage(gOptions.Language);
                }
                if(p==NULL)
		{
       			if (LoadStrFromFile(fdLanguage, ln+2, tmp, FALSE, 0) != -1)
				p = strCache(tmp);
			else
				p = NULL;

		}
                if(p==NULL)
				{
					if(DefStr)
						p=(char*)DefStr;
					else
						p=(char*)ln;
				}
	}
	return p;

//       return ((LoadStrFromFile(fdLanguage, ln, tmp, FALSE, 0)!=-1)?strCache(tmp):NULL);
}
char* LoadStrByID(int ID)
{
//	printf("LoadStrByID ID=%d\n",ID);

	return LoadStrByIDDef(ID, NULL);
}

char* LoadStrByIDPadDef(int ID, int Len, const char *DefStr)
{
	char *p;
	int i;

	p=LoadStrByIDDef(ID, DefStr);
	memset(ln,' ',Len); ln[Len]=0;
	if(p)
	{
		for(i=0;i<Len;i++)
		{
			if(p[i]==0) break;
			ln[i]=p[i];
		}
	}
	return ln;
}

char* LoadStrByIDPad(int ID, int Len)
{
	return LoadStrByIDPadDef(ID, Len, NULL);
}

char* GetYesNoName(int Yes)
{
	if(Yes) return LoadStrByID(HID_YES); else return LoadStrByID(HID_NO);
}

static char SMSBuf[100];

char *GetSMS(int UserID)
{
        int i, id;
        char *p;
        for(i=0;i<=100;i++)
        {
                sprintf(SMSBuf, "SMS%d", i);
                p=LoadStrOld(SMSBuf);
                if(p && *p)
                {
                        id=(Hex2Char(p)<<12)+(Hex2Char(p+1)<<8)+(Hex2Char(p+2)<<4)+Hex2Char(p+3);
                        if(UserID==id)
                        {
                                memset(SMSBuf, 0, 100);
                                return nstrcpy(SMSBuf, p+5, 100);
                        }
                }
        }
        return NULL;
}

int ClearAllACOpt(int All)
{
	char name[20], *Buffer;
	int i, oldsize, size;
	char p[1024];

	size=lseek(fdOptions, 0, SEEK_END);
	Buffer=(char*)malloc(size);
	lseek(fdOptions, 0, SEEK_SET);
	if (read(fdOptions, Buffer, size)!=size)
	{
	    free(Buffer);
	    return FALSE;
	}

	size=PackStrBuffer(Buffer, "NONE", size);
	oldsize=size;
	//清除分组设置
	for(i=1;i<10;i++)
	{
		sprintf(name,"GRP%d", i);
		size=PackStrBuffer(Buffer, name, size);
	}

	//清除开锁组合
	if(All)
		size=PackStrBuffer(Buffer, "ULG", size);

	//清除时间段设置
	if(All)
	    for(i=1;i<=50;i++)
	    {
	        sprintf(name,"TZ%d", i);
		 size=PackStrBuffer(Buffer, name, size);
	    }

	//清除组时间段设置
	if(All)
	    for(i=1;i<=50;i++)
	    {
	       sprintf(name,"GTZ%d", i);
		size=PackStrBuffer(Buffer, name, size);
	    }

	//Clear 用户时间段设置
	for(i=1;i<65535;i++)
	{
		sprintf(name,"UTZ%d", i);
		if(LoadStr(name, p))
			size=PackStrBuffer(Buffer, name, size);
	}
	if(oldsize!=size)
	{
		TruncOptionAndSaveAs(Buffer, size);
	}
	free(Buffer);
	return TRUE;
}

int ClearOptionItem(char *name)
{
       int size, orisize;
       char *Buffer;

       size=lseek(fdOptions, 0, SEEK_END);
       Buffer=(char*)malloc(size);
       lseek(fdOptions, 0, SEEK_SET);
       if (read(fdOptions, Buffer, size)!=size)
       {
	       free(Buffer);
	       return FALSE;
       }
       orisize=size;

       size=PackStrBuffer(Buffer, name, size);

       if(orisize!=size)
       {
	       TruncOptionAndSaveAs(Buffer, size);
       }
       free(Buffer);
       return TRUE;
}

//Language
void SelectLanguage(char Language)
{
	char buffer[128];
	char *tmp;

	if ((tmp=LoadStrOld("FONTFILEPATH"))!=NULL)
		sprintf(buffer, "%s%s.%c", tmp, "LANGUAGE", Language);
	else
		sprintf(buffer, "%s.%c", "/mnt/mtdblock/res/LANGUAGE", Language);
	if (Language!=CurLanguage)
	{
		if (fdLanguage > 0) close(fdLanguage);
		fdLanguage = open(buffer, O_RDONLY);

		printf("------------------fdLanguage=%d----------------%s\n",fdLanguage,buffer);

		CurLanguage = Language;
	}
}
/*
const char *localeStr(const char *str)
{
	char *p;
	if(strs==NULL)
	{
		char fname[80];
		strs=slCreate(":=");
		sprintf(fname, "res/strings-%s.txt", optGetStr("Language","EN"));
		printf("%s\n", fname);
		if(slLoadFromFile(strs, fname)<=0)
		{
			slLoadFromFile(strs, "res/strings.txt");
		}
	}
	p=slGetValue(strs, str);
	if(p==NULL)
	{
		return str;
	}
	return p;
}
*/


int GetLocaleID(int fd, int LngID)
{
	char *p, buf[]="E/_0_";
	char tmp[VALUE_BUFFERLEN];

	buf[0]=LngID;
	p=((LoadStrFromFile(fd, buf, tmp, FALSE, 0)!=-1)?strCache(tmp):NULL);
	if(p)
                return str2int(p,LID_INVALID);
	else
		return -2;
}

int GetDefaultLocaleID(void)
{
        return GetLocaleID(fdLanguage, gOptions.Language);
}

char *GetLangName(char LngID)
{
        char *p, buf[]="E/_0_";
	int fdTmp;
	char path[128];
	char value[VALUE_BUFFERLEN];
	char *tmp;

	buf[0]=LngID;
	//该资源的语言与当前的系统语言一致,则取本地化的语言名称，否则取英语名称
	if(CurLanguage==LngID)
	{
                buf[3]='1';
		p=((LoadStrFromFile(fdLanguage, buf, value, FALSE, 0)!=-1)?strCache(value):NULL);
		//English name
		if(p==NULL)
		{
			buf[3]='2';
			p=((LoadStrFromFile(fdLanguage, buf, value, FALSE, 0)!=-1)?strCache(value):NULL);
		}
	}
	else
	{
		buf[3]='2';
		if ((tmp=LoadStrOld("FONTFILEPATH"))!=NULL)
			sprintf(path, "%s%s.%c", tmp, "LANGUAGE", LngID);
		else
			sprintf(path, "%s.%c", "LANGUAGE", LngID);
		fdTmp=open(path, O_RDONLY);
		if(fdTmp==-1)
			p=NULL;
		else
		{
			p=((LoadStrFromFile(fdTmp, buf, value, FALSE, 0)!=-1)?strCache(value):NULL);
			if(p==NULL)
			{
				buf[3]='1';
				p=((LoadStrFromFile(fdTmp, buf, value, FALSE, 0)!=-1)?strCache(value):NULL);
			}
			close(fdTmp);
		}
	}
	return p;
}

int GetSupportedLang(int *LngID, int MaxLngCnt)
{
	DIR *dir;
        struct dirent *entry;
	char *filename;
	int LngCnt=0;
	char path[128];
	char *tmp;

	if ((tmp=LoadStrOld("FONTFILEPATH"))!=NULL)
		sprintf(path, "%s", tmp);
	else
		sprintf(path, "./");
        dir=opendir(path);
        if(dir)
        {
		while((LngCnt <= MaxLngCnt)&&((entry=readdir(dir))!=NULL))
		{
			filename=entry->d_name;
			if((strlen(filename)==10)&&(strncmp(filename, "LANGUAGE.", 9)==0))
			{
				LngID[LngCnt++]=filename[9];
			}
		}
		closedir(dir);
		dir=0;
	}
        return LngCnt;
}

BOOL UpdateNetworkInfoByDHCP(char *dhcp)
{
	FILE *fp;
	char buffer[1024];
	char tmp[128];
	char *name, *value;
	int len, i;
	char OpName[128];
	BOOL bSign=FALSE;

	if((fp=fopen(dhcp, "rb"))==NULL) return FALSE;
	while(!feof(fp))
	{
		memset(buffer, 0, 1024);
		if(!fgets(buffer, 1024, fp)) break;
		i=0;
		name=buffer;
		value=NULL;
		while(buffer[i])
		{
			if(buffer[i]=='=')
			{
				buffer[i]='\0';
				value=buffer+i+1;
				//trunc the CR
				i=0;
				while(value[i])
				{
					if((value[i]=='\r')||(value[i]=='\n'))
					{
						value[i]='\0';
						break;
					}
					i++;
				}
				TrimRightStr(value);
				break;
			}
			i++;
		}
		//OK, we get a valid line
		if(value)
		{
			memset(OpName, 0, 128);
			if(strcmp(name, "ip")==0)
			{
				strcpy(OpName, "IPAddress");
				str2ip(value, gOptions.IPAddress);
			}
			else if(strcmp(name, "router")==0)
			{
				strcpy(OpName, "GATEIPAddress");
				str2ip(value, gOptions.GATEIPAddress);
			}
			else if(strcmp(name, "subnet")==0)
			{
				strcpy(OpName, "NetMask");
				str2ip(value, gOptions.NetMask);
			}
			if(OpName[0])
			{
				//Check OpName
				if(LoadStr(OpName, tmp))
				{
					//the value is the same as old value, then return.
					if (0==strcmp(value, tmp)) continue;
				}
				CombineNameAndValue(OpName, value, TRUE, tmp);
				len=lseek(fdOptions, 0, SEEK_END);
				if (len>=MAX_OPTION_SIZE)
				{
					ClearOptionItem("NONE");
					len=lseek(fdOptions, 0, SEEK_END);
				}
				if (len<MAX_OPTION_SIZE)
					write(fdOptions, tmp, strlen(tmp));
				bSign=TRUE;
			}
		}
	}
	fclose(fp);
	return bSign;
}

#ifndef URU
#ifndef UPEK
int TestEEPROM(BYTE *data, int size)
{
        BYTE Buffer[1024];
        memset(Buffer, 0, 1024);
        if(0==Read24WC02(0,Buffer,size))
        if(0==Read24WC02(0,Buffer,size))
        if(0==Read24WC02(0,Buffer,size))
                return -1;
        if(nmemcmp(Buffer, data, size))
                return 1;
        else
                return 0;
}

int CheckSensorData(short *data)
{
        int i, sum;
        sum=1;
        for(i=0;i<14;i++) sum+=data[i];
        if(data[14]!=(sum & 0x7FFF)) return FALSE;
        if((data[2]<50) || (data[3]<50) ||(data[2]>CMOS_WIDTH) || (data[3]>CMOS_HEIGHT) ||
                  (data[12]<50) || (data[13]<50)) //是否合法的数据
                  return FALSE;
        return TRUE;
}

int ReadSensorOptions(POptions opts)
{
        short data[15];
        int i=10;
        while(i--)
        if(Read24WC02(0,(BYTE*)data,sizeof(data)))
	{
		DBPRINTF("Read data from EEPROM OK!\n");
		for(i=0;i<14;i++)
			DBPRINTF("data[%d]=%d\n", i, data[i]);
                if(!CheckSensorData(data))
                {
                        //不合法的数据则重写
                        return 0;//WriteSensorOptions(opts, TRUE);
                }
                else
                {
                        opts->OLeftLine         =data[0];
                        opts->OTopLine         	=data[1];
                        opts->OImageWidth       =data[2];
                        opts->OImageWidth 	=((opts->OImageWidth+2)/4)*4;
                        opts->OImageHeight     	=data[3];

			opts->ZF_WIDTH          =data[12]&0xFFF;
                        opts->ZF_WIDTH		=((opts->ZF_WIDTH+2)/4)*4;
                        opts->ZF_HEIGHT         =data[13]&0xFFF;
			gOptions.NewFPReader	=(data[12]>>12) + ((data[13]>>12)<<4);

			opts->CPY[0]    =data[8];
			opts->CPY[1]    =data[9];
			opts->CPY[2]    =data[10];
			opts->CPY[3]    =data[11];

			opts->CPX[0] 	=data[5];
			opts->CPX[1]    =data[4];
			opts->CPX[2]    =data[7];
			opts->CPX[3]    =data[6];
                        return 1;
                }
        }
        else
                return 0;
}

int EEPROMWriteOpt(BYTE * data, int size, int Rewrite)
{
        int i;
        if(Rewrite)
                i=1;
        else
                i=TestEEPROM(data, size);
        if(i==-1)
                return FALSE;
        else if(i==0)
                return TRUE;
        else
        {
                i=10;
                while(i--)
                {
			if(size<=16)
			{
                        	Write24WC02(0, data, size);
			}else
			{
                        	Write24WC02(0, data, 16);
				Write24WC02(16,data+16,size-16);
			}
                        if(TestEEPROM(data, size)==0)
                                return TRUE;
                }
                return FALSE;
        }
}

int WriteSensorOptions(POptions opts, int Rewrite)
{
	short data[15];
        int i, sum;
        data[0] =opts->OLeftLine;
        data[1] =opts->OTopLine;
        data[2] =opts->OImageWidth;
        data[3] =opts->OImageHeight;
        data[5] =opts->CPX[0];
        data[4] =opts->CPX[1];
        data[7] =opts->CPX[2];
        data[6] =opts->CPX[3];
        data[9] =opts->CPY[0];
        data[8] =opts->CPY[1];
        data[11]=opts->CPY[2];
        data[10]=opts->CPY[3];
        data[12]=((gOptions.NewFPReader&0x0F)<<12)+opts->ZF_WIDTH;
        data[13]=(((gOptions.NewFPReader&0xF0)>>4)<<12)+opts->ZF_HEIGHT;
        sum=1;
        for(i=0;i<14;i++) sum+=data[i];
        data[14]=(sum & 0x07FFF);
	DBPRINTF("Write data to EEPROM OK!\n");
	for(i=0;i<14;i++)
		DBPRINTF("data[%d]=%d\n", i, data[i]);
        return EEPROMWriteOpt((BYTE*)data, sizeof(data), Rewrite);
}
#endif
#endif

char * macformat(char *str, BYTE *value)
{
	sprintf(str,"%02X:%02X:%02X:%02X:%02X:%02X", value[0],value[1],value[2],value[3],value[4],value[5]);
	return str;
}

char * ipformat(char *str, BYTE *value)
{
	sprintf(str,"%d.%d.%d.%d",value[0],value[1],value[2],value[3]);
	return str;
}

TOptionsResStr OptionsResStr[]={
	//配置名称	长度	缺省值				是否需要恢复出厂设置
	{"MAC",		6,	{0x00,0x17,0x61,0x09,0x11,0x23},0,	optoffset(MAC),		str2mac,	macformat	},
	{"CardKey",	6,	{0xFF,0xFF,0xFF,0xFF,0xFF,0xFF},0,	optoffset(CardKey),	str2cardkey,	GetCardKeyStr	},
	{"IPAddress",	4,	{192,168,1,201},		1,	optoffset(IPAddress),	str2ip,		ipformat	},
//	{"GATEIPAddress",4,	{192,168,1,201},			1,	optoffset(GATEIPAddress),str2ip,	ipformat	},
	{"GATEIPAddress",4,	{0,0,0,0},			1,	optoffset(GATEIPAddress),str2ip,	ipformat	},
	{"NetMask",	4,	{255,255,255,0},		1,	optoffset(NetMask),	str2ip,		ipformat	},
	{"AuthServerIP",4,	{0,0,0,0},			1,	optoffset(AuthServerIP),str2ip,		ipformat	},
	{"WebServerIP",	4,	{0,0,0,0},			1,	optoffset(WebServerIP),	str2ip,		ipformat	},
	{"TimeServerIP",4,	{0,0,0,0},			1,	optoffset(TimeServerIP),str2ip,		ipformat	},
	{"ProxyServerIP",4,	{0,0,0,0},			1,	optoffset(ProxyServerIP),str2ip,	ipformat	},
	{"ComPwd",16,	{0},					1,	optoffset(ComPwd),NULL,	NULL	},
	////协迫密码
	{"Door1ForcePassWord",8,	{0},			0,	optoffset(Door1ForcePassWord),NULL,	NULL	},
	{"Door2ForcePassWord",8,	{0},			0,	optoffset(Door2ForcePassWord),NULL,	NULL	},
	{"Door3ForcePassWord",8,	{0},			0,	optoffset(Door3ForcePassWord),NULL,	NULL	},
	{"Door4ForcePassWord",8,	{0},			0,	optoffset(Door4ForcePassWord),NULL,	NULL	},

	////特权（紧急）密码
	{"Door1SupperPassWord",8,	{0},			0,	optoffset(Door1SupperPassWord),NULL,	NULL	},
	{"Door2SupperPassWord",8,	{0},			0,	optoffset(Door2SupperPassWord),NULL,	NULL	},
	{"Door3SupperPassWord",8,	{0},			0,	optoffset(Door3SupperPassWord),NULL,	NULL	},
	{"Door4SupperPassWord",8,	{0},			0,	optoffset(Door4SupperPassWord),NULL,	NULL	},
	{"FirmVer",32,	{0},			0,	optoffset(FirmVer),NULL,	NULL	},
	{"MainMCUVer",10,	{0},			0,	optoffset(MainMCUVer),NULL,	NULL	},
	{"SubMCUVer1",10,	{0},			0,	optoffset(SubMCUVer1),NULL,	NULL	},
	{"SubMCUVer2",10,	{0},			0,	optoffset(SubMCUVer2),NULL,	NULL	},
	{"SubMCUVer3",10,	{0},			0,	optoffset(SubMCUVer3),NULL,	NULL	},
	{"SubMCUVer4",10,	{0},			0,	optoffset(SubMCUVer4),NULL,	NULL	}
};

TOptionsResInt OptionsResInt[]={
	//配置名称		缺省值			是否需要恢复出厂设置		菜单项资源 		最大值		最小值
	{"~ML",			1,			0,	optoffset(MultiLanguage),	0,			1,		0 	},
	{"Language",		LanguageSimplifiedChinese,0,	optoffset(Language),		MID_OS_LANGUAGE,	255,		32+1 	},
	{"DeviceID",		0,			0,	optoffset(DeviceID),		MID_OS_DEVNUMBER,	255,		1 	},
	{"MThreshold",		55,			1,	optoffset(MThreshold),								},
	{"EThreshold",		45,			0,	optoffset(EThreshold),								},
	{"VThreshold",		35,			1,	optoffset(VThreshold),								},
	{"LastAttLo",		0,			1,	optoffset(LastAttLog),								},
	{"UDPPort",		0x1112,			0,	optoffset(UDPPort),								},
	{"TCPPort",		0x1112,			0,	optoffset(TCPPort),								},
	{"OImageWidth",		404,			0,	optoffset(OImageWidth),		0,			CMOS_WIDTH,	200 	},
	{"OImageHeight",	300,			0,	optoffset(OImageHeight),	0,			CMOS_HEIGHT,	200 	},
	{"OTopLine",		40,			0,	optoffset(OTopLine),		0,			CMOS_HEIGHT,	0 	},
	{"OLeftLine",		144,			0,	optoffset(OLeftLine),		0,			CMOS_WIDTH,	0 	},
	{"CPX0",		377,			0,	optoffset(CPX[0]), 								},
	{"CPX1",		28,			0,	optoffset(CPX[1]),								},
	{"CPX2",		424,			0,	optoffset(CPX[2]),								},
	{"CPX3",		-20,			0,	optoffset(CPX[3]),								},
	{"CPY0",		300,			0,	optoffset(CPY[0]),								},
	{"CPY1",		300,			0,	optoffset(CPY[1]),								},
	{"CPY2",		0,			0,	optoffset(CPY[2]),								},
	{"CPY3",		0,			0,	optoffset(CPY[3]),								},
	{"ZF_WIDTH",		276,			0,	optoffset(ZF_WIDTH),								},
	{"ZF_HEIGHT",		294,			0,	optoffset(ZF_HEIGHT),								},
	{"MSpeed",		MSPEED_AUTO,		0,	optoffset(MSpeed),								},
	{"AttState",		0,			1,	optoffset(AttState),								},
	{"~MaxUserCount",	300,			1,	optoffset(MaxUserCount),							},
	{"~MaxAttLogCount",	10,			1,	optoffset(MaxAttLogCount),							},
	{"~MaxFingerCount",	8,			1,	optoffset(MaxFingerCount),							},
	{"LockOn",		150,			1,	optoffset(LockOn),		MID_OS_LOCK,		500,		0 	},
	{"AlarmAttLog",		99,			1,	optoffset(AlarmAttLog),								},
	{"AlarmOpLog",		99,			1,	optoffset(AlarmOpLog),								},
	{"AlarmReRec",		0,			1,	optoffset(AlarmReRec),								},
	{"RS232BaudRate",	38400,			1,	optoffset(RS232BaudRate),	0,			115200,		300	},
	{"RS232CRC",		0,			0,	optoffset(RS232CRC),								},
	{"RS232Stop",		1,			0,	optoffset(RS232Stop),								},
	{"WEBPort",		80,			0,	optoffset(WEBPort),								},
	{"~ShowState",		0,			0,	optoffset(ShowState),								},
	{"~KeyLayout",		KeyLayout_BioClockIII,	0,	optoffset(KeyLayout),								},
	{"VoiceOn",		1,			1,	optoffset(VoiceOn),		HMID_VOICEON,		1,		0	},
	{"AutoPowerOff",	0xFFFF,			1,	optoffset(AutoPowerOff),	MID_OSA_POWEROFF				},
	{"AutoPowerOn",		0xFFFF,			1,	optoffset(AutoPowerOn),		MID_OSA_POWERON					},
	{"AutoPowerSuspend",	0xFFFF,			1,	optoffset(AutoPowerSuspend),	MID_OSA_SUSPEND					},
        {"IdlePower",		HID_SUSPEND,		1,	optoffset(IdlePower),MID_OSA_IDLE,HID_SUSPEND,	HID_POWEROFF},
        {"IdleMinute",		3,			1,	optoffset(IdleMinute),		MID_OSA_IDLETIME				},
        {"ShowScore",		0,			1,	optoffset(ShowScore),		HMID_SHOWSCORE,		1,		0	},
        {"NetworkOn",		1,			1,	optoffset(NetworkOn),		MID_OC_NETOFF,		1,		0	},
        {"RS232On",		1,			1,	optoffset(RS232On),		MID_OC_RS232OFF,	1,		0	},
        {"RS485On",		1,			1,	optoffset(RS485On),		MID_OC_RS485OFF,	1,		0	},
	{"~NetworkFunOn",	1,			0,	optoffset(NetworkFunOn),	MID_OI_NET,		1,		0	},
	{"~LockFunOn",		1,			0,	optoffset(LockFunOn),								},
	{"~RFCardOn",		1,			0,	optoffset(RFCardFunOn),								},
	{"~One2OneFunOn",	1,			0,	optoffset(One2OneFunOn),							},
	{"~PowerMngFunOn",	1,			0,	optoffset(PowerMngFunOn),	MID_OI_POWERMNG,	1,		0	},
	{"~NewFPReader",	0,			0,	optoffset(NewFPReader),								},
	{"~ShowName",		1,			0,	optoffset(ShowName),		0,			1,		0	},
	{"UnlockPerson",	1,			1,	optoffset(UnlockPerson),							},
	{"ShowCheckIn",		0,			0,	optoffset(ShowCheckIn),								},
	{"OnlyPINCard",		1,			1,	optoffset(OnlyPINCard),		MID_OC_PINCARD,		1,		0	},
	{"~IsTestMachine",	0,			1,	optoffset(IsTestMachine),},
	{"~MustChoiceInOut",	0,			0,	optoffset(MustChoiceInOut),},
	{"HiSpeedNet",		8,			1,	optoffset(HiSpeedNet),	},
	{"~MenuStyle",		1,			0,	optoffset(MenuStyle),	},
	{"CCCKey",		1,			0,	optoffset(CanChangeCardKey),	},
	{"Must1To1",		0,			1,	optoffset(Must1To1),		MID_OS_MUST1TO1,	1,		0	},
	{"LCDM",		0,			0,	optoffset(LCDModify),								},
	{"COMKey",		0,			1,	optoffset(ComKey),								},
	{"MustEnroll",		1,			1,	optoffset(MustEnroll),		MID_OC_MUSTENROLL				},
	{"TOMenu",		60,			0,	optoffset(TimeOutMenu),		0,			65535*32768,	10	},
	{"TOPin",		10,			0,	optoffset(TimeOutPin),		0,			65535,		5	},
	{"TOState",		10,			0,	optoffset(TimeOutState),							},
	{"SaveAttLog",		1,			0,	optoffset(SaveAttLog),								},
	{"RS232Fun",		1,			0,	optoffset(RS232Fun),								},
        {"~IsModule",		0,			0,	optoffset(IsModule),								},
	{"~ShowSecond",		0,			0,	optoffset(ShowSecond),								},
	{"~RFSStart",		0,			0,	optoffset(RFCardSecStart),							},
	{"~RFSLen",		10,			0,	optoffset(RFCardSecLen),							},
	{"~RFFPC",		1,			0,	optoffset(RFCardFPC),								},
	{"~PIN2Width",		5,			0,	optoffset(PIN2Width),		0,			10,		5	},
	{"DtFmt",		0,			1,	optoffset(DateFormat),								},
	{"~OPLM1",		-1,			0,	optoffset(OPLogMask1),								},
	{"~OPLM2",		0,			0,	optoffset(OPLogMask2),								},
	{"~DC",			3,			0,	optoffset(DelayCount),								},
	{"~IncThr",		14,			0,	optoffset(IncThr),								},
	{"~TopThr",		50,			0,	optoffset(TopThr),								},
	{"~MinThr",		30,			0,	optoffset(MinThr),								},
	{"NoiseThreshold",	100,			0,	optoffset(MaxNoiseThr),								},
	{"~MinM",		12,			0,	optoffset(MinMinutiae),								},
	{"~MaxTL",		MAXVALIDTMPSIZE,	0,	optoffset(MaxTempLen),								},
	{"AdminCnt",		1,			0,	optoffset(AdminCnt),								},
	{"ODD",			10,			1,	optoffset(OpenDoorDelay),							},
	{"DSM",			2,			1,	optoffset(DoorSensorMode),							},
	{"ECnt",		3,			0,	optoffset(EnrollCount),		0,			10,		3	},
	{"~AAFO",		0,			0,	optoffset(AutoAlarmFunOn),							},
	{"AADelay",		10,			1,	optoffset(AutoAlarmDelay),							},
	{"~ASFO",		0,			0,	optoffset(AutoStateFunOn),							},
	{"DUHK",		0,			1,	optoffset(DuressHelpKeyOn),	MID_AD_DURESSHELP,	1,		0	},
	{"DU11",		0,			1,	optoffset(Duress1To1),		MID_AD_DURESS11,	1,		0	},
	{"DU1N",		0,			1,	optoffset(Duress1ToN),		MID_AD_DURESS1N,	1,		0	},
	{"DUPWD",		0,			1,	optoffset(DuressPwd),		MID_AD_DURESSPWD,	1,		0	},
	{"DUAD",		10,			1,	optoffset(DuressAlarmDelay),							},
	{"LockPWRButton",	0,			1,	optoffset(LockPowerButton),	0,			1,		0	},
	{"SUN",			3,			0,	optoffset(StartUpNotify),							},
	{"I1NFrom",		0,			1,	optoffset(I1ToNFrom),								},
	{"I1NTo",		0,			1,	optoffset(I1ToNTo),								},
	{"I1H",			0,			1,	optoffset(I1ToH),		MID_OS_1TOH		,1		,0	},
	{"I1G",			0,			1,	optoffset(I1ToG),		MID_OS_1TOG		,1		,0	},
	{"~MaxUserFingerCount",	10,			1,	optoffset(MaxUserFingerCount),							},
	{"~MIFARE",		0,			0,	optoffset(IsSupportMF),								},
	{"~FlashLed",		1,			0,	optoffset(IsFlashLed),								},
	{"~IsInit",		1,			0,	optoffset(IsInit),								},
	{"CMOSGC",		0,			0,	optoffset(CMOSGC),		0,			255,		0	},
	{"~ADMATCH",		0,			0,	optoffset(AdvanceMatch),},
	{"ERRTimes",		0,			1,	optoffset(ErrTimes),		MID_AD_ERRPRESS,	255,		0       },
	{"~IsOnlyOneSensor",	1,			1,	optoffset(IsOnlyOneSensor),	0,			1,		0	},
	{"AuthServerEnabled",	0,			1,	optoffset(AuthServerEnabled),							},
	{"ConnectMODEM",	0,			1,	optoffset(IsConnectModem),	0,			1,		0	},
	{"AATimes",		2,			1,	optoffset(AutoAlarmTimes),							},
	{"DNSCheckTime",	0,			1,	optoffset(DNSCheckInterval),							},
	{"AutoUPLogTime",	0,			1,	optoffset(AutoUploadAttlog),							},
	{"DisableUser",		0,			1,	optoffset(DisableNormalUser),	0,			1,		0 	},
	{"KeyPadBeep",		0,			1,	optoffset(KeyPadBeep),		0, 			1,		0	},
	{"WorkCode",		0,			1,	optoffset(WorkCode),		0,			10,		0	},
#ifdef ZEM300
	{"VOLUME",		67,			1,	optoffset(AudioVol), 		0,			99,		1 	},
	{"AAVOLUME",		67,			1,	optoffset(AutoAlarmAudioVol),	0,			99,		1 	},
#else
	{"VOLUME",		34,			1,	optoffset(AudioVol), 		0,			99,		1 	},
	{"AAVOLUME",		34,			1,	optoffset(AutoAlarmAudioVol),	0,			99,		1 	},
#endif
	{"DHCP",		0,			1,	optoffset(DHCP),		0,			1,		0	},
	{"AutoSyncTime",	0xFFFF,			1,	optoffset(AutoSyncTime),							},
	{"~IsOnlyRFMachine",	1,			0,	optoffset(IsOnlyRFMachine),	0,			1, 		0 	},
	{"~OS",			1,			0,	optoffset(OS),			0,			255, 		0 	},
	{"~IsWiegandKeyPad",	0,			0,	optoffset(IsWiegandKeyPad),	0,			1, 		0 	},
	{"~SMS",		0,			0,	optoffset(IsSupportSMS),	0,			1, 		0 	},
	{"~USBDisk",		1,			0,	optoffset(IsSupportUSBDisk),	0,			1, 		0 	},
	{"~MODEM",		1,			0,	optoffset(IsSupportModem),	0,			3, 		0 	},
	{"~AuthServer",		0,			0,	optoffset(IsSupportAuthServer),	0,			1, 		0 	},
	{"~ACWiegand",		0,			0,	optoffset(IsACWiegand),		0,			1, 		0 	},
	{"~ExtendFmt",		0,			0,	optoffset(AttLogExtendFormat),	0,			1, 		0 	},
	{"~DRPass",		0,			0,	optoffset(DisableRootPassword),	0,			1, 		0 	},
	{"~MP3",		0,			0,	optoffset(IsSupportMP3),	0,			1, 		0 	},
	{"~MIFAREID",		0,			0,	optoffset(MifareAsIDCard),	0,			10, 		0 	},
	{"~GroupVoice",		0,			0,	optoffset(PlayGroupVoice),	0,			1, 		0 	},
	{"~TZVoice",		0,			0,	optoffset(PlayTZVoice),		0,			1, 		0 	},
	{"~ASTFO",		0,			0,	optoffset(AutoSyncTimeFunOn),	0,			1, 		0 	},
	{"~CFO",		0,			0,	optoffset(CameraFunOn),		0,			1, 		0 	},
	{"~SaveBitmap",		0,			0,	optoffset(SaveBitmap),		0,			1, 		0 	},
	{"~ProcessImage",	0,			0,	optoffset(ProcessImage),	0,			1, 		0 	},
	{"ASTimeOut",		10,			0,	optoffset(AuthServerTimeOut),	0,			30, 		0 	},
	{"~TLLCM",		0,			0,	optoffset(TwoLineLCM),		0,			1, 		0 	},
	{"~UserExtFmt",		0,			0,	optoffset(UserExtendFormat),	0,			1, 		0 	},
	{"RefreshUserData",	0,			1,	optoffset(RefreshUserData),							},
	{"~DisableAU",		0,			0,	optoffset(DisableAdminUser),							},
	{"~ICFO",		0,			0,	optoffset(IClockFunOn),								},
	{"ProxyServerPort",	0,			1,	optoffset(ProxyServerPort),							},
	{"~C2",			0,			0,	optoffset(IsSupportC2),								},
	{"EnableProxyServer",	0,			1,	optoffset(EnableProxyServer),							},
	{"~WCFO",		0,			0,	optoffset(WorkCodeFunOn),							},
	{"~VALF",		0,			0,	optoffset(ViewAttlogFunOn),	0,			10,		0	},
	{"~DHCPFunOn",		0,			0,	optoffset(DHCPFunOn),		0,			1,		0 	},
	{"~OutSensorFunOn",	0,			0,	optoffset(OutSensorFunOn),	0,			1,		0 	},
	{"SaveAuthServerLog", 	0,			1,	optoffset(SaveAuthServerLog),	0,			1,		0 	},
	{"SetGatewayWaitCount", 30,			1,	optoffset(SetGatewayWaitCount),	0,			65535*32768,	0 	},
	{"~HID", 		0,			0,	optoffset(IsSupportHID),	0,			1,		0 	},
	{"DoorSensorTimeout", 	30,			1,	optoffset(DoorSensorTimeout),	0,			99,		0 	},
	{"DAM", 		0,			1,	optoffset(DoorAlarmMode),							},
	{"~ASDewarpedImage",0,                          0,     	optoffset(ASDewarpedImage),     0,			1,		0	},
	{"Nideka",		0,			0,	optoffset(Nideka), 		0,			1,		0	},
	{"~PrinterFunOn",       0,            		0,      optoffset(PrinterFunOn),	0,			0			},
	{"PrinterOn",           0,             		1,      optoffset(PrinterOn), 		MID_PRINTERON, 		10, 		0 	},
	{"CheckOutWc",		0,			0,	optoffset(CheckOutWc), 		0,			1,		0	},
	{"SyncTmAuthServer",    30,      		0,  	optoffset(SyncTimeFromAuthServer),						},
        {"FPOpenRelay", 	1,                      0,      optoffset(FPOpenRelay),		0,                 	1,              0	},
        {"AutoOpenRelay",       0,                      0,      optoffset(AutoOpenRelay),	0,			1,              0	},
        {"AutoOpenRelayTimes", 	5,                    	0,      optoffset(AutoOpenRelayTimes),	0,			20,             1	},
        {"~AutoOpenRelayFunOn", 0,                      0,      optoffset(AutoOpenRelayFunOn),	0,			1,              0	},
	{"DefaultGroup",       	1,			0,	optoffset(DefaultGroup), 	0,			5,		1	},
	{"GroupFpLimit",       	200,			0,	optoffset(GroupFpLimit), 	0,			0,		0	},
	{"LimitFpCount",	3200,			0,	optoffset(LimitFpCount),	0,			0,		0	},
	{"WelfareAtt",		0,			0,	optoffset(WelfareAtt),								},
	{"AutoTransLog",	0,			1,	optoffset(AutoTransLog),							},
	{"LatestInTimeAM",	0,			1,	optoffset(LatestInTimeAM),							},
	{"LatestInTimeFM",	0,			1,	optoffset(LatestInTimeFM),							},
	{"AutoIdleTransLog",	0,			1,	optoffset(AutoIdleTransLog),							},
	{"LastSendLogTime",	0,			1,	optoffset(LastSendLogTime),							},
	{"WireLessBell",	0,			0,	optoffset(WireLessBell),	0,			0,		0	},
	{"USB232On",		1,			1,	optoffset(USB232On),		0,			0,		0	},
        {"AuthServerREQ",       3,                      1,      optoffset(AuthServerREQ),       0,			0,		0	},
        {"ModemKeepLive",	0,                      1,      optoffset(ModemKeepLive),       0,                      0,              0       },
        {"AuthServerCheckMode",	0,                      1,      optoffset(AuthServerCheckMode), 0,                      2,              0       },
	{"~DSTF",					0,								0,	optoffset(DaylightSavingTimeFun),0,},
	{"DaylightSavingTimeOn",	0,								1,	optoffset(DaylightSavingTimeOn),MID_DAYLIGHTSAVINGTIMEON,				1		,0},
	{"CurTimeMode",				0,								1,	optoffset(CurTimeMode)		,0,					2		,0},
	{"DaylightSavingTime",		0,								1,	optoffset(DaylightSavingTime),MID_DAYLIGHTSAVINGTIME,},
	{"StandardTime",			0,								1,	optoffset(StandardTime),MID_STANDARDTIME,},
	{"Bell",			0,								0,	optoffset(Bell),0,},
	{"WiegandID",				0,								0,	optoffset(WiegandID),},
	{"~USB232FunOn",		0,			0,	optoffset(USB232FunOn),},
	{"~VFO",		1,			0,	optoffset(VoiceFunOn),								},
	{"ExternalAC",		-1,			0,	optoffset(ExternalAC),MID_AC_EXTERNALAC,		10,	-1},
	{"~RS485Port",		1,			0,	optoffset(RS485Port	)		,},
	{"WavBeepOn",	0,				0,	optoffset(WavBeepOn),},
	{"~ExtWGInFunOn",0,				0,	optoffset(ExtWGInFunOn),			},
	{"~IsLF",1,				0,	optoffset(IsLeaveFactory),				},
	{"PageState",0,				0,	optoffset(PageState),					},
	{"StartState",		-1,			0,	optoffset(StartState),},
	{"WIFI",		0,			0,	optoffset(IsSupportWIFI),	0,			1, 		0 	},//ccc
	{"wifidhcp",		0,			0,	optoffset(wifidhcpfunon),	0,			1, 		0 	},//ccc
	{"AmPmFormatFunOn",             0,              0,      optoffset(AmPmFormatFunOn),             0,      0,      0},
	{"AmPmFormat",          HID_24H,         0,     optoffset(AmPmFormat),   MID_AMPMFORMAT,      HID_24H,      HID_12H},
	{"UnSaveLogDeny",	0,			0,	optoffset(UnSaveLogDeny),	0,			1,		0	},
	{"SAEscortFunOn",	0,			0,	optoffset(SAEscortFunOn),	},
	{"CardkeyKeypad",	0,			0,	optoffset(CardkeyKeypad),},
	{"IsHolidayValid",	1,			1,	optoffset(IsHolidayValid),MID_OA_HOLIDAYVALID,1,0},
	{"~APBFO",		0,			0,	optoffset(AntiPassbackFunOn)		,0,},
	{"AntiPassbackOn",	0,			1,	optoffset(AntiPassbackOn)		,0,4,0},
	{"~MSFO",		0,			0,	optoffset(MasterSlaveFunOn)	,},
	{"MasterSlaveOn",	0,			1,	optoffset(MasterSlaveOn)		,0,},
	{"MasterState",		0,			1,	optoffset(MasterState)	,0,1,-1},
	{"CT232On",		0,		0,		optoffset(CT232On),},
	{"HzImeOn",             0,                      0,      optoffset(HzImeOn),0,                1,              0},
        {"ImeFunOn",            0,                      0,      optoffset(ImeInputFunOn),0,               1,              0},
	{"FKeyFunOn",		0,			0,	optoffset(FKeyFunOn),0,},
	{"SwitchAttStateByTimeZone",	0,	0,		optoffset(SwitchAttStateByTimeZone), 	0,	0,	0},
	{"FKeyOn",		0,			1,	optoffset(FKeyOn),0,},
	{"CardFPAuthServerOn", 	0,			0,	optoffset(CardFPAuthServerOn),	0,	0,	0},
	{"TagLogDoorAction",	0,			0,	optoffset(TagLogDoorAction),0,		10,	0},
	{"UnSaveInvaildLogOn", 	0,	0,	optoffset(UnSaveInvaildLogOn), 	0,	0,	0},
	{"DLSTMode",  0,      0,      optoffset(DLSTMode),  0,      0,      0},
	{"WeekOfMonth1",         1,                 0,      optoffset(StartWeekOfMonth[0]),},
	{"WeekOfMonth2",         1,                 0,      optoffset(StartWeekOfMonth[1]),},
	{"WeekOfMonth3",         0,                 0,      optoffset(StartWeekOfMonth[2]),},
	{"WeekOfMonth4",         0,                 0,      optoffset(StartWeekOfMonth[3]),},
	{"WeekOfMonth5",         0,                 0,      optoffset(StartWeekOfMonth[4]),},
	{"WeekOfMonth6",         1,                 0,      optoffset(EndWeekOfMonth[0]),},
	{"WeekOfMonth7",         1,                 0,      optoffset(EndWeekOfMonth[1]),},
	{"WeekOfMonth8",         0,                 0,      optoffset(EndWeekOfMonth[2]),},
	{"WeekOfMonth9",         0,                 0,      optoffset(EndWeekOfMonth[3]),},
	{"WeekOfMonth10",        0,                 0,      optoffset(EndWeekOfMonth[4]),},
	{"AttLogByAuth",	0,	0,	optoffset(AttLogByAuth),	0,	1,	0},//mjh 07/08/20
	{"AutoDelFunOn",	0,	0,	optoffset(AutoDelFunOn),	0,	1,	0},//mjh 07/10/16
	{"AutoDelAftUSBDnld",	0,	0,	optoffset(AutoDelAftUSBDnld),	0,	1,	0},//mjh 07/10/16
	{"NLockAtReRecordState",0,	0,	optoffset(NLockAtReRecordState),0,	0,	0},
	{"~iCLASS",		0, 			0,	optoffset(IsSupportiCLSRW),0,3,0}, //iCLSRW
	{"~iCLASSID",		0,			0, 	optoffset(iCLASSAsIDCard),0,2,0},//iCLSRW
	{"~iCLASSTYPE",	0,			0,	optoffset(iCLASSCardType),0,10,0},
	{"RedialTimeInterval",		4,			1,	optoffset(RedialTimeInterval),	0,			0, 		0 	},	//ccc add for gprs
	{"RedialCount",				1,			1,	optoffset(RedialCount),	0,			0, 		0 	},			//ccc gprs
	{"isgprstest",				0,			1,	optoffset(isgprstest),	0,			1, 		0 	},			//ccc gprs
	{"DeploymentFunOn",                          0,                      1,      optoffset(DeploymentFunOn),  0,            1,              0       },        //Deployment cxy add
	{"ShowIDCardFunOn",                          0,                      0,      optoffset(ShowIDCardFunOn),  0, 	        1,              0       },        //showmain cxy add
	{"WorkCode0",             0,                      0,      optoffset(WorkCode0),0,1,0}, //cxy add workcode 0
	{"SensorIdleTimeOut",	300,	1,	optoffset(SensorIdleTimeOut),	0,	0,	0},
	{"HasFPThreshold",	320,	0,	optoffset(HasFPThreshold),	0,	0,	0},
	{"NoFPThreshold",	280,	0,	optoffset(NoFPThreshold),	0,	0,	0},
	{"RedialTimeInterval",	4,	1,	optoffset(RedialTimeInterval),	0,	0,	0},	//ccc add for gprs
	{"RedialCount",		1,	1,	optoffset(RedialCount),	0,	0, 0 },	//ccc gprs
	{"DelRecord",        10000, 1, optoffset(DelRecord),0, 65530,10},
	{"WebServerPort",   80, 1, optoffset(WebServerPort), 0, 65530,80},
	{"ProtectOutputData", 1,	0,	optoffset(ProtectOutputData),	0,	1, 0 },//
	{"isgprstest",		0,	1,	optoffset(isgprstest),	0,	1, 0 },//ccc gprs
	{"SchBellByWeekday",	0,	0,	optoffset(SchBellByWeekday),	0,	1,	0},//C_SchBellByWeekday
	{"RecheckWC",		0,		0,	optoffset(RecheckWC),	0,	1,	0},
	{"KeyBeepFunOn",		0,		0,	optoffset(KeyBeepFunOn),	0,	1,	0},
	{"ApiPort",      8000,      0,  optoffset(ApiPort), 0,      65533, 20},
	{"SupportCust",		0,	0,	optoffset(SupportCust),	0,	0,	0},
	{"CreateCardOn",	0,	0,	optoffset(CreateCardOn),0,	0,	0},
	{"IclockSvrFun",        0,      0,      optoffset(IclockSvrFun),0,      0,      0},
	{"MCUDog",   0,      0,      optoffset(MCUDog),   0,      1,      0},
	{"DetectFpAlg",   0,      0,      optoffset(DetectFpAlg),   0,      1,      0},

	//add by cn 090401
        {"IrFunOn",     0,              0,      optoffset(IR_SensorOn),         0,      0,      255     },
        {"IrBLS",    	1,              0,      optoffset(IRSS_BLSwitch),       0,      0,      1       },
        {"IrBLOff",    	0,              0,      optoffset(IRSS_BLOffR),         0,      0,      255     },
        {"IrBLOn",    	128,            0,      optoffset(IRSS_BLOnR),          0,      0,      255     },
        {"IrBLDT",    	30,             0,      optoffset(IRSS_BLDlyT),         0,      0,      255     },
        {"IrAuto",   	63,             0,      optoffset(IRSS_AutoCon),        0,      0,      255     },
        {"IrRange",     32,             0,      optoffset(IRSS_Range),          0,      0,      255     },

//配置名称	缺省值是否需要恢复出厂设置菜单项资源 		最大值		最小值
	//add by oscar for c3

#ifdef MACHINETYPE_C4
//机器类别 c3: 1 c4:2
	{"ACPanelFunOn" ,      2,              0,      optoffset(ACPanelFunOn),         0,      0,      255     },
//门数量  n为门数量为n,一般n为2或4
	{"LockCount" ,      4,              0,      optoffset(LockCount),         0,      0,      255     },
#else
//机器类别 c3: 1 c4:2
	{"ACPanelFunOn" ,      1,              0,      optoffset(ACPanelFunOn),         0,      0,      255     },
//门数量  n为门数量为n,一般n为2或4
	{"LockCount" ,      4,              0,      optoffset(LockCount),         0,      0,      255     },
#endif

//读头数量  n为读头数量为n,一般n为2或4
	{"ReaderCount" ,      4,              0,      optoffset(ReaderCount),         0,      0,      255     },
//自定义输入数量
	{"AuxInCount" ,      4,              0,      optoffset(AuxInCount),         0,      0,      255     },
//自定义输出数量
	{"AuxOutCount" ,      4,              0,      optoffset(AuxOutCount),         0,      0,      255     },

	//反潜回规则
	{"AntiPassback" ,      0,              0,      optoffset(AntiPassback),         0,      0,      255     },
	//0无11/2号门互锁23/4号门互锁31/2/3号门互锁
       {"InterLock" ,      0,              0,      optoffset(InterLock),         0,      0,      255     },


//闭门回锁   1 启用，0不启用   即如果开锁时间为10s,如果在门开后，门又关上，之间时间少于10s,即马上关掉开锁信号
   	{"Door1CloseAndLock" ,      1,              0,      optoffset(CloseDoorAndLock[0]),         0,      0,      255     },
   	{"Door2CloseAndLock" ,      1,              0,      optoffset(CloseDoorAndLock[1]),         0,      0,      255     },
    {"Door3CloseAndLock" ,      1,              0,      optoffset(CloseDoorAndLock[2]),         0,      0,      255     },
    {"Door4CloseAndLock" ,      1,              0,      optoffset(CloseDoorAndLock[3]),         0,      0,      255     },

	//门磁类型0 无，1，常开，2 常闭
	{"Door1SensorType",         0,                 0,      optoffset(DoorDetectortype[0]),},
	{"Door2SensorType",         0,                 0,      optoffset(DoorDetectortype[1]),},
	{"Door3SensorType",         0,                 0,      optoffset(DoorDetectortype[2]),},
	{"Door4SensorType",         0,                 0,      optoffset(DoorDetectortype[3]),},
	//开锁时长
	{"Door1Drivertime",         5,                 0,      optoffset(DoorDrivertime[0]),},
	{"Door2Drivertime",         5,                 0,      optoffset(DoorDrivertime[1]),},
	{"Door3Drivertime",         5,                 0,      optoffset(DoorDrivertime[2]),},
	{"Door4Drivertime",         5,                 0,      optoffset(DoorDrivertime[3]),},
	//门磁信号限时报警限时长   s为单位，0为不启用限时报警
	{"Door1Detectortime",         20,                 0,      optoffset(DoorDetectortime[0]),},
	{"Door2Detectortime",         20,                 0,      optoffset(DoorDetectortime[1]),},
	{"Door3Detectortime",         20,                 0,      optoffset(DoorDetectortime[2]),},
	{"Door4Detectortime",         20,                 0,      optoffset(DoorDetectortime[3]),},
	//开门方式0，卡  卡+密码  见__VERIFY_STYLE 定义 （超级密码 和协迫密码始终开门有效）
	{"Door1VerifyType",         0,                 0,      optoffset(DoorOpenMode[0]),},
	{"Door2VerifyType",         0,                 0,      optoffset(DoorOpenMode[1]),},
	{"Door3VerifyType",         0,                 0,      optoffset(DoorOpenMode[2]),},
	{"Door4VerifyType",         0,                 0,      optoffset(DoorOpenMode[3]),},
	//门的出入状态 0.进入口 1.出口
	{"Door1InOutState",         0,                 0,      optoffset(DoorInOutState[0]),},
	{"Door2InOutState",         0,                 0,      optoffset(DoorInOutState[1]),},
	{"Door3InOutState",         0,                 0,      optoffset(DoorInOutState[2]),},
	{"Door4InOutState",         0,                 0,      optoffset(DoorInOutState[3]),},
	//多卡开门启用否  0不启用  1启用
	{"Door1MultiCardOpenDoor",         0,                 0,      optoffset(DoorMultiCardOpenDoor[0]),},
	{"Door2MultiCardOpenDoor",         0,                 0,      optoffset(DoorMultiCardOpenDoor[1]),},
	{"Door3MultiCardOpenDoor",         0,                 0,      optoffset(DoorMultiCardOpenDoor[2]),},
	{"Door4MultiCardOpenDoor",         0,                 0,      optoffset(DoorMultiCardOpenDoor[3]),},
	//首卡开门启用否  首卡开门模式 3种: 0:不启用 1.首卡常开  2.首卡激活
	{"Door1FirstCardOpenDoor",         1,                 0,      optoffset(DoorFirstCardOpenDoor[0]),},
	{"Door2FirstCardOpenDoor",         1,                 0,      optoffset(DoorFirstCardOpenDoor[1]),},
	{"Door3FirstCardOpenDoor",         1,                 0,      optoffset(DoorFirstCardOpenDoor[2]),},
	{"Door4FirstCardOpenDoor",         1,                 0,      optoffset(DoorFirstCardOpenDoor[3]),},
	//门激活时区 (接收有效刷卡时区)
	{"Door1ValidTZ",         1,                 0,      optoffset(DoorPollCardTimeZoneOfValidity[0]),},
	{"Door2ValidTZ",         1,                 0,      optoffset(DoorPollCardTimeZoneOfValidity[1]),},
	{"Door3ValidTZ",         1,                 0,      optoffset(DoorPollCardTimeZoneOfValidity[2]),},
	{"Door4ValidTZ",         1,                 0,      optoffset(DoorPollCardTimeZoneOfValidity[3]),},
	//门常开时区号
	{"Door1KeepOpenTimeZone",         0,                 0,      optoffset(DoorKeepOpenTimeZone[0]),},
	{"Door2KeepOpenTimeZone",         0,                 0,      optoffset(DoorKeepOpenTimeZone[1]),},
	{"Door3KeepOpenTimeZone",         0,                 0,      optoffset(DoorKeepOpenTimeZone[2]),},
	{"Door4KeepOpenTimeZone",         0,                 0,      optoffset(DoorKeepOpenTimeZone[3]),},
/*	//门对应的读头wg格式
	{"Reader1WGType",         0,                 0,      optoffset(DoorReaderWGType[0]),},
	{"Reader2WGType",         0,                 0,      optoffset(DoorReaderWGType[1]),},
	{"Reader3WGType",         0,                 0,      optoffset(DoorReaderWGType[2]),},
	{"Reader4WGType",         0,                 0,      optoffset(DoorReaderWGType[3]),},
*/	//双卡间隔 0为无间隔 以S为单位
       {"Door1Intertime",     0,              0,      optoffset(DoorIntertimepreset[0]), },
       {"Door2Intertime",     0,              0,      optoffset(DoorIntertimepreset[1]), },
       {"Door3Intertime",     0,              0,      optoffset(DoorIntertimepreset[2]), },
       {"Door4Intertime",     0,              0,      optoffset(DoorIntertimepreset[3]), },
	//读头类型0.卡 1.卡+密码 2.指纹
       {"Door1ReaderType",     0,              0,      optoffset(DoorReaderType[0]), },
       {"Door2ReaderType",     0,              0,      optoffset(DoorReaderType[1]), },
       {"Door3ReaderType",     0,              0,      optoffset(DoorReaderType[2]), },
       {"Door4ReaderType",     0,              0,      optoffset(DoorReaderType[3]), },
//另定义的变量供程序内部使用
//机型
	{"MachineType" ,     C3_400,              0,      optoffset(MachineType),         0,      0,      255     },
//c3_400是否当作c3_200使用
	{"Door4ToDoor2" ,    0,              0,      optoffset(Door4ToDoor2),         0,      0,      255     },

	//取消门常开日期;
	{"Door1CancelKeepOpenDay" ,    1,              0,      optoffset(CancelKeepOpenDay[0]),         0,      0,      255     },
	{"Door2CancelKeepOpenDay" ,    1,              0,      optoffset(CancelKeepOpenDay[1]),         0,      0,      255     },
	{"Door3CancelKeepOpenDay" ,    1,              0,      optoffset(CancelKeepOpenDay[2]),         0,      0,      255     },
	{"Door4CancelKeepOpenDay" ,    1,              0,      optoffset(CancelKeepOpenDay[3]),         0,      0,      255     },
	// 看门狗是否启用
	{"WatchDog" ,    1,              1,      optoffset(WatchDog),         0,      0,      255     },
	{"BackupTime" ,    3,              0,      optoffset(BackupTime),         0,      0,      23     },
	{"ShortCardFunOn" ,    0,              0,      optoffset(ShortCardFunOn),         0,      0,      255     },
	{"Reader1LastTimeAPB" ,    0,              0,      optoffset(ReaderLastTimeAPB[0]),         0,      0,      23     },
	{"Reader2LastTimeAPB" ,    0,              0,      optoffset(ReaderLastTimeAPB[1]),         0,      0,      23     },
	{"Reader3LastTimeAPB" ,    0,              0,      optoffset(ReaderLastTimeAPB[2]),         0,      0,      23     },
	{"Reader4LastTimeAPB" ,    0,              0,      optoffset(ReaderLastTimeAPB[3]),         0,      0,      23     },
	{"Door1LastTimeAPB" ,    0,              0,      optoffset(DoorLastTimeAPB[0]),         0,      0,      0     },
	{"Door2LastTimeAPB" ,    0,              0,      optoffset(DoorLastTimeAPB[1]),         0,      0,      0     },
	{"Door3LastTimeAPB" ,    0,              0,      optoffset(DoorLastTimeAPB[2]),         0,      0,      0     },
	{"Door4LastTimeAPB" ,    0,              0,      optoffset(DoorLastTimeAPB[3]),         0,      0,      0     },
	{"KeepOpenDayFunOn" ,    1,              0,      optoffset(KeepOpenDayFunOn),         0,      0,      23     },
};
TStateOptions gStateOptions;

void InitStateOptions(void)
{

	gStateOptions.DoorOpenstatus = 0x01010101; //门的开关状态，
	gStateOptions.DoorAlarmStatus = 0; //门的报警状态，

}

POptionsResInt QueryOptResByOffset(int Offset)
{
	int i;
	for(i=0;i<OPTIONSRESINTCOUNT;i++)
	{
		if(OptionsResInt[i].Offset==Offset)
			return OptionsResInt+i;
	}
	return NULL;
}

POptions GetDefaultOptions(POptions opts)
{
	int i=0;
	//Get common default value
	for(i=0;i<OPTIONSRESSTRCOUNT;i++)
	{
		if(OptionsResStr[i].IsNeedRestoreFactory)
		{
			memcpy(((char*)opts)+OptionsResStr[i].Offset, OptionsResStr[i].DefaultValue, OptionsResStr[i].OptionLong);
		}
	}

	for(i=0;i<OPTIONSRESINTCOUNT;i++)
	{
		if(OptionsResInt[i].IsNeedRestoreFactory)
		{
			memcpy(((char*)opts)+OptionsResInt[i].Offset, &(OptionsResInt[i].DefaultValue), 4);
		}
	}

#ifdef OEM_CMI
	opts->MaxNoiseThr=124;
	opts->RS232On =0;
	opts->RS485On =0;
#endif
	//special options
	if(LOCKFUN_ADV & LoadInteger("~LockFunOn",0))
	{
		opts->MThreshold=65;
		opts->VThreshold=55;
	}
	if(!gOptions.USB232FunOn)
	{
		gOptions.USB232On=0;
	}
	else
	{
		gOptions.RS232On=0;
	}

	if(!gOptions.VoiceFunOn)
	{
		gOptions.VoiceOn=0;
	}

	opts->Saved =1;
	return opts;
}

POptions LoadOptions(POptions opts)
{
	int i = 0;
/*#ifndef URU
#ifndef UPEK
	static BOOL LoadSign=FALSE;
#endif
#endif
*/	char name1[128], value1[VALUE_BUFFERLEN];
	char buffer[VALUE_BUFFERLEN];
	int size = 0;
	BOOL exitsign;
	int value = 0;

	memset(name1, 0, 128);
	memset(value1, 0, VALUE_BUFFERLEN);
	memset(buffer, 0, VALUE_BUFFERLEN);
	//setting default value

	for(i=0;i<OPTIONSRESSTRCOUNT;i++)
	{
		memcpy(((char*)opts)+OptionsResStr[i].Offset, OptionsResStr[i].DefaultValue, OptionsResStr[i].OptionLong);
	}
	for(i=0;i<OPTIONSRESINTCOUNT;i++)
	{
		memcpy(((char*)opts)+OptionsResInt[i].Offset, &(OptionsResInt[i].DefaultValue), 4);
	}

	//Read option from options.cfg
	lseek(fdOptions, 0, SEEK_SET);
	while(TRUE)
	{
		if(ReadOneLine(fdOptions, buffer, &size))
		{
			exitsign=FALSE;
			SplitByChar(buffer, name1, value1, '=');
			for(i=0;i<OPTIONSRESSTRCOUNT;i++)
			{
				if(strcmp(name1, OptionsResStr[i].OptionName)==0)
				{
					if(OptionsResStr[i].Convertor)
						OptionsResStr[i].Convertor(value1, ((BYTE*)opts)+OptionsResStr[i].Offset);
					else
						strcpy(((char*)opts)+OptionsResStr[i].Offset, value1);
					exitsign=TRUE;
					break;
				}
			}
			if(!exitsign)
			{
				for(i=0;i<OPTIONSRESINTCOUNT;i++)
				{
					if(strcmp(name1, OptionsResInt[i].OptionName)==0)
					{
						value=str2int(value1, OptionsResInt[i].DefaultValue);
						if(OptionsResInt[i].MaxValue>OptionsResInt[i].MinValue)
						{
							if(OptionsResInt[i].MaxValue<value)
								value=OptionsResInt[i].MaxValue;
							else if(OptionsResInt[i].MinValue>value)
								value=OptionsResInt[i].MinValue;
						}
						memcpy(((char*)opts)+OptionsResInt[i].Offset, &value, 4);
						break;
					}
				}
			}
		}
		else
			break;
	}

	return opts;
}

BOOL SaveStrIgnoreCheck(const char *name, const char *value)
{
	char buffer[VALUE_BUFFERLEN] = {0};
	int len;

	len=strlen(value);
	//check language item whether can be setup or not
	if(!gOptions.MultiLanguage)
	{
		if(strcmp(name, "Language")==0) return TRUE;
	}
	CombineNameAndValue(name, value, TRUE, buffer);
	len=lseek(fdOptions, 0, SEEK_END);
	if (len>=MAX_OPTION_SIZE)
	{
		ClearOptionItem("NONE");
		len=lseek(fdOptions, 0, SEEK_END);
	}
	if (len<MAX_OPTION_SIZE)
		write(fdOptions, buffer, strlen(buffer));

	ExecuteActionForOption(name, value);

	return ((len<MAX_OPTION_SIZE)?TRUE:FALSE);
}

POptions SaveOptions(POptions opts)
{
	int i;
	TOptions OldOpt;
	char Buf[20];
	char buffer[1000]={0};
	int value;

	LoadOptions(&OldOpt);

	for(i=0;i<OPTIONSRESSTRCOUNT;i++)
	{
		memset(buffer, 0x00, 1000);
		if(memcmp((((char*)opts)+OptionsResStr[i].Offset), (((char*)&OldOpt)+OptionsResStr[i].Offset), OptionsResStr[i].OptionLong))
		{
			//DBPRINTF("Name=%s  value= %s  New=%d old=%d \n", OptionsResInt[i].OptionName, buffer,*(int*)(((char*)opts)+OptionsResInt[i].Offset), *(int*)(((char*)&OldOpt)+OptionsResInt[i].Offset));
			if(OptionsResStr[i].Formator)
			{
				OptionsResStr[i].Formator(buffer, (BYTE*)(((char*)opts)+OptionsResStr[i].Offset));
			}
			else
			{
				nstrcpy(buffer,(((char*)opts)+OptionsResStr[i].Offset), OptionsResStr[i].OptionLong);
			}
			//printf("Name = %s\n",OptionsResStr[i].OptionName);
			SaveStrIgnoreCheck(OptionsResStr[i].OptionName, buffer);
		}
	}
	for(i=0;i<OPTIONSRESINTCOUNT;i++)
	{
		if(memcmp((((char*)opts)+OptionsResInt[i].Offset), (((char*)&OldOpt)+OptionsResInt[i].Offset), 4))
		{
			memset(Buf, 0x00, 20);
			memcpy(&value, ((char*)opts)+OptionsResInt[i].Offset, 4);
			sprintf(Buf, "%d", value);
			SaveStrIgnoreCheck(OptionsResInt[i].OptionName, Buf);
		}
	}
	//flush the cached data to disk
	sync();
	opts->Saved=TRUE;
	return opts;
}

//for option that not saved in flash
char *GetDefaultOption(const char *name,char *value)
{
//	printf("GetDefaultOption name is %s,value is %s\n",name,value);

	char *s=NULL;
	*value= 0;	//取值前清0

	int i;
	for(i=0;i<OPTIONSRESINTCOUNT;i++)
	{
		if(strcmp(name,OptionsResInt[i].OptionName)==0)
		{
			int v=LoadInteger(OptionsResInt[i].OptionName, OptionsResInt[i].DefaultValue);
			if(v!=-1)
			if(OptionsResInt[i].MaxValue>OptionsResInt[i].MinValue)
			{
				if(OptionsResInt[i].MaxValue<v)
					v=OptionsResInt[i].MaxValue;
				else if(OptionsResInt[i].MinValue>v)
					v=OptionsResInt[i].MinValue;
			}
			sprintf(value,"%d",v);
			break;
		}
	}
	if(strlen(value)==0)
	{
		if(strcmp(name, "WiegandFmt")==0)
		{
			LoadStr("WiegandFmt",s);
			if(s)
				sprintf(value,"%s",s);
			else
				sprintf(value,"%d",26);
		}
		else if(strcmp(name,"~RFSStart")==0)
			sprintf(value,"%d",gOptions.RFCardSecStart);
		else if(strcmp(name,"~RFSLen")==0)
			sprintf(value,"%d",gOptions.RFCardSecLen );
		else if(strcmp(name,"~RFFPC")==0)
			sprintf(value,"%d",gOptions.RFCardFPC);
	}
//	printf("after GetDefaultOption name is %s,value is %s\n",name,value);
	return value;
}

int UpdateLanguageByID(int LanID, const char *Language)
{
	//dsl 2008.4.16
	char buffer[128];
	char *tmp;
 	int fd = -1;

	if ((tmp=LoadStrOld("FONTFILEPATH"))!=NULL)
		sprintf(buffer, "%s%s.%c", tmp, "LANGUAGE", CurLanguage);
	else
		sprintf(buffer, "%s.%c", "LANGUAGE", CurLanguage);

	printf("path:%s\n", buffer);
	fd = open(buffer, O_CREAT|O_APPEND|O_RDWR|O_SYNC, S_IRWXU|S_IRWXG|S_IRWXO);
	if(fd > 0)
	{
		sprintf(buffer, "%c/_%d_=%s\n", CurLanguage, LanID, Language);
		DBPRINTF("customize language=%s\n", buffer);
		lseek(fd, 0, SEEK_END);
		return (write(fd, buffer, strlen(buffer)) > 0);
		close(fd);
	}
	else
	{
		return FALSE;
	}

}

//int GetMAC(const char *HwName, char *DevMac)
int GetMAC(char *DevMac)
{
#define 	ETH_ALEN    6
	char* device="eth0";
    	unsigned char macaddr[ETH_ALEN];

    	int s=socket(AF_INET,SOCK_DGRAM,0);
    	struct ifreq req;
    	int err;

    	strcpy(req.ifr_name,device);
    	err=ioctl(s,SIOCGIFHWADDR,&req);
    	close(s);
    	if(err!=-1)
    	{
        	memcpy(macaddr,req.ifr_hwaddr.sa_data,ETH_ALEN);
	    	sprintf(DevMac,"%.2X:%.2X:%.2X:%.2X:%.2X:%.2X",macaddr[0],macaddr[1],macaddr[2],macaddr[3],macaddr[4],macaddr[5]);
		return 1;
    	}
    	else
    	{
		return 0;
    	}
}

int UpdateOptionsByUSB(int surfd, int destfd)
{
       	char buffer[VALUE_BUFFERLEN];
       	int size;

	if ((surfd > 0) && (destfd > 0))
	{

		lseek(surfd, 0, SEEK_SET);
	       	lseek(destfd, 0, SEEK_END);
       		while(TRUE)
		{
			memset(buffer, 0, sizeof(buffer));
	   		if(ReadOneLine(surfd, buffer, &size))
			{
				sprintf(buffer, "%s\n", buffer);
				write(destfd, buffer, strlen(buffer));
			}
			else
			{
				break;
			}
	       	}
		return TRUE;
	}
	else
	{
		return FALSE;
	}
}

