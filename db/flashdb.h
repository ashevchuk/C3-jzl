/*************************************************

 ZEM 200

 flashdb.h define all functions for database mangement of flash

 Copyright (C) 2003-2005, ZKSoftware Inc.

*************************************************/

#ifndef _FLASHDB_H_
#define _FLASHDB_H_

#include <time.h>
#include <sys/types.h>
#include "arca.h"
#include "../public/ccc.h"
//#include "ccc.h"
#include "exfun.h"


#define STAT_COUNT 		0
#define STAT_VALIDLEN 		1
#define STAT_CNTADMINUSER 	2
#define STAT_CNTADMIN 		3
#define STAT_CNTPWDUSER		4
#define STAT_CNTTEMPLATE	5
#define STAT_NEWVALIDLEN	6


#define MAX_USER_FINGER_COUNT 10	/* Max Finger Count for a User */
//空扇区
#define FCT_EMPTY (U8)0xFF
//考勤记录
#define FCT_ATTLOG (U8)1
//指纹数据
#define FCT_FINGERTMP (U8)2
//操作记录
#define FCT_OPLOG (U8)4
//用户记录
#define FCT_USER (U8)5
//SMS
#define FCT_SMS (U8)6
//UDATA
#define FCT_UDATA (U8)7
//System Options
#define FCT_SYSOPTIONS (U8)0x0a
//System Reserved
#define FCT_SYSTEM	(U8)0x0b
//Not for used
#define FCT_SYSTEM_NONE (U8)0xF0
//Web pages
#define FCT_WEBPAGES (U8)0x0c

#define FCT_CUSTATTLOG	(U8)0x70

#define FCT_ALL	-1
//Extend user infomation
#define FCT_EXTUSER	(U8) 8
//workcode infomation
#define FCT_WorkCode	(U8)9

//dsl 2008.4.18
//customize attend state
#define FCT_CUSTATTSTATE	(U8)0x0d
//custmoize voice
#define FCT_CUSTVOICE		(U8)0x0e


//add by oscar for c3
#define FCT_C3USER							(U8)0x10
#define FCT_C3CARDAUTHORIZE 				(U8)0x11
#define FCT_C3HOLIDAY 						(U8)0x12
#define FCT_C3TIMEZONE						(U8)0x13
#define FCT_C3FIRSTCARD						(U8)0x14
#define FCT_C3MULTICARDOPENDOOR				(U8)0x15
#define FCT_C3GUARDEREVENTLOG				(U8)0x16
#define FCT_C3INOUTFUN						(U8)0x17

#define FCT_C3GUARDEREVENTLOG_OFFSET        (U8)0x18
#define FCT_C3GUARDEREVENTLOG_BAK           (U8)0x19  //保存门禁记录的同时，把门禁记录保存在SD（transaction.dat）中。
#define FCT_C3GUARDEREVENTLOG_OFFSET_BAK    (U8)0x1a  //保存文件指针，文件指针是每次备份到SD后门禁记录文件的当前指针。
//end

#define PIN_WIDTH 5
#define MAX_PIN ((U16)0xFFFE)
#define MAX_PIN2 ((U32)0x7FFFFFFE)

#define FDB_OK 0				/* Success */
#define FDB_ERROR_NOTINIT	-1  		/* Database Not initialized */
#define FDB_ERROR_OP -2			/* Wrong Operation */
#define FDB_ERROR_IO -3			/* Flash I/O Error */
#define FDB_ERROR_NODATA -4		/* No (matched) Data */
#define FDB_ERROR_NOSPACE -5		/* No more space */
#define FDB_ERROR_DATA	-6		/* Data is not correct */

#define FDB_OVER_FLIMIT 1
#define FDB_OVER_UFLIMIT 2
#define FDB_OVER_ULIMIT	3
#define	FDB_FID_EXISTS 4



enum __VERIFY_STYLE
{
	VS_FP_OR_PW_OR_RF=0,
	VS_FP,
	VS_PIN,
	VS_PW,
	VS_RF,
	VS_FP_OR_PW,
	VS_FP_OR_RF,
	VS_PW_OR_RF,
	VS_PIN_AND_FP,
	VS_FP_AND_PW,
	VS_FP_AND_RF,
	VS_PW_AND_RF,
	VS_FP_AND_PW_AND_RF,
	VS_PIN_AND_FP_AND_PW,
	VS_FP_AND_RF_OR_PIN,
	VS_OTHER=200,
	VS_NUM=16
};

