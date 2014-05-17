/*************************************************

 ZEM 200

 flashdb.c define all functions for database mangement of flash

 Copyright (C) 2003-2005, ZKSoftware Inc.

*************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/time.h>
#include <sys/vfs.h>
#include <errno.h>
#include "ccc.h"
#include "flashdb.h"
#include "utils2.h"
#include "options.h"
#include "main.h"
//#include "lcdmenu.h"
#include "usb_helper.h"
#include "sdcard_helper.h"


typedef struct _FSizes_{  //
	int Total, TotalSector,
	SectorCnt, SectorFree,
	UserCnt, UserFree,
	TmpCnt, TmpFree,
	AttLogCnt, AttLogFree,
	OpLogCnt, OpLogFree,
	AdminCnt, PwdCnt,
	StdTmp, StdUser, StdLog,
	ResTmp, ResUser, ResLog;
}GCC_PACKED TFSizes, *PFSizes;

time_t BaseTime=0;
void resetBaseTime()
{
	BaseTime=0;
}

#define AttLogSize2 8
//transactions
static int fdTransaction=-1;
//users record
static int fdOpLog=-1;
//user sms
//add by ocar for c3
int fdC3User = -1;
static int fdC3CardAuthorize = -1;
static int fdC3Holiday = -1;
static int fdC3TimeZone = -1;
static int fdC3FirstCard = -1;
static int fdC3MultiCardOpenDoor = -1;
int fdC3GuarderEventLog	= -1;
int fdC3InOutFuntLog = -1;
int fdC3TransactionOffset = -1;
int fdC3TransactionBak = -1;
int fdC3TransactionBakOffset = -1;

//#ifdef WEBSERVER
int * pSelectFDFromConentType(int ContentType) /******** Add For Web Server ********/
{
        if (FCT_OPLOG==ContentType){
                return (int*)&fdOpLog;
        }
        else if (FCT_C3USER==ContentType)
		return (int*)&fdC3User;
	else if (FCT_C3CARDAUTHORIZE==ContentType)
		return (int*)&fdC3CardAuthorize;
	else if (FCT_C3HOLIDAY==ContentType)
		return (int*)&fdC3Holiday;
	else if (FCT_C3FIRSTCARD==ContentType)
		return (int*)&fdC3FirstCard;
	else if (FCT_C3TIMEZONE==ContentType)
		return (int*)&fdC3TimeZone;
	else if (FCT_C3MULTICARDOPENDOOR==ContentType)
		return (int*)&fdC3MultiCardOpenDoor;
	else if (FCT_C3GUARDEREVENTLOG==ContentType)
		return (int*)&fdC3GuarderEventLog;
	else if (FCT_C3INOUTFUN==ContentType)
		return (int*)&fdC3InOutFuntLog;
	else
        return NULL;
}
//#endif

//get file handle from File type
int SelectFDFromConentType(int ContentType)
{
	if (FCT_OPLOG==ContentType){
		return fdOpLog;
	}
	else if (FCT_C3USER==ContentType)
		return fdC3User;
	else if (FCT_C3CARDAUTHORIZE==ContentType)
		return fdC3CardAuthorize;
	else if (FCT_C3HOLIDAY==ContentType)
		return fdC3Holiday;
	else if (FCT_C3FIRSTCARD==ContentType)
		return fdC3FirstCard;
	else if (FCT_C3TIMEZONE==ContentType)
		return fdC3TimeZone;
	else if (FCT_C3MULTICARDOPENDOOR==ContentType)
		return fdC3MultiCardOpenDoor;
	else if (FCT_C3GUARDEREVENTLOG==ContentType)
		return fdC3GuarderEventLog;
	else if (FCT_C3INOUTFUN==ContentType)
		return fdC3InOutFuntLog;
	else if(FCT_C3GUARDEREVENTLOG_OFFSET==ContentType)
	{
		return fdC3TransactionOffset;
	}
	else if(FCT_C3GUARDEREVENTLOG_BAK==ContentType)
	{
		return fdC3TransactionBak;
	}
	else if(FCT_C3GUARDEREVENTLOG_OFFSET_BAK==ContentType)
	{
		return fdC3TransactionBakOffset;
	}

	return -1;
}

void SearchFirst(PSearchHandle sh)
{
	sh->fd=SelectFDFromConentType(sh->ContentType);
	lseek(sh->fd, 0, SEEK_SET);
	sh->bufferlen=0;
	sh->datalen=0;  //valid data length
}

void SearchEnd(PSearchHandle sh)
{
	sh->fd=SelectFDFromConentType(sh->ContentType);
	lseek(sh->fd, 0, SEEK_END);
	sh->bufferlen=0;
	sh->datalen=0;  //valid data length
}


void CopyTableToMem(PSearchHandle sh,BYTE *buf,int *len)
{
	sh->fd=SelectFDFromConentType(sh->ContentType);
	lseek(sh->fd, 0, SEEK_SET);
	*len = lseek(sh->fd, 0, SEEK_END);
	lseek(sh->fd, 0, SEEK_SET);
	read(sh->fd, buf, *len);
	sh->bufferlen=0;
	sh->datalen=0;  //valid data length
}

void SearchNewFirst(PSearchHandle sh)
{
	sh->fd=SelectFDFromConentType(sh->ContentType);
	lseek(sh->fd, 0,SEEK_SET );
	sh->bufferlen=0;
	sh->datalen=0;  //valid data length
}

