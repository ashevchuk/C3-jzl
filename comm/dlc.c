//data link layer process by oscar  20100105
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "arca.h"
#include "dlc.h"
#include "crc16.h"
#include "options.h"


//example data: //aa 01 01 02 00 cd cd a8 ac 55


void DEBUGTRACE(char *buf,int len)
{	int i;

	printf("Debug data trace,len =%d\n",len);
	for(i = 0;i<len && i < 32;i++)
	{
		if(i%16 ==0 && i !=0)
			printf("\n");			
		printf("%02x,",buf[i] & 0xff);
	}
	printf("\n");
}


void DEBUGTRACE2(char *buf,int len)
{	int i;

	printf("DEBUGTRACE2 data trace,len =%d\n",len);
	for(i = 0;i<len;i++)
	{
		if(i%16 ==0 && i !=0)
			printf("\n");			
		printf("%02x,",buf[i] & 0xff);
	}
	printf("\n");
}

int isCommbufLenOK(int len, int CommNetType)
{
	if((CommNetType == TCP_MODE && len <= COMMBUFLEN_MAX_TCP )
	   || (CommNetType == UDP_MODE && len <= COMMBUFLEN_MAX_UDP ) 	
	   || (CommNetType == UDP_BROAD_MODE && len <= COMMBUFLEN_MAX_UDP ) 	
	   || (CommNetType == RS485_232_MODE && len <= COMMBUFLEN_MAX_RS485_232) 	)
		return 0x01;
	else
		return 0x00;
}

int Process_ReceiveData(char *buf,int len,void  *outbuf,int CommNetType)
{
	int rs,i,size;
	WORD CRC16;
	BYTE c,myaddr,CRC16H, CRC16L;

//	printf("checkRecData start ......\n");
	size = 0;	
	CRC16 = 0;
	CRC16H =  CRC16L = 0;


	if(!(isCommbufLenOK(len,CommNetType)))
		return 	EC_BUFOVERFLOW;	

	PC3Cmdstruct c3buf = (PC3Cmdstruct)outbuf;
	for(rs=C3_STANDBY,i =0;i< len;i++) 
	{
		c= buf[i];

		switch (rs) 
		{
			case C3_STANDBY:
				if (c == C3_HEAD) //
				{
					CRC16_Clear(&CRC16);
					rs = C3_GET_HEAD;
					rs = C3_GET_ADDR;
				}
				break;

			case C3_GET_ADDR:
				myaddr = c;
				if(myaddr ==gOptions.DeviceID  || myaddr == 0x00 || CommNetType != RS485_232_MODE )//0x00Ϊ��̫��ͨѸ����485�㲥��ַ
				{
					CRC16_Calc(&CRC16, c);
					rs = C3_GET_CMD;
				} 
				else
				{
					rs = C3_STANDBY;
					return  EC_ADDRERR;
				}
				break;

			case C3_GET_CMD:
				c3buf->command = c;
				CRC16_Calc(&CRC16, c);
				rs = C3_GET_LENH;
//				printf("checkRecData cmd = %d\n",((PC3Cmdstruct)c3buf)->command);
				break;

			case C3_GET_LENH:
				c3buf->datalen = (DWORD)c;
				CRC16_Calc(&CRC16, c);
				rs = C3_GET_LENL;
				break;

			case C3_GET_LENL:
				c3buf->datalen += ((DWORD)c << 8);
		/*		if (len > MAXBUF) {
					r = EC_BUFOVERFLOW;
					goto RET;
				}*/
				CRC16_Calc(&CRC16, c);
//				printf("checkRecData len = %d\n",((PC3Cmdstruct)c3buf)->datalen);
				if(c3buf->datalen == 0)
					rs = C3_GET_CRC16H;
				else
					rs = C3_GET_DATA;				
				break;
			case C3_GET_DATA:
				c3buf->buf[size] = c;
				CRC16_Calc(&CRC16, c);
				size++;
				if(size == c3buf->datalen)
					rs = C3_GET_CRC16H;
				else
					rs = C3_GET_DATA;				
				break;

			case C3_GET_CRC16H:
				CRC16L = c;
				rs = C3_GET_CRC16L;
				break;

			case C3_GET_CRC16L:
				CRC16H = c;
				printf("checkRecData crc= %x \n",CRC16);				
				CRC16_Calc(&CRC16, CRC16L);
				CRC16_Calc(&CRC16, CRC16H);
//				rs = C3_GET_END;
				if(CRC16 == 0) 
					rs = C3_GET_END;
				else
					return EC_CRCERR;
				break;
			case C3_GET_END:
				if (c == C3_END) 
				{
					rs = C3_STANDBY;
					return EC_FINISH;
				}
				break;
		}
	}

	return EC_FRAMEERR;				
}


int Make_SendData(char *buf,char cmd,int len,char *outbuf,int *SendLen)
{
	WORD CRC16;
	BYTE c,*P;
	int cs,i,size;
	
	c = 0;
	CRC16 = 0;
	size = 0;
	P = outbuf;
	for(i= 0,cs=C3_PUT_HEAD;; )
	{
		switch (cs)
		{
		case C3_PUT_HEAD:
			c = C3_HEAD;
			CRC16_Clear(&CRC16);
			cs = C3_PUT_ADDR;
			break;

		case C3_PUT_ADDR:
			c = gOptions.DeviceID & 0xff;
			CRC16_Calc(&CRC16, c);
			cs = C3_PUT_CMD;
			break;

		case C3_PUT_CMD:
//			printf("SENDData cmd\n");				
			c= cmd;							
			CRC16_Calc(&CRC16, c);
			cs = C3_PUT_LENH;
			break;

		case C3_PUT_LENH:
			c= len  & 0xff;	
			CRC16_Calc(&CRC16, c);
			cs = C3_PUT_LENL;
			break;

		case C3_PUT_LENL:
			c= (len >> 8) & 0xff;							
			size = 0;
			CRC16_Calc(&CRC16, c);
			if(len !=0)
				cs = C3_PUT_DATA;
			else
				cs = C3_PUT_CRC16H;
			break;

		case C3_PUT_DATA:
			c= buf[size];
			size++;
			CRC16_Calc(&CRC16, c);
			if(size == len)
				cs = C3_PUT_CRC16H;
			else
				cs = C3_PUT_DATA;				
			break;

		case C3_PUT_CRC16H:
			c= CRC16  & 0xff;	
			cs = C3_PUT_CRC16L;
			break;

		case C3_PUT_CRC16L:
			c= (CRC16 >> 8) & 0xff;							
			cs = C3_PUT_END;
			break;


		case C3_PUT_END:
//			printf("SENDData end\n");				
			c= C3_END;							
			P[i] = c;
			i++;
			*SendLen = i;

//			DEBUGTRACE2(outbuf, *SendLen);	
			return 1;
			break;
		}

		P[i] = c;
		i++;
	}

	*SendLen = i;

//	DEBUGTRACE2(outbuf, *SendLen);	
	return 0;	

}


