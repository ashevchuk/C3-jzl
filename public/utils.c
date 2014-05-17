/*************************************************

 ZEM 200

 utils.c

 Copyright (C) 2003-2005, ZKSoftware Inc.

*************************************************/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <sys/time.h>
#include <sys/socket.h>
#include <ifaddrs.h>
#include <crypt.h>
#define _XOPEN_SOURCE
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include "utils.h"
#include "arca.h"
#include "flash.h"
#include "options.h"
#include "rtc.h"
#include <dirent.h>
//#include"lcm.h"
#include"main.h"
#include "serial.h"







typedef struct _Config_ {
	unsigned long key1;
	unsigned char config_data[1024-(3*4)];
	unsigned long key2;
	unsigned long cksum;
}TConfig, *PConfig;

U32 GetTickCount(void)
{
	struct timeval tv;
	struct timezone tz;
	gettimeofday(&tv, &tz);
	return (tv.tv_sec*1000 + tv.tv_usec/1000);
}

U32 GetUS()
{
	//return GetTickCount()*1000*1000;
	return GetTickCount()*1000;
}

char *GetEnvFilePath(const char *EnvName, const char *filename, char *fullfilename)
{
	if (getenv(EnvName))
		sprintf(fullfilename, "%s%s", getenv(EnvName), filename);
	else
		sprintf(fullfilename, "%s", filename);
	return fullfilename;
}


void RebootMachine(void)
{
	system("reboot");
}

int Decode16(char *String, char* Data)
{
	int i,v1,v2, c;
	int Size=(int)strlen(String)/2;
	for(i=0;i<Size;i++)
	{
		c=String[i*2];
		v1=(c>='a')?(c-'a'+10):(c>='A'?(c-'A'+10):(c-'0'));
		c=String[i*2+1];
		v2=(c>='a')?(c-'a'+10):(c>='A'?(c-'A'+10):(c-'0'));
		if(v1<0) return 0;
		if(v2<0) return 0;
		if(v1>15) return 0;
		if(v2>15) return 0;
		Data[i]=v1*16+v2;
	}
	return Size;
}

char *ConvertMonth(char *version, char *iversion)
{
	char *FWMonths[]={"Jan","Feb","Mar","Apr","May","Jun","Jul","Aug","Sep","Oct","Nov","Dec"};
	int i = 0;
	char mon[10] = {0};
	char verbuf[8] = {0};
	char *psubversion = NULL;
	sprintf(verbuf, "%s", "Ver");

	psubversion = strstr(version, verbuf);

	if(NULL == psubversion)
	{
		return iversion;
	}
	memset(iversion, 0, 64);
	memset(mon, 0, 10);
	strncpy(mon, psubversion+9, 3); //month name
	for(i=1;i<=12;i++)
	{
		if(strcmp(mon, FWMonths[i-1])==0)
			sprintf(iversion+13, " %02d ", i);
	}
	memcpy(iversion, psubversion, 9);
	memcpy(iversion+9, psubversion+16, 4);
	memcpy(iversion+17, psubversion+13, 2);

	return iversion;
}

void doPrint(char * buffer, char * fmt, va_list ap)
{
    void *p1, *p2, *p3, *p4, *p5, *p6;

    p1 = va_arg(ap,void*);
    p2 = va_arg(ap,void*);
    p3 = va_arg(ap,void*);
    p4 = va_arg(ap,void*);

    p5 = va_arg(ap,void*);
    p6 = va_arg(ap,void*);

    sprintf(buffer,fmt,p1,p2,p3,p4,p5,p6);
}

char *nstrcpy(char *Dest, const char *Source, int Size)
{
	char *s;
	s=Dest;
	while((*Source) && Size)
	{
		*s++=*Source++;
		Size--;
	}
	return Dest;
}

BYTE *nmemcpy(BYTE *Dest, const BYTE *Source, int Size)
{
	BYTE *s;
	s=Dest;
	while(Size--)
	{
		*s++=*Source++;
	}
	return Dest;
}

int nstrcmp(const char *s1, const char *s2, int size)
{
	while(size)
	{
		if((*s2)<(*s1)) return -1;
		if((*s2)>(*s1)) return 1;
		if((*s1)==0) return 0;
		s1++;s2++;
		size--;
	}
	return 0;
}

int nmemcmp(const BYTE *s1, const BYTE *s2, int size)
{
	while(size)
	{
		if((*s2)<(*s1)) return -1;
		if((*s2)>(*s1)) return 1;
		s1++;s2++;
		size--;
	}
	return 0;
}