BOOL SearchNext(PSearchHandle sh)
{
	BOOL eof;
//	int tmp;
	eof = TRUE;
	sh->bufferlen=0;
	sh->datalen=0;
	switch(sh->ContentType)
	{
	case FCT_OPLOG:
		if (read(sh->fd, sh->buffer, sizeof(TOPLog))==sizeof(TOPLog)){
			sh->bufferlen=sizeof(TOPLog);
			sh->datalen=sh->bufferlen;
			eof = FALSE;
		}
		else
			lseek(sh->fd, 0, SEEK_END);
		break;
	case FCT_C3USER:
		if (read(sh->fd, sh->buffer, sizeof(TC3User))==sizeof(TC3User)){
			sh->bufferlen=sizeof(TC3User);
	//		printf("FCT_C3USER,  cardno = %d\n",((PC3User)sh->buffer)->cardno);
			if(((PC3User)sh->buffer)->PIN > 0)
				sh->datalen=sh->bufferlen;
			eof = FALSE;
		}
		break;
	case FCT_C3CARDAUTHORIZE:
		if (read(sh->fd, sh->buffer, sizeof(TC3authorize))==sizeof(TC3authorize)){
			sh->bufferlen=sizeof(TC3authorize);
			if(((PC3authorize)sh->buffer)->pin > 0)
				sh->datalen=sh->bufferlen;
			eof = FALSE;
		}
		break;
	case FCT_C3HOLIDAY:
		if (read(sh->fd, sh->buffer, sizeof(TC3Holiday))==sizeof(TC3Holiday)){
			sh->bufferlen=sizeof(TC3Holiday);
			if(((PC3Holiday)sh->buffer)->holidayday > 0)
				sh->datalen=sh->bufferlen;
			eof = FALSE;
		}
		break;
	case FCT_C3FIRSTCARD:
		if (read(sh->fd, sh->buffer, sizeof(TC3Firstcardopendoor))==sizeof(TC3Firstcardopendoor)){
			sh->bufferlen=sizeof(TC3Firstcardopendoor);
			if(((PC3Firstcardopendoor)sh->buffer)->pin > 0)
				sh->datalen=sh->bufferlen;
			eof = FALSE;
		}
		break;
	case FCT_C3TIMEZONE:
		if (read(sh->fd, sh->buffer, sizeof(TC3Timezone))==sizeof(TC3Timezone)){
			sh->bufferlen=sizeof(TC3Timezone);
			if(((PC3Timezone)sh->buffer)->timezoneid > 0)
				sh->datalen=sh->bufferlen;
			eof = FALSE;
		}
		break;
	case FCT_C3MULTICARDOPENDOOR:
		if (read(sh->fd, sh->buffer, sizeof(TC3Multicardassemble))==sizeof(TC3Multicardassemble)){
			sh->bufferlen=sizeof(TC3Multicardassemble);
			if(((PC3Multicardassemble)sh->buffer)->index > 0)
				sh->datalen=sh->bufferlen;
			eof = FALSE;
		}
		break;
	case FCT_C3GUARDEREVENTLOG:
		if (read(sh->fd, sh->buffer, sizeof(TC3AcessLog))==sizeof(TC3AcessLog))
		{
			sh->bufferlen=sizeof(TC3AcessLog);
			//printf("TC3AcessLog:pin =%d\n", ((PC3AcessLog)sh->buffer)->pin);
			//if(((PC3AcessLog)sh->buffer)->cardno > 0)
				sh->datalen=sh->bufferlen;
			eof = FALSE;
		}
		else
		{
			printf("FCT_C3GUARDEREVENTLOG read TC3AcessLog: failed\n");
		}
		break;
	case FCT_C3INOUTFUN:
		if (read(sh->fd, sh->buffer, sizeof(TC3InOutFunDefine))==sizeof(TC3InOutFunDefine))
		{
			sh->bufferlen=sizeof(TC3InOutFunDefine);
			if(((PC3InOutFunDefine)sh->buffer)->index > 0)
			{
				sh->datalen=sh->bufferlen;
			}
			eof = FALSE;
		}
		else
		{
			printf("FCT_C3INOUTFUN  read err \n");
		}
		break;
	case FCT_C3GUARDEREVENTLOG_OFFSET:
		if (read(sh->fd, sh->buffer, sizeof(TFdOffset))==sizeof(TFdOffset))
		{
			sh->bufferlen=sizeof(TFdOffset);
			if(FCT_C3GUARDEREVENTLOG_OFFSET == (((PFdOffset)sh->buffer)->ContentType) && (((PFdOffset)sh->buffer)->currpos > 0))
				sh->datalen=sh->bufferlen;
			eof = FALSE;
		}
		break;
	case FCT_C3GUARDEREVENTLOG_BAK:
		if (read(sh->fd, sh->buffer, sizeof(TC3AcessLog))==sizeof(TC3AcessLog))
		{
			sh->bufferlen=sizeof(TC3AcessLog);
			//printf("TC3AcessLog:pin =%d\n", ((PC3AcessLog)sh->buffer)->pin);
			//if(((PC3AcessLog)sh->buffer)->cardno > 0)
				sh->datalen=sh->bufferlen;
			eof = FALSE;
		}
		else
		{
			printf("FCT_C3GUARDEREVENTLOG_BAK read TC3AcessLog: failed\n");
		}
		break;
	case FCT_C3GUARDEREVENTLOG_OFFSET_BAK:
		if (read(sh->fd, sh->buffer, sizeof(TFdOffset))==sizeof(TFdOffset))
		{
			sh->bufferlen=sizeof(TFdOffset);
			if(FCT_C3GUARDEREVENTLOG_OFFSET_BAK == (((PFdOffset)sh->buffer)->ContentType) && (((PFdOffset)sh->buffer)->currpos > 0))
			{
				sh->datalen=sh->bufferlen;
			}
			eof = FALSE;
		}
		break;

	}
	return eof;
}

//append or overwrite data to file
int SearchAndSave(int ContentType, char *buffer, U32 size)
{
	int fd = 0;
	int rc = 0;
//	char *cbuf=NULL;

	fd = SelectFDFromConentType(ContentType);
	switch(ContentType)
	{
	case FCT_C3GUARDEREVENTLOG:
		lseek(fd, 0, SEEK_END);
		break;
	case FCT_OPLOG:
		lseek(fd, 0, SEEK_END);
		break;
	case FCT_C3GUARDEREVENTLOG_OFFSET:
		if (FDB_GetFdOffset(0, NULL))
		{
			lseek(fd, -1*sizeof(TFdOffset), SEEK_CUR);
		}
		else
		{
			lseek(fd, 0, SEEK_END);
		}
		break;
	case FCT_C3GUARDEREVENTLOG_BAK:
		lseek(fd, 0, SEEK_END);
		break;
	case FCT_C3GUARDEREVENTLOG_OFFSET_BAK:
		if (FDB_GetFdOffset(0, NULL))
		{
			lseek(fd, -1*sizeof(TFdOffset), SEEK_CUR);
		}
		else
		{
			lseek(fd, 0, SEEK_END);
		}
		break;
	}

	rc=((write(fd, buffer, size)==size)?FDB_OK:FDB_ERROR_IO);

	if((ContentType==FCT_C3GUARDEREVENTLOG)&&(rc==FDB_OK))
	{
		CurGuarderEventLogCount+=size/sizeof(TC3AcessLog);
		//if(CurGuarderEventLogCount%400==0)
			//RefreshJFFS2Node(fd, 400*AttLogSize2);

		if(CurGuarderEventLogCount >= gOptions.MaxAttLogCount*10000 )
		{
			int i;
			i = FDB_DelOldC3GuarderEventLog(gOptions.DelRecord);

			//i = FDB_DelOldC3GuarderEventLog(gOptions.MaxAttLogCount*1000);
			printf("FDB_DelOldC3GuarderEventLog reserve logcount = %d\n",i);
		}

	}

	return rc;
}


