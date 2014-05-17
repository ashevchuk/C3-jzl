//data link layer process by oscar
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <unistd.h>
#include "ccc.h"
#include "arca.h"
#include "dlc.h"
#include "crc16.h"
#include "crc32.h"
#include "options.h"
#include "net/net.h"
#include "proc_commfun.h"
#include "commfun.h"
#include "utils.h"
#include "flashdb.h"
#include "main.h"
#include "dataapi.h"
#include "c4i2c.h"
#include "msg.h"

typedef struct _bigdata_
{
	BYTE  compressed;// = Buf[0];	//数据压缩标志
	U32 dataLen;// = GETDWORD(Buf + 1);	//数据传送长度
	U32 oriLen;// = GETDWORD(Buf + 1 + 4);	//原始数据长度
	U32 hash;// = GETDWORD(Buf + 1 + 8);	//哈希校验码
	U32 packageLen;// = GETDWORD(Buf + 1 + 12);
} praparedata;//大数据传输准备结构


#define BIGBUF_LEN 4*1024*1024

typedef struct _bigdatabuf_{
	U32 len;
	U32 index;
	BYTE bigbuf_cmd;
	BYTE buf[BIGBUF_LEN];
}TBigDataBuf, *PBigDataBuf;


////小包结构体
typedef struct _Recdatabuf_{
	U32 len;
	U32 index;
	BYTE buf[COMMBUFLEN_MAX_TCP];
}TLittleDataBuf, *PLittleDataBuf;


TBigDataBuf CurBigDataBuf;


extern PSharedStateOptions shared_stateoptions_stuff;
extern POverseeRTLog shared_RTLog_stuff;

extern PCommData shared_Comm_stuff;
#define MAX_SEND_DATA_LEN    1024*8

int MaxLen = 5*1024;//大数据包初始长度，根据SDK发的改变

int SendCommCmd(int Cmd, int NewsType, char *SendData, int SendDataLen, int TimeOutMS)
{
	int ret = 0;

	int WaitMainTimeOutMS = TimeOutMS;
	shared_Comm_stuff->CommCmdRet = 1;
	shared_Comm_stuff->CreateCommCmd = 0;
	shared_Comm_stuff->CommSendMark = 1;


	if(SendDataLen > MAX_SEND_DATA_LEN)
	{
		return ERR_SEND_COMMCMD_OVERMAXLEN;
	}

	if(1 == shared_Comm_stuff->MainSendMark)
	{
		shared_Comm_stuff->CommSendMark = 0;
		return ERR_SEND_COMMCMD_MAINING;
	}

	shared_Comm_stuff->CommSendMark = 1;
	shared_Comm_stuff->CreateCommCmd = 1;
	shared_Comm_stuff->Cmd = Cmd;
	shared_Comm_stuff->NewsType = NewsType;

	if(NULL == SendData)
	{
		memset(shared_Comm_stuff->SendData, 0, MAX_SEND_DATA_LEN);
	}
	else
	{
		memcpy(shared_Comm_stuff->SendData, SendData, SendDataLen);
	}

	while((WaitMainTimeOutMS/50) > 0)
	{
		shared_Comm_stuff->CommSendMark = 1;

		//printf("comm SendCommCmd shared_Comm_stuff->NewsType: %d\n", shared_Comm_stuff->NewsType);
		if(0 == shared_Comm_stuff->CommCmdRet)
		{
			ret = SEND_COMMCMD_OK;        //main进程处理NewsType指令完毕。
			return ret;;
		}
		else if(-1 == shared_Comm_stuff->NewsType)
		{
			ret = ERR_SEND_COMMCMD_DATAERR;  //comm进程发送了错误的数据给main进程。比如DoorID > 4  或  SignalID > 2
			return ret;
		}

		mmsleep(50);
		WaitMainTimeOutMS -= 50;
	}

	shared_Comm_stuff->CommSendMark = 0;

	return ERR_SEND_COMMCMD_TIMEOUT;
}


#define 	CDM_RESPONE_OK_NOPARA() 	 *cmd = 200; outbuf[0] = 0; *outbuflen = 0;
#define 	CDM_RESPONE_OK() 	 *cmd = 200

#define 	CDM_RESPONE_ERROR() 	 *cmd = 201;  *outbuflen = 1;

extern int fdExtLog;
extern int fdC3User;

U32 isDeviceToPC = 0;


