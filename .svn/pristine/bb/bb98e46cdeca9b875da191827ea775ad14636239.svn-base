/*************************************************

 ZEM 200

 rs232comm.c

 Copyright (C) 2003-2004, ZKSoftware Inc.

*************************************************/
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <termios.h>
#include "arca.h"
#include "serial.h"
#include "utils.h"
//#include "commu.h"
#include "options.h"
#include "exfun.h"
#include "rs232comm.h"
#include "dlc.h"
#include "commfun.h"


typedef struct _RSHeader_{
	unsigned char HeaderTag[2];
	unsigned char Cmd;
	unsigned char CmdCheck;
	WORD Len;
	WORD CheckSum;
}TRSHeader, *PRSHeader;

extern int MaxLen;

void RS_SendDataPack(serial_driver_t *rs, unsigned char *buf, int len)
{
	U32 etime=GetTickCount();
	DBPRINTF("485 send:%d\n",len);
//// 485收发控制	低电平收,高电平发
	RS485_setmode(TRUE);
	DelayUS(1000);//
	DEBUGTRACE(buf,len);
//	rs->write(0x88);//多发送一个字节，修正485网络干扰
	rs->write_buf(buf,len);
	rs->tcdrain();
	while(rs->get_lsr()!=TIOCSER_TEMT);	//wait for the transmit shift register empty
	DBPRINTF("get_lsr() time:%d\n",GetTickCount()-etime);
//	mmsleep(3*115200/gOptions.RS232BaudRate);
	RS485_setmode(FALSE);
}

int SendRSData(void *buf, int size, void *param)
{
	RS_SendDataPack(*(void **)param, (unsigned char *)buf, size);
	return 0;
}

WORD rsc=0, PS1[256], PS2[256], PSIndex=0;

int RS232WaitRespose(serial_driver_t *rs,int DeviceID, char *OutBuf,int TimeoutMS)
{
	char Sender[SENDER_LEN] = {0};
	int waitingChars = 0;
	char buf[MaxLen];
	memset(buf,0x00,MaxLen);
	BYTE *p=buf;
	int size = 0;
	U32 timeval = 0;
	U32 timeout = 0;
	PProtoHeader pchd = NULL;

	timeval = GetTickCount();
	while(timeout<TimeoutMS)
	{
		size=rs->poll();
		if(size>=1)
		{
			*p = rs->read();
			if(*p  == PROTO_START_TAG)//校验到头字节合适，开始接受下面的数据
			{
				break;
			}
			else
			{
				//BYTE ch = *p;
				//printf("Noise Data:[%x]\n",ch);
			}
		}else
		{
			mmsleep(10);
		}
		timeout = GetTickCount() - timeval;
	}
	if(size<=0)
	{
		printf("--- time out wait for data\n");
		return -1;
	}
	else
	{
		rs->read_buf((char*)p+1, size-1);	//已读一个字节
		pchd = (PProtoHeader)p;

		if(pchd->StartTag == PROTO_START_TAG && pchd->DestAddr == DeviceID)
		{
			int curLen = size;
			int len = sizeof(TProtoHeader) + pchd->DataSize + 3; //2个校验字节+1个包尾 = 3
			printf("get len = %d\n",len);
			//if((curLen)<len)
			{
				timeval = GetTickCount();
				timeout = 0;
				mmsleep(10);
				while(timeout<TimeoutMS)
				{
					size=rs->poll();
					if(size>0)
					{
						if(size>=(MaxLen-curLen))
						{
							size=(MaxLen-curLen);
						}
						rs->read_buf(p +curLen, size);
						curLen+=size;
						timeval = GetTickCount();
						//mmsleep(10);
						//printf("---get-size %d\n",size);
						if(curLen>=MaxLen)
						{

							memcpy(OutBuf,p,MaxLen);
							return MaxLen;
						}
					}else
					{
						mmsleep(10);
					}
					timeout = GetTickCount() - timeval;
				}
				memcpy(OutBuf,p,curLen);
				//printf("WaitTerminalRead==WAIT_TIMEOUT,recived len=%d,total len=%d\n",curLen,len);
				return curLen;
			}
			//else
			{
				printf("---- %2x %d %d\n",pchd->StartTag, pchd->DestAddr,  DeviceID );
				rs->flush_input();
				return -2;
			}
		}
		else
		{
			//printf("zhc no2 ------- %2x %d %d\n",pchd->StartTag, pchd->DestAddr,  DeviceID );
			printf("--BadData(%d)-\n",size);
			rs->flush_input();
			return -3;
		}
	}
}