int FDB_InitDBs(BOOL OpenSign)
{
	int i = 0;
	U8 buf[1024] = {0};
	TSearchHandle sh;

	BaseTime=0;
	CurAttLogCount=0;
	CurGuarderEventLogCount = 0;
	CurGuarderEventLogCount2 = 0;
	if (OpenSign)
	{

		fdOpLog=open(GetEnvFilePath("USERDATAPATH", "oplog.dat", buf), O_RDWR|O_CREAT|O_SYNC, S_IRWXU|S_IRWXG|S_IRWXO);
				//add by ocar for c3
		fdC3User=open(GetEnvFilePath("USERDATAPATH", "user.dat", buf), O_RDWR|O_CREAT|O_SYNC, S_IRWXU|S_IRWXG|S_IRWXO);
		fdC3CardAuthorize=open(GetEnvFilePath("USERDATAPATH", "userauthorize.dat", buf), O_RDWR|O_CREAT|O_SYNC, S_IRWXU|S_IRWXG|S_IRWXO);
		fdC3Holiday=open(GetEnvFilePath("USERDATAPATH", "holiday.dat", buf), O_RDWR|O_CREAT|O_SYNC, S_IRWXU|S_IRWXG|S_IRWXO);
		fdC3TimeZone=open(GetEnvFilePath("USERDATAPATH", "timezone.dat", buf), O_RDWR|O_CREAT|O_SYNC, S_IRWXU|S_IRWXG|S_IRWXO);
		fdC3MultiCardOpenDoor=open(GetEnvFilePath("USERDATAPATH", "multicard.dat", buf), O_RDWR|O_CREAT|O_SYNC, S_IRWXU|S_IRWXG|S_IRWXO);
		fdC3FirstCard=open(GetEnvFilePath("USERDATAPATH", "firstcard.dat", buf), O_RDWR|O_CREAT|O_SYNC, S_IRWXU|S_IRWXG|S_IRWXO);
		fdC3GuarderEventLog=open(GetEnvFilePath("USERDATAPATH", "transaction.dat", buf), O_RDWR|O_CREAT|O_SYNC, S_IRWXU|S_IRWXG|S_IRWXO);
		fdC3InOutFuntLog=open(GetEnvFilePath("USERDATAPATH", "inoutfun.dat", buf), O_RDWR|O_CREAT|O_SYNC, S_IRWXU|S_IRWXG|S_IRWXO);

		fdC3TransactionOffset = open(GetEnvFilePath("USERDATAPATH", "fdoffset.dat", buf), O_RDWR|O_CREAT|O_SYNC, S_IRWXU|S_IRWXG|S_IRWXO);
		//fdC3TransactionBak=open(GetEnvFilePath("USERDATAPATH", "../BKtransaction.dat", buf), O_RDWR|O_CREAT|O_SYNC, S_IRWXU|S_IRWXG|S_IRWXO);
		fdC3TransactionBak=open("/mnt/ramdisk/BKtransaction.dat", O_RDWR|O_CREAT|O_SYNC, S_IRWXU|S_IRWXG|S_IRWXO);
		fdC3TransactionBakOffset = open(GetEnvFilePath("USERDATAPATH", "BakOffset.dat", buf), O_RDWR|O_CREAT|O_SYNC, S_IRWXU|S_IRWXG|S_IRWXO);
	}

	//get log count
	memset(buf, 0, 1024);
	CurGuarderEventLogCount = 0;
	sh.ContentType=FCT_C3GUARDEREVENTLOG;
	sh.buffer=buf;
	SearchFirst(&sh);
	while(!SearchNext(&sh))
	{
		CurGuarderEventLogCount++;
	}

	return i;
}

void FDB_FreeDBs(void)
{
	close(fdTransaction);
	close(fdOpLog);

	close(fdC3User);
	close(fdC3CardAuthorize);
	close(fdC3Holiday);
	close(fdC3FirstCard);
	close(fdC3TimeZone);
	close(fdC3MultiCardOpenDoor);
	close(fdC3GuarderEventLog);
	close(fdC3TransactionOffset);
	close(fdC3TransactionBak);
	close(fdC3TransactionBakOffset);
}

int GetDataInfo(int ContentType, int StatType, int Value)
{
	int tmp;
	unsigned char buf[1024];
	TSearchHandle sh;

	sh.ContentType=ContentType;
	sh.buffer=buf;

	tmp = 0;
	if(StatType==STAT_NEWVALIDLEN)
		SearchNewFirst(&sh);
	else
		SearchFirst(&sh);
	while(!SearchNext(&sh)){
		switch(StatType)
		{
		case STAT_COUNT:
//			if (sh.datalen>0) tmp++;
			if (sh.datalen>0 && *(int *)buf != 0xffffffff)//过滤脏数据

				tmp++;
			break;
		case STAT_VALIDLEN:
			tmp+=sh.datalen;
			break;
		case STAT_CNTADMINUSER:
			//if ((sh.datalen>0)&&(ISADMIN(((PUser)sh.buffer)->Privilege))) tmp++;
			break;
		case STAT_CNTADMIN:
			//if ((sh.datalen>0)&&(Value & (((PUser)sh.buffer)->Privilege))) tmp++;
			break;
		case STAT_CNTPWDUSER:
			//if ((sh.datalen>0)&&(((PUser)sh.buffer)->Password[0])) tmp++;
			break;
		case STAT_CNTTEMPLATE:
			//if ((sh.datalen>0)&&((((PTemplate)sh.buffer)->PIN)==Value)) tmp++;
			break;
		}
	}
	return tmp;
}

