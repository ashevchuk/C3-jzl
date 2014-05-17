/*************************************************

 ZEM 200

 net.c Simple network layer with UDP & TCP

 Copyright (C) 2003-2004, ZKSoftware Inc.

 $Log: net.c,v $

*************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <net/if.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <errno.h>
//#define __KERNEL__
//#include <asm/types.h>
#include "arca.h"
#include "net.h"
#include "utils.h"
//#include "commu.h"
#include "options.h"
#include "exfun.h"
#include "main.h"
#include "flashdb.h"
#include "dlc.h"
#include "commfun.h"

typedef unsigned short u16;
typedef unsigned int u32;
typedef unsigned char u8;
typedef unsigned long u64;
#include <linux/sockios.h>
#include <linux/ethtool.h>


#define THEIP gOptions.IPAddress
#define THEUDP gOptions.UDPPort
#define BROADCASTPORT 65535

static int comm_socket,listenAuthServer_socket, receive_broadcast_socket; //UDP
static int server_socket; //TCP listen
fd_set rfds;
struct timeval tv;

static int image_socket=-1; //TCP AuthServer
int AuthServerSessionCount=0;
PAuthServerSession AuthServerSessions=NULL;
int MaxAuthServerSessionCount=0;
int StaticDNSCount=0;
static int mIsTcpInit=0;

int skfd = 0;
struct mii_data
{
unsigned short phy_id;
unsigned short reg_num;
unsigned short val_in;
unsigned short val_out;
};

int ReadNetState(void)
{
	//int tmp = 0;
	//U16 mii_regs_number = 0;
	//U16 mii_regs_val = 0x3100;

	if( ( skfd = socket( AF_INET, SOCK_DGRAM, 0 ) ) < 0 )
	{
		perror( "socket" );
		return -1;
	}

	struct ifreq ifr;
	bzero( &ifr, sizeof( ifr ) );
	strncpy( ifr.ifr_name, "eth0", IFNAMSIZ - 1 );
	ifr.ifr_name[IFNAMSIZ - 1] = 0;
	if( ioctl( skfd, SIOCGMIIPHY, &ifr ) < 0 )
	{
		perror( "ioctl" );
		return -1;
	}
	struct mii_data* mii = NULL;
	mii = (struct mii_data*)&ifr.ifr_data;
	mii->reg_num = 0;
	if( ioctl( skfd, SIOCGMIIREG, &ifr ) < 0 )
	{
		perror( "ioctl2" );
		return -1;
	}
	//printf("reg 0 val: %x\n", mii->val_out);

	return mii->val_out;
}

int RebootNetwork(U16 NetState)
{
	struct ifreq ifr;
	bzero( &ifr, sizeof( ifr ) );
	strncpy( ifr.ifr_name, "eth0", IFNAMSIZ - 1 );
	ifr.ifr_name[IFNAMSIZ - 1] = 0;
	struct mii_data* mii = NULL;
	mii = (struct mii_data*)&ifr.ifr_data;
	mii->reg_num = 0;
	system("ifconfig eth0 down");
	if( ioctl( skfd, SIOCGMIIREG, &ifr ) < 0 )
	{
		perror( "ioctl2" );
		return -1;
	}
	if(NetState != mii->val_out)
	{
		mii->val_in = NetState;
		if( ioctl( skfd, SIOCSMIIREG, &ifr ) < 0 )
		{
			perror("SIOCSMIIREG");
			return -1;
		}
	}
	system("ifconfig eth0 up");
	return 0;
}

//send data with UDP protocol
//buf - buffer will be send
//size - length of data buffer
//param - this parameter save the ip address and port number of receiver, pls refer E TFPResult, *PFPResultthCommCheck function
//该参数用于识别接收数据方的ip地址和端口号，该参数由 eth_rx 在一个通讯会话建立时保存下来。
int SendUDPData(int socketId,void *buf, int size, void *param)
{
//	printf("udp send: %d\n",size);
//	DEBUGTRACE(buf,size);
	return sendto(socketId, buf, size, 0, (struct sockaddr*)param, sizeof(struct sockaddr_in));
}

//检查是否有接收到的UDP数据，若有的话，进行处理；
//Check whether data arrival or not
int EthCommCheck(void)
{
	struct sockaddr_in from;
	char buf[COMMBUFLEN_MAX_UDP];
	int fromlen=sizeof(from);

	memset(buf,0x00,COMMBUFLEN_MAX_UDP);
	//Maximize data packet size is 1032
//	int len = recvfrom(comm_socket, buf, 1032, 0, (struct sockaddr*)&from, &fromlen);
	int len = recvfrom(comm_socket, buf, COMMBUFLEN_MAX_UDP, 0, (struct sockaddr*)&from, &fromlen);
	if(len>0)
	{
		TC3Cmdstruct c3buf;
		int result;
		char sendbuf[COMMBUFLEN_MAX_UDP];
		int sendlen = 0;

		GPIO_netled_active_Status(1);
		memset(sendbuf, 0 , sizeof(sendbuf));
		memset(&c3buf.framehead,0x00,sizeof(TC3Cmdstruct));
		DBPRINTF("EthCommCheck:Get data  packet len =%d\n",len);
		DEBUGTRACE(buf,len);
		result = Process_ReceiveData(buf,len,&c3buf.framehead,UDP_MODE);//将接收的数据转化为命令及参数
		if(result==  EC_FINISH)
		{
			printf("c3 cmd = %d,data's len = %d \n ",c3buf.command,c3buf.datalen);
			//命令处理，转化为输出命令及参数
//			if(process_comm_cmd(&c3buf.command,c3buf.buf,c3buf.datalen,sendbuf,&sendlen,UDP_MODE) == CMD_SUCCESS)
			process_comm_cmd(&c3buf.command,c3buf.buf,c3buf.datalen,sendbuf,&sendlen,UDP_MODE);
			{
				c3buf.datalen = sendlen;//
				if(sendlen)
					memcpy(c3buf.buf,sendbuf,sendlen);
				//将命令及参数转为数据流
				Make_SendData(c3buf.buf,c3buf.command,c3buf.datalen,sendbuf,&sendlen);

				char Sender[SENDER_LEN];
				//获取发送方ip and port
				memset(Sender, 0, SENDER_LEN);
				memcpy(Sender, (void*)&from, sizeof(struct sockaddr));
				SendUDPData(comm_socket,sendbuf,sendlen,Sender);
				printf("udp runcommcmd OK ! sendlen = %d \n ",sendlen);
			}
/*			else//命令执行失败
			{
			printf("----------------------------------runcommcmd error ! \n ");
			}*/
		}
		else
			printf("-----------------------------checkRecData error = 0x%x,\n", 	result);

		GPIO_netled_active_Status(0);
	}
	return 0;
}