//门打开原因，刷卡1，门常开2，首卡常开3,远程通迅4，联动5，意外为 0
enum _OPENLOCKCAUSE
{
	NOCAUSE,
	POLLCARD = 1,
	DOORKEEPOPEN,
	FIRSTCARDCAUSE,
	COMMCONTROL,
	LINKCON
};



enum __EVENT_STYPE
{
	EVENT_OK = 0,					//0正常刷卡开门
	EVENT_DOORKEEPOPEN_OK = 1,	    //1门常开时段刷卡记录
	EVENT_FIRSTCARD_OK,		        //2首卡激活常开时段刷卡记录
	EVENT_MULTICARD_OK,	            //3多卡开门正常开门
	EVENT_SUPPERPASSWORD_OK,	    //4超级密码开门
	EVENT_DOORKEEPOPENACTIVED_OK,	//5门常开时段到了，开始激活,并启动开门
	EVENT_LINKCONTROL,	            //6联动事件类型
	EVENT_COMMCONTROL,	            //7取消报警事件
	EVENT_COMMOPENDOOR,	            //8通迅远程开门事件
	EVENT_COMMCLOSEDOOR,	        //9通迅远程关门事件
	EVENT_CANCEL_DOORKEEPOPEN = 10, //10取消常开
	EVENT_START_DOORKEEPOPEN,       //11启用常开
	EVENT_COMMOPENALARM,            //12远程开启辅助输出
	EVENT_COMMCLOSEALARM,           //13远程关闭辅助输出
	EVENT_INTERVAL_ERROR = 20,      //20刷卡间隔太短
	EVENT_DOORSLEEP_ERROR,		    //21门非激活时间段
	EVENT_ILLEGALTIME_ERROR,        //22本时段无合法权限
	EVENT_NORIGHT_ERROR,            //23本时段此门无权限
	EVENT_ANTIBACK_ERROR, 		    //24返潜回
	EVENT_MUTILOCKLINGAGE_ERROR, 	//25双门互锁没通过
	EVENT_MULTICARD_ERROR, 	        //26多卡开门刷卡
	EVENT_ILLEGALCARD,	            //27非法卡，即用户表中无此卡
	EVENT_DOORCONTACTOVERTIME,	    //28  门磁超时
	EVENT_CARD_NOTVALID_TIME,	    //29卡过了有效期
	EVENT_PASSWORD_ERROR,           //30密码错误
	EVENT_FP_INTERVAL_ERROR,        //31按指纹间隔太短
	EVENT_MULTIFINGER_ERROR,        //32多指纹验证
	EVENT_FP_NOTVALID_TIME,         //33指纹已过有效期
	EVENT_ILLEGALFP,                //34指纹未注册
	EVENT_DOORTIME_ERROR_FP,        //35门非激活时间段按指纹
	EVENT_BUTTONOPEN_ERROR,         //36门非时间段按出门按钮
	EVENT_DOORKEEPOPEN_ERROR,         //37门常开时段无法关门
	EVENT_ANTIPASSBACK=41,          //41后台验证
	EVENT_ANTIPASSBACK_FAILED,      //42后台验证失败
	EVENT_ANTIPASSBACK_TIMEOUT,     //43后台验证超时
	EVENT_BACKGROUND_VALID,			//44后台验证事件
	EVENT_NOUSER = 45,              //用户不存在

	EVENT_ANTISTRIPALARM = 100,		//100防拆报警
	EVENT_FORCEALARM,		        //101协迫密码开门报警
	EVENT_UNEXPECTED_DOOROPEN,	    //102门意外被打开

	EVENT_DOOROPEN= 200,	        //(门打开)
	EVENT_DOORCLOSE,	            //(门关闭)
	EVENT_BUTTONOPENDOOR,	        //202按键开门
	EVENT_MULTI_CARD_AND_FINGER,    //203多卡加指纹开门
	EVENT_DOORKEEPOPEN_END,         //204门常开时间段结束
	EVENT_COMM_KEEPOPENDOOR,        //205远程常开
	EVENT_MACHINE_START,            //206设备启动
	ECENT_PW_OPENDOOR,              //207密码开门
	EVENT_AUXIN_OPEN = 220,		    //220扩展输入口开
	EVENT_AUXIN_CLOSE,	            //221扩展输入口关
	EVENT_DOORSTATUS =255	        //门状态
};