int TruncFDAndSaveAs(int fd, char *filename, char *buffer, int size)
{
	if (fd > 0) close(fd);
	fd = open(filename, O_RDWR|O_CREAT|O_TRUNC|O_SYNC, S_IRWXU|S_IRWXG|S_IRWXO);
	if (buffer!=NULL)
	{
		write(fd, buffer, size);
	}
	sync();
	return fd;
}

int FDB_DelOldC3GuarderEventLog(int delCount)
{
	TFdOffset FdOffset;
	printf("FDB_DelOldC3GuarderEventLog() %d old att logs will be truncated\n", delCount);
	delFileHead(fdC3GuarderEventLog, delCount*sizeof(TC3AcessLog));

	CurGuarderEventLogCount=FDB_CntC3EvenLog();
	printf("FDB_DelOldC3GuarderEventLog() %d old att logs inoutstate\n", CurGuarderEventLogCount);

	FDB_GetFdOffset(FCT_C3GUARDEREVENTLOG_OFFSET, &FdOffset);
	FdOffset.currpos -= delCount*sizeof(TC3AcessLog);
	if(FdOffset.currpos < 0)
	{
		FdOffset.currpos = 0;
	}

	if(FDB_OK == FDB_AddFdOffset(&FdOffset))
	{
		//printf("FDB_AddFdOffset FDB_OK  FdOffset.ContentType: %d FdOffset.currpos: %d\n", FdOffset.ContentType, FdOffset.currpos);
	}
	else
	{
		printf("FDB_AddFdOffset FDB_ERROR_IO\n");
	}

	return CurGuarderEventLogCount;
}


int FDB_ClearData_helper(int ContentType, int delTemp)
{
	char buf[80];

	if ((ContentType==FCT_ALL) || (ContentType==FCT_OPLOG))
	{
		fdOpLog = TruncFDAndSaveAs(fdOpLog, GetEnvFilePath("USERDATAPATH", "oplog.dat", buf), NULL, 0);
	}

	if(ContentType==FCT_ALL || ContentType == FCT_C3USER)
	{
		fdC3User = TruncFDAndSaveAs(fdC3User, GetEnvFilePath("USERDATAPATH", "user.dat", buf), NULL, 0);
	}
	if(ContentType==FCT_ALL || ContentType == FCT_C3CARDAUTHORIZE)
	{
		fdC3CardAuthorize = TruncFDAndSaveAs(fdC3CardAuthorize, GetEnvFilePath("USERDATAPATH", "userauthorize.dat", buf), NULL, 0);
	}
	if(ContentType==FCT_ALL || ContentType == FCT_C3HOLIDAY)
	{
		fdC3Holiday = TruncFDAndSaveAs(fdC3Holiday, GetEnvFilePath("USERDATAPATH", "holiday.dat", buf), NULL, 0);
	}
	if(ContentType==FCT_ALL || ContentType == FCT_C3TIMEZONE)
	{
		fdC3TimeZone = TruncFDAndSaveAs(fdC3TimeZone, GetEnvFilePath("USERDATAPATH", "timezone.dat", buf), NULL, 0);
	}
	if(ContentType==FCT_ALL || ContentType == FCT_C3FIRSTCARD)
	{
		fdC3Holiday = TruncFDAndSaveAs(fdC3Holiday, GetEnvFilePath("USERDATAPATH", "firstcard.dat", buf), NULL, 0);
	}
	if(ContentType==FCT_ALL || ContentType == FCT_C3MULTICARDOPENDOOR)
	{
		fdC3MultiCardOpenDoor = TruncFDAndSaveAs(fdC3MultiCardOpenDoor, GetEnvFilePath("USERDATAPATH", "multicard.dat", buf), NULL, 0);
	}
	if(ContentType==FCT_ALL || ContentType == FCT_C3GUARDEREVENTLOG)
	{
		fdC3GuarderEventLog = TruncFDAndSaveAs(fdC3GuarderEventLog, GetEnvFilePath("USERDATAPATH", "transaction.dat", buf), NULL, 0);
		fdC3TransactionOffset = TruncFDAndSaveAs(fdC3TransactionOffset, GetEnvFilePath("USERDATAPATH", "fdoffset.dat", buf), NULL, 0);
		fdC3TransactionBakOffset = TruncFDAndSaveAs(fdC3TransactionBakOffset, GetEnvFilePath("USERDATAPATH", "BakOffset.dat", buf), NULL, 0);
	}

	if(ContentType == FCT_C3GUARDEREVENTLOG_BAK)
	{
		fdC3TransactionBak = TruncFDAndSaveAs(fdC3TransactionBak, "/mnt/ramdisk/BKtransaction.dat", NULL, 0);
	}

	if(ContentType==FCT_ALL || ContentType == FCT_C3INOUTFUN)
	{
		fdC3InOutFuntLog = TruncFDAndSaveAs(fdC3InOutFuntLog, GetEnvFilePath("USERDATAPATH", "inoutfun.dat", buf), NULL, 0);
	}

	//flush the cached data to disk
	sync();
	return FDB_OK;
}

int FDB_ClearData(int ContentType)
{
	return FDB_ClearData_helper(ContentType, 1);
}

//为适应 dataapi 接口修改		zhangwei		2008-7-11
int FDB_ClearData2(int ContentType)
{
	return FDB_ClearData_helper(ContentType, 0);
}

int FDB_GetSizes(char* Sizes)
{
	PFSizes p=(PFSizes)Sizes;

	memset((void*)p, 0, sizeof(TFSizes));

	p->OpLogCnt=GetDataInfo(FCT_OPLOG, STAT_COUNT, 0);
	p->StdUser=gOptions.MaxUserCount*100;
	p->StdLog=gOptions.MaxAttLogCount*10000;
	p->ResUser=p->StdUser-p->UserCnt;
	p->ResLog=p->StdLog-p->AttLogCnt;

	return sizeof(TFSizes);
}


int FDB_CntData(int fct)
{
	return GetDataInfo(fct, STAT_COUNT, 0);
}