int process_comm_cmd(BYTE *cmd,BYTE *buf,int inbuflen,BYTE *outbuf,int *outbuflen,int commmode)
{
	printf("runcommcmd  cmd = %d \n",*cmd);

	/*if(RS485_232_MODE == commmode)
	{
		MaxLen = COMMBUFLEN_MAX_RS485_232;
	}
	else
	{
		MaxLen = COMM_BUFLEN_MAX;
	}*/
	switch (*cmd)
	{
	//ComPwd
		case CMD_CONNECT://						//连接  暂时没有启用密码
		{
			BYTE StrCommPwd[17] = {0};
			memset(StrCommPwd, 0, 17);
			printf("inbuflen = %d\n",inbuflen);
			if(inbuflen > 0)
			{
				memcpy(StrCommPwd ,buf ,inbuflen);	//数据总长度
			}
			printf("my pwd is : %s,  pc send pwd is : %s \n",gOptions.ComPwd,StrCommPwd);
			if((inbuflen == 0 &&  gOptions.ComPwd[0] == 0)
				|| (inbuflen && strcmp(StrCommPwd, gOptions.ComPwd)== 0))
			{
				printf("-commkey- OK-- cmd = 0x%x\n",*cmd);
				memset(&CurBigDataBuf, 0, sizeof(TBigDataBuf));//在每次连接成功时，对当前大数据对象进行清零。
				shared_RTLog_stuff->LogReadIndex = 0;
				shared_RTLog_stuff->LogWriteIndex = 0;
				Oversee_RTLog.LogWriteIndex = 0;
				Oversee_RTLog.LogReadIndex = 0;
				CDM_RESPONE_OK_NOPARA();
				return CMD_SUCCESS;
			}
			else
			{
				printf("commkey error---- cmd = 0x%x\n",*cmd);
				CDM_RESPONE_ERROR();
				*outbuf = ERR_CMD_COMKEY_ERROR;
				return ERR_CMD_COMKEY_ERROR;
			}
		}

		break;
		case CMD_DISCONNECT://					//断开连接
			printf("--------------- cmd = 0x%x\n",*cmd);
			CDM_RESPONE_OK_NOPARA();
			return CMD_SUCCESS;
		break;
		case CMD_SET_DEVICE_PARAM://			//设置设备参数
		{
			int result;

			if(inbuflen==0)
			{
				CDM_RESPONE_ERROR();
				*outbuf = ERR_CMD_DATA_PARA_ERROR;
				return ERR_CMD_DATA_PARA_ERROR;
			}
			//支持一次设置多个option
			result=SetOptionNameAndValue(buf, inbuflen);
			printf("SetOptionNameAndValue result = %d\n",result);

			if(TRUE == result)
			{
				int ret = 0;

				printf("------------------------------LoadOptions  pwd is : %s \n",gOptions.ComPwd);

				ret = SendCommCmd(MSG_COMM_CMD,SUBCMD_UPDATE_OPTIONS, NULL, 0,6000);
				if(SEND_COMMCMD_OK == ret)
				{
					printf("---comm Send Cmd to main Success!\n");
					CDM_RESPONE_OK_NOPARA();
					return CMD_SUCCESS;
				}
				else if(ERR_SEND_COMMCMD_TIMEOUT == ret)
				{
					printf("---comm send Cmd to main timeout!\n");
					*outbuf = ERR_CMD_DATA_PARA_ERROR;
					CDM_RESPONE_ERROR();
					return ERR_CMD_DATA_PARA_ERROR;
				}
				else if(ERR_SEND_COMMCMD_DATAERR == ret)
				{
					printf("----comm send data to main  err!\n");
					*outbuf = ERR_CMD_DATA_PARA_ERROR;
					CDM_RESPONE_ERROR();
					return ERR_CMD_DATA_PARA_ERROR;
				}
				else if(ERR_SEND_COMMCMD_OVERMAXLEN == ret)
				{
					printf("----comm send data len is over max len \n");
					*outbuf = ERR_CMD_DATA_PARA_ERROR;
					CDM_RESPONE_ERROR();
					return ERR_CMD_DATA_PARA_ERROR;
				}
				else if(ERR_SEND_COMMCMD_MAINING == ret)
				{
					printf("----main is sending data !\n");
					*outbuf = ERR_CMD_DATA_PARA_ERROR;
					CDM_RESPONE_ERROR();
					return ERR_CMD_DATA_PARA_ERROR;
				}
				CDM_RESPONE_OK_NOPARA();
				return CMD_SUCCESS;
			}
			else
			{
				*outbuf = ERR_CMD_PROCESS_ERROR;
				CDM_RESPONE_ERROR();
				return ERR_CMD_PROCESS_ERROR;
			}
		}
		break;
		case CMD_GET_DEVICE_PARAM://			//读取设备参数
			if(inbuflen==0)
			{
				CDM_RESPONE_ERROR();
				*outbuf = ERR_CMD_DATA_PARA_ERROR;
				return ERR_CMD_DATA_PARA_ERROR;
			}
			*outbuflen=GetOptionNameAndValue(buf, inbuflen,outbuf);

			if(*outbuflen)
			{
				printf("LEN = %d,GET para  is : %s\n",*outbuflen,outbuf);
				CDM_RESPONE_OK();
				return CMD_SUCCESS;
			}
			else
			{
				*outbuf = ERR_CMD_PROCESS_ERROR;
				CDM_RESPONE_ERROR();
				return ERR_CMD_PROCESS_ERROR;
			}
		break;

		case CMD_CONTROL_DEVICE://				//控制设备执行动作
		{
			int SubCmd = buf[0] & 0xff;
   			int Param1 = buf[1] & 0xff;
  			int Param2 = buf[2] & 0xff;
			int Param3 = buf[3] & 0xff;
			int Param4 = buf[4] & 0xff;

			printf("comm rec data form PC: SubCmd = %d, Param1 = %d, Param2 = %d,Param3 =%d, Param4 = %d \n",
					SubCmd, Param1,Param2,Param3,Param4);

			int ret = 0;
			ret = SendCommCmd(MSG_COMM_CMD, SubCmd, buf+1,inbuflen, 2000);
			if(SEND_COMMCMD_OK == ret)
			{
				printf("comm Send Cmd to main Success!\n");
				CDM_RESPONE_OK_NOPARA();
				return CMD_SUCCESS;
			}
			else if(ERR_SEND_COMMCMD_TIMEOUT == ret)
			{
				printf("comm send Cmd to main timeout!\n");
				*outbuf = ERR_CMD_DATA_PARA_ERROR;
				CDM_RESPONE_ERROR();
				return ERR_CMD_DATA_PARA_ERROR;
			}
			else if(ERR_SEND_COMMCMD_DATAERR == ret)
			{
				printf("comm send data to main  err!\n");
				*outbuf = ERR_CMD_DATA_PARA_ERROR;
				CDM_RESPONE_ERROR();
				return ERR_CMD_DATA_PARA_ERROR;
			}
		}
		break;

		case CMD_GET_TABLE_STRUCT:	//获取所有表结构
		{
			GetTableStruct(CurBigDataBuf.buf,&CurBigDataBuf.len);
			if(CurBigDataBuf.len <= MaxLen)//小数据通道
			{
				memcpy(outbuf,CurBigDataBuf.buf,CurBigDataBuf.len);
				*outbuflen = CurBigDataBuf.len;
				printf("----outbuf,outbuflen = %d, ",*outbuflen);
				DEBUGTRACE(outbuf,*outbuflen);

				CDM_RESPONE_OK();
				return CMD_SUCCESS;
			}
			else//大数据通道
			{
				U32 hashdata;
				hashdata = crc32(CurBigDataBuf.buf,CurBigDataBuf.len);

				*cmd = CMD_PREPARE_DATA;
				*outbuf = 0;	//数据压缩标志 0:不压缩 1:压缩
				SET_DWORD(outbuf+1,CurBigDataBuf.len,0);	//数据传送长度(如果没有压缩，等于原始数据长度)
				SET_DWORD(outbuf+1+4,CurBigDataBuf.len,0);;	//原始数据长度
				SET_DWORD(outbuf+1+8,hashdata,0);;	//哈希校验码
				SET_DWORD(outbuf+1+12,MaxLen - 12,0);//每个包传送的最大长度

				printf("datalen = %d,crc32 = %d ,perlen = %d\n",CurBigDataBuf.len,hashdata,MaxLen);
//				DEBUGTRACE2(CurBigDataBuf.buf,CurBigDataBuf.len);
				printf("-----------------debug-------------------------\n");

				*outbuflen = 1+12 +4;
				isDeviceToPC = 1;
				return CMD_SUCCESS;
			}

			return CMD_SUCCESS;
		}
		break;
		case  CMD_DELETE_DATA://	//删除数据
		{
			TLittleDataBuf LittleDataBuf;
			memset(&LittleDataBuf, 0, sizeof(TLittleDataBuf));

			LittleDataBuf.len = inbuflen;	//数据总长度
			LittleDataBuf.index =0;//从0 开始
			printf("receive data ,datalen = %d\n",	LittleDataBuf.len);
			memcpy(LittleDataBuf.buf ,buf,LittleDataBuf.len);

			processBuf_DelDatabase(LittleDataBuf.buf ,LittleDataBuf.len);
			LittleDataBuf.len = 0;
			LittleDataBuf.index = 0;
			CDM_RESPONE_OK_NOPARA();
			return CMD_SUCCESS;
		}
		break;

		case CMD_GET_DATA_COUNT://				//得到数据总数
			*outbuf = 0;
			*outbuflen = 0;
			GetDataCount(buf,inbuflen,outbuf,outbuflen);
			CDM_RESPONE_OK();
			return CMD_SUCCESS;
		break;
		case CMD_GET_REALTIME_LOG://		//下载实时数据
			printf("CMD_GET_REALTIME_LOG cmd = 0x%x\n",*cmd);
			{
				//将share中的数据copy到结构体中
				//memcpy(&Oversee_RTLog,shared_RTLog_stuff,sizeof(TOverseeRTLog));

				if(shared_RTLog_stuff->LogReadIndex != shared_RTLog_stuff->LogWriteIndex )
				{
					memcpy(outbuf,&shared_RTLog_stuff->RTLog[shared_RTLog_stuff->LogReadIndex],sizeof(TC3AcessLog));
					*outbuflen = sizeof(TC3AcessLog);

					printf("RTLog pin=%d,cardno = %d,verified=%d,DoorID=%d,eventype=%d,reserverd=%d,time=%d \n",
							shared_RTLog_stuff->RTLog[shared_RTLog_stuff->LogReadIndex].pin,
							shared_RTLog_stuff->RTLog[shared_RTLog_stuff->LogReadIndex].cardno,
							shared_RTLog_stuff->RTLog[shared_RTLog_stuff->LogReadIndex].verified,
							shared_RTLog_stuff->RTLog[shared_RTLog_stuff->LogReadIndex].doorID,
							shared_RTLog_stuff->RTLog[shared_RTLog_stuff->LogReadIndex].EventType,
							shared_RTLog_stuff->RTLog[shared_RTLog_stuff->LogReadIndex].inoutstate,
							shared_RTLog_stuff->RTLog[shared_RTLog_stuff->LogReadIndex].time_second);

					shared_RTLog_stuff->LogReadIndex ++;
					if(shared_RTLog_stuff->LogReadIndex >= MAXRTLOGCOUNT)
					{
						shared_RTLog_stuff->LogReadIndex = 0;
					}

					//memcpy(shared_RTLog_stuff,&Oversee_RTLog,sizeof(TOverseeRTLog));
					CDM_RESPONE_OK();
					return CMD_SUCCESS;
				}
				else
				{
					TC3AcessLog Log;
					memcpy(&gStateOptions, shared_stateoptions_stuff, sizeof(TStateOptions));
					Log.cardno = gStateOptions.DoorAlarmStatus;//表示门的报警状态，
					Log.pin =  gStateOptions.DoorOpenstatus;// GetDoorState();//表示门的开关状态，
					printf("DoorOpenstatus :0x%08x \n",gStateOptions.DoorOpenstatus);
					Log.verified = VS_OTHER;
					Log.doorID = 0;
					Log.EventType = EVENT_DOORSTATUS;//取门的状态
					Log.inoutstate = 0;
					Log.time_second = OldEncodeTime(&gCurTime);
					memcpy(outbuf,&Log.cardno,sizeof(TC3AcessLog));
					*outbuflen = sizeof(TC3AcessLog);

					CDM_RESPONE_OK();
					return CMD_SUCCESS;
				}
			}
			return CMD_SUCCESS;
		break;

		case CMD_SET_DATA://					//上传数据 PC->Device
		{
			int ret = CMD_SUCCESS;
			TLittleDataBuf LittleDataBuf;
			memset(&LittleDataBuf, 0, sizeof(TLittleDataBuf));

			LittleDataBuf.len = inbuflen;	//数据总长度
			LittleDataBuf.index =0;//从0 开始
			printf("receive data ,datalen = %d\n",	LittleDataBuf.len);
			memcpy(LittleDataBuf.buf ,buf,LittleDataBuf.len);

			ret = BufToDatabase(LittleDataBuf.buf ,LittleDataBuf.len);//,int handle_datafile)
			if(CMD_SUCCESS != ret)
			{
				CurBigDataBuf.len = 0;
				CurBigDataBuf.index = 0;
				CurBigDataBuf.bigbuf_cmd = 0;

				*outbuf = ret;
				CDM_RESPONE_ERROR();
				return ret;
			}

			LittleDataBuf.len = 0;
			LittleDataBuf.index = 0;
			CDM_RESPONE_OK_NOPARA();
			return CMD_SUCCESS;
		}
		break;

		case CMD_GET_DATA://					//下载数据 Device->PC
		{
			TLittleDataBuf LittleDataBuf;
			memset(&LittleDataBuf, 0, sizeof(TLittleDataBuf));

			LittleDataBuf.len = inbuflen;	//数据总长度
			LittleDataBuf.index =0;//从0 开始
			printf("CMD_GET_DATA ,datalen = %d\n",	LittleDataBuf.len);
			memcpy(LittleDataBuf.buf ,buf,LittleDataBuf.len);

			CurBigDataBuf.len = 0;
			CurBigDataBuf.index =0;//从0 开始
			memset(CurBigDataBuf.buf ,0x00,BIGBUF_LEN);

			DatabaseToBuf(LittleDataBuf.buf,LittleDataBuf.len);//,outbuf,outbuflen);

			if(CurBigDataBuf.len <= MaxLen)//小数据通道
			{
				memcpy(outbuf,CurBigDataBuf.buf,CurBigDataBuf.len);
				*outbuflen = CurBigDataBuf.len;
				printf("----outbuf,outbuflen = %d, ",*outbuflen);
				DEBUGTRACE(outbuf,*outbuflen);

				CDM_RESPONE_OK();
				return CMD_SUCCESS;
			}
			else//大数据通道
			{
				U32 hashdata;
				hashdata = crc32(CurBigDataBuf.buf,CurBigDataBuf.len);

				*cmd = CMD_PREPARE_DATA;
				*outbuf = 0;	//数据压缩标志 0:不压缩 1:压缩
				SET_DWORD(outbuf+1,CurBigDataBuf.len,0);	//数据传送长度(如果没有压缩，等于原始数据长度)
				SET_DWORD(outbuf+1+4,CurBigDataBuf.len,0);;	//原始数据长度
				SET_DWORD(outbuf+1+8,hashdata,0);;	//哈希校验码
				SET_DWORD(outbuf+1+12,MaxLen - 12,0);//每个包传送的最大长度

				printf("datalen = %d,crc32 = %d ,perlen = %d\n",CurBigDataBuf.len,hashdata,MaxLen);
//				DEBUGTRACE2(CurBigDataBuf.buf,CurBigDataBuf.len);
				printf("-----------------debug-------------------------\n");

				*outbuflen = 1+12 +4;
				isDeviceToPC = 1;
				return CMD_SUCCESS;
			}
		}
		break;
		case CMD_QUERY_DATA://					//下载大数据
		break;

		case CMD_READ_DATA://读数据  device -> pc
		{
			U32 recbufpos,curdatalen;//本次接收的或要取的数据位置

			recbufpos = GETDWORD(buf);
			CurBigDataBuf.index = recbufpos;
			SET_DWORD(outbuf,CurBigDataBuf.index,0);

			if(recbufpos  >= CurBigDataBuf.len)
			{
				printf("error recbufpos = %d,len = %d\n",recbufpos , CurBigDataBuf.len);
				*outbuf = ERR_CMD_DATA_PARA_ERROR;
				CDM_RESPONE_ERROR();
				return ERR_CMD_DATA_PARA_ERROR;
			}
			if((recbufpos + MaxLen-12) >= CurBigDataBuf.len)
			{
				curdatalen =  CurBigDataBuf.len - recbufpos;
				printf("the last datbufframe ,len = %d\n",curdatalen);
			}
			else
			{
				curdatalen = MaxLen - 12;
			}

			printf("CMD_READ_DATA:  recbufpos = %d,curdatalen = %d\n",recbufpos , curdatalen);

			memcpy(outbuf+4,CurBigDataBuf.buf +CurBigDataBuf.index,curdatalen);
			*outbuflen = 4 + curdatalen;
			CDM_RESPONE_OK();
			return CMD_SUCCESS;
		}
		break;

		case  CMD_PREPARE_DATA:
		{
			praparedata curdata;
			curdata.dataLen = GETDWORD(buf);	//数据总长度
			curdata.packageLen = GETDWORD(buf + 4);//以后每个包长度
			MaxLen = curdata.packageLen;
			printf("prapare bigdata ,datalen = %d,packageLen = %d,\n",
				curdata.dataLen,curdata.packageLen );

			isDeviceToPC = 0;
			CurBigDataBuf.len = curdata.dataLen;
			CurBigDataBuf.index =0;//从0 开始
			CDM_RESPONE_OK_NOPARA();
			return CMD_SUCCESS;

		}
		break;
		case CMD_DATA:
		{
			U32 recbufpos;//本次接收的数据位置

			recbufpos = GETDWORD(buf);

			if(inbuflen >= 4 && (recbufpos + inbuflen -4) <= CurBigDataBuf.len)
			{
					memcpy(CurBigDataBuf.buf + recbufpos,buf +4,inbuflen -4);
					CurBigDataBuf.index += inbuflen -4;
					CDM_RESPONE_OK_NOPARA();
					return CMD_SUCCESS;
			}
			else
			{
				printf("data error ---------recbufpos = %d,len = %d \n",recbufpos, inbuflen -4);
				*outbuf = ERR_CMD_DATA_PARA_ERROR;
				CDM_RESPONE_ERROR();
				return ERR_CMD_DATA_PARA_ERROR;
			}

		}
		break;
		case CMD_BROADCAST://广播 命令寻找IP
		{
				DBPRINTF("%s\n",buf);
//				if(strcmp(buf,"zksoftware") == 0 )
				if(strcmp(buf,"CallSecurityDevice") == 0 )
				{
					sprintf(outbuf, "MAC=%s,IP=%s,SN=%s,Device=%s,Ver=%s", LoadStrOld("MAC"), LoadStrOld("IPAddress"),SerialNumber,DeviceName,MAINVERSION);
					*outbuflen = strlen(outbuf);
					DBPRINTF("Yes!%s,len=%d \n", outbuf, *outbuflen);
					CDM_RESPONE_OK();
					return CMD_SUCCESS;
				}
				else
				{
//					CDM_RESPONE_ERROR();
//					return ERR_NORESPOSE;
				}
		}
		break;
		case CMD_BROAD_MODIYIP://广播 命令修改IP
		{
			int RetBroadModiyIP = 0;
			RetBroadModiyIP = BroadModiyIP(buf, inbuflen);
			printf("RetBroadModiyIP: %d\n", RetBroadModiyIP);

			if(CMD_SUCCESS == RetBroadModiyIP)
			{
				CDM_RESPONE_OK_NOPARA();//
				return CMD_SUCCESS;
			}
			else
			{
				CDM_RESPONE_ERROR();
				*outbuf = RetBroadModiyIP;
				return RetBroadModiyIP;
			}
		}
		break;

		case CMD_SET_BINARYDATA://上传文件  pc ->device
		{
			int ret = 0;

			ret = SaveFile(buf, inbuflen);

			if(CMD_SUCCESS == ret)
			{
				CDM_RESPONE_OK_NOPARA();
				return CMD_SUCCESS;
			}
			else
			{
				CurBigDataBuf.len = 0;
				CurBigDataBuf.index = 0;
				CurBigDataBuf.bigbuf_cmd = 0;

				*outbuf = ret;
				CDM_RESPONE_ERROR();
				return ret;
			}
		}
		break;
		case CMD_GET_BINARYDATA://下载文件   device->pc
		{
			char filename[100] = {0};
			U32 filenamelen = 0;
			int readsize = 0;
			char TmpFileName[100] = {0};

			filenamelen = buf[0];
			memcpy(TmpFileName,&buf[1],filenamelen);

			GetEnvFilePath("USERDATAPATH",TmpFileName, filename);
			printf("filenamelen = %d,filename =%s \n",filenamelen,filename);

			CurBigDataBuf.len = 0;
			CurBigDataBuf.index =0;//从0 开始
			memset(CurBigDataBuf.buf ,0x00,BIGBUF_LEN);

			readsize = checkfilestatus(filename);
			DBPRINTF("read files size is %d\n", readsize);

			if (readsize > 0)
			{
				if ((readsize = readfile(filename, CurBigDataBuf.buf + CurBigDataBuf.index)) < 0)
				{
					CDM_RESPONE_ERROR();
					*outbuf = ERR_CMD_FILE_NOTEXIST;
					return ERR_CMD_FILE_NOTEXIST;
				}

				CurBigDataBuf.len += readsize;

				if(CurBigDataBuf.len <= MaxLen)//小数据通道
				{
					memcpy(outbuf,CurBigDataBuf.buf,CurBigDataBuf.len);
					*outbuflen = CurBigDataBuf.len;
					printf("----outbuf,outbuflen = %d, ",*outbuflen);
					DEBUGTRACE(outbuf,*outbuflen);

					CDM_RESPONE_OK();
					return CMD_SUCCESS;
				}
				else//大数据通道
				{
					U32 hashdata;
					hashdata = crc32(CurBigDataBuf.buf,CurBigDataBuf.len);

					*cmd = CMD_PREPARE_DATA;
					*outbuf = 0;	//数据压缩标志 0:不压缩 1:压缩
					SET_DWORD(outbuf+1,CurBigDataBuf.len,0);	//数据传送长度(如果没有压缩，等于原始数据长度)
					SET_DWORD(outbuf+1+4,CurBigDataBuf.len,0);;	//原始数据长度
					SET_DWORD(outbuf+1+8,hashdata,0);;	//哈希校验码
					SET_DWORD(outbuf+1+12,MaxLen - 12,0);//每个包传送的最大长度

					printf("datalen = %d,crc32 = %d ,perlen = %d\n",CurBigDataBuf.len,hashdata,MaxLen);

					*outbuflen = 1+12 +4;
					isDeviceToPC = 1;
					return CMD_SUCCESS;
				}
			}
			else
			{
				CDM_RESPONE_ERROR();
				*outbuf = ERR_CMD_FILE_NOTEXIST;
				return ERR_CMD_FILE_NOTEXIST;
			}
		}
		break;
		case CMD_HASH_DATA://取哈希值
		{
			U32 hashdata;
			CurBigDataBuf.bigbuf_cmd = buf[0];//  在取哈希值同时，发送大数据处理的命令是下传，还是删除   pc -->device
			hashdata = crc32(CurBigDataBuf.buf,CurBigDataBuf.len);
			SET_DWORD(outbuf,hashdata,0);
			*outbuflen = sizeof(U32);
			printf("---------------crc32 = 0x%x   bigcmd= %d,\n",hashdata,CurBigDataBuf.bigbuf_cmd & 0xff);
			printf("bigdata ,datalen = %d,rece datalen = %d\n",
			CurBigDataBuf.len,CurBigDataBuf.index);
			CDM_RESPONE_OK();
			return CMD_SUCCESS;
		}
		break;
		case CMD_FREE_DATA:
		{
			int ret = CMD_SUCCESS;
			if(isDeviceToPC == 0)//如果是pc到device的大数据，说明是buftodatabase
			{
				printf("---PCToDevicec --bigcmd= %d,\n",CurBigDataBuf.bigbuf_cmd & 0xff);
				if(CurBigDataBuf.bigbuf_cmd == CMD_SET_DATA)//下传数据  pc -->device
				{
					ret = BufToDatabase(CurBigDataBuf.buf ,CurBigDataBuf.len);
					if(CMD_SUCCESS != ret)
					{
						CurBigDataBuf.len = 0;
						CurBigDataBuf.index = 0;
						CurBigDataBuf.bigbuf_cmd = 0;

						*outbuf = ret;
						CDM_RESPONE_ERROR();
						return ret;
					}
				}
				else if(CurBigDataBuf.bigbuf_cmd == CMD_DELETE_DATA)//删除数据   pc --> device
				{
					processBuf_DelDatabase(CurBigDataBuf.buf ,CurBigDataBuf.len);
				}
				else  if(CurBigDataBuf.bigbuf_cmd == CMD_SET_BINARYDATA)
				{
					ret = SaveFile(CurBigDataBuf.buf, CurBigDataBuf.len);
					if(CMD_SUCCESS != ret)
					{
						CurBigDataBuf.len = 0;
						CurBigDataBuf.index = 0;
						CurBigDataBuf.bigbuf_cmd = 0;

						*outbuf = ret;
						CDM_RESPONE_ERROR();
						return ret;
					}
				}
			}
			//否则清buffer

			CurBigDataBuf.len = 0;
			CurBigDataBuf.index = 0;
			CurBigDataBuf.bigbuf_cmd = 0;
			CDM_RESPONE_OK_NOPARA();

			return ret;
		}
		break;
		case CMD_GET_DATA_OK:
			{
				int file_ID = 0;
				int ret = 0;

				file_ID = buf[0];
				ret = SaveFdOffset(file_ID, FCT_C3GUARDEREVENTLOG_OFFSET);

				if(FDB_OK == ret)
				{
					CDM_RESPONE_ERROR();
					return CMD_SUCCESS;
				}
				else
				{
					*outbuf = ERR_CMD_ERROR_CMD;
					CDM_RESPONE_ERROR();
					return ERR_CMD_PROCESS_ERROR;;
				}
			}
			break;
		default:
			printf("---------------default  cmd = 0x%x\n",*cmd);
			*outbuf = ERR_CMD_ERROR_CMD;
			CDM_RESPONE_ERROR();
			return ERR_CMD_ERROR_CMD;
	}

	*outbuf = ERR_CMD_UNKNOWN;
	CDM_RESPONE_ERROR();
	return ERR_CMD_UNKNOWN;
}