int nmemset(BYTE *Dest, BYTE Data, int Size)
{
	while(Size--)
		*Dest++=Data;
	return 1;
}

/* test for a digit. return value if digit or -1 otherwise */
static int digitvalue(char isdigit)
{
	if (isdigit >= '0' && isdigit <= '9' )
		return isdigit - '0';
	else
		return -1;
}

/* test for a hexidecimal digit. return value if digit or -1 otherwise */
static int xdigitvalue(char isdigit)
{
	if (isdigit >= '0' && isdigit <= '9' )
		return isdigit - '0';
	if (isdigit >= 'a' && isdigit <= 'f')
		return 10 + isdigit - 'a';
	return -1;
}

/* convert a string to an u32 value. if the string starts with 0x, it
 * is a hexidecimal string, otherwise we treat is as decimal. returns
 * the converted value on success, or -1 on failure. no, we don't care
 * about overflows if the string is too long.
 */
int strtou32(const char *str, unsigned int *value)
{
	int i;

	*value = 0;

	if((str[0]=='0') && (str[1]=='x')) {
		/* hexadecimal mode */
		str += 2;

		while(*str != '\0') {
			if((i = xdigitvalue(*str)) < 0)
				return -1;

			*value = (*value << 4) | (unsigned int)i;

			str++;
		}
	} else {
		/* decimal mode */
		while(*str != '\0') {
			if((i = digitvalue(*str)) < 0)
				return -1;

			*value = (*value * 10) + (unsigned int)i;

			str++;
		}
	}

	return 0;
}

int str2mac(char* str, BYTE* mac)
{
	int i=0;
	char *p=str;
	char t[6];
	int addr;

	while(i < 6) {
		addr = 0;
		while( *p && (*p != ':') ) {
			if( (*p >= '0') && (*p <= '9') ) {
				addr = addr * 16 + (*p - '0');
				p++;
			}
			else if( (*p >= 'a') && (*p <= 'f') ) {
				addr = addr * 16 + (*p - 'a'+10);
				p++;
			}
			else if ( (*p >= 'A') && (*p <= 'F') ) {
				addr = addr * 16 + (*p - 'A'+10);
			       	p++;
			}
			else return -1; /* invalid */
		}

		if (addr <= 255) t[i] = (addr&0xFF);
		else break;

		i++;

		if(*p) p++;
		else break;
	}

	if( i!=6 )  return -1;

	memcpy(mac, t, sizeof(t));

	return 0;
}

int str2ip(char* str, BYTE* ip)
{
	int i=0;
	char *p=str;
	char t[4];
	int addr;

	while(i < 4) {
		addr = 0;
		while( *p && (*p != '.') ) {
			if( (*p >= '0') && (*p <= '9') ) {
				addr = addr * 10 + (*p - '0');
			       	p++;
			}
			else return -1; /* invalid */
		}

		if (addr <= 255) t[i] = (addr&0xFF);
		else break;

		i++;

		if(*p) p++;
		else break;

	}

	if( i!=4 )  return -1;

	memcpy(ip, t, sizeof(t));

	return 0;
}

int str2int(char *buf, int DefaultValue)
{
        int v,n=1,d,c;
        if('-'==*buf)
        {
                n=-1;
                buf++;
        }
        v=0;c=0;
        do{
                d=buf[c];
                if(d==0) break;
                if(d==' ')
                {
                        if(v) break;
                }
                else if(d<'0' || d>'9')
                {
                        return DefaultValue;
                }
                else
                        v=v*10+(d-'0');
                c++;
        }while(1);
        if(c)
                return n*v;
        else
                return DefaultValue;
}

char *TrimRightStr(char *buf)
{
        int i=strlen(buf);
        while(i--)
                if(((BYTE*)buf)[i]==0x20) buf[i]=0; else break;
        return buf;
}

char* SPadCenterStr(char *buf, int size, const char *s)
{
        int len;
        for(len=0;len<size;len++) buf[len]=' ';buf[size]=0;
        if(NULL==s) return buf;
        len=0;
        while(*s)
                if(' '==*s) s++;
                else break;
                while(s[len]) len++;
                if(len==0) return buf;
                while(' '==s[len-1]) len--;
                if(size<len)
                        memcpy(buf, s, size);
                else
                        memcpy(buf+(size-len)/2,s,len);
                return buf;
}