int EthBoradcastCheck(void)
{
	struct sockaddr_in from;
	char buf[500];
	int fromlen=sizeof(from);

	memset(buf,0x00,500);
	//Maximize data packet size is 1032
	int len = recvfrom(receive_broadcast_socket, buf, 500, 0, (struct sockaddr*)&from, &fromlen);
	if(len>0)
	{
		TC3Cmdstruct c3buf;
		int result;
		char sendbuf[2000];
		int sendlen = 0;
		int RetProcessCommCmd = 0;
		int TmpCmd = 0;
		char ErrCode = 0;  //获取返回给SDK的错误代码，目的甄别出与本地MAC相同的操作的错误代码，发给SDK，否则不发送。

		//printf("         From socket info:");
		//DEBUGTRACE((char *)&from,fromlen);

		GPIO_netled_active_Status(1);
		memset(sendbuf, 0 , sizeof(sendbuf));
		memset(&c3buf.framehead,0x00,sizeof(TC3Cmdstruct));
		//DBPRINTF("EthBoradcastCheck:Get data  packet len =%d\n",len);
		//DEBUGTRACE(buf,len);
		result = Process_ReceiveData(buf,len,&c3buf.framehead,UDP_BROAD_MODE);//将接收的数据转化为命令及参数
		if(result==  EC_FINISH)
		{
			//printf("c3 cmd = %d,data's len = %d \n ",c3buf.command,c3buf.datalen);
			//DEBUGTRACE(buf,len);
			TmpCmd = c3buf.command;
			//命令处理，转化为输出命令及参数
			RetProcessCommCmd = process_comm_cmd(&c3buf.command,c3buf.buf,c3buf.datalen,sendbuf,&sendlen,UDP_MODE);

			c3buf.datalen = sendlen;//
			int sendResult = 0;
			if(sendlen)
			{
				memcpy(c3buf.buf,sendbuf,sendlen);
			}
			///如果加帧头"zksoftware"在这里加
			ErrCode = sendbuf[0];
			//将命令及参数转为数据流
			Make_SendData(c3buf.buf,c3buf.command,c3buf.datalen,sendbuf,&sendlen);

//				printf(" Make_SendData OK,then start to send\n");

			//获取发送方ip and port
			char Sender[SENDER_LEN];
			memset(Sender, 0, SENDER_LEN);

			//允许 广播发送
			int  so_broadcast = 1;
			int broadcastResult = setsockopt(receive_broadcast_socket, SOL_SOCKET, SO_BROADCAST, &so_broadcast, sizeof (so_broadcast));
//				printf(" SEND socket broadcastResult =%d\n",broadcastResult);

			//INVALID，或NOWHERE
			from.sin_family=AF_INET;
			from.sin_addr.s_addr= INADDR_BROADCAST;
//				from.sin_family = AF_INET;
//				from.sin_addr.s_addr = inet_addr("192.168.255.255");

			memcpy(Sender, (void*)&from, sizeof(struct sockaddr));

//				printf(" SEND socket info:");
//				DEBUGTRACE((char *)&Sender,sizeof(struct sockaddr));
			if(CMD_SUCCESS == RetProcessCommCmd)
			{
				//usleep(gOptions.MAC[5] * 2000);
				sendResult = SendUDPData(receive_broadcast_socket,sendbuf,sendlen,Sender);
				//printf("udp runcommcmd reult = %d sendlen = %d \n ",sendResult,sendlen);
				if(sendResult == -1)
				{
					perror("SendUDPData error");
					SetGateway("add", gOptions.IPAddress);
					SendUDPData(receive_broadcast_socket,sendbuf,sendlen,Sender);
					SetGateway("add", gOptions.GATEIPAddress);
				}

				if(CMD_BROAD_MODIYIP == TmpCmd)
				{
					ExecuteActionForFixOption(NULL,NULL); //把IP地址相关的参数设置到系统中。

					EthFreeBroadcastSocket();
					if(FALSE == InitBroadCastPort())
					{
						printf("InitBroadCastPort failed!\n");
					}

					ForceClearMACCacheIP(gOptions.GATEIPAddress);
				}
			}
			else
			{
				if(CMD_BROAD_MODIYIP == TmpCmd)
				{

					if(ERR_CMD_COMKEY_ERROR == ErrCode || ERR_CMD_PROCESS_ERROR == ErrCode)
					{
						printf("ErrCode ------------- :%d\n",ErrCode);
						sendResult = SendUDPData(receive_broadcast_socket,sendbuf,sendlen,Sender);
						//printf("udp runcommcmd reult = %d sendlen = %d \n ",sendResult,sendlen);
						if(sendResult == -1)
						{
							perror("SendUDPData error");
							SetGateway("add", gOptions.IPAddress);
							SendUDPData(receive_broadcast_socket,sendbuf,sendlen,Sender);
		                                        SetGateway("add", gOptions.GATEIPAddress);
						}
					}

				}
			}
		}
		else if(result==  EC_BUFOVERFLOW)
		{
			printf("EC_BUFOVERFLOW!!! \n ");
		}


		GPIO_netled_active_Status(0);
	}
	return 0;
}



