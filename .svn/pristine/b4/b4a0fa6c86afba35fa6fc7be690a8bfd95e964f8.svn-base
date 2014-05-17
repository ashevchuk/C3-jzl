#ifndef __C4I2C_H__
#define __C4I2C_H__

//C3不分主次MCU版本号, C3共2个MCU：MCU1、MCU2。C4共5个MCU：MAINMCU、MCU1、MCU2、MCU3、MCU4
#define MAINMCU		0x03   //C4主MCU地址
#define MCU1		0x7c   //C3、C4共用
#define MCU2		0x7d   //C3、C4共用
#define MCU3		0x7e
#define MCU4		0x7f

 //	0x01表示蜂鸣器，0x02表示红灯，0x03表示绿灯；
#define READERBEEP 0x01
#define READERREDLED 0x02
#define READERGREENLED 0x03
#define READEROKLED 0x04 //自动 从红色变为绿灯

/*
3）	按键声音许可控制指令（按键时是否允许蜂鸣器响）
 	指令代码：0x30
 	指令功能：用于控制单片机按键时是否响蜂鸣器。
 	发送数据：DATA[0]=0x00表示禁用；DATA[0]=0xff表示使能。
 	成功返回：应答码=0x00；无数据。
 	失败返回：应答码=错误代码。
4）	读按键声音许可状态指令
 	指令代码：0x31
 	指令功能：用于读取主任务单片机的按键声音许可状态。
 	发送数据：无数据。
 	成功返回：应答码=0x00；DATA[0]=0x00表示禁用；DATA[0]=0xff表示使能。
 	失败返回：应答码=错误代码。
5）	设置是否显示时钟圆点
 	指令代码：0x32
 	指令功能：用于设置MCU是否在LCD屏上显示RTC时钟的两个圆点。
 	发送数据：DATA[0]=0x00表示不显示时钟圆点；DATA[0]=0xff表示显示时钟圆点。
 	操作成功返回：应答码=0x00。
 	操作失败返回：应答码=错误代码。
6）	读取当前时钟圆点显示状态
 	指令代码：0x33
 	指令功能：
 	发送数据：无数据。
 	操作成功返回：应答码=0x00；DATA[0]= 0x00表示不显示时钟圆点；DATA[0]=0xff表示显示时钟圆点。
 	操作失败返回：应答码=错误代码。
7）	设置屏参数指令
 	指令代码：0x34
 	指令功能：用户设置屏的参数，如分辨率等（保留今后扩展升级用）
 	发送数据：
 	操作成功返回：应答码=0x00。
 	操作失败返回：应答码=错误代码。
8）	读屏参数指令
 	指令代码：0x35
 	指令功能：读屏型号和屏参数，屏信号和参数可以由MCU通过硬件电路自己识别（保留今后扩展升级用）
 	发送数据：无数据。
 	操作成功返回：应答码=0x00。
 	操作失败返回：应答码=错误代码。
9）	写LCD屏指令
 	指令代码：0x36
 	指令功能：
 	发送数据：DATA[0]=写类型，0x01表示16*8点阵，0x02表示8*16点阵（先写第一行的8*8，再写第二行的8*8），0x03表示32*32点阵（汉字）（先写第一行的8*16，再写第二行的8*16）；DATA[1]=地址，高四位为字符行地址，低四位为字符列地址。
 	操作成功返回：应答码=0x00；无数据。
 	操作失败返回：应答码=错误代码。
10）	设置开关信号的功能
 	指令代码：0x37
 	指令功能：
 	发送数据：DATA[0]= 开关编号（从1开始编号）；DATA[1]=开关的功能，0x00表示自定义，0x01表示出门开关。
 	操作成功返回：应答码=0x00；无数据。
 	操作失败返回：应答码=错误代码。
11）	设置输出继电器的功能
 	指令代码：0x39
 	指令功能：
 	发送数据：DATA[0]= 继电器编号（从1开始编号）；DATA[1]=继电器的功能，0x00表示自定义，0x01表示锁控继电器。
 	操作成功返回：应答码=0x00；无数据。
 	操作失败返回：应答码=错误代码。
*/