char* SPadRightStr(char *buf, int size, const char *s)
{
        int len;
        for(len=0;len<size;len++) buf[len]=' ';buf[size]=0;
        if(NULL==s) return buf;
        len=0;
        while(*s)
                if(' '==*s) s++;else break;
        while(s[len]) len++;
        if(len==0) return buf;
        while(' '==s[len-1]) len--;
        if(size<len)
                memcpy(buf, s, size);
        else
                memcpy(buf+size-len,s,len);
        return buf;
}

char *TrimLeftStr(char *buf)
{
        BYTE *p=(BYTE*)buf, *p0=(BYTE*)buf;
        while(0x20==*p)
                if(*p)
                        p++;
                else
                {
                        *buf=0;
                        return buf;
                }
        while(1)
        {
                BYTE x=*p++;
                *p0++=x;
                if(x==0) break;
        }
        return buf;
}

char *TrimStr(char *buf)
{
        return TrimLeftStr(TrimRightStr(buf));
}

int SearchIndex(char **Items, const char *Text, int ItemCount)
{
	int i;
	for(i=0;i<ItemCount;i++)
	{
		if(strcmp(Items[i], Text)==0) return i;
	}
	return -1;
}

char *Pad0Num(char *buf, int size, int value)
{
	char fmt[20];
	sprintf(fmt,"%%0%dd",size);
	sprintf(buf, fmt, value);
	return buf;
}

int StrValue(const char *p, int *Next)
{
	int i;
	unsigned ret=0;
	for(i=0;i<19;i++)
	{
		if((p[i]>='0') && (p[i]<='9'))
			ret=ret*10+(p[i]-'0');
		else if(!((p[i]==' ') && (ret==0)))
		{
			if(Next) *Next=i;
			break;
		}
	}
	return ret;
}

int HexStrValue(const char *p, int *Next)
{
	int i;
	unsigned ret=0;
	for(i=0;i<19;i++)
	{
		if((p[i]>='0') && (p[i]<='9'))
			ret=ret*16+(p[i]-'0');
		else
		if((p[i]>='A') && (p[i]<='F'))
			ret=ret*16+(p[i]-'A'+10);
		else
		if((p[i]>='a') && (p[i]<='f'))
			ret=ret*16+(p[i]-'a'+10);
		else
		{
			if(Next) *Next=i;
			break;
		}
	}
	return ret;
}

int HexStrToInt(const char *str, int *value)
{
	int i;
	*value=HexStrValue(str, &i);
	return str[i];
}

//从p中复制第index个值到buf中
int SCopyStrFrom(char *buf, char *p, int index)
{
	int i,j;

	j=0;
	if(p){
	    for(i=0; p[i] && (i<8*1024); i++)
	    {
		if(p[i]==':')
		{
		    if(index==0)
		    {
			buf[j]=0;
			break;
		    }
		    else
			index--;
		}
		else if(index==0)
		    buf[j++]=p[i];
	    }
	    if(p[i]==0) buf[j]=0;
	}
	return j;
}

int SPackStr(char *buf)
{
	int c=0;
	char ch, lc=0, *p=buf;
	if(!buf) return c;
	lc=*p;
	while(1)
	{
		ch=*buf;
		if(ch==':')
		{
			if(lc==':')
			{
				buf++;
				continue;
			}
		}
		else if(!ch)
			break;
		*p=ch;
		buf++;
		p++;
		c++;
		lc=ch;
	}
	*p=0;
	p--;
	if(*p==':')
		*p=0;
	return TRUE;
}

int SaveIntList(char *buf, int *INTs, int Count, int InvalidInt, int PackInvalidInt)
{
	char *p;
	int i;
	p=buf;
	*p=0;
	if(Count<=0) return 0;
	if(INTs[0]!=InvalidInt) sprintf(p, "%d", INTs[0]);
	for(i=1;i<Count;i++)
	{
		p+=strlen(p);
		*p=':'; *++p=0;
		if(INTs[i]!=InvalidInt)
			sprintf(p, "%d", INTs[i]);
	}
	if(PackInvalidInt)
		return SPackStr(buf);
	else
		return (p-buf)+strlen(p);
}

//计算p中被分割的字符串的个数
int SCountStr(char *p)
{
	int i, ret=0;
	if(':'!=*p) ret=1;
	if(*p==0) return 0;
	while(1)
	{
		i=*p;
		if(i==0)
			return ret;
		else if(i==':')
		{
			if(p[1])
				ret++;
		}
		p++;
	}
	return ret;
}
//返回p中第index个值的整数形式，并在Next中返回该数值的结束地址
int SIntValueFrom(char *p, int index, int *Next)
{
	int c=*p,i=0;
	while(index)
	{
		c=*p;
		if(c==':') index--; else if(c==0) break;
		p++;
		i++;
	}
	if(c)
	{
		c=StrValue(p, Next);
		*Next=*Next+i;
		return c;
	}
	else
	{
		*Next=i;
		return 0;
	}
}