//TCP communication then close the temp socket
int CloseTCPSocket(void *param)
{
	int tmp_server_socket;
	memcpy(&tmp_server_socket, param, 4);
	close(tmp_server_socket);
	return 1;
}

int SendTCPData(void *buf, int size, void *param,int TimeoutMS)
{
	int tmp_server_socket;
	int sendlen = 0;
	int len = 0;
	int count = 0;
	memcpy(&tmp_server_socket, param, 4);
	sendlen = send(tmp_server_socket, buf, size, 0);//首次发送
	while(sendlen < size)  //发送的数据小于要发送的，需再次发送
	{
		len = send(tmp_server_socket, buf+sendlen, size-sendlen, 0);
		if(len > 0)
		{
			sendlen += len;
		}
		count++;
		if(count >= (TimeoutMS/10))//超时退出
		{
			break;
		}
		else
		{
			mmsleep(10);
		}

	}
	return sendlen;
}

int ProcessTCPPackage(void *buf, int len, int tmp_server_socket)
{
	TC3Cmdstruct c3buf;
	int result = 0;
	char sendbuf[COMMBUFLEN_MAX_TCP];
	int sendlen = 0;
	int RetProcessCommCmd = 0;
	int TmpCmd = 0;

	memset(&c3buf.framehead,0x00,sizeof(TC3Cmdstruct));
	memset(sendbuf, 0 , sizeof(sendbuf));
	DBPRINTF("ProcessTCPPackage:Get data  packet len =%d\n",len);
	//DEBUGTRACE(buf,len);
	result = Process_ReceiveData(buf,len,&c3buf.framehead,TCP_MODE);//将接收的数据转化为命令及参数
	if(result == EC_FINISH)
	{
		printf("c3 cmd = %d,data's len = %d \n ",c3buf.command,c3buf.datalen);

		TmpCmd = c3buf.command;
		//命令处理，转化为输出命令及参数
//		if(process_comm_cmd(&c3buf.command,c3buf.buf,c3buf.datalen,sendbuf,&sendlen,TCP_MODE) == CMD_SUCCESS)
		RetProcessCommCmd = process_comm_cmd(&c3buf.command,c3buf.buf,c3buf.datalen,sendbuf,&sendlen,TCP_MODE) ;
		{
			c3buf.datalen = sendlen;//
			if(sendlen)
			{
				memcpy(c3buf.buf,sendbuf,sendlen);
			}
			//将命令及参数转为数据流
			Make_SendData(c3buf.buf,c3buf.command,c3buf.datalen,sendbuf,&sendlen);

			char Sender[SENDER_LEN];
			//获取发送方ip and port
			memset(Sender, 0, SENDER_LEN);
			memcpy(Sender, (void*)&tmp_server_socket, 4);
			SendTCPData(sendbuf,sendlen,Sender,2000);
			printf("tcp runcommcmd OK ! sendlen = %d \n ",sendlen);

			if(CMD_SUCCESS == RetProcessCommCmd)
			{
				if(CMD_BROAD_MODIYIP == TmpCmd || CMD_SET_DEVICE_PARAM == TmpCmd)
				{
					ExecuteActionForFixOption(NULL,NULL); //把IP地址相关的参数设置到系统中。
					EthFreeBroadcastSocket();
					if(FALSE == InitBroadCastPort())
					{
						printf("InitBroadCastPort failed!\n");
					}
					ForceClearMACCacheIP(gOptions.GATEIPAddress);
				}

				if(CMD_DISCONNECT == TmpCmd)
				{
					CloseTCPSocket(Sender);
					return CMD_DISCONNECT;
				}

			}

			if(ERR_CMD_COMKEY_ERROR == RetProcessCommCmd)
			{
				if(CMD_CONNECT == TmpCmd)
				{
					CloseTCPSocket(Sender);
					return ERR_CMD_COMKEY_ERROR;
				}
			}
		}
//		else
		{
//			printf("----------------------------------runcommcmd error ! \n ");
		}
	}
	else
	{
		printf("------------checkRecData error = 0x%x,\n", 	result);
	}

	return 0;
}