U32 CurAttLogCount;
U32 CurGuarderEventLogCount;
U32 CurGuarderEventLogCount2;
//database initialization, return Fingertemplate count
int FDB_InitDBs(BOOL OpenSign);
int GetDataInfo(int ContentType, int StatType, int Value);

void FDB_FreeDBs(void);
int FDB_GetSizes(char* Sizes);
//Clear all data
int FDB_ClearData(int ContentType);

#define PRI_VALIDUSER 	0
#define PRI_INVALIDUSER 1
#define PRI_VALIDBIT	1
#define PRI_ENROLL	2
#define PRI_OPTIONS 	4
#define PRI_SUPERVISOR 	8
#define ISADMIN(p)	(((p) & ~PRI_VALIDBIT)!=0)

//Added by mjh 07/08/20
#define PRI_ATTLOG	10
///////////////////////End add

#define PRIVILLEGE0 PRI_VALIDUSER
#define PRIVILLEGE1 (PRI_ENROLL)
#define PRIVILLEGE2 (PRI_ENROLL|PRI_OPTIONS)
#define PRIVILLEGE3 (PRI_ENROLL|PRI_OPTIONS|PRI_SUPERVISOR)

#define ISINVALIDUSER(user) (((user).Privilege & PRI_VALIDBIT)!=PRI_VALIDUSER)

#define MAXTEMPLATESIZE 602
#define MAXVALIDTMPSIZE 598 //for 4 bytes aligned

extern char PRIVALUES[];

#define MAXNAMELENGTH  8

//data structure for file data searching
typedef struct _SearchHandle{
	int ContentType;
	char *buffer;
	int bufferlen;
	int datalen;
	int fd;
}TSearchHandle, *PSearchHandle;

void SearchFirst(PSearchHandle sh);
void SearchEnd(PSearchHandle sh);
BOOL SearchNext(PSearchHandle sh);
void CopyTableToMem(PSearchHandle sh,BYTE *buf,int *len);

typedef struct _OPLog_{
	U16 Admin;		//2
	U16 OP;		//2
	time_t time_second;	//4
	U16 Users[4];//		//2*4
}GCC_PACKED TOPLog, *POPLog;

typedef struct _AlarmRec_{
	U32 PIN;
	//time_t LastTime;
	U8  State;//反潜
}GCC_PACKED TAlarmRec, *PAlarmRec;
//时间反潜，和读头有关
typedef struct _LastTimeAPB_{
	U32 PIN;
	time_t LastTime;
}GCC_PACKED TLastTimeAPB, *PLastTimeAPB;

int FetchLastLogs(PAlarmRec CurAlarmLog);
int GetUserLastLog(PAlarmRec lastlogs,U16 PIN,int LastLogsCount);
int GetCheckInCount(PAlarmRec lastlogs, int logsCount);
#define OP_POWER_ON    	0    //开机
#define OP_POWER_OFF 	1    //关机
#define OP_ALARM_VERIFY 2    //验证失败
#define OP_ALARM 	3    	//报警
#define OP_MENU 	4    	//进入菜单
#define OP_CHG_OPTION 	5    //更改设置, Users[3]定义为具体的修改项目
#define OP_ENROLL_FP   	6    //登记指纹
#define OP_ENROLL_PWD  	7    //登记密码
#define OP_ENROLL_RFCARD 	8//登记HID卡
#define OP_DEL_USER   	9    //删除用户
#define OP_DEL_FP      	10   //删除指纹
#define OP_DEL_PWD     	11   //删除密码
#define OP_DEL_RFCARD  	12   //删除射频卡
#define OP_CLEAR_DATA  	13   //清除数据
#define OP_MF_CREATE   	14	//创建MF卡
#define OP_MF_ENROLL   	15	//登记MF卡
#define OP_MF_REG      	16	//注册MF卡
#define OP_MF_UNREG    	17	//删除MF卡注册
#define OP_MF_CLEAR    	18	//清除MF卡内容
#define OP_MF_MOVE     	19	//把登记数据移到卡中
#define OP_MF_DUMP     	20	//把卡中的数据复制到机器中
#define OP_SET_TIME    	21   //设置时间
#define OP_RES_OPTION  	22   //恢复出厂设置
#define OP_CLEAR_LOG   	23   //删除进出记录
#define OP_CLEAR_ADMIN 	24   //清除管理员权限}
#define OP_ACC_GRP     	25   //修改门禁组设置
#define OP_ACC_USER    	26   //修改用户门禁设置
#define OP_ACC_TZ      	27   //修改门禁时间段
#define OP_ACC         	28   //修改开锁组合设置
#define OP_UNLOCK      	29   //开锁
#define OP_ENROLL_USER 	30   //登记新用户
#define OP_CHG_FP       31   //更改指纹属性
#define OP_DURESS       32   //胁迫报警
#define OP_VERIFYFAILED	33		//Verify failed
#define OP_ANTIPASSBACK 34