int FDB_AddAcessEvenLog(U32 cardno,U32 pin,BYTE verified,BYTE ReaderID,
	BYTE EventType,BYTE inoutstate,time_t time_second)
{
	char buf[128] = {0};
	int size;
	TC3AcessLog log;
	int RetOpenSDCard = 0;
	log.pin = pin;
	log.cardno= cardno;
	log.verified = verified;
	log.EventType = EventType;
	log.doorID = ReaderID;
	log.inoutstate = inoutstate;
	log.time_second = time_second;
	size = sizeof(TC3AcessLog);

	//SearchAndSave(FCT_C3GUARDEREVENTLOG_BAK,(char *)&log, size);

	return SearchAndSave(FCT_C3GUARDEREVENTLOG,(char *)&log, size);
}

int FDB_CntUser(void)
{
        return GetDataInfo(FCT_C3USER, STAT_COUNT, 0);
}

int FDB_CntC3EvenLog(void)
{
	return GetDataInfo(FCT_C3GUARDEREVENTLOG, STAT_COUNT, 0);
}

int FDB_AddOPLog(char *log,int len)
{
	int ret;
	int fd;

	fd = SelectFDFromConentType(FCT_OPLOG);
	lseek(fd, 0, SEEK_END);

	ret=((write(fd, log, len)==len)?FDB_OK:FDB_ERROR_IO);

}

int FDB_CheckIntegrate(void)		//进行数据库正确性、完整性检查
{
	return FDB_OK;
}

int writeAndHash(int handle, void *buffer, int size, unsigned int *sum)
{
	if(size<=0) return size;
	unsigned int hash=hashpjw((const void *)buffer, size);
	*sum+=hash;
	return write(handle, buffer, size);
}

static TFdOffset gFdOffset;

PFdOffset FDB_GetFdOffset(int ContentType, PFdOffset FdOffset)
{
	TSearchHandle sh;

	sh.ContentType = ContentType;
	sh.buffer=(unsigned char*)&gFdOffset;
	SearchFirst(&sh);
	while(!SearchNext(&sh))
	{
		if (((PFdOffset)sh.buffer)->ContentType==ContentType)
		{
			if (FdOffset)
				memcpy(FdOffset, sh.buffer, sizeof(TFdOffset));
			return (PFdOffset)sh.buffer;
		}
	}
	return NULL;
}


int FDB_AddFdOffset(PFdOffset FdOffset)
{
	PFdOffset s = NULL;
	int fd = -1;

	switch(FdOffset->ContentType)
	{
		case FCT_C3GUARDEREVENTLOG_OFFSET:
			fd = fdC3TransactionOffset;
			break;
		case FCT_C3GUARDEREVENTLOG_OFFSET_BAK:
			fd = fdC3TransactionBakOffset;
			break;
		default:
			break;
	}

	if ((s=FDB_GetFdOffset(FdOffset->ContentType, NULL))==NULL)
	{
		lseek(fd, 0, SEEK_END);
		//return FDB_ERROR_NODATA;
	}
	else
	{
		if (0==memcmp((void*)FdOffset, (void*)s, sizeof(TFdOffset)))
		{
			return FDB_OK;
		}
		//overwrite
		lseek(fd, -1*sizeof(TFdOffset), SEEK_CUR);
	}

	if (write(fd, (void*)FdOffset, sizeof(TFdOffset))==sizeof(TFdOffset))
	{
		return FDB_OK;
	}
	else
	{
		return FDB_ERROR_IO;
	}
}


off_t GetCurFdrPos(int fd)
{
	off_t currpos = 0;
	currpos = lseek(fd, 0, SEEK_CUR);

	return currpos;
}

off_t GetCurrFdOffset(int ContentType)
{
	int fd = -1;
	fd = SelectFDFromConentType(ContentType);
	return GetCurFdrPos(fd);
}

off_t GetEndFdrPos(int fd)
{
	off_t currpos = 0;
	currpos = lseek(fd, 0, SEEK_END);

	return currpos;
}

off_t GetEndFdOffset(int ContentType)
{
	int fd = -1;
	fd = SelectFDFromConentType(ContentType);
	return GetEndFdrPos(fd);
}


int FDB_ClrFdOffset(void)
{
	return FDB_ClearData(FCT_C3GUARDEREVENTLOG_OFFSET);
}

int FDB_CntFdOffset(void)
{
	return GetDataInfo(FCT_C3GUARDEREVENTLOG_OFFSET, STAT_COUNT, 0);
}

int SaveFdOffset(int file_ID, int FdOffsetContentTpye)
{
	int FileContentType = 0;
	//int FdOffsetContentTpye = 0;

	TFdOffset FdOffset;
	memset(&FdOffset, 0, sizeof(TFdOffset));

	switch(file_ID)
	{
		case 1:
			//user file
			break;
		case 2:
			//userauthorize file
			break;
		case 3:
			//holiday file
			break;
		case 4:
			//timezone file
			break;
		case 5:
			{
				// transaction file
				FileContentType = FCT_C3GUARDEREVENTLOG;
				//FdOffsetContentTpye = FCT_C3GUARDEREVENTLOG_OFFSET;

				FdOffset.ContentType = FdOffsetContentTpye;
				FdOffset.currpos = GetCurrFdOffset(FileContentType);

				printf("SaveFdOffset: FdOffset.currpos: %d\n",FdOffset.currpos);
				if(FdOffset.currpos >= 0)
				{
					if(FDB_OK == FDB_AddFdOffset(&FdOffset))
					{
						printf("FDB_AddFdOffset FDB_OK\n");
						return FDB_OK;
					}
					else
					{
						printf("SaveFdOffset FDB_AddFdOffset failed! file_ID: %d\n",file_ID);
						return FDB_ERROR_OP;
					}
				}
				else
				{
					printf("SaveFdOffset GetCurrFdOffset failed! file_ID: %d\n",file_ID);
					return FDB_ERROR_DATA;
				}
			}
			break;
		case 6:
			//firstcard file
			break;
		case 7:
			//multimcard file
			break;
		case 8:
			//inoutfun file
			break;
	}

	return FDB_OK;
}