//i = int s=string S=长度大于256的string型 b=长度大于256字节流(指纹模版) B=长度大于256字节流(指纹模版)
void GetDataType(DATA_TYPE data,char *c)
{
	switch (data)
	{
		 case CHAR_T:
			sprintf(c,"%s","i");
		break;
		 case STRING_T:
			sprintf(c,"%s","s");
		break;
		 case INT_T:
		 case UINT_T:
		 case TIME_T:
			sprintf(c,"%s","i");
		break;
		default:
			sprintf(c,"%s","b");
	}
}
//获取表结构
int 	GetTableStruct(BYTE *outbuf,int *outbuflen)
{
	C3DPARAM param;
	char s[1000],c[20];
	int i,j;

	memset(s,0x00,1000);
	for(i=1;i<= GetTableCount() ;i++)
	{
//		printf("tabel No %d:     %s\n",i,s);
		memset(&param,0x00,sizeof(C3DPARAM));
		param.C3tableDesc = getc3TableDesc(i);//根据ID取表名
		if(NULL == param.C3tableDesc)
		{
			printf("GetTableStruct: NULL == param.C3tableDesc\n");
			return 0;
		}

		sprintf(s,"%s%s=%d",s,param.C3tableDesc->name,param.C3tableDesc->tableID);
		for(j = 0;j< param.C3tableDesc->fieldCount;j++)
		{
			memset(c,0x00,20);
			GetDataType(param.C3tableDesc->fieldDesc[j].type,c);
			sprintf(s,"%s,%s=%s%d",s,param.C3tableDesc->fieldDesc[j].name,c,param.C3tableDesc->fieldDesc[j].fieldID);
		}
		sprintf(s,"%s\n",s);
	}
//	printf("datastruct stream---------------------------------:\n%s\n",s);
	*outbuflen = strlen(s);
	memcpy(outbuf,s,*outbuflen);
	printf("datastruct stream: %s,len = %d\n",outbuf,*outbuflen);

	return 1;
}