//返回p中第index个值（16进值）的整数形式，并在Next中返回该数值的结束地址
int SHexValueFrom(char *p, int index, int *Next)
{
	int c=*p,i=0;
	while(index)
	{
		c=*p;
		if(c==':') index--; else if(c==0) break;
		p++;
		i++;
	}
	if(c)
	{
		c=HexStrValue(p, Next);
		*Next=*Next+i;
		return c;
	}
	else
	{
		*Next=i;
		return 0;
	}
}


int Hex2Char(char *s)
{
	int ret=0;
	ret=*s;
	if(ret>='A')
	{
		ret=ret-'A'+10;
		if(ret>15) ret=-1;
	}
	else if(ret>='0')
	{
		ret=ret-'0';
		if(ret>9) ret=-1;
	}
	else
		return -1;
	return ret;
}

const BYTE HEXCHARS[16]={'0','1','2','3','4','5','6','7','8','9','A','B','C','D','E','F'};

int EncodeHex(BYTE *String, BYTE *Data, int Size)
{
	int i;
	for(i=0;i<Size;i++)
	{
		String[i*2]=HEXCHARS[Data[i]/16];
		String[i*2+1]=HEXCHARS[Data[i]&0x0F];
	}
	String[2*i]=0;
	return 2*i;
}

void SetBit(BYTE *Buffer, int Index)
{
	//DBPRINTF("SetBit:%d\n",Index);
	Buffer[Index>>3]|=(1<<(Index & 7));
}

void ClearBit(BYTE *Buffer, int Index)
{
	Buffer[Index>>3]&=~(1<<(Index & 7));
}

int GetBit(BYTE *Buffer, int Index)
{
	return (Buffer[Index>>3]>>(Index & 7)) & 1;
}

void memor(char *s1, char *s2, int len)
{
        do {
                *s1++ |= *s2++;
        } while (--len);
}

char *GenerateMacBySN(BYTE *vendor, U32 serialnumber, char *MAC)
{
        sprintf(MAC,"%02X:%02X:%02X:%02X:%02X:%02X",
                vendor[0],vendor[1],vendor[2],
                (serialnumber>>16)&0xFF,(serialnumber>>8)&0xFF,serialnumber&0xFF);
        return MAC;
}

#ifdef ZEM300

#define CFG_MONITOR_BASE 	((U32)FlashBaseAddr)
#define CFG_LEN			16*1024
#define CFG_UBOOT_CFG_LEN	2048
#define CFG_MAC_LEN		6
#define CFG_MAC_INDEX		(STD_SECTOR_SIZE-CFG_LEN+CFG_UBOOT_CFG_LEN)
#define CFG_INDEX		(STD_SECTOR_SIZE-CFG_LEN)

#define TOP_SECTOR_SIZE		8*1024

BOOL SaveBufferToTopSector(int sector_index, char *data, int index, int size)
{
        WORD buffer[STD_SECTOR_SIZE/2];
        U32 StartAddr, EndAddr;
        U32 Addr, i=0;

        StartAddr=CFG_MONITOR_BASE+TOP_SECTOR_SIZE*sector_index;
        memcpy((BYTE *)buffer, (BYTE *)StartAddr, size);

        if(FlashSectorErase(StartAddr))
        {
                DBPRINTF("DELETE SECTOR %d,addr=%d,size=%d OK!\n", sector_index,StartAddr,size);
                memcpy((BYTE *)buffer+index, data, size);
                EndAddr=StartAddr+size;
		DBPRINTF("START WRITE FLASH\n");
                for(Addr=StartAddr;Addr<EndAddr;Addr+=2,i++)
                {
                        if(!FlashWriteWord(Addr, buffer[i]))
                        {
                                return FALSE;
                        }
                }
		DBPRINTF("WRITE SECTOR %d OK!\n", sector_index);
                return TRUE;
        }
        return FALSE;
}