int RS232Check(serial_driver_t *rs)
{
	int waitingChars = 0;
	char buf[MaxLen];
	int len = 0;
	int i = 0;
	/*sleep(1);
	char Sender[SENDER_LEN];
	//获取发送方ip and port
	memset(Sender, 0, SENDER_LEN);
	memcpy(Sender, (void*)&rs, sizeof(rs));
	SendRSData(buf,4, Sender);*/


	if((waitingChars = rs->poll()) == 0) return 0;

	//DBPRINTF("---Begin poll size:%d   %d\n",waitingChars,COMMBUFLEN_MAX_RS485_232);

	memset(buf,0x00,MaxLen);
	//接收完所有数据，考虑中间中断等，须分多次接收
/*	do
	{
		if(len >= COMMBUFLEN_MAX_RS485_232)
		{
			printf("------RS232Check:Get data len  = %d\n",len);
			break;
		}

		waitingChars = rs->read_buf(buf+len,waitingChars);
		len += waitingChars;

		// 如果是9600, 传输1 byte,约要1ms,
		mmsleep(10);
		//DBPRINTF("Get data  packet len  = %d\n",len);
		waitingChars=rs->poll();
		if(waitingChars > 0)
		{
			i = 5;
		}
		else
		{
			mmsleep(10);
			i--;
		}
	}
	while(i);

	printf("RS232Check:Get data len  = %d\n",len);
*/
	TC3Cmdstruct c3buf;
	int result;
	char sendbuf[MaxLen];
	int sendlen = 0;

	memset(sendbuf, 0 , sizeof(sendbuf));
	i = 5;
	while(i--)
	{
		len = RS232WaitRespose(rs,gOptions.DeviceID,buf,200);
		printf("len %d\n",len);
		//len = 8;
		//buf[0]=0xaa;buf[1]=0x01;buf[2]=0x01;buf[3]=0x00;buf[4]=0x00;buf[5]=0x50;buf[6]=0x3c;buf[7]=0x55;
		if(0 > len)
		{
			return 0;
		}
		memset(&c3buf, 0, sizeof(TC3Cmdstruct));
		DEBUGTRACE(buf,len);
		result = Process_ReceiveData(buf,len,&c3buf.framehead,RS485_232_MODE);

		if(result==  EC_FINISH)
		{
			printf("c3 cmd = %d,data's len = %d \n ",c3buf.command,c3buf.datalen);
			BYTE currcmd = c3buf.command;
	//		if(process_comm_cmd(&c3buf.command,c3buf.buf,c3buf.datalen,sendbuf,&sendlen,RS485_232_MODE) == CMD_SUCCESS)
			process_comm_cmd(&c3buf.command,c3buf.buf,c3buf.datalen,sendbuf,&sendlen,RS485_232_MODE) ;
			{
	//			if(currcmd == c3buf.command)
	//				c3buf.command = 200;//命令执行成功
				c3buf.datalen = sendlen;//
				if(sendlen)
					memcpy(c3buf.buf,sendbuf,sendlen);
				Make_SendData(c3buf.buf,c3buf.command,c3buf.datalen,sendbuf,&sendlen);

				char Sender[SENDER_LEN];
				//获取发送方ip and port
				memset(Sender, 0, SENDER_LEN);
				memcpy(Sender, (void*)&rs, sizeof(rs));
				SendRSData(sendbuf,sendlen, Sender);
				printf("Rs232 runcommcmd OK ! sendlen = %d \n ",sendlen);
				return 0;
			}
	//		else
	//			printf("RS232 runcommcmd error ! \n ");
		}
		else
		{
			printf("-RS232Check error = 0x%x,\n", 	result);
		}
	}
	return -1;
}