int FDB_AddOPLog(char *log,int len);

#define MAX_OPLOG_COUNT (64*1024/sizeof(TOPLog))

/*
0 - Read out all data now
-1 - Not read out any data
other - offset(in flash) of last read out
*/
#define RTLOGDATASIZE	16 //14->16 dsl 2007.7.30

typedef struct _RTLog_{
	U16 EventType;
	char Data[RTLOGDATASIZE];
} TRTLog, *PRTLog;

struct TRTLogNode;
typedef struct TRTLogNode *PRTLogNode;
struct TRTLogNode{
	TRTLog log;
	PRTLogNode Next;
};


typedef int (*ForARecFun)(void *rec, int index, void *param);
int FDB_ForData_C3(int ContentType, off_t fdOffset,ForARecFun fun, int MaxRecCount, void *param);

//add by oscar for c3
typedef struct _C3AcessLog_{
	U32 cardno;			//  卡号
	U32 pin;				//  pin号
	BYTE verified;		//验证类型   见上面定义
	BYTE doorID;			//门ID
	BYTE EventType;		//事件类型    见上面定义
	BYTE inoutstate;		// 进出状态   进为0，出为1  2为其它
	U32 time_second;      //linux 格式时间
}GCC_PACKED TC3AcessLog,*PC3AcessLog;


//输入密码结构
typedef struct _inputpassword_{
	U32 keybuftimeout;//输入密码计时
	U32 keybufindex;//密码位索引
	BYTE keybuf[12];//密码内容
	U32 keyvalue;//转化为10进制值
	U32 superpwdtimeout;
}GCC_PACKED TC3Inputpassword,*PC3Inputpassword;


//卡+ 密码 验证输入结构
typedef struct _cardandpassword_{
	U32 cardandpasswordsign;//判断记号
	U32 cardno;//卡号
	char password[8];//个人密码
}GCC_PACKED TC3Cardandpassword;


//卡号用户表
typedef struct _C3User_{
	U32 cardno;//卡号
	U32  PIN;//用户识别号
	char password[8];//4////个人密码
	U32 group;//分组
	U32 start_time;		//开始有效时间 20100101表示2010年1-1
	U32 end_time;		//失效启用时间
	U32 superAuthorize; //超级权限
//	U16 APBCount;       //时间反潜次数，次数为0时无权限开门
}GCC_PACKED TC3User, *PC3User;


//卡号授权表卡号 授权时区，适用门区  允许 一个卡有多条记录
typedef struct _cardauthorize_{
	U32 pin;//
	U32 authorizetimezoneid;//4//时区ID
	U32 authorizedoor;//本时区id适用哪几个门，采用位映射方式 0x01 表示1号门，0x0f 表示全部4个门
}GCC_PACKED TC3authorize, *PC3authorize;

//假日及类型表
typedef struct _holiday_{
	U32 holidayday;//20100101表示2010年1月1日
 	U32 holidaytype;//假日类型，只能为 1，2，3
 	U32 loop;		//1表示年循环，月日相等即可，2表示必须年月日相等
}GCC_PACKED TC3Holiday, *PC3Holiday;


//时区表
typedef struct _Threetimeperiod_{
	U32 time1;//
	U32 time2;//
	U32 time3;//
}GCC_PACKED TC3Timeperiod,*PC3Timeperiod;