int FDB_ForDataFun(int ContentType, int fromFirst, ForARecFun fun, int MaxRecCount, void *param)
{
	TSearchHandle sh;
	int count=0;
	char buf[2048]={0};
	sh.buffer=buf;
	sh.ContentType=ContentType;
	sh.buffer=buf;

	printf("FDB_ForDataFun ContentType = %d\n",ContentType);

	if(fromFirst)
		SearchFirst(&sh);
	else
		sh.fd=SelectFDFromConentType(ContentType);
	while(!SearchNext(&sh))
	{
//		printf("sh.datalen =%d,\n",sh.datalen);
		//zsliu change 2008-11-06
		if(sh.datalen > 0)
		{
			int ret;
//			printf("sh.datalen =%d,\n",sh.datalen);
//			DEBUGTRACE(sh.buffer,sh.datalen);

			ret=fun(sh.buffer, count, param);
			if(ret<0)
				break;
			if(ret>0)
				count++;
			if(MaxRecCount!=NO_LIMIT)
			if(count>=MaxRecCount)
				break;
		}
		//zsliu change end ... ...
	}
	return count;
}

int FDB_ForAllData(int ContentType, ForARecFun fun, int MaxRecCount, void *param)
{
	return FDB_ForDataFun(ContentType, TRUE, fun, MaxRecCount, param);
}

int FDB_ForDataFromCur(int ContentType, ForARecFun fun, int MaxRecCount, void *param)
{
	return FDB_ForDataFun(ContentType, FALSE, fun, MaxRecCount, param);
}

int SaveTableDataToFile(int FCT,char *buf,int size)
{
	if(-1 == FCT)
	{
		printf("file no find\n");
		return -1;
	}
	BYTE SNLen = 0;
	BYTE FirLen = 0;
	TSearchHandle sh;
	sh.ContentType = FCT;
	SearchFirst(&sh);
	SNLen = strlen(SerialNumber);
	FirLen = strlen(gOptions.FirmVer);
	write(sh.fd,&SNLen,sizeof(BYTE));
	write(sh.fd,SerialNumber,SNLen);
	write(sh.fd,&FirLen,sizeof(BYTE));
	write(sh.fd,gOptions.FirmVer,FirLen);
	if(write(sh.fd, buf, size) != size);
	{
		printf("----------------------len %d\n",size);
	}
	sync();

	return 0;
}

int FDB_ForData_C3(int ContentType, off_t fdOffset,ForARecFun fun, int MaxRecCount, void *param)
{
	TSearchHandle sh;
	int count=0,i;
	unsigned   char buf[2048]={0};
	sh.buffer=buf;
	sh.ContentType=ContentType;
	sh.buffer=buf;

	printf("FDB_ForData_C3 ContentType = %d ,fdOffset = %d \n",ContentType,fdOffset);

	if(fdOffset ==0)
		SearchFirst(&sh);
	else
	{
		SearchFirst(&sh);

		lseek(sh.fd, fdOffset, SEEK_SET);

		/*
		for(i= 0;i< DataIndex;i++)
		{
			if(!SearchNext(&sh))
				return 0;
		}
		*/
	}
	while(!SearchNext(&sh))
	{
//		if(sh.datalen > 0)
//		if(sh.datalen > 0  &&
//			(buf[0] != 0xff   &&  buf[1] != 0xff
//
		//printf("sh.datalen: %d  buf[3] != 0xff: %d\n", sh.datalen,  *(U32 *)&buf[0] != 0xffffffff);;
//		&& buf[2] != 0xff   &&  buf[3] != 0xff)) //ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½
		if(sh.datalen > 0  &&  *(U32 *)&buf[0] != 0xffffffff)//过滤脏数据
		{
			int ret;

//			printf("source data---Num=%d---",count);
//			DEBUGTRACE2(sh.buffer,sh.datalen );

			ret=fun(sh.buffer, count, param);
/*
			printf("cardno : %d, pin %d, verified: %d, doorID: %d, EventType: %d, inoutstate: %d, time_second: %d\n",
			((PC3AcessLog)sh.buffer)->cardno,
			((PC3AcessLog)sh.buffer)->pin,
			((PC3AcessLog)sh.buffer)->verified,
			((PC3AcessLog)sh.buffer)->doorID,
			((PC3AcessLog)sh.buffer)->EventType,
			((PC3AcessLog)sh.buffer)->inoutstate,
			((PC3AcessLog)sh.buffer)->time_second
			);
*/
			if(ret<0)
				break;
			if(ret>0)
				count++;
			if(MaxRecCount!=NO_LIMIT)
			if(count>=MaxRecCount)
				break;
		}
		//zsliu change end ... ...
	}
	return count;

}


typedef struct __catInfo{
	TBuffer *buffer;
	time_t t;
}TCatInfo;

int checkfilestatus(const char* filename)
{
	struct stat statbuf;
	memset(&statbuf, 0, sizeof(stat));

	if(stat(filename,&statbuf)==-1)
	{
		DBPRINTF("error at stat file(checkfilestatus) %s\n",filename);
		return -1;
	}

	//test file type
	char *ptr = NULL;
	if(S_ISREG(statbuf.st_mode))
	{
		ptr = "regular";

	}
	else if(S_ISDIR(statbuf.st_mode))
	{
		ptr = "directory";
	}
	else if(S_ISCHR(statbuf.st_mode))
	{
		ptr = "char special";
	}
	else if(S_ISBLK(statbuf.st_mode))
	{
		ptr = "block special";
	}
	else if(S_ISFIFO(statbuf.st_mode))
	{
		ptr = "fifo";
	}
	else
	{
		ptr = "Others";
	}

	printf("file type ---------- %s\n", ptr);

	return statbuf.st_size;
}

int readfile(const char* filename,char *buf)
{
	struct stat statbuf;
	int fd = -1;
	int read_size = 0;


	memset(&statbuf, 0, sizeof(stat));
	fd=open(filename, O_RDONLY);
	if(fd < 0)
	{
		return -1;
	}
	stat(filename,&statbuf);
	read_size=read(fd,buf,statbuf.st_size);
        DBPRINTF("(readfile)read file %s success size %d\n ",filename,read_size);
	close(fd);
	return read_size;

}

//add by oscar for c3
int FDB_GetC3UserByCard(int *card, PC3User user)
{
	TSearchHandle sh;
//	int value;
	int i =0;
	TC3User userdatabuf;

	sh.ContentType=FCT_C3USER;
	sh.buffer = (char *)&userdatabuf;

	SearchFirst(&sh);

	while(!SearchNext(&sh))
	{
		if((sh.datalen>0) && ( *card == ((PC3User)sh.buffer)->cardno) && *card != 0)
		{
			memcpy(user, sh.buffer, sizeof(TC3User));
			return 1;
		}
		i++;
	}

	return 0;
}