int	GetDataCount(BYTE *buf,int inbuflen,BYTE *outbuf,int *outbuflen)
{
//	int i,j,m;
	int data_index,DataCount;
	C3DPARAM param;
	int file_ID;

	DEBUGTRACE(buf,inbuflen);

	if(inbuflen < 1 )
	{
		printf("fun:GetDataCount --- inbuflen < 1, inbuflen: %d\n ",inbuflen);
		return 0;
	}

	memset(&param,0x00,sizeof(C3DPARAM));

	data_index = 0;
	file_ID = buf[data_index++];
	if(file_ID >= 10)//目前为7个表
	{
		printf("fun:GetDataCount --- file_ID >= 10, file_ID: %d\n",file_ID);
		return 0;
	}

	param.C3tableDesc = getc3TableDesc(file_ID);//根据ID取表名
	if(NULL == param.C3tableDesc)
	{
		printf("GetDataCount: NULL == param.C3tableDesc\n");
		return 0;
	}

	printf("getc3TableDesc ID = %d ,fd = %d,name = %s\n",	file_ID,param.C3tableDesc->FCT,param.C3tableDesc->name);


	DataCount =GetDataInfo(param.C3tableDesc->FCT, STAT_COUNT, 0);

	printf("GetDataInfo  count = %d \n",	DataCount);

	SET_DWORD(outbuf,DataCount,0);
	*outbuflen = sizeof(DataCount);
	return 1;

}