//extern int CommSessionCount;
//extern PCommSession CommSessions;
//check tcp communication

//读取TCP数据
int TcpRecv(int socket, char *buf, int TimeoutMS)
{
	int len = 0;
	int curLen = 0;
	int count = 0;
	int packageLen = 0;
	PProtoHeader pchd = NULL;

	len=recv(socket, buf, COMMBUFLEN_MAX_TCP, 0); //首次读取
	curLen = len;
	if(len > 0)
	{
		pchd = (PProtoHeader) buf;
		packageLen = pchd->DataSize + sizeof(TProtoHeader) + 3; //2个校验字节+1个包尾
		while(curLen < packageLen)//读取的包比发送的小，需重新读取。
		{
			len=recv(socket, buf+curLen, COMMBUFLEN_MAX_TCP-curLen, 0);
			if(len > 0)
			{
				curLen += len;
			}
			count++;
			if(count >= (TimeoutMS/10))//超时退出
			{
				break;
			}
			else
			{
				mmsleep(10);
			}
		}
		return curLen;
	}
	else
	{
		return curLen;
	}
}

int EthCommTCPCheck(void)
{
	int address_size=sizeof(struct sockaddr);
	static struct sockaddr_in pin;
	char buf[COMMBUFLEN_MAX_TCP];
//	long save_file_flags;
	static int tmp_server_socket = -1;
	static int tmp_server_socket_bak = -1;
//	static int cur_comm_count=0;
	int rc=0;
	int retval;
	int ErrCode = 0;

	if(mIsTcpInit<0)
		return mIsTcpInit;
	//whether new tcp connection is coming or not
	FD_ZERO(&rfds);
	FD_SET(server_socket, &rfds);
	tv.tv_sec=0;
	tv.tv_usec=0;
	retval=select(server_socket+1, &rfds, NULL, NULL, &tv);
//	printf("*");
	if((retval>0) && FD_ISSET(server_socket, &rfds))
	{
		//DBPRINTF("EthCommTCPCheck  001 server_socket = %d, retval =%d\n",server_socket,retval );

		tmp_server_socket=accept(server_socket, (struct sockaddr *)&pin, &address_size);
		if (tmp_server_socket!=-1)
		{
			DBPRINTF("EthCommTCPCheck accept OK  tmp_server_socket = %d \n",tmp_server_socket);
	        //预防非常连接（没有断开又连接），造成系统被打开的文件句柄达到最大值（1023）。
			if(tmp_server_socket > 888)
	        {
	        	system("reboot");
	        }
			fcntl(tmp_server_socket, F_SETFL, O_NONBLOCK);
		}
		else
		{
			perror("accept");
		}
	}
	else if(tmp_server_socket)
	{
//		DBPRINTF("EthCommTCPCheck  step2  retval = %d \n",retval);
		int curLen = 0;

		curLen = TcpRecv(tmp_server_socket, buf, 2000);

		if(curLen > 0)
		{
			DEBUGTRACE(buf,curLen);
			GPIO_netled_active_Status(1);
			rc=ProcessTCPPackage(buf, curLen, tmp_server_socket);
			if(CMD_DISCONNECT == rc)//断开连接，会关闭当前连接句柄，所以备份的连接句柄清0。
			{
				tmp_server_socket_bak = -1;
			}
			else if (ERR_CMD_COMKEY_ERROR == rc)//通信密码错误
			{
				tmp_server_socket = tmp_server_socket_bak;
				printf("tmp_server_socket %",tmp_server_socket);
			}
			else
			{
				if(tmp_server_socket_bak != tmp_server_socket)//新连接，需要关闭旧的连接句柄。
				{
					if(0 < tmp_server_socket_bak )
					{
						close(tmp_server_socket_bak);
					}
					tmp_server_socket_bak = tmp_server_socket;
				}

			}
			GPIO_netled_active_Status(0);
		}
	}

	return rc;
}