//add by oscar for c3
int FDB_GetC3UserByPwd(char * pwd, PC3User user)
{
	TSearchHandle sh;
//	int value;
	int i =0;
	TC3User userdatabuf;

	sh.ContentType=FCT_C3USER;
	sh.buffer = (char *)&userdatabuf;
	printf("pwd=%s\n",pwd);
	SearchFirst(&sh);

	while(!SearchNext(&sh))
	{
		if((sh.datalen>0) && strcmp(pwd,((PC3User)sh.buffer)->password)== 0)
		{
			memcpy(user, sh.buffer, sizeof(TC3User));
			return 1;
		}
		i++;
	}

	return 0;
}

//找寻日期的节日期信息情况(是否为节假日，假日类型为多少
int FDB_GetholidayInfo(TTime time,int *holidaytype)
{
	TSearchHandle sh;
	int year,month,day,curdayinfo,curdayinfo2;//ï¿½

	TC3Holiday databuf;

	year = time.tm_year + 1900;
	month = time.tm_mon+1;
	day = time.tm_mday;
	curdayinfo = year*10000+month*100+day;
	curdayinfo2 = month*100+day;

	printf("YEAR = %d,MONTH= %d,DAY = %d,curdayinfo = %d ",year,month,day,curdayinfo);
	sh.ContentType=FCT_C3HOLIDAY;
	sh.buffer = (char *)&databuf;

	SearchFirst(&sh);
	while(!SearchNext(&sh))
	{
//		printf("Database holidayyearmonthday = %d\n ",((PC3Holiday)sh.buffer)->holidayday);

		if(sh.datalen>0)
		{
			if(((PC3Holiday)sh.buffer)->loop == 2 //loop;	1年循环，月日相等即可，2必须年月日相等
			&& ((PC3Holiday)sh.buffer)->holidayday == curdayinfo)
			{
				*holidaytype =  ((PC3Holiday)sh.buffer)->holidaytype;
				return 1;
			}
			else if(((PC3Holiday)sh.buffer)->loop == 1)//loop;1年循环，月日相等即可
			{
				if((((PC3Holiday)sh.buffer)->holidayday) % 10000 == curdayinfo2)
				{
					*holidaytype =  ((PC3Holiday)sh.buffer)->holidaytype;
					return 1;
				}
			}
		}
	}
	return 0;
}

//找当天的三个时段
//先比较是否是假日
int FDB_GetTimezone(int timzoneid,TTime time,PC3Timeperiod Timeperiod)
{
	TSearchHandle sh;
	int weekday,holidaytype,n;//,i
	int isholiday = 0;
	TC3Timezone databuf;

	weekday =  n = 0;
	isholiday = FDB_GetholidayInfo(time,&holidaytype);

	printf("isholiday = %d \n",isholiday);

	if(!isholiday)
		weekday = time.tm_wday;

	sh.ContentType=FCT_C3TIMEZONE;
	sh.buffer = (char *)&databuf;
	SearchFirst(&sh);
	while(!SearchNext(&sh))
	{
		if((sh.datalen>0)
			&& ((PC3Timezone)sh.buffer)->timezoneid == timzoneid)
		{
			if(isholiday)
				n= 	7  + holidaytype-1;
			else
				n = weekday;

			printf("Database timezone = %d  time.tm_wday= %d, weekday = %d \n",((PC3Timezone)sh.buffer)->timezoneid,
				time.tm_wday,n);
			memcpy(Timeperiod, sh.buffer +( n * 3 + 1)*4, sizeof(TC3Timeperiod));
			return 1;
		}
	}
	return 0;

/*
	if(isholiday)//æ¯èåæ¥
		return 2;
	else
		return 3;
*/
}


int	GetTimefromtimezone(int timezoneID,TTime currtime, int *starttime,int *endtime)
{
	TC3Timeperiod Timeperiod;
	U32 hourMin;//,i;

	hourMin = currtime.tm_hour * 100 + currtime.tm_min;

	if(FDB_GetTimezone(timezoneID, currtime,&Timeperiod))
	{
		if(((Timeperiod.time1 & 0xffff) - (Timeperiod.time1 >>16 & 0xffff) > 0) && hourMin >= (Timeperiod.time1 >>16 & 0xffff) &&  hourMin <= (Timeperiod.time1 & 0xffff))
		{
			*starttime = Timeperiod.time1 >>16 & 0xffff;
			*endtime = Timeperiod.time1 & 0xffff;
			return 1;
		}
		else if(((Timeperiod.time2 & 0xffff) - (Timeperiod.time2 >>16 & 0xffff) > 0) && hourMin >= (Timeperiod.time2 >>16 & 0xffff) &&  hourMin <= (Timeperiod.time2 & 0xffff))
		{
			*starttime = Timeperiod.time2 >>16 & 0xffff;
			*endtime = Timeperiod.time2 & 0xffff;
			return 1;
		}
		else if(((Timeperiod.time3 & 0xffff) - (Timeperiod.time3 >>16 & 0xffff) > 0) && hourMin >= (Timeperiod.time3 >>16 & 0xffff) &&  hourMin <= (Timeperiod.time3 & 0xffff))
		{
			*starttime = Timeperiod.time3 >>16 & 0xffff;
			*endtime = Timeperiod.time3 & 0xffff;
			return 1;
		}
		else
			return 0;
	}
	else
		return 0;
}

//判断门区权限
int FDB_GetDoorAuthInfo(int pin,int DoorID)
{
	TSearchHandle sh;
	TC3authorize databuf;

	U32 result;
	result = 0;

	sh.ContentType=FCT_C3CARDAUTHORIZE;
	sh.buffer = (char *)&databuf;
	SearchFirst(&sh);
	while(!SearchNext(&sh))
	{
		if((sh.datalen>0)
			&& ((PC3authorize)sh.buffer)->pin == pin)
		{

			if((DoorID == 1 && (databuf.authorizedoor & 0x01))
			   || (DoorID == 2 && (databuf.authorizedoor & 0x02))
			   || (DoorID == 3 && (databuf.authorizedoor & 0x04))
			   || (DoorID == 4 && (databuf.authorizedoor & 0x08)))
			{
				return 1;
			}
		}
	}
	return 0;
}