static int fdUserauthorizeTest = -1;
// 将buff数据处理后,将表中相等数据删除
//judgePrimaryKey为true时，仅仅判断主键，如果为否，则比较所提示的所有字段
void processBuf_DelDatabase(char *buf ,int inbuflen)// ,int judgePrimaryKey)
{
	int i,j,m;
	int data_index,len;
	C3DPARAM param;
	int file_ID;

	DEBUGTRACE(buf,inbuflen);
	memset(&param,0x00,sizeof(C3DPARAM));

	data_index = 0;
	file_ID = buf[data_index++];
	param.C3tableDesc = getc3TableDesc(file_ID);//根据ID取表名

	if(NULL == param.C3tableDesc)
	{
		printf("processBuf_DelDatabase: NULL == param.C3tableDesc\n");
		return 0;
	}
	printf("Buf_DelDatabase name = %s\n",param.C3tableDesc->name);

	param.condition_field_count = buf[data_index++];
	if( (data_index + param.condition_field_count) > inbuflen)
	{
		printf("PARA ERROR \n");
		return;
	}
	//如果存在条件字段，则先取值，如无，则直接删除全部表内容
	if(param.condition_field_count)//如果存在条件选项,取条件字段数量及具体ID
	{
		printf("param.condition_field_count = %d \n",param.condition_field_count);

		for(i = 0;i< param.condition_field_count;i++)
		{
			param.condition_field_desc[i].fieldID =  buf[data_index++];
			printf("---fieldID: %d ,filed_Name: %s ---\n",param.condition_field_desc[i].fieldID,
				param.C3tableDesc->fieldDesc[param.condition_field_desc[i].fieldID-1].name);
		}
	}
	else//无条件字段，直接删除表
	{
		printf("del all data from table !!!\n");
		FDB_ClearData(param.C3tableDesc->FCT);
		return;
	}
	printf("inbuflen= %d curr_datar_index = %d,select_count = %d\n",
		inbuflen,	data_index,param.condition_field_count);


	TBigDataBuf processBuf_DelDatabase_BigDataBuf;
	TSearchHandle sh;
	BYTE tempbuf[500];

	TSearchHandle sh2;
	BYTE tempbuf2[500];

	int x = 0;

	sh.ContentType=param.C3tableDesc->FCT;
	printf("file_ftc = %d\n",sh.ContentType);

	sh.buffer=tempbuf;
	SearchFirst(&sh);

	//把接收的数据全部转化结构体方式的大数据缓冲中
	memset(&processBuf_DelDatabase_BigDataBuf,0x00,sizeof(TBigDataBuf));
	while(data_index < inbuflen )
	{
		int length,length2,length3;
//		printf("param.C3tableDesc->fieldCount = %d\n",param.C3tableDesc->fieldCount);
		for(i = 0,length = 0;i< param.C3tableDesc->fieldCount;i++)
			length += param.C3tableDesc->fieldDesc[i].width;
		length2 = param.C3tableDesc->fieldDesc[param.C3tableDesc->fieldCount-1].width
					+ param.C3tableDesc->fieldDesc[param.C3tableDesc->fieldCount-1].offset;
		length3 = param.C3tableDesc->rowDataWidth;
//		printf("width=%d, offset=%d, len1 = %d,len2 =%d  len3 = %d\n",param.C3tableDesc->fieldDesc[param.C3tableDesc->fieldCount- 1].width ,
//					 param.C3tableDesc->fieldDesc[param.C3tableDesc->fieldCount-1].offset,
//					length,length2 ,length3);

		memset(sh.buffer,0,length);
		//先获得一组数据
		for(j = 0;j<param.condition_field_count;j++)
		{
			len = buf[data_index++];
//			printf("select fieldID = %d,j = %d,len = %d:			",param.condition_field_desc[j].fieldID,j,len);
			if(len && param.condition_field_desc[j].fieldID <= param.C3tableDesc->fieldCount)
			{
				//如果设备的相应字段小于pc送过来的长度，取值以设备的字段长度为准
				//如 假设姓名长度为8，pc送过来"山本副司令员",则实际只取"山本副司"
				m = (param.C3tableDesc->fieldDesc[param.condition_field_desc[j].fieldID-1].width > len) ? len:param.C3tableDesc->fieldDesc[param.condition_field_desc[j].fieldID-1].width;
//				printf("---------m = %d,offset = %d\n",m ,param.C3tableDesc.fieldDesc[param.select_field_desc.fieldID].offset);
				memcpy(sh.buffer  + param.C3tableDesc->fieldDesc[param.condition_field_desc[j].fieldID-1].offset,
				buf+data_index,m);
				data_index = data_index + len;

//				if(param.C3tableDesc->fieldDesc[param.condition_field_desc[j].fieldID-1].type != STRING_T)
//					printf("%s = %d\n",param.C3tableDesc->fieldDesc[param.condition_field_desc[j].fieldID-1].name,*(int *)( sh.buffer  + param.C3tableDesc->fieldDesc[param.condition_field_desc[j].fieldID-1].offset));
//				else
//					printf("%s : %s\n",param.C3tableDesc->fieldDesc[param.condition_field_desc[j].fieldID-1].name, (char *)(sh.buffer  + param.C3tableDesc->fieldDesc[param.condition_field_desc[j].fieldID-1].offset));
			}
			else
			{
				data_index = data_index + len;
//				printf("******123******* len = 0 or   id = %d,maxid is %d\n",param.condition_field_desc[j].fieldID,param.C3tableDesc->fieldCount);
			}
		}
	//
		memcpy(processBuf_DelDatabase_BigDataBuf.buf + processBuf_DelDatabase_BigDataBuf.index,sh.buffer,param.C3tableDesc->rowDataWidth);
		processBuf_DelDatabase_BigDataBuf.index += param.C3tableDesc->rowDataWidth;
		processBuf_DelDatabase_BigDataBuf.len =processBuf_DelDatabase_BigDataBuf.index;
	}

		printf("received OK,after trans   DataBuf.len=%d\n",processBuf_DelDatabase_BigDataBuf.len);

	//根据获得的数据判断删除表中相同的内容
	sh2.ContentType=param.C3tableDesc->FCT;

	sh2.buffer=tempbuf2;
	SearchFirst(&sh2);

	while(!SearchNext(&sh2))
	{
//			printf("sh2.buff");DEBUGTRACE(sh2.buffer,param.C3tableDesc->rowDataWidth);
//			printf("sh.buff");DEBUGTRACE(sh.buffer,param.C3tableDesc->rowDataWidth);
		if(sh2.datalen>0)
		{
//			printf("next group!\n");
			for(processBuf_DelDatabase_BigDataBuf.index = 0,x= 0;
				processBuf_DelDatabase_BigDataBuf.index<processBuf_DelDatabase_BigDataBuf.len;
				processBuf_DelDatabase_BigDataBuf.index += param.C3tableDesc->rowDataWidth)
			{
				//轮流比较选择的字段,只要有一个选择字段相等，即将内容换为0xffffff
				for(j = 0;j<param.condition_field_count;j++)
				{
	//					printf("j = %d,param.condition_field_desc[j].fieldID=%d\n param.C3tableDesc->fieldDesc[param.condition_field_desc[j].fieldID-1].offset=%d\n",
	//						j,param.condition_field_desc[j].fieldID,param.C3tableDesc->fieldDesc[param.condition_field_desc[j].fieldID-1].offset);

					if(nmemcmp(sh2.buffer + param.C3tableDesc->fieldDesc[param.condition_field_desc[j].fieldID-1].offset,
						processBuf_DelDatabase_BigDataBuf.buf + processBuf_DelDatabase_BigDataBuf.index + param.C3tableDesc->fieldDesc[param.condition_field_desc[j].fieldID-1].offset,
						param.C3tableDesc->fieldDesc[param.condition_field_desc[j].fieldID-1].width) == 0)
					{
						x++;
						//printf("i = %d,x= %d ,nmemcmp is false \n",i,x);
						//DEBUGTRACE(sh2.buffer,param.C3tableDesc->rowDataWidth);
					}
					else
					{
						x = 0; j = 100;
						//printf("i = %d,x= %d ,nmemcmp is false \n",i,x);
					}
//					printf("j = %d,x= %d\n",j,x);
				 }
				if(x)//说明有关键字相等的，要先删除掉
				{
//					printf("is need del first ,then add.  x= %d\n",x);
//					printf("sh2.buff");DEBUGTRACE(sh2.buffer,param.C3tableDesc->rowDataWidth);
					memset(sh2.buffer,0xff,param.C3tableDesc->rowDataWidth);
					lseek(sh.fd, -1*param.C3tableDesc->rowDataWidth, SEEK_CUR);
					write(sh.fd, sh2.buffer, param.C3tableDesc->rowDataWidth);
					x =0;
					fsync(sh.fd);
				}
			}
		}
	}
}