#define MCU_CMD_SETKEYBEEP	0x30	

#define MCU_CMD_CLOCK		0x32
#define MCU_CMD_GETCLOCK	0x33	

#define MCU_CMD_WRITELCD	0x36

#define MCU_CMD_SETBUTTON	0x37
#define MCU_CMD_SETLOCK	    0x39

#define MCU_CMD_LCD_LIGHT	0x3A   //关闭或打开LCD背光。

#define RCMD_LCD_SIZE 		0x3F

#define MCU_CMD_RESETMAINMCU	0x51
#define MCU_CMD_READERSET		0x52
#define MCU_CMD_RESETMCU		0x53

#define MCU_CMD_SETAUX		0x55
#define MCU_CMD_GETAUX		0x56

#define MCU_CMD_C4_READERSTATE	0x57
#define MCU_CMD_C3_READERSTATE	0x58

#define MCU_CMD_BEEP		0x59
#define MCU_CMD_DOG		0x5B
#define MCU_CMD_CLEARLCD	0x5D

#define MCU_CMD_VERSION 		0x80
#define MCU_CMD_INPUT			0x82

#define MCU_CMD_GETDOORNUM	0x8b	
#define MCU_CMD_GET485ADDR	0x8c	



int C4IIC_open();
void C4IIC_close();

int i2c_set_addr(unsigned int slave_addr);
int i2c_set_clk(unsigned int clk);
int ExComI2C(char Addr, char cmd, char *data, int DataLen);
int ExComI2CWait(char Addr, int *cmd, char *data);


int WriteC4IIC(char Address, char cmd, char *data, int size);
int ReadC4IIC(int Address, int *cmd, char *data);
int CheckC4State();


int ExInit(void);

int GetMCUVersion(char Addr,char *buf);
int Get485HardAddr(int *Addr);
int GetDoorNumByMcu(int *Num);
void ExBeepByMCU(int delay);	//delay 0:open  0xff:off 	else:open time 
void ExAuxOut(int AuxIndex, int OpenDelay);
void ExGetLCDSize(int *Width, int *Height, int *Data);
void ExClearLCD(void);
void ExPutPixelBuffer16H(int row, int col, unsigned char *Buffer);
//void ExPutPixelBuffer16(int row, int col, unsigned char *Buffer);
void ExPutPixelBuffer_32(int row, int col, unsigned char *Buffer);
//void ExPutPixelBuffer32(int row, int col, unsigned char *Buffer);

int ExSetState(int ioFunction,  int Addr, int ioIndex, int OpenDelay);		//OpenDelay 0x00琛ㄧず鍏筹紝0xff琛ㄧず, 鍏朵綑琛ㄧず寮�闀挎椂闂�
int ExGetState(int ioFunction, int Addr, int ioIndex);

int ExSetWiegand(int Addr, int bit);

#define IOCTL_READ_REG		0
#define IOCTL_WRITE_REG		1
#define IOCTL_SET_SLAVE_ID 	4
#define IOCTL_SET_CLK		5


#ifdef MACHINETYPE_C4
#define ERTC_WR_ADDR		0xD0	
#define ERTC_RD_ADDR		0xD1
#else
#define ERTC_WR_ADDR		0x64//	0xD0	
#define ERTC_RD_ADDR		0x65//	0xD1
#endif

#define ERTC_SECOND_REG		0x00	
#define ERTC_MINUTE_REG		0x01	
#define ERTC_HOUR_REG			0x02	
#define ERTC_DAYOFWEEK_REG	0x03	//Weekday 1-7
#define ERTC_DAY_REG			0x04	//01-31
#define ERTC_MONTH_REG			0x05	//01-12
#define ERTC_YEAR_REG			0x06	//00-99
//#define ERTC_CENTURY_REG		ERTC_MONTH_REG