BOOL SaveBufferToSector(char *data, int index, int size)
{
	WORD buffer[STD_SECTOR_SIZE/2];
	U32 StartAddr, EndAddr;
	U32 Addr, i=0;

	StartAddr=CFG_MONITOR_BASE+STD_SECTOR_SIZE;
	memcpy((BYTE *)buffer, (BYTE *)StartAddr, STD_SECTOR_SIZE);

	if(FlashSectorErase(StartAddr))
	{
		DBPRINTF("DELETE SECTOR OK!,index=%d,size=%d\n",index,size);
		memcpy((BYTE *)buffer+index, data, size);
		DBPRINTF("MAC %x:%x:%x:%x:%x:%x\n", *((BYTE *)buffer+index), *((BYTE *)buffer+index+1), *((BYTE *)buffer+index+2), *((BYTE *)buffer+index+3), *((BYTE *)buffer+index+4), *((BYTE *)buffer+index+5));
		EndAddr=StartAddr+STD_SECTOR_SIZE;
		for(Addr=StartAddr;Addr<EndAddr;Addr+=2,i++)
		{
			if(!FlashWriteWord(Addr, buffer[i]))
			{
				return FALSE;
			}
		}
		DBPRINTF("WRITE SECTOR OK!\n");
		return TRUE;
	}
	DBPRINTF("SET MAC Failed\n");
	return FALSE;
}

BOOL SaveMACToFlash(const char *MAC)
{
        BYTE mac[6];

	printf("SaveMACToFlash   001\n");

        if(str2mac((char *)MAC, mac)==0)
        {
		return SaveBufferToSector(mac, CFG_MAC_INDEX, CFG_MAC_LEN);
	}
	return FALSE;
}

BOOL SaveMACToFlash2(const char *MAC)
{
        BYTE mac[6];

	printf("SaveMACToFlash   002\n");

	return SaveBufferToSector(mac, CFG_MAC_INDEX, CFG_MAC_LEN);
	return FALSE;
}


//offset is from 16K config area    logo offset is 0x812=2048(uboot config)+6(MAC)+12(License)
BOOL SaveLOGOToFlash(int offset, char *config, int size)
{
	if((offset+size)<=CFG_LEN)
		return SaveBufferToSector(config, CFG_INDEX+offset, size);
	else
		return FALSE;
}

//UBOOT size= 8*8k + 64k

#else

#define CONFIG_KEY1    0x0BADFACE
#define CONFIG_KEY2    0xDEADDEAD

unsigned long  _fconfig_cksum(unsigned long *buf, int len)
{
	unsigned long cksum = 0;
	int shift = 0;
	// Round 'len' up to multiple of longwords
	len = (len + (sizeof(unsigned long)-1)) / sizeof(unsigned long);
	while (len-- > 0)
	{
		cksum ^= (*buf | (*buf << shift));
		buf++;
		if (++shift == 16) shift = 0;
	}
	return cksum;
}

BOOL SaveMACToFlash(const char *MAC)
{
        PConfig config;
        BYTE mac[6];
        int i;
        BYTE *buffer;
        BOOL rc=FALSE;

        if(str2mac((char *)MAC, mac)==0)
        {
                buffer=malloc(0x400);
                FlashReadBuffer(GetFlashStartAddress(FLASH_SECTOR_COUNT-2), buffer, 0x400);
                config=(PConfig)buffer;
                config->key1 = CONFIG_KEY1;
                config->key2 = CONFIG_KEY2;
                for(i=0;i<6;i++) config->config_data[i]=mac[i];
                config->cksum = _fconfig_cksum((unsigned long *)config, sizeof(TConfig)-4);
                if(FlashSaveBuffer(GetFlashStartAddress(FLASH_SECTOR_COUNT-2), buffer, 0x400)==1)
                        rc=TRUE;
                free(buffer);

        }
        return rc;
}
#endif

void RedBootMac(const char *serialnumber)
{
        U32 sn=0;
        BYTE vendor[]={0x00, 0x0A, 0x5D};
        char mac[16];

        if(!serialnumber[0])
                sn=0;
        else if((serialnumber[0]>'9')||(serialnumber[0]<'0'))
                sn=atol(serialnumber+1);
        else
                sn=atol(serialnumber);
        if(sn==0) sn=time(NULL);
        GenerateMacBySN(vendor, sn, mac);
        SaveStr("MAC", mac, FALSE);
}