// 将buff数据处理后保存入表中
//先将通迅数据转化为结构体数据buf,再从表中删除关键字相等的数据，再追加数据
int BufToDatabase(char *buf ,int inbuflen )//,int handle_datafile)
{
	int i,j,m;
	int data_index,len;
	C3DPARAM param;
	int file_ID;
	int UserCount;
	int NoSpaceFlg = FALSE;
	int LogWidth = 0;

	printf("PARA BufToDatabase data: \n");
	DEBUGTRACE(buf,inbuflen);
	memset(&param,0x00,sizeof(C3DPARAM));

	data_index = 0;
	file_ID = buf[data_index++];
	printf("----file_ID-----------%d",file_ID);
	param.C3tableDesc = getc3TableDesc(file_ID);//根据ID取表名
	if(NULL == param.C3tableDesc)
	{
		printf("BufToDatabase: NULL == param.C3tableDesc\n");
		return 0;
	}

	param.select_field_count = buf[data_index++];
	if( (data_index +param.select_field_count) >= inbuflen)
	{
		printf("PARA ERROR \n");
		return 0;
	}
	if(param.select_field_count)//取需要的字段数量及具体ID
	{
		for(i = 0;i< param.select_field_count;i++)
			param.select_field_desc[i].fieldID =  buf[data_index++];
	}
	printf("inbuflen= %d curr_datar_index = %d,select_count = %d\n",
		inbuflen,	data_index,param.select_field_count);

	TBigDataBuf BufToDatabase_Buf;
	TSearchHandle sh;
	BYTE tempbuf[500]={0};

	TSearchHandle sh2;
	int isfindOK;
	U32 time_start;

	BYTE tempbuf2[500]={0};

	int x = 0,y = 0;

	sh.ContentType=param.C3tableDesc->FCT;
	printf("file_ftc = %d\n",sh.ContentType);
	sh.buffer=tempbuf;
	SearchFirst(&sh);
	memset(&BufToDatabase_Buf,0x00,sizeof(TBigDataBuf));

	while(data_index < inbuflen)
	{
		memset(sh.buffer,0,param.C3tableDesc->rowDataWidth);
		//先获得一组数据
		for(j = 0; j<param.select_field_count; j++)
		{
			len = buf[data_index++];
			if(len > 10)//目前最大长度为6 ，如果大于10，说明数据错误
				return 0;
			//sdk传来的字段ID小于等于表中的字段总数，则取值。否则丢弃。直接取下一个字段。
			if(len && param.select_field_desc[j].fieldID <= param.C3tableDesc->fieldCount)
			{	//取SDK传来的数据长度与固件中定义的数据长度中取最小值。
				m = (param.C3tableDesc->fieldDesc[param.select_field_desc[j].fieldID-1].width > len)
						? len:param.C3tableDesc->fieldDesc[param.select_field_desc[j].fieldID-1].width;
//				printf("---------m = %d,offset = %d\n",m ,param.C3tableDesc.fieldDesc[param.select_field_desc.fieldID].offset);
				memcpy(sh.buffer  + param.C3tableDesc->fieldDesc[param.select_field_desc[j].fieldID-1].offset,
				buf+data_index,m);
				data_index = data_index + len;

			}
			else
			{
				data_index = data_index + len;
				//printf("******************* len = 0 or    id = %d,maxid is %d\n",param.select_field_desc[j].fieldID,param.C3tableDesc->fieldCount);
			}
		}

		//得到一整行数据，保存到BufToDatabase_Buf.buf中。
		memcpy(BufToDatabase_Buf.buf + BufToDatabase_Buf.index,sh.buffer,param.C3tableDesc->rowDataWidth);
		BufToDatabase_Buf.index += param.C3tableDesc->rowDataWidth;
		BufToDatabase_Buf.len =BufToDatabase_Buf.index;
	}
	printf("---param.C3tableDesc->tableID---- %d\n ",param.C3tableDesc->tableID);
	printf("BufToDatabase_Buf len = %d,width =%d\n",BufToDatabase_Buf.len,param.C3tableDesc->rowDataWidth);
	//根据获得的数据判断是追加，还是修改，如果是修改，则先删除原来的
	//判断的标准是，比较所有的关键字段，如果新的数据在旧表中有，就删除旧表中的相关数据(以0xff代替原来的数据内容)
	//先作删除处理


	sh2.ContentType=param.C3tableDesc->FCT;
	sh2.buffer=tempbuf2;
	SearchFirst(&sh2);
	x = 0;
	time_start = GetTickCount();
	printf("start to del old  data !\n");

	while(!SearchNext(&sh2))
	{

//		printf("del_process  next group!\n");
		if(sh2.datalen>0)
		{
			//读取表的一条数据，再与内存中的所有的数据比较一次
			for(BufToDatabase_Buf.index = 0,x= 0,isfindOK = 0;
				BufToDatabase_Buf.index<BufToDatabase_Buf.len ;// && !isfindOK;
				BufToDatabase_Buf.index += param.C3tableDesc->rowDataWidth)//
			{
				for(x=0,i = 0;i<param.C3tableDesc->fieldCount;i++)//表的全部字段数
				{
				//根据是否有关键字决定是使用直接append,还是查找后，有替换，无追加
					if(param.C3tableDesc->fieldDesc[i].isPrimaryKey)//有关键字，关键字不一定为第一个字段
					{
//							printf("i = %d,x= %d,  len =%d,  sh,sh2 datais :   \n",i,x,param.C3tableDesc->rowDataWidth);
//							DEBUGTRACE(sh2.buffer,param.C3tableDesc->rowDataWidth);
//							DEBUGTRACE(sh.buffer,param.C3tableDesc->rowDataWidth);
						if(nmemcmp(sh2.buffer + param.C3tableDesc->fieldDesc[i].offset,
							BufToDatabase_Buf.buf + BufToDatabase_Buf.index + param.C3tableDesc->fieldDesc[i].offset,
							param.C3tableDesc->fieldDesc[i].width) == 0)
						{
							x++;
							//printf("i = %d,x= %d ,nmemcmp is false \n",i,x);
							//DEBUGTRACE(sh2.buffer,param.C3tableDesc->rowDataWidth);
						}
						else
						{
							x = 0;
							break; //关键字有一个不等就退出
							//printf("i = %d,x= %d ,nmemcmp is false \n",i,x);
						}
					}
//					else
//						i =100;
				}

				if(x)//说明关键字全部相等，要先删除掉
				{
//					printf("is need del first ,then add.  x= %d\n",x);
					memset(sh2.buffer,0xff,param.C3tableDesc->rowDataWidth);
					lseek(sh.fd, -1*param.C3tableDesc->rowDataWidth, SEEK_CUR);
					write(sh.fd, sh2.buffer, param.C3tableDesc->rowDataWidth);
					x =0;
					isfindOK = 1;
				}
			}
		}
	}
	printf("del old  data ok,take time %d\n",GetTickCount() -time_start);
	time_start = GetTickCount();

	if(1 == param.C3tableDesc->tableID)
	{
		UserCount = GetDataInfo(FCT_C3USER, STAT_COUNT, 0);
		LogWidth = param.C3tableDesc->rowDataWidth;
		if(BufToDatabase_Buf.len/LogWidth >= gOptions.MaxUserCount*100 - UserCount)
		{
			BufToDatabase_Buf.len = (gOptions.MaxUserCount*100 - UserCount)*LogWidth;
			NoSpaceFlg = TRUE;
			printf("-------BufToDatabase_Buf.len  over---- \n ");
		}

	}

	//删除完了，开始增加新数据
	printf("BufToDatabase del ok ,and begin to append\n");
	SearchFirst(&sh2);
	x = 0;
	for(BufToDatabase_Buf.index = 0,x= 0,y=0;
		BufToDatabase_Buf.index<BufToDatabase_Buf.len;
		BufToDatabase_Buf.index += param.C3tableDesc->rowDataWidth)//
	{
		while(!SearchNext(&sh2))
		{
			if(sh2.datalen>0  &&  *(U32 *)sh2.buffer == 0xffffffff)
			{
				//定位空位置。
				lseek(sh.fd, -1*param.C3tableDesc->rowDataWidth, SEEK_CUR);
				break;
			}
		}
		write(sh2.fd, BufToDatabase_Buf.buf + BufToDatabase_Buf.index, param.C3tableDesc->rowDataWidth);

	}
	fsync(sh2.fd);
	if(TRUE == NoSpaceFlg)
	{
		return ERROR_CMD_NO_SPAC;
	}

	printf("append OK ,take time %d\n",GetTickCount() -time_start);
//	printf("begin RefreshJFFS2Node_dataApi\n");
//	RefreshJFFS2Node_dataApi(sh2.fd, 200*param.C3tableDesc->rowDataWidth);

	return CMD_SUCCESS;
}