typedef struct _timezone_{
	U32 timezoneid;	//索引号
	U32 suntime1;	//如例8:30 ~12:00   值 为(830 << 16 + 1200)   即0x33e04b0
	U32 suntime2;
	U32 suntime3;
	U32 montime1;	//如例8:30 ~12:00   值 为(830 << 16 + 1200)   即0x33e04b0
	U32 montime2;
	U32 montime3;
	U32 tuetime1;	//如例8:30 ~12:00   值 为(830 << 16 + 1200)   即0x33e04b0
	U32 tuetime2;
	U32 tuetime3;
	U32 wedtime1;	//如例8:30 ~12:00   值 为(830 << 16 + 1200)   即0x33e04b0
	U32 wedtime2;
	U32 wedtime3;
	U32 thutime1;	//如例8:30 ~12:00   值 为(830 << 16 + 1200)   即0x33e04b0
	U32 thutime2;
	U32 thutime3;
	U32 fritime1;	//如例8:30 ~12:00   值 为(830 << 16 + 1200)   即0x33e04b0
	U32 fritime2;
	U32 fritime3;
	U32 sattime1;	//如例8:30 ~12:00   值 为(830 << 16 + 1200)   即0x33e04b0
	U32 sattime2;
	U32 sattime3;
	U32 hol1time1;//假日1	//如例8:30 ~12:00   值 为(830 << 16 + 1200)   即0x33e04b0
	U32 hol1time2;
	U32 holt1ime3;
	U32 hol2time1;//假日2	//如例8:30 ~12:00   值 为(830 << 16 + 1200)   即0x33e04b0
	U32 hol2ime2;
	U32 hol2ime3;
	U32 hol3time1;//假日3	//如例8:30 ~12:00   值 为(830 << 16 + 1200)   即0x33e04b0
	U32 hol3time2;
	U32 hol3time3;
}GCC_PACKED TC3Timezone,*PC3Timezone;

//首卡开门表
typedef struct _firstcardopendoor_{
	U32 pin;//
	U32 doorID;//门ID
	U32 timezoneid;//4//
}GCC_PACKED TC3Firstcardopendoor, *PC3Firstcardopendoor;

//多卡开门组合表
typedef struct _multicardassemble_{
	U32 index;
	U32 doorID;
	U32 group[5];//组
}GCC_PACKED TC3Multicardassemble,*PC3Multicardassemble;

//输入输出自定义表
//支持输入io 与事件类型的组合
typedef struct _3InOutFunDefine_{
	U16 index;
	BYTE EventType;//触发事件索引
	BYTE InAddr;      //触发事件位置   即门ID或辅佐输入的ID  1~4    0为任意位置
	BYTE OutType;	//输出类型  0为门锁，1为辅助输出
	BYTE OutAddr;	//输出动作位置 目前c3_400/c4门锁为1~4, 辅助输出为1~4(1~6/c4)
	BYTE OutTime;	//输出动作时间 0关闭，1~254打开n秒，255常开
	BYTE Reserved;	//保留 以保证对齐
}GCC_PACKED TC3InOutFunDefine,*PC3InOutFunDefine;

enum KEEPOPEN_TYPE
{
	NO_REASON = 0, //0.没原因 1.首卡，2.门设置常开 3.远程命令
	FIRSTCARD,
	DOORSETKEEPOPEN,
	CMD_CONTROL
};

//门常开结构
typedef struct _doorkeepOpen_{
	U32 keepOpenSign;//是否启用常开
	U32  keepopenreason;//启用常开的原因  //0.没启用，1.首卡，2.门设置常开 3.远程命令
	U32 starttime;// 有效开始时间 08:00  即为800
	U32 endtime;	//结束时间
}GCC_PACKED TC3doorkeepOpen,*PC3doorkeepOpen;

//多卡开门组合
typedef struct _cardgroup_{
	U32  PIN;//用户识别号
	U32 group;//分组
}GCC_PACKED TC3Cardgroup;

//多卡开门结构
typedef struct _multicardopendoor_{
int MultiCardTimeCount;//多卡计时
TC3Cardgroup CardGroup[5];
}GCC_PACKED TC3Multicardopendoor,*PC3Multicardopendoor;

//门打开超时判断结构
typedef struct _dooropentimeover_{
	U32 DoorLockState;//门锁状态  //0为无门磁状态，1为关，2为开
	U32 Dooropentimelimit;//门打开限时时间 来自于goption
	U32 Dooropentime;//门打开计时
	U32 LockOpenTime;//驱动锁的时长
}GCC_PACKED TC3doorOpenTimeOver;