/*void SaveMACAddr(void)
{
	BYTE *buffer;
	char MacAddr[32];
	BYTE i;

	buffer=malloc(0x400);
	FlashReadBuffer(GetFlashStartAddress(FLASH_SECTOR_COUNT-2), buffer, 0x400);
	sprintf(MacAddr, "%02X:%02X:%02X:%02X:%02X:%02X",
		((PConfig)buffer)->config_data[0], ((PConfig)buffer)->config_data[1],
		((PConfig)buffer)->config_data[2], ((PConfig)buffer)->config_data[3],
		((PConfig)buffer)->config_data[4], ((PConfig)buffer)->config_data[5]);
	for(i=0;i<6;i++)
		gOptions.MAC[i]=((PConfig)buffer)->config_data[i]; //update option
	SaveStr("MAC", MacAddr, FALSE);
	free(buffer);
	DBPRINTF("%s\n", MacAddr);
}*/

void CreateRootPwd(char *pwdfile, char *newpassword)
{
	int fp;
	char pwd[128], buf[256], tmp[8];
	char *p;

	if(strlen(newpassword)<5)
		strcpy(tmp, "solokey");
	else
		strcpy(tmp, newpassword);
	if(strlen(tmp)>=5)
	{

		fp=open(pwdfile, O_CREAT|O_RDWR|O_TRUNC|O_SYNC, S_IRUSR|S_IWUSR);
		if(fp!=-1)
		{
			memset(pwd, 0, 128);
			p=crypt(tmp, "$1$");
			if(!p) p=crypt("solokey", "$1$");
			if(!p) return;
			nstrcpy(pwd, p, 128);
			sprintf(buf, "root:%s:0:0:root:/root:/bin/sh\n", pwd);
			write(fp, buf, strlen(buf));
			close(fp);
		}
	}
}

void SetRootPwd(int comkey)
{
	char buf[128], tmp[16];

	//change root password
	if(!gOptions.DisableRootPassword)
	{
		sprintf(tmp, "%d", comkey);
		CreateRootPwd(GetEnvFilePath("USERDATAPATH", "passwd", buf), tmp);
	}
}

int GetSensorSerialNumber(char *TitleStr, char *IDStr)
{

        FILE *fp;
        char buffer[4096];
        char *p;
        int i;
	BOOL bSign=FALSE;

        fp=fopen("/proc/bus/usb/devices", "rb");
        if(!fp) return 0;
        while(!feof(fp))
        {
                memset(buffer, 0, 4096);
                if(!fgets(buffer, 4096, fp)) break;
                i=0;
                while(buffer[i])
                {
                        if(buffer[i]=='=')
                        {
                                p=buffer+i+1;
                                buffer[strlen(buffer)-1]='\0';
                                //printf("%s\n", p);
				if((!bSign)&&(TitleStr)&&(strncmp(p, TitleStr, strlen(TitleStr))==0))
					bSign=TRUE;
				else if(bSign)
				{
					strcpy(IDStr, p);
					return 1;
				}
                                break;
                        }
                        i++;
		}
	}
        fclose(fp);
        return 0;
}

void SyncTimeByTimeServer(BYTE *TimeServerIP)
{
	char buffer[128];
        time_t t;
	TTime tt;

	if(TimeServerIP[0])
	{
		sprintf(buffer, "rdate -s %d.%d.%d.%d", TimeServerIP[0], TimeServerIP[1], TimeServerIP[2], TimeServerIP[3]);
		if(system(buffer)==EXIT_SUCCESS)
		{
			t=time(NULL);
			//process the timzezone offset(hour) seconds
			t+=LoadInteger("TZAdj", 0)*60*60;
			SetRTCClock(localtime(&t));
			//dsl
			#ifdef ZEM500
				DelayUS(1000);
			#else
				DelayUS(100*1000);
			#endif
			ReadRTCClockToSyncSys(&tt);
			DelayUS(1000);
			ReadRTCClockToSyncSys(&gCurTime);
		}
	}
}

int strkeycard(char* str, BYTE* cardkey)
{
	int i=0;
	char *p=str;
	char t[6];
	int addr,ffc=0;

	while(i < 6)
	{
		addr = 0;
		while( *p && (*p != '-') )
		{
			if( (*p >= '0') && (*p <= '9') )
			{
				addr = addr * 16 + (*p - '0');
				p++;
			}
			else if( (*p >= 'a') && (*p <= 'f') )
			{
				addr = addr * 16 + (*p - 'a'+10);
				p++;
			}
			else if ( (*p >= 'A') && (*p <= 'F') )
			{
				addr = addr * 16 + (*p - 'A'+10);
			       	p++;
			}
			else return -1; /* invalid */
		}

		if (addr <= 255) t[i] = (addr&0xFF);
		else ffc++;

		i++;

		if(*p) p++;
		else break;
	}

	if( i!=6 )  return -1;
	if(ffc>4) return -1;
	memcpy(cardkey, t, sizeof(t));
	return 0;
}