//将表中数据倒出到buff中
void DatabaseToBuf(char *buf,int inbuflen)//,char *outbuf,int *outbuflen)
{
	int i;//j,m;
	int data_index,len;//buffer数据索引
	C3DPARAM param;
	int file_ID,inhead_to_outhead_len;

////表ID    要获取的字段总数 字段1 ID 字段2 ID ... 字段N ID 表参数个数 表参数 条件字段总数 字段1 ID 字段2 ID ... 字段N ID 字段1值长度 字段1 值 字段2 值长度 ... 字段N 值  压缩标志
////1bytes    1 byte  1 byte  1 byte ...  1 byte  1 byte 1 byte  1 byte ...  1 byte  1或2 bytes N bytes  1或2 bytes  ...  N bytes 1 byte
	memset(&param,0x00,sizeof(C3DPARAM));

	data_index = 0;
	file_ID = buf[data_index++];
	param.C3tableDesc = getc3TableDesc(file_ID);//根据ID取表名
	if(param.C3tableDesc == NULL)
	{
		printf("PARA ERROR \n");
		return;
	}
	printf("---------------------------------------------------------------------- \n");
	printf("FCT = %d, file_ID: %d Table_Name = %s \n",param.C3tableDesc->FCT,file_ID,param.C3tableDesc->name);


	param.select_field_count = buf[data_index++];
	if( (data_index +param.select_field_count) >= inbuflen)
	{
		printf("PARA ERROR \n");
		return;
	}
	if(param.select_field_count)//取需要的字段数量及具体ID
	{
		printf("select field_count = %d \n",param.select_field_count);
		for(i = 0;i< param.select_field_count;i++)
		{
			param.select_field_desc[i].fieldID =  buf[data_index++];
			printf("<<  fieldID: %d ,filed_Name: %s   >>\n",param.select_field_desc[i].fieldID,
				param.C3tableDesc->fieldDesc[param.select_field_desc[i].fieldID-1].name);
		}
	}
//	printf("inbuflen= %d curr_datar_index = %d,select_count = %d\n",
//		inbuflen,	data_index,param.select_field_count);
	inhead_to_outhead_len = data_index;

	//
	param.table_para_num = buf[data_index++];//表参数数量
	if(param.table_para_num)//目前最多一个参数
	{
		param.table_para = buf[data_index++];
	}
	else
	{	param.table_para = 0;
	}
	printf("param.table_para num= %d,data= %d \n",param.table_para_num,param.table_para);


	param.condition_field_count = buf[data_index++];
//	if( (data_index + param.condition_field_count) > inbuflen)
//	{
//		printf("PARA ERROR \n");
//		return;
//	}
	if(param.condition_field_count)//如果存在条件选项,取条件字段数量及具体ID  以及具体相等内容
	{
		printf("param.condition_field_count = %d \n",param.condition_field_count);

		for(i = 0;i< param.condition_field_count;i++)
		{
			param.condition_field_desc[i].fieldID =  buf[data_index++];

			printf("---fieldID: %d ,filed_Name: %s ---\n",param.condition_field_desc[i].fieldID,
				param.C3tableDesc->fieldDesc[param.condition_field_desc[i].fieldID-1].name);
		}
		for(i = 0;i< param.condition_field_count;i++)
		{
			len = buf[data_index++];
			if(len)
			{
				param.condition_field_desc[i].len = len;
				printf("param.condition_field_desc[%d].len =  %d \n",i,param.condition_field_desc[i].len );
				memcpy(param.condition_field_desc[i].condition_buffer,buf+data_index,len);
				data_index = data_index + len;
			}
		}
	}

	//para的buffer借用bigdatabuf结构
	param.buffer.buffer  = CurBigDataBuf.buf;
	memcpy(param.buffer.buffer,buf,inhead_to_outhead_len);
	param.buffer.bufferSize = inhead_to_outhead_len;	//保留数据流的前面参数内容。
	CurBigDataBuf.index = param.buffer.bufferSize;

	printf("---------------------------------------------------------------------- \n");
//	FDB_ForAllData(param.C3tableDesc->FCT, c3CbQuery, NO_LIMIT, &param);
	if(file_ID != TRANSACTION_TABLE_ID || param.table_para_num == 0)//不是刷卡记录表 或者不是新记录
	{
		FDB_ForData_C3(param.C3tableDesc->FCT, 0,c3CbQuery, NO_LIMIT, &param);
	}
	else
	{
		TFdOffset FdOffset;
		memset(&FdOffset, 0, sizeof(TFdOffset));
		FDB_GetFdOffset(FCT_C3GUARDEREVENTLOG_OFFSET, &FdOffset);
		printf("start - FdOffset.ContentType: %d, FdOffset.currpos: %d\n", FdOffset.ContentType, FdOffset.currpos);
		FDB_ForData_C3(param.C3tableDesc->FCT, FdOffset.currpos, c3CbQuery, NO_LIMIT, &param);

	}
	CurBigDataBuf.index = param.buffer.bufferSize;
	CurBigDataBuf.len = param.buffer.bufferSize;

	printf("DatabaseToBuf len1 =%d,len = %d \n",inhead_to_outhead_len,CurBigDataBuf.len);
}