int InitUDPSocket(U16 port, int *udpsocket)
{
	struct sockaddr_in sin;
	long save_file_flags;

	//Initialize socket address structure for internet protocol
	bzero(&sin, sizeof(sin));
	sin.sin_family = AF_INET;
	sin.sin_addr.s_addr = htonl(INADDR_ANY);
	sin.sin_port = htons(port);

	//create a receive UDP Scoket
	*udpsocket=socket(AF_INET, SOCK_DGRAM, 0);
	if (*udpsocket==-1) return -1;
	//bind it to the port
	if (bind(*udpsocket, (struct sockaddr *)&sin, sizeof(sin))==-1)
	{
		close(*udpsocket);
		*udpsocket=-1;
		return -1;
	}
	//set socket to non-blocking
	save_file_flags = fcntl(*udpsocket, F_GETFL);
	save_file_flags |= O_NONBLOCK;
	if (fcntl(*udpsocket, F_SETFL, save_file_flags) == -1)
	{
		close(*udpsocket);
		*udpsocket=-1;
		return -1;
	}
	return 0;
}

void ForceClearMACCacheIP(unsigned char *ipaddress)
{
	char buffer[128];
	struct sockaddr_in pin;
	char msg[128];
	int i;

	if(ipaddress[0])
	{
		bzero(&pin, sizeof(pin));
		sprintf(buffer, "%d.%d.%d.%d", ipaddress[0], ipaddress[1], ipaddress[2], ipaddress[3]);
		pin.sin_family=AF_INET;
		pin.sin_addr.s_addr=inet_addr(buffer);
		pin.sin_port=htons(THEUDP+1);
		if(gOptions.StartUpNotify&2)
		{
			sprintf(msg, "\"%s-%s-%d\" Started.", DeviceName, SerialNumber, gOptions.DeviceID);
			for(i=0;i<5;i++)
				sendto(comm_socket, msg, strlen(msg), 0, (struct sockaddr*)&pin, sizeof(struct sockaddr_in));
		}
	}
}