int str2cardkey(char *str, BYTE* cardkey)
{
	int ret;
	if(gOptions.CardkeyKeypad)
	{
		ret= strkeycard(str,cardkey);
		DBPRINTF("str2cardkey:%d||%s:%d\n",cardkey[0],(char*)cardkey,ret);
		return ret;
	}
        memset(cardkey,0xff,6);
        if(str) nstrcpy((char*)cardkey, str, 6);
        return 0;
}

void ExportProxySetting(void)
{
	char buf[128];
	int fd;

	if(gOptions.IClockFunOn)
	{
		fd=open(GetEnvFilePath("USERDATAPATH", "icserver.cfg", buf), O_RDWR|O_CREAT|O_TRUNC|O_SYNC, S_IRWXU|S_IRWXG|S_IRWXO);
		//http proxy server ip&port
		sprintf(buf, "%d.%d.%d.%d:%d\n",
			gOptions.ProxyServerIP[0], gOptions.ProxyServerIP[1], gOptions.ProxyServerIP[2], gOptions.ProxyServerIP[3],
			gOptions.ProxyServerPort);
		write(fd, buf, strlen(buf));
		//serial number
		sprintf(buf, "%s\n", LoadStrOld("~SerialNumber"));
		write(fd, buf, strlen(buf));
		//Enabled sign
		sprintf(buf, "%d\n", gOptions.EnableProxyServer);
		write(fd, buf, strlen(buf));
		close(fd);
	}
}

#define PAP_SECRETS	"/etc/ppp/pap-secrets"
#define	CDMA		"/etc/ppp/peers/cdma"
#define GPRS		"/etc/ppp/peers/gprs"
#define CONNECT_CDMA	"/etc/ppp/peers/connect_cdma"
#define CONNECT_GPRS	"/etc/ppp/peers/connect_gprs"

/*
void SetGSMCDMAPara(int ModemType)
{
	char *tmp=NULL;
	char *tmp2=NULL;
	char buf[1024];
	int fd;

	//pap_secrets
	tmp=LoadStrOld("ModemUserName");
	fd=open(PAP_SECRETS, O_RDWR|O_CREAT|O_TRUNC|O_SYNC, S_IRWXU|S_IRWXG|S_IRWXO);
	if(tmp && strlen(tmp))
	{
		 tmp2 = LoadStrOld("ModemPassword");
		if (tmp2 && strlen(tmp2))
			sprintf(buf, "%s * %s\n", tmp, tmp2);
		else
			sprintf(buf, "%s * *\n", tmp);
	}
	else
		sprintf(buf, "* * \"\" *\n");
	write(fd, buf, strlen(buf));
	close(fd);
	if(tmp && strlen(tmp))
	{
		//gprs cdma script
		fd=open(ModemType==2?GPRS:CDMA, O_RDWR|O_SYNC);
		lseek(fd, 0, SEEK_END);
		sprintf(buf, "name %s\n", tmp);
		write(fd, buf, strlen(buf));
		close(fd);
	}

	//connect_gpra cdma script
        fd=open(ModemType==2?CONNECT_GPRS:CONNECT_CDMA, O_RDWR|O_SYNC);
	//GPRS 2007.04.11
	if (ModemType==2)
	{
        	lseek(fd, 0, SEEK_END);
	        sprintf(buf, "OK AT+CGDCONT=1,\"\IP\"\,\"\%s\"\nOK AT+CGQREQ=1,0,0,3,0,0\n",
			LoadStrOld("APN"));
        	write(fd, buf, strlen(buf));
	}
        lseek(fd, 0, SEEK_END);
        sprintf(buf, "OK ATDT%s\nTIMEOUT 120\nSAY \"\\nwaiting up to 2 minutes fot connect...\\n\"\nCONNECT \"\"\n",
			LoadStrOld("ModemDialNumber"));
        write(fd, buf, strlen(buf));
        close(fd);
}
*/
void modemautosh(void)	//ccc 20071228
{
/*	#ifdef CMUX	//start cmux funtions daemon
	printf("start cmux daemon\n");
	system("./gsmMuxd -p /dev/ttyS3 -b 115200 -m m22a  -s /dev/cmux /dev/ptmx /dev/ptmx");
	#endif

#ifndef CMUX
	if(gOptions.IsConnectModem)	//ccc
	check_sim_card();		//check sim card id
#endif

	if(opendir("/etc/ppp")==NULL)
	{
		system("mkdir /etc/ppp");
		system("chmod 755 /etc/ppp");
	}

	system("cp -f /mnt/mtdblock/chat /usr/sbin/");
	system("chmod 755 /usr/sbin/chat");
	if(opendir("/etc/ppp/peers")==NULL)
	{
		system("mkdir /etc/ppp/peers");
	}
	system("cp -f /mnt/mtdblock/ppp-set/*  /etc/ppp/peers/");
	system("chmod 755 /etc/ppp/peers/*");
	system("mv /etc/ppp/peers/pap-secrets /etc/ppp/");

	#ifdef CMUX	//copy cmux script over write the common script
	system("cp -f /mnt/mtdblock/ppp-set/gprs_cmux  /etc/ppp/peers/gprs");
	#endif

	system("sync");
*/
}

