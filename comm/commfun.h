//---------------------------------------
/*
新版通讯协议命令定义
*/

#ifndef _COMMFUN_H_
#define _COMMFUN_H_

#define SENDER_LEN	16


#define PROTO_START_TAG 0xaa
#define PROTO_END_TAG   0x55

#define CMD_CONNECT				1		//连接
#define CMD_DISCONNECT			2		//断开连接
#define CMD_SET_DEVICE_PARAM	3		//设置设备参数
#define CMD_GET_DEVICE_PARAM	4		//读取设备参数
#define CMD_CONTROL_DEVICE		5		//控制设备执行动作
//
#define CMD_GET_TABLE_STRUCT      6		//获取所有表结构,一次完成
#define CMD_SET_DATA			7		//上传数据 PC->Device
#define CMD_GET_DATA			8		//下载数据 Device->PC
//#define CMD_GET_DATA2			238		//下载数据 Device->PC  debug

#define CMD_DELETE_DATA			9		//删除数据
#define CMD_GET_DATA_COUNT		10		//得到数据总数
#define CMD_GET_REALTIME_LOG	       11		//下载实时数据
#define CMD_QUERY_DATA			12		//下载大数据

#define CMD_PREPARE_DATA	13	// 准备接收大数据
#define CMD_READ_DATA		14	//读数据  device -> pc
#define CMD_FREE_DATA		15	// 数据接收或处理完毕，告诉通讯对象释放内存
#define CMD_PREPARE_BREAK_DATA	16	//断点续传命令
#define CMD_DATA			17	// Send a data packet     pc -> device

//
#define CMD_BROADCAST		20		//广播 命令寻找IP
#define CMD_BROAD_MODIYIP		21		//广播 命令修改IP
#define CMD_SET_BINARYDATA 22 //上传文件  pc ->device

#define CMD_GET_BINARYDATA 23 //下载文件   device->pc

#define CMD_GET_DATA_OK			25

#define CMD_ALARM_CANCEL			27		//解除报警

//--------------------------------------
#define CMD_ACK_OK 200		// 命令成功执行
#define CMD_ACK_ERROR 201	// 命令执行失败


//add
#define CMD_HASH_DATA		119		//取哈希值


#define CMD_SUCCESS	0
//命令执行失败原因 接在命令执行失败命令字201后面
#define ERR_SEND_FAILED 				-1			// 命令发送失败
#define ERR_NORESPOSE					-2			// 命令没有回应
#define ERR_BUFFER_NOT_ENOUGH		    -3	        // 需要的缓存不足
#define ERR_DATA_COMPRESS_FAILED		-4	        //解压失败
#define ERR_DATA_READ_LENGTH_ERROR	    -5		    //读取数据长度不对
#define ERR_DATA_CPLENGTH_ERROR		    -6	        //解压的长度和期望的长度不一致
#define ERR_CMD_REPEAT 				    -7	        // 命令重复
#define ERR_CMD_UNAUTH 				    -8	        // 连接尚未授权
#define ERR_CMD_DATA_CRC_ERROR		    -9	        // 数据错误，CRC校验失败
#define ERR_CMD_DATA_API_ERROR		    -10     	// 数据错误，dataapi无法解析
#define ERR_CMD_DATA_PARA_ERROR		    -11	        // 数据参数错误
#define ERR_CMD_PROCESS_ERROR 		    -12	        // 命令执行错误
#define ERR_CMD_ERROR_CMD 				-13	        // 命令错误，没有此命令
#define ERR_CMD_COMKEY_ERROR			-14	        // 通迅密码错误
#define ERR_CMD_WFILE_ERROR 			-15	        // 写文件失败
#define ERR_CMD_RFILE_ERROR 			-16	        // 读文件失败
#define ERR_CMD_FILE_NOTEXIST 		    -17	        // 文件不存在
#define ERROR_CMD_NO_SPAC				-18         //设备没有空间
#define ERR_CMD_DATA_CHKSUM_ERROR		-19         //校验和出错
#define ERR_CMD_DATA_LEN_ERROR			-20         //接受到的数据长度与给出的数据长度不一致
#define ERR_CMD_NO_PLATFORME_ERROR		-21         //设备中，没有设置平台参数
#define ERR_CMD_DIFF_PLATFORM_ERROR 	-22     	//固件升级，传来的固件的平台与本地的平台不一致
#define ERR_CMD_UPDATE_OLDVER_ERROR		-23         //升级的固件版本比设备中的固件版本老
#define ERR_CMD_UPDATE_FILEFLG_ERROR	-24         //升级的文件标识出错
#define ERR_CMD_FILENAME_ERROR 		    -25	        //固件升级，传来的文件名不对，即不是emfw.cfg

#define ERR_CMD_UNKNOWN 			    -99	        // 未知错误



#define PROTO_NODATA_LEN	8	//协议中不包含数据域的长度
//
#define CMD_ACK_DATA		2002		/* Return Data */
#define CMD_ACK_NO			0xfffe		/* device not reply or wait timeout */
#define CMD_ACK_ERROR_INIT	0xfffc		/* Not Initializated */
#define CMD_ACK_ERROR_DATA	0xfffb
#define CMD_ACK_RETRY		2003		/* Regstered event occorred */


#define	MAX_TRY_CMD_TIME	3	//失败重试次数

#define	SEND_COMMCMD_OK	0	//向main进程发送指令，main进程处理完成后，在设定时间内返回。
#define	ERR_SEND_COMMCMD_TIMEOUT	-1   //main进程返回给comm进程超时。
#define	ERR_SEND_COMMCMD_DATAERR	-2   //comm进程发送了错误的数据给main进程。
#define	ERR_SEND_COMMCMD_OVERMAXLEN -3   //comm发送的数据超过了最大值。
#define	ERR_SEND_COMMCMD_MAINING	-4   //comm进程检测到main进程在发送。

#define RAMPATH "/mnt/ramdisk/"

typedef struct
{
	BYTE StartTag;
	BYTE DestAddr;
	BYTE Command;
	WORD DataSize;
}GCC_PACKED TProtoHeader,*PProtoHeader;

int process_comm_cmd(BYTE *cmd,BYTE *buf,int inbuflen,BYTE *outbuf,int *outbuflen,int commmode);
int 	GetTableStruct(BYTE *outbuf,int *outbuflen);
int	GetDataCount(BYTE *buf,int inbuflen,BYTE *outbuf,int *outbuflen);

void processBuf_DelDatabase(char *buf ,int inbuflen);// ,int judgePrimaryKey);

int BufToDatabase(char *buf ,int len );//,int handle_datafile)
void DatabaseToBuf(char *inbuf,int inbuflen);//,char *outbuf,int *outbuflen);
int SaveTableContent(int FCT,char *saveBuf);

#endif