int InitTCPSocket()
{
	struct sockaddr_in sin;
	if ((server_socket=socket(AF_INET, SOCK_STREAM, 0))==-1)
	{
		return -1;
	}

	bzero(&sin, sizeof(sin));
	sin.sin_family = AF_INET;
	sin.sin_addr.s_addr = htonl(INADDR_ANY);
	sin.sin_port = htons(gOptions.TCPPort);

	int val = 1;
	setsockopt( server_socket, SOL_SOCKET, SO_REUSEADDR, &val, sizeof(val) );

	if (bind(server_socket, (struct sockaddr *)&sin, sizeof(sin))==-1)
	{
		printf("TCP bind ERROR \n");
		return -1;
	}

	if (listen(server_socket, 1)==-1)
	{
		close(server_socket);
		printf("TCP listen ERROR \n");
		return -1;
	}

	return 0;
}



//初始化UDP通讯，一般是建立通讯的UDP socket.
//Initilization UDP

int InitBroadCastPort(void)
{
	if(InitUDPSocket(BROADCASTPORT, &receive_broadcast_socket)!=0)
	{
		return FALSE;
	}
	else
	{
		return TRUE;
	}
}

int EthInit(void)
{
//	struct sockaddr_in sin;

	if(InitUDPSocket(THEUDP, &comm_socket)!=0) return -1;
	DBPRINTF("InitUDPSocket THEUDP = %d,comm_socket = %d\n",THEUDP,comm_socket);
//	DBPRINTF("EthInit _DEBUG TRACE 000\n");
	if(InitUDPSocket(THEUDP+1, &listenAuthServer_socket)!=0) return -1;
	if(InitUDPSocket(BROADCASTPORT, &receive_broadcast_socket)!=0) return -1;

//	int  so_broadcast = 1;
//	int broadcastResult = setsockopt(receive_broadcast_socket, SOL_SOCKET, SO_BROADCAST, &so_broadcast, sizeof (so_broadcast));
//	if(broadcastResult < 0)
//		printf("set broadcast send error.\n");



	//Force gateway router clear cached IP Address infomation
	ForceClearMACCacheIP(gOptions.GATEIPAddress);

/*
	//create a tcp socket for monitor connection from clients
	if ((server_socket=socket(AF_INET, SOCK_STREAM, 0))==-1) return -1;
	//bind tcp socket with current parameter
	bzero(&sin, sizeof(sin));
	sin.sin_family = AF_INET;
	sin.sin_addr.s_addr = htonl(INADDR_ANY);
	sin.sin_port = htons(gOptions.TCPPort);

//	int val = 1;
//	setsockopt( server_socket, SOL_SOCKET, SO_REUSEADDR, &val, sizeof(val) );

	if (bind(server_socket, (struct sockaddr *)&sin, sizeof(sin))==-1)
	{
		DBPRINTF("InitTCPSocket bind error \n");
		return -1;
	}
//	if (listen(server_socket, 1)==-1) return -1;
	if (listen(server_socket, 5)==-1)
	{
		DBPRINTF("InitTCPSocket listen error \n");
		return -1;
	}

	FD_ZERO(&rfds);
	FD_SET(server_socket, &rfds);
	tv.tv_sec=0;
	tv.tv_usec=0;
*/

	mIsTcpInit = InitTCPSocket();

	DBPRINTF("InitTCPSocket TCPPort = %d,server_socket = %d\n",gOptions.TCPPort,server_socket);

	return 0;
}

