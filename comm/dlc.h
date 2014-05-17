#define C3_HEAD		0xaa			// 通讯帧前导字节
#define C3_END		0x55			// 通讯帧结束字节

#define TCP_MODE  					0x01
#define UDP_MODE  					0x02
#define RS485_232_MODE  			0x03
#define USB_MODE  					0x04
#define HTTP_MODE  					0x05
#define UDP_BROAD_MODE 			0x06


#define COMMBUFLEN_MAX_RS485_232   DATABUFLEN_MAX				//2048
#define COMMBUFLEN_MAX_UDP  		   DATABUFLEN_MAX				//1472//548
#define COMMBUFLEN_MAX_TCP		DATABUFLEN_MAX				//64000

#define COMM_BUFLEN_MAX   5100
#define DATABUFLEN_MAX   5200							//COMMBUFLEN_MAX_TCP+20//5200

typedef struct _C3Cmdstruct_{
	BYTE framehead;
	BYTE addr;
	BYTE command;
	WORD datalen;
	BYTE buf[DATABUFLEN_MAX];
	WORD CRC16;
	BYTE framelof;
}TC3Cmdstruct, *PC3Cmdstruct;

// 通讯收发状态表
typedef enum _C3cmdstatus_
{
	C3_STANDBY = 0x10,				// 等待

	C3_GET_HEAD= 0x20,				// 接收帧前导字节
	C3_GET_ADDR,					// 接收来源地址
	C3_GET_CMD,					// 接收帧命令
	C3_GET_LENH,					// 接收帧长度高字节
	C3_GET_LENL,					// 接收帧长度低字节
	C3_GET_DATA,					// 接收帧内容
	C3_GET_CRC16H,					// 接收循环校验码高字节
	C3_GET_CRC16L,					// 接收循环校验码低字节
	C3_GET_END,					// 接收帧结束

	C3_PUT_HEAD = 0x40,			// 发送帧前导字节
	C3_PUT_ADDR,					// 发送目的地址
	C3_PUT_CMD,					// 发送帧命令
	C3_PUT_LENH,					// 发送帧长度高字节
	C3_PUT_LENL,					// 发送帧长度低字节
	C3_PUT_DATA,					// 发送帧内容
	C3_PUT_CRC16H,					// 发送循环校验码高字节
	C3_PUT_CRC16L,					// 发送循环校验码低字节
	C3_PUT_END						// 发送帧结束
}TC3Cmdstatus;

typedef enum _C3cmdResult_
{

	EC_FINISH = 0,
	EC_UNKNOWNERR = 0 + 0x100,

	EC_BUFOVERFLOW = 0 + 0x200,
	EC_ADDRERR,
	EC_DATAERR,
	EC_FRAMEERR,
	EC_CRCERR,

	EC_NORECEIVE= 0 + 0x400,
	EC_TIMEOUT,
	EC_ERRTARGET,
	EC_STANDBY
}TC3CmdResult;



void DEBUGTRACE(char *buf,int len);
void DEBUGTRACE2(char *buf,int len);

int Process_ReceiveData(char *buf,int len,void  *outbuf,int CommNetType);
int Make_SendData(char *buf,char cmd,int len,char *outbuf,int *SendLen);