//在文件开始保存表结构，有记录长度、字段个数、每个字段类型和长度。
int SaveTableContent(int FCT,char *saveBuf)
{
	C3DPARAM param;
	int rowWidth = 0;//记录长度
	BYTE fieldCount = 0;
	int i = 0,j = 0;
	int index = 0;
	int len = 0;
	char fieldType[8] = {0};

	memset(&param,0x00,sizeof(C3DPARAM));
	param.C3tableDesc = getTableFromFCT(FCT);//根据ID取表名
	if(NULL == param.C3tableDesc)
	{
		printf("BufToDatabase: NULL == param.C3tableDesc\n");
		return -1;
	}
	rowWidth = param.C3tableDesc->rowDataWidth;//取记录长度
	fieldCount = param.C3tableDesc->fieldCount;//取字段个数
	memcpy(saveBuf+index,&rowWidth,sizeof(int));
	index += sizeof(int);
	memcpy(saveBuf+index,&fieldCount,sizeof(BYTE));
	index += sizeof(BYTE);
	while(i < fieldCount)
	{
		memset(fieldType,0x00,sizeof(fieldType));
		GetDataType(param.C3tableDesc->fieldDesc[i].type,fieldType);
		memcpy(saveBuf+index,fieldType,sizeof(char));
		index += sizeof(char);
		printf("param.C3tableDesc->fieldDesc[i].width %d\n",param.C3tableDesc->fieldDesc[i].width);
		memcpy(saveBuf+index,&(param.C3tableDesc->fieldDesc[i].width),sizeof(U16));
		index += sizeof(U16);
		i++;
	}
	printf("buflen %d\n",index);

	return index;
}