void EthFree(void)
{
	if (comm_socket>0) close(comm_socket);
	if (server_socket>0) close(server_socket);
	if (listenAuthServer_socket>0) close(listenAuthServer_socket);
	if (receive_broadcast_socket>0) close(receive_broadcast_socket);
}

void EthFreeBroadcastSocket(void)
{
	if (receive_broadcast_socket>0)
	{
		close(receive_broadcast_socket);
	}
}


/*
 检测网卡与网线方法一：
 interface_detect_beat_ethtool 调用  ioctl的SIOCETHTOOL实现，net/core/dev.c里面的dev_ethtool函数处理。。
   这个函数仅在linux 2.4内核下可用。即支持zen510平台。

*/

int get_netlink_status(void)
{
    const char *if_name="eth0";
    int skfd;
    struct ifreq ifr;
    struct ethtool_value edata;
    //edata.cmd = ETHTOOL_GLINK;
    edata.cmd = 0x0000000a;
    edata.data = 0;

    memset(&ifr, 0, sizeof(ifr));
    strncpy(ifr.ifr_name, if_name, sizeof(ifr.ifr_name) - 1);
    ifr.ifr_data = (char *) &edata;

    if (( skfd = socket( AF_INET, SOCK_DGRAM, 0 )) == 0)
    {
            return -1;
    }

    if(ioctl( skfd, SIOCETHTOOL, &ifr ) == -1)
    {
			//printf("ETHTOOL_GLINK failed: %s\n",strerror(errno));
            close(skfd);
            return -2;
    }

    close(skfd);

    return edata.data;
}

/*
 检测网卡与网线方法二：
 使用检测ifconfig eth0， 从中找到RUNNING，则认为连接OK，否则为掉线。
  此方法，可在任何平台使用，但效率差。

*/

int get_netlink_status_by_RUNNING(void)
{
	FILE *fd = NULL;
	char *p = NULL;
	char buf[1024] = {0};

	memset(buf, 0x00, sizeof(buf));
	fd = popen("ifconfig eth0", "r");

	fread(buf, 1, 1023, fd);
	fclose(fd);

	//printf("ifconfig eth0: %s\n", buf);
	p = strstr(buf, "RUNNING");

	//printf("p = %s \n", p);
	if(NULL != p)
	{
		return 1;
	}
	else
	{
		return 0;
	}
}

/*
 检测网卡与网线方法三：
 interface_detect_beat_ethtool 调用  ioctl的SIOCETHTOOL实现，net/core/dev.c里面的dev_ethtool函数处理。
   这个函数可在linux 2.6内核环境下使用。可支持560平台。

*/



int get_netlink_status_by_mii(void)
{
	struct mii_data *mii = NULL;
	int sockfd;
	struct ifreq ifr;


	if((sockfd = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP)) == -1)
	{
		perror("socket");
		return -1;
	}

	memset(&ifr, '\0', sizeof(ifr));
	strncpy(ifr.ifr_name, "eth0", IFNAMSIZ - 1);
	if(ioctl(sockfd, SIOCGMIIPHY, &ifr) == -1)
	{
		perror("ioct SIOCGMIIPHY");
		return -1;
	}

	mii = (struct mii_data*)&ifr.ifr_data;
	mii->reg_num = 0x01;
	if(ioctl(sockfd, SIOCGMIIREG, &ifr) == -1)
	{
		perror("ioctl SIOCGMIIREG");
		return -1;
	}

	close(sockfd);
	if(mii->val_out & 0x0004)
	{
		//printf("link up\n");
		return 1;
	}
	else
	{
		//printf("link down\n");
		return 0;
	}
}