#define CTRL_REG1		0x0F 		// 定义的控制寄存器1地址
#define CTRL_REG2		0x10 		// 定义的CLKOUT设置寄存器地址

#define ERTC_ALARM1_SECOND_REG	0x07
#define ERTC_ALARM1_MINUTE_REG	0x08
#define ERTC_ALARM1_HOUR_REG	0x09
#define ERTC_ALARM1_DATE_REG	0x0A


#define ERTC_ALARM2_MINUTE_REG	0x0B
#define ERTC_ALARM2_HOUR_REG	0x0C
#define ERTC_ALARM2_DATE_REG	0x0D

#define ERTC_CONTROL_REG		0x0E
#define ERTC_STATUS_REG			0x0F
#define ERTC_AGING_OFFSET		0x10

#define ERTC_TEMP_UPER_REG		0x11
#define ERTC_TEMP_LOWER_REG		0x12

#define Bcd2Dec(a)	((a>>4)*10+ (a&0x0F))
#define Dec2Bcd(a)	( ((a/10)<<4) | (a%10) )

#define ERTC_CENTURY_BIT		1<<7
#define ERTC_AM_PM_BIT			1<<5

/*DS3231可以运行于12小时或者24小时模
的第6位定义为12或24小时模式选择
选择12小时模式。在12小时模式下，第
示位，逻辑高时为PM。在24小时模式
个十位小时位 (20–23小时)。当年寄存
时，会转换世纪位 月寄存器的第 位 �
*/
#ifdef MACHINETYPE_C4
#define ERTC_12_24_BIT			1<<6
#else
#define ERTC_12_24_BIT			1<<7
#endif

#define ERTC_ALARM_DY_DT_BIT		1<<6
#define ERTC_ALARM_MASK_BIT		1<<7

#define ERTC_CR_A1IE_BIT		1
#define ERTC_CR_A2IE_BIT		1<<1
#define ERTC_CR_INTCN_BIT		1<<2
#define ERTC_CR_RS_BITS			3<<3
#define ERTC_CR_CONV_BIT		1<<5
#define ERTC_CR_BBSQW_BIT		1<<6
#define ERTC_CR_EOSC_BIT		1<<7

#define ERTC_SR_A1F_BIT			1
#define ERTC_SR_A2F_BIT			1<<1
#define ERTC_SR_BUSY_BIT		1<<2
#define ERTC_SR_EN32KHZ_BIT		1<<3
#define ERTC_SR_OSF_BIT			1<<7



#define	IO_OUTPUT	0        //输出点
#define 	IO_INPUT        1       //输入点
#define 	IO_BEEP     	2
#define 	IO_LED          3
#define 	IO_BUTTON	4
#define 	IO_AUX		5

#define OUTPUT 		0
#define INPUT		1


typedef enum
{
	eYEAR = 0,
	eMONTH,
	eDAYOFWEEK,
	eDAY,
	eHOUR,
	eMINUTE,
	eSECOND
} DATEANDTIME, *PDATEANDTIME;


typedef struct _APPTIME_ {
	U16 year;
	U8 month;
	U8 day;
	U8 hour;
	U8 minute;
	U8 second;
	U8 weekday;
} APPTIME;


typedef struct
{
	unsigned char reg;
	unsigned char val;
} S_CFG;

int GetExternelRTC(TTime *time);
int SetExternelRTC(TTime *tm);
void IIC_ConctrolLock(int MachineType,int index,int time);
void Ex_ConctrolAlarm(int MachineType,int index, int time);
void EX_ConctrolAuxAlarm(int index, int time);//控制扩展报警 ，仅仅在 c4上用 于out 9 out 10
void EX_ConctrolReaderLedorBeep(int MachineType,int ReaderID,int type,int time);
int ExBeep(int Enable);
int ExResetMainMCU();
//int ExMcuDog(int delay);
int ExMcuDog(int index,int delay);
int ExEnableClock(int Enable);


int CheckSDCard();

#endif