//寻找卡片权限组 (权限组包括，门区，时间段),再判断其是否符合时间段及门区权限
int FDB_GetcardauthorizeInfo2(int pin,int DoorID,TTime time)
{
	TSearchHandle sh;
	TC3authorize databuf;

	TC3Timeperiod Timeperiod;

	U32 hourMin;//,i;


	U32 result;

	result = 0;
	hourMin = time.tm_hour * 100 + time.tm_min;

	sh.ContentType=FCT_C3CARDAUTHORIZE;
	sh.buffer = (char *)&databuf;
	SearchFirst(&sh);
	while(!SearchNext(&sh))
	{
		if((sh.datalen>0)
			&& ((PC3authorize)sh.buffer)->pin == pin)
		{
			printf("authorizeInfo  DoorID=%d,authorizedoor=%d,authorizezoneID=%d\n",DoorID,databuf.authorizedoor,databuf.authorizetimezoneid);
					//判断门区权限
			if((DoorID == 1 && (databuf.authorizedoor & 0x01))
			   || (DoorID == 2 && (databuf.authorizedoor & 0x02))
			   || (DoorID == 3 && (databuf.authorizedoor & 0x04))
			   || (DoorID == 4 && (databuf.authorizedoor & 0x08)) 	)
			{
				//时区判断
				if(FDB_GetTimezone(databuf.authorizetimezoneid, time,&Timeperiod))
				{


					U16 StartTime1 = Timeperiod.time1 >>16 & 0xffff;
					U16 EndTime1 = Timeperiod.time1 & 0xffff;

					U16 StartTime2 = Timeperiod.time2 >>16 & 0xffff;
					U16 EndTime2 = Timeperiod.time2 & 0xffff;

					U16 StartTime3 = Timeperiod.time3 >>16 & 0xffff;
					U16 EndTime3 = Timeperiod.time3 & 0xffff;

					if((((EndTime1 - StartTime1) > 0) && (hourMin >= StartTime1) &&  (hourMin <= EndTime1))
					||(((EndTime2 - StartTime2) > 0) && (hourMin >= StartTime2) &&  (hourMin <= EndTime2))
					||(((EndTime3 - StartTime3) > 0) && (hourMin >= StartTime3) &&  (hourMin <= EndTime3)))
					{

							return 1;
					}
					printf("curhourmin= %d,time1=%d-%d,time1=%d-%d,time1=%d-%d \n",
									hourMin,StartTime1, EndTime1,StartTime2,EndTime2,StartTime3, EndTime3);
				}
				else
				{
					printf("FDB_GetTimezone error ......\n ");
				}

			}
//			else
//				printf("authorizeInfo doorArea is error ......DoorID = %d,authorizedoor = %d\n ",DoorID ,databuf.authorizedoor);

		}
	}
	return 0;
}


//找首卡开门信息
int FDB_Getfirstcardopendoor(int pin,PC3Firstcardopendoor firstcardinfo)
{
	TSearchHandle sh;
	char buf[100];

	sh.ContentType=FCT_C3FIRSTCARD;
	sh.buffer = buf;
	SearchFirst(&sh);
	while(!SearchNext(&sh))
	{
		printf("Firstcard cardno = %d , timezone = %d\n ",
			((PC3Firstcardopendoor)sh.buffer)->pin,
			((PC3Firstcardopendoor)sh.buffer)->timezoneid);

		if((sh.datalen>0)
			&& ((PC3Firstcardopendoor)sh.buffer)->pin == pin)
		{
			memcpy(firstcardinfo, (PC3Firstcardopendoor)sh.buffer,sizeof(TC3Firstcardopendoor));
			return 1;
		}
	}

	return 0;
}

int OpenSDCard(void)
{
	int ret = 0;
	int RetDoMountSDCard = 0;

	if (CheckSDCard())
	{
		DBPRINTF("CheckSDCard ... OK\n");

		RetDoMountSDCard = DoMountSDCard();

		if(0 == RetDoMountSDCard)
		{
			ret = 0;
		}
		else
		{
			ret = -1;
			printf("DoUmountSDCard failed\n");
		}
	}
	else
	{
		ret = -2;
		DBPRINTF("Insert sdcard please\n");
	}

	return ret;
}

int FDB_BakData(int ContentType, off_t fdOffset)
{
	TSearchHandle sh;
	char *buf = NULL;
	int FileLen = 0;
	int WriteSize = 0;
	int ret = 0;

	sh.ContentType=ContentType;

	SearchFirst(&sh);

	FileLen = lseek(sh.fd, 0, SEEK_END);
	WriteSize = FileLen - fdOffset;

	buf = (char *)malloc(WriteSize+1);
	if(NULL == buf)
	{
		printf("FDB_BakData  malloc failed size: %d\n",WriteSize);
		ret = -1;
		return ret;
	}
	printf("FDB_BakData ContentType = %d ,fdOffset = %d FileLen: %d WriteSize: %d \n",ContentType,fdOffset, FileLen, WriteSize);

	lseek(sh.fd, fdOffset, SEEK_SET);
	memset(buf, 0 , sizeof(WriteSize+1));
	sh.buffer=buf;
	if (read(sh.fd, sh.buffer, WriteSize) != WriteSize)
	{
		printf("FDB_BakData read TC3AcessLog: failed\n");
		ret = -2;
		if(NULL != buf)
		{
			free(buf);
		}
		return ret;
	}

	sh.ContentType = FCT_C3GUARDEREVENTLOG_BAK;
	SearchFirst(&sh);
	lseek(sh.fd, 0, SEEK_END);
	if(write(sh.fd, sh.buffer, WriteSize)!=WriteSize)
	{
		printf("FDB_BakData write TC3AcessLog: failed\n");
		ret = -3;
		if(NULL != buf)
		{
			free(buf);
		}
		return ret;
	}

	if(NULL != buf)
	{
		free(buf);
	}

	return ret;
}


int GetBKData(void)
{
	int ret = 0;

	TFdOffset FdOffset;
	memset(&FdOffset, 0, sizeof(TFdOffset));
	FDB_GetFdOffset(FCT_C3GUARDEREVENTLOG_OFFSET_BAK, &FdOffset);

	ret = FDB_BakData(FCT_C3GUARDEREVENTLOG, FdOffset.currpos);

	return ret;
}