/*void SetGSMCDMAPara(int ModemType)	//ccc 20071228
{
	char *tmp=NULL;
	char *tmp2=NULL;
	char buf[1024];
	int fd;
	static int child_pid=-1;

	modemautosh();		//ccc

	//pap_secrets
	tmp=LoadStrOld("ModemUserName");
	fd=open(PAP_SECRETS, O_RDWR|O_CREAT|O_TRUNC|O_SYNC, S_IRWXU|S_IRWXG|S_IRWXO);
	if(tmp && strlen(tmp))
	{
		tmp2 = LoadStrOld("ModemPassword");
		if (tmp2 && strlen(tmp2))
		sprintf(buf, "%s * %s\n", tmp, tmp2);
		else
		sprintf(buf, "%s * *\n", tmp);
	}
	else
	sprintf(buf, "* * \"\" *\n");
	write(fd, buf, strlen(buf));
	close(fd);
	if(tmp && strlen(tmp))
	{
		//gprs cdma script
		fd=open(ModemType==2?GPRS:CDMA, O_RDWR|O_SYNC);
		lseek(fd, 0, SEEK_END);
		sprintf(buf, "name %s\n", tmp);
		write(fd, buf, strlen(buf));
		close(fd);
	}

	//connect_gpra cdma script
	fd=open(ModemType==2?CONNECT_GPRS:CONNECT_CDMA, O_RDWR|O_SYNC);
	//GPRS 2007.04.11
	if (ModemType==2)
	{
		lseek(fd, 0, SEEK_END);
		// sprintf(buf, "OK AT+CGDCONT=1,\"\IP\"\,\"\%s\"\nOK AT+CGQREQ=1,0,0,3,0,0\n",
		sprintf(buf, "OK AT+CGDCONT=1,\"\IP\"\,\"\%s\"\nOK AT+CGQREQ=1,0,0,0,0,0\n",
		LoadStrOld("APN"));
		write(fd, buf, strlen(buf));
	}

	lseek(fd, 0, SEEK_END);
	sprintf(buf, "OK ATDT%s\nTIMEOUT 30\nSAY \"\\nwaiting 30s connect...\\n\"\nCONNECT \"\"\n",
	LoadStrOld("ModemDialNumber"));
	write(fd, buf, strlen(buf));
	close(fd);
}
*/

#define PRIME_TBLSIZ 65536*255

unsigned int hashpjw(const void *key, int keySize)
{
	unsigned char     *ptr=(unsigned char *)key;
	unsigned int       val=0;
	while (keySize--) {
	   int tmp;
	   val = (val << 4) + (*ptr);
	   if ((tmp = (val & 0xf0000000))!=0) {
		  val = val ^ (tmp >> 24);
		  val = val ^ tmp;
	   }
	   ptr++;
	}
	return val % (PRIME_TBLSIZ-1);
}

unsigned int hashBuffer(TBuffer *buffer)
{
	return hashpjw(buffer->buffer, buffer->bufferSize);
}

TBuffer *createRomBuffer(unsigned int Address, int size)
{
	TBuffer *ret=(TBuffer*)malloc(sizeof(TBuffer));
	ret->buffer=(unsigned char *)Address;
	ret->bufferSize=size;
	ret->bufEnd=ret->buffer+size;
	ret->bufPtr=ret->buffer;
	ret->isRom=1;
	return ret;
}

TBuffer *createRamBuffer(int size)
{
	TBuffer *ret=createRomBuffer((unsigned int)malloc(size), size);
	ret->isRom=0;
	return ret;
}

void freeBuffer(TBuffer *buffer)
{
	if(buffer==NULL) return;
	if((buffer->buffer) && (!buffer->isRom))
		free(buffer->buffer);
	free(buffer);
}