//门常开 同一张授权卡连续刷5次取消结构
typedef struct _canceldoorkeepopen_{
	U32 PIN;//用户识别号
	U32 PollCount;//刷卡次数
	U32 PollTime;//刷 卡计时
}GCC_PACKED TC3CancelDoorKeepOpen;


//出厂初始化
typedef struct _machine_init_{
	U32 status_now;//当前状态
	U32 status_pre;//上次状态
	U32 change_count;//初始化码盘变化次数
	U32 change_times;//初始化码盘时间
}GCC_PACKED  TC3machineinit;


typedef struct _POLLCARD_{
	U32 time_count[8];//刷卡间隔计时
	TC3User c3userinfo[8];//当前刷卡的卡号用户信息
	TC3authorize c3cardauthorizeinfo[8];
	TC3Inputpassword c3doorpassword[8];// 4个门对应的输入密码，作协迫密码，超级密码判断用
	TC3Cardandpassword c3cardandpasswordverify[8];//卡密码双重难证
	TC3doorkeepOpen c3doorkeepopen[4];//门常开控制参数
	TC3Multicardopendoor c3MultiCardOpendoor[4];//多卡开门组合
	TC3doorOpenTimeOver  DoorLockOpenTimeOver[4];//门打开超时判断
	U32 DoorOpenCause[4];//门打开原因，刷卡1，门常开2，首卡常开3,远程通迅4，联动5，意外为 0

	TC3machineinit machineintit;//
	TC3CancelDoorKeepOpen CancelDoorKeepOpen[4];//门常开取消刷卡结构
}GCC_PACKED Tpollcard, *Ppollcard;    //

#define NO_LIMIT        -1

typedef struct _fdoffset_
{
	int ContentType;
	off_t currpos;
}GCC_PACKED TFdOffset,*PFdOffset;

int FDB_AddAcessEvenLog(U32 cardno,U32 pin,BYTE verified,BYTE ReaderID,
	BYTE EventType,BYTE inoutstate,time_t time_second);

int FDB_GetC3UserByCard(int *card, PC3User user);
int FDB_GetC3UserByPwd(char * pwd, PC3User user);
int FDB_GetDoorAuthInfo(int pin,int DoorID);
int FDB_GetcardauthorizeInfo2(int pin,int DoorID,TTime time);
int FDB_GetTimezone(int timzoneid,TTime time,PC3Timeperiod Timeperiod);
int	GetTimefromtimezone(int timezoneID,TTime currtime, int *starttime,int *endtime);

int FDB_DelOldC3GuarderEventLog(int delCount);
int FDB_AddFdOffset(PFdOffset FdOffset);

int FDB_Getfirstcardopendoor(int pin,PC3Firstcardopendoor firstcardinfo);

//end
#define MAXRTLOGCOUNT 30
typedef struct _OVERSEERTLog_
{
	TC3AcessLog RTLog[MAXRTLOGCOUNT];
	U32 Logcount;//产生的实时记录条数
	U32 LogIndex;//当前取的实时记录索引，必须小于等于Logcount
	U32 LogWriteIndex;//保存记录指针
	U32 LogReadIndex;//读取记录指针
}GCC_PACKED TOverseeRTLog, *POverseeRTLog;

TOverseeRTLog Oversee_RTLog;

//共享内存，最小为一页。一页大小为4096. 用户通信的共享内存，暂申请4096。
#define MAX_SEND_DATA_LEN    1024*8
typedef struct _commdata_
{
	int CommSendMark;//如果被comm进程修改，就改为1，如果main读取完了，就改为0
	int MainSendMark;//如果被main进程修改，就改为1，如果comm读取完了，就改为0
	int Cmd;
	int NewsType;
	int MainCmdRet;
	int CommCmdRet;
	int CreateCommCmd;
	int CreateMainCmd;
	int SendLen;
	char SendData[MAX_SEND_DATA_LEN];
}GCC_PACKED TCommData, *PCommData;
TCommData g_CommData;

PFdOffset FDB_GetFdOffset(int ContentType, PFdOffset FdOffset);
int SaveFdOffset(int file_ID, int FdOffsetContentTpye);
int OpenSDCard(void);

#endif
