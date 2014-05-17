#include <stdlib.h>
#include <stdio.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include "exfun.h"
#include "c4i2c.h"
#include "arca.h"
#include "gpio.h"
#include "options.h"

#define CMD_QUERY       0x03
#define CMD_QUERY_REQ   0x05

#define IOCTL_READ_C4		6
#define IOCTL_WRITE_C4		7

static int gLCDData=0;
static int gLCDWidth=0;
static int gLCDHeight=0;

typedef struct
{
	U8 len;
	U8 offset;
	U8 buf[256];
} EEPROM;

static int dev_iic_fd=-1;

void C4IIC_close()
{
	close(dev_iic_fd);
	dev_iic_fd=-1;
}

int C4IIC_open()
{
	if(dev_iic_fd>0) return dev_iic_fd;
	dev_iic_fd = open("/dev/sensor", O_RDWR);
	if (dev_iic_fd < 0)
	{
		DBPRINTF("Error opening /dev/sensor\n");
		return 0;
	}

	if(i2c_set_clk(100*1000)<0)
	{
		DBPRINTF(">>>mcdebug:\twarning: i2c_set_clk failed!\n");
		return 0;
	}

	return dev_iic_fd;
}

int WriteC4IIC(char Address, char cmd, char *data, int size)
{
	int ret;
	U32 s=0, i;
	EEPROM rom;
	static U8 C4PackBuf[256];

	if(!C4IIC_open()) return 0;


#ifndef MACHINETYPE_C4
	if(Address== MAINMCU)
		return 1;
#endif


	memset(&rom, 0, sizeof(EEPROM));
	rom.len=size+6;
	rom.offset=Address;

	memset(C4PackBuf, 0, 256);
	C4PackBuf[0]=0xAA;
	C4PackBuf[1]=size;
	C4PackBuf[2]=~size;
	C4PackBuf[3]=Address;
	C4PackBuf[4]=cmd;
	memcpy(C4PackBuf+5, data, size);

	s=Address+cmd;
//	DBPRINTF("WriteIIC  Address = %d,cmd = %d\n", Address, cmd&0xFF);
	for (i=0; i<size; i++)
	{
		s+=(data[i]&0xFF);
		//DBPRINTF("%d,", data[i]&0xFF);
	}
	C4PackBuf[size+5]=(s & 0xFF);
	//DBPRINTF("\nwrite check s=%d, %d\n", s, (s&0xFF));

	memcpy(rom.buf, C4PackBuf, rom.len);
	ret=ioctl(dev_iic_fd, IOCTL_WRITE_C4, &rom);
//	DBPRINTF("WriteC4IIC...dev_iic_fd=%d, Address=%d, cmd =%d, ret=%d\n", dev_iic_fd, Address, cmd, ret);
	return ret;
}

int ReadC4IIC(int Address, int *cmd, char *data)
{
	U32 s=0,  i;
	EEPROM rom;
	int len=0, offset=0, addr=0, command=0;

	if(!C4IIC_open())
	{
		return 0;
	}


#ifndef MACHINETYPE_C4
	if(Address== MAINMCU)
		return 1;
#endif


//	DBPRINTF("ReadIIC  Address = %d \n", Address);

	memset(&rom, 0, sizeof(EEPROM));
	rom.offset=Address;
//	rom.offset=(Address >> 1);
	if(ioctl(dev_iic_fd, IOCTL_READ_C4, &rom)==0)
	{
//		DBPRINTF("read EFROM rom.len=%d ret=%d..\n", rom.len, rom.buf[4]);
		if (rom.buf[0] != 0xAA)
		{
			return -1;
		}

		len=rom.buf[1];
		offset=0xFF-rom.buf[2];
		addr=rom.buf[3];
		command=rom.buf[4];

		if (len != offset)
		{
			return -2;
		}

		if (addr != Address)
		{
			return -3;
		}

		if (len)
		{
			for (i=0; i<len; i++)
			{
				s+=(rom.buf[i+5]&0xFF);
			}
		}

		s += addr + command;
		//DBPRINTF("%d, %d\n", rom.buf[len+5], s);
		if (rom.buf[len+5] != (s & 0xFF))
		{
			return -4;
		}

		*cmd=command;
		if (len)
		{
			memcpy(data, rom.buf+5, len);
		}

		return len;
	}
	else
	{
		return -6;
	}
}

int CheckC4State()
{
	int i=200,c=0;
	while(--i)
	{
		//DBPRINTF("__gpio_get_pin=%d\n", __gpio_get_pin(IO_C4_STATE));
		if(!__gpio_get_pin(IO_C4_STATE))
		{
			if(++c>20) return FALSE;
		}
		DelayUS(5);
	}
	return TRUE;
}

int CheckSDCard()
{
	int i=200,c=0;
	while(--i)
	{
		if(!__gpio_get_pin(SDCARD_CHECK))
		{
			if(++c>20) return TRUE;
		}
		DelayUS(5);
	}
	return FALSE;
}

int ExComI2C(char Addr, char cmd, char *data, int DataLen)
{
	return WriteC4IIC(Addr, cmd, data, DataLen);
}

int ExComI2CWait(char Addr, int *cmd, char *data)
{
	int ret=ReadC4IIC(Addr, cmd, data);
	//DBPRINTF("I2CWait ret=%d\n", ret);
	return ret;
}

//如果无法猎取ver，返回0
int GetMCUVersion(char Addr,char *buf)
{

	int i=3, cmd = 0;
	while(i--)
	{
		buf[0]=0;
		ExComI2C(Addr, MCU_CMD_VERSION, 0, 0);

		if (ExComI2CWait(Addr, &cmd, buf)>=0)
		{
			if(CMD_OK == cmd)
			{
				return 1;
			}
			else
			{
				printf("rev MCU_CMD_VERSION == cmd Addr: %d cmd: %d\n",Addr,cmd);
			}
		}
	}

	return 0;
}

int Get485HardAddr(int *Addr)
{
	int i=5, cmd;
	char buf[5];
	while(i--)
	{
		buf[0]=0;
		ExComI2C(MCU1, MCU_CMD_GET485ADDR, 0, 0);
		if (ExComI2CWait(MCU1, &cmd, buf)>=0)
		{
			*Addr = buf[0];
			printf("Get485HardAddr cmd=%d,Addr=%d,\n",cmd,*Addr);
			if(CMD_OK == cmd)
			{
				return 1;
			}
		}
	}
	return 0;
}

int GetDoorNumByMcu(int *Num)
{
	int i=5, cmd;
	char buf[5];
	while(i--)
	{
		buf[0]=0;
		ExComI2C(MCU1, MCU_CMD_GETDOORNUM, 0, 0);
		if (ExComI2CWait(MCU1, &cmd, buf)>=0)
		{
			*Num = buf[0];
			printf("GetDoorNumByMcu cmd=%d,Num=%d,\n",cmd,*Num);
			if(CMD_OK == cmd)
			{
				return 1;
			}
		}
	}
	return 0;
}



void ExBeepByMCU(int delay)	//delay 0:open  0xff:off 	else:open time
{
	U8 data[1];
	data[0]=delay;
	ExComI2C(MAINMCU, MCU_CMD_BEEP, data, 1);
}

void ExGetLCDSize(int *Width, int *Height, int *Data)
{
	int c=5;
	char buf[20];
	*Width=128;
	*Height=64;
	while(c--)
	{
		ExComI2C(MAINMCU, RCMD_LCD_SIZE, 0, 0);
		//if (ExComI2CWait(MAINMCU, &cmd, buf)>=0) break;
	}
	if(c>0)
	{
		*Height=((BYTE*)buf)[0];
		*Width=((BYTE*)buf)[1];
		*Data=((BYTE*)buf)[2]|((BYTE*)buf)[3]<<8|((BYTE*)buf)[4]<<16|((BYTE*)buf)[5]<<24;
	}
}

void ExClearLCD(void)
{
	char data[128];
	int s=3, cmd;
	while(s--)
	{
		ExComI2C(MAINMCU, MCU_CMD_CLEARLCD, 0, 0);
		DBPRINTF("ExClearLCD....\n");
		if (ExComI2CWait(MAINMCU, &cmd, data) >=0)
		{
			if(CMD_OK == cmd)
			{
				break;
			}
		}
	}
}
/*
	写LCD屏指令
 	指令代码：0x36
 	指令功能：
 	发送数据：DATA[0]=写类型，0x01表示16*8点阵，0x02表示8*16点阵（先写第一行的8*8，再写第二行的8*8），
 	0x03表示32*32点阵（汉字）（先写第一行的8*16，再写第二行的8*16）；
 	DATA[1]=地址，高四位为字符行地址，低四位为字符列地址。
 	操作成功返回：应答码=0x00；无数据。
 	操作失败返回：应答码=错误代码。
*/

void ExPutPixelBuffer16H(int row, int col, unsigned char *Buffer)
{
	char Buf[20], data[128];
	int i, s=3, cmd;
	if(row>7) return;
	if(col>15) return;
	Buf[2]=0x01;
	Buf[3]=col+row*16;
	for(i=0;i<4;i++)
		((U32*)(Buf+4))[i]=((U32*)Buffer)[i];

	while (s--)
	{
		ExComI2C(MAINMCU, MCU_CMD_WRITELCD, Buf+2, 18);
		if (ExComI2CWait(MAINMCU, &cmd, data) >=0)
		{
			if(CMD_OK == cmd)
			{
				break;
			}
		}
	}
}

void ExPutPixelBuffer16(int row, int col, unsigned char *Buffer)
{
	char Buf[24], data[128];

//	DBPRINTF("---------------ExPutPixelBuffer16\n");
	int i, s=3, cmd;
	if(row>7) return;
	if(col>15) return;
	Buf[2]=0x02;
	Buf[3]=col+row*16;
	for(i=0;i<4;i++)
		((U32*)(Buf+4))[i]=((U32*)Buffer)[i];
	while (s--)
	{
		ExComI2C(MAINMCU, MCU_CMD_WRITELCD, Buf+2, 18);
		if (ExComI2CWait(MAINMCU, &cmd, data) >=0)
		{
			if(CMD_OK == cmd)
			{
				break;
			}
		}
	}
}

void ExPutPixelBuffer_32(int row, int col, unsigned char *Buffer)
{
	char Buf[40], data[128];
	int i, s=3, cmd;

//	DBPRINTF("--------------ExPutPixelBuffer_32\n");
	Buf[2]=0x03;
	Buf[3]=col+row*16;
	for(i=0;i<8;i++)
		((U32*)(Buf+4))[i]=((U32*)Buffer)[i];
	while (s--)
	{
		ExComI2C(MAINMCU, MCU_CMD_WRITELCD, Buf+2, 34);
		if (ExComI2CWait(MAINMCU, &cmd, data) >=0)
		{
			if(CMD_OK == cmd)
			{
				break;
			}
		}
	}
}

void ExPutPixelBuffer32(int row, int col, unsigned char *Buffer)
{
	if(row>7) return;
	if(col>15) return;

//	DBPRINTF("--------------ExPutPixelBuffer32\n");
	if(col==7)
	{
		unsigned char Buf[16];
		memcpy(Buf, Buffer+8, 8);
		memcpy(Buffer+8, Buffer+16, 8);
		memcpy(Buf+8, Buffer+24, 8);
		ExPutPixelBuffer16(row*2, col, Buffer);
		ExPutPixelBuffer16(row*2, col+1, Buf);
	}
	else
	{
		ExPutPixelBuffer_32(row*2, col, Buffer);
	}
}

int ExInit(void)
{
	int MCUVersion=0;
	char buf[100];

	gLCDWidth=128;
	gLCDHeight=64;
//#ifdef PC
//#else
	MCUVersion=GetMCUVersion(MAINMCU,buf);
	DBPRINTF("MCUVersion:%s\n",buf);
	ExGetLCDSize(&gLCDWidth, &gLCDHeight, &gLCDData);


	DBPRINTF("gLCDWidth=%d, gLCDHeight=%d, gLCDData=%d\n",gLCDWidth, gLCDHeight, gLCDData);

//#endif
	return 0;
}

int ExGetState(int ioFunction, int Addr, int ioIndex)
{
	int i=3, cmd=-1;
	char buf[2]={0};
//	DBPRINTF("ExGetState Addr=%d\n", Addr);
	while(i--)
	{
		buf[0]=ioIndex;
		if (ioFunction == OUTPUT)
		{
			ExComI2C(Addr, MCU_CMD_GETAUX, buf, 1);
		}
		else if (ioFunction == INPUT)
		{
			ExComI2C(Addr, MCU_CMD_INPUT, buf, 1);
		}
		if (ExComI2CWait(Addr, &cmd, buf)>=0)
		{
			if ((cmd==CMD_OK) && (buf[0]==ioIndex))
			{
				return buf[1];
			}
		}
	}
	return 0;
}


int ExSetState(int ioFunction,  int Addr, int ioIndex, int OpenDelay)		//OpenDelay 0x00琛ㄧず鍏筹紝0xff琛ㄧず, 鍏朵綑琛ㄧず寮�闀挎椂闂�
{
	char data[2];
	int i=3, cmd=-1;
	data[0]=ioIndex;
	data[1]=OpenDelay;


	DBPRINTF("ExSetState fun = %d,mcuaddr =%d,index = %d,opendelay %d \n",
		ioFunction,   (Addr >=MCU1)? (Addr - MCU1+1) :(Addr - MAINMCU),ioIndex,  OpenDelay);
	while(i--)
	{
		if (ioFunction==IO_BUTTON)
		{
			data[0] = ioIndex%2;
			data[1] = OpenDelay;
			ExComI2C(Addr, MCU_CMD_SETBUTTON, data, 2);
		}
		else if (ioFunction==IO_AUX)
		{
			data[0] = ioIndex%2;
			data[1] = OpenDelay;
			ExComI2C(Addr, MCU_CMD_SETLOCK, data, 2);
		}
		else if (ioFunction==IO_OUTPUT)
		{
//			DBPRINTF("ExSetState addr=%d\n", Addr);
			ExComI2C(Addr, MCU_CMD_SETAUX, data, 2);
		}
		else if (ioFunction == IO_INPUT)
		{
			ExComI2C(Addr, MCU_CMD_INPUT, data, 2);
		}
		else if (ioFunction == IO_BEEP)
		{
			if (OpenDelay) data[0] = 0xFF; else data[0] = 0x0;
			ExComI2C(Addr, MCU_CMD_SETKEYBEEP, data, 1);
		}
		else if (ioFunction == IO_LED)
		{
			ExComI2C(Addr, MCU_CMD_READERSET, data, 2);
		}

//		DBPRINTF("send over! get response\n");
		if (ExComI2CWait(Addr, &cmd, data)>=0)
		{
//			DBPRINTF("ExSetState cmd=%d\n", cmd);
			if (cmd==CMD_OK) break;
		}
		else
		{
			DBPRINTF("no response....\n");
		}
	}
	return cmd;
}

int ExSetAux(int addr, int index, int state)
{
	int i=3, cmd=-1;
	char data[2];

	data[0] = index%2;
	data[1] = state;

	while(i--)
	{
		ExComI2C(addr, MCU_CMD_SETAUX, data, 2);
		if (ExComI2CWait(addr, &cmd, data)>=0)
		{
			if (cmd == CMD_OK) break;
			else DBPRINTF("ExSetAux  defail=%d\n", cmd);
		}
		else
			DBPRINTF("ExComI2CWait defail\n");

	}
	return cmd;
}

//DATA[0]=继电器编号（从1开始编号）；DATA[1]=继电器的动作，0x00表示关，0xff表示开，其余表示开多长时间（单位秒），最大为 254秒。
//其中编号为1和2的继电器固分别用来控制两路锁；编号3的继电器为扩展输出继电器，可自定义其作用
//如果是c3_200，index只能为,1,2
//如果是c3_400  c4 index可为,1,2,3,4
void IIC_ConctrolLock(int MachineType,int index,int time)
{
	if(MachineType == C3_200 || MachineType == C3_100 || MachineType == C3_400To_200)
		ExSetState(IO_OUTPUT, MCU1 + index  -1, 1, time);
	else if(MachineType == C3_400)
		ExSetState(IO_OUTPUT, MCU1 + ( (index > 2)? 1 :0),(index+1)%2+1, time);
	else if(MachineType == C4)
	{
		if(index <=4)
			ExSetState(IO_OUTPUT, MCU1 + index  -1, 1, time);
		//else if(index == 5)
			//ExSetState(IO_OUTPUT, MAINMCU, 2, time);
		//else if(index == 6)
			//ExSetState(IO_OUTPUT, MAINMCU, 1, time);
	}
	else if(MachineType == C4_200)
	{
		if(index <=2)
			ExSetState(IO_OUTPUT, MCU1 + index  -1, 1, time);
	}
	else if(MachineType == C4_400To_200)
		{
			if(index == 1)
				ExSetState(IO_OUTPUT, MCU1 , 1, time);
			else if(index == 2)
				ExSetState(IO_OUTPUT, MCU3 , 1, time);
		}
}

void Ex_ConctrolAlarm(int MachineType,int index, int time)
{
	if(MachineType == C3_200 || MachineType == C3_100)
		ExSetState(IO_OUTPUT, MCU1 + index  -1, 3, time);
	else if(MachineType == C3_400 || MachineType == C3_400To_200)
		ExSetState(IO_OUTPUT, MCU1 + ( (index > 2)? 1 : 0),(index+1)%2+3, time);
	else if(MachineType == C4|| MachineType == C4_400To_200)
	{
		if(index <=8)
			ExSetState(IO_OUTPUT, MCU1 + index/2  -1, 2, time);
		else if(index == 9)
			ExSetState(IO_OUTPUT, MAINMCU, 1, time);
		else if(index == 10)
			ExSetState(IO_OUTPUT, MAINMCU, 2, time);
	}
	else if(MachineType == C4_200)
		{
			if(index <=4)
				ExSetState(IO_OUTPUT, MCU1 + index  -1, 2, time);
			else if(index == 9)
				ExSetState(IO_OUTPUT, MAINMCU, 1, time);
			else if(index == 10)
				ExSetState(IO_OUTPUT, MAINMCU, 2, time);
		}
}

void EX_ConctrolAuxAlarm(int index, int time)//控制扩展报警 ，仅仅在 c4上用 于out 9 out 10
{
	ExSetState(IO_OUTPUT, MAINMCU, index, time);
}

int ExSetC3Reader(int addr,int ReaderID,  int type, int time)
{
	int i=3, cmd=-1;
	char data[3];

	data[0] = ReaderID;
	data[1] = type;
	data[2] = time;

	while(i--)
	{
		ExComI2C(addr, MCU_CMD_C3_READERSTATE, data, 3);
		if (ExComI2CWait(addr, &cmd, data)>=0)
		{
			if (cmd == CMD_OK) break;
			else DBPRINTF("ExSetC3Reader  defail=%d\n", cmd);
		}
		else
			DBPRINTF("ExSetC3Reader defail\n");
	}
	return cmd;
}

int ExSetC4Reader(int addr, int type, int time)
{
	int i=3, cmd=-1;
	char data[3];

	data[0] = type;
	data[1] = time;

	while(i--)
	{
		ExComI2C(addr, MCU_CMD_C4_READERSTATE, data, 2);
		if (ExComI2CWait(addr, &cmd, data)>=0)
		{
			if (cmd == CMD_OK) break;
			else DBPRINTF("ExSetC4Reader  defail=%d\n", cmd);
		}
		else
			DBPRINTF("ExSetC4Reader defail\n");
	}
	return cmd;
}


//type:0x01表示蜂鸣器，0x02表示红灯，0x03表示绿灯；0x04 表示 原来为红灯，现在改为绿灯
//0x00表示关，0xff表示开，其余值表示开多长时间（10ms为单位
void EX_ConctrolReaderLedorBeep(int MachineType,int ReaderID,int type,int time)
{
	printf("ConctrolReaderLedorBeep machineType=%d,ReadID=%d,type=%d,time=%d\n",MachineType,ReaderID, type,time);

	if(MachineType == C3_200 || MachineType == C3_400  || MachineType == C3_100 || MachineType == C3_400To_200)
		ExSetC3Reader(MCU1 + ( (ReaderID > 2)? 1 :0), (ReaderID > 2)? (ReaderID-2 ):ReaderID,   type,  time);
	else if(MachineType == C4 || MachineType == C4_200 || MachineType == C4_400To_200)
		ExSetC4Reader(MCU1 + ReaderID -1,  type,  time);
}

int ExBeep(int Enable)
{
	int i=3, cmd=-1;
	char data[1];
	if (Enable) data[0] = 0xFF; else data[0] = 0x00;

	while(i--)
	{
		ExComI2C(MAINMCU, MCU_CMD_BEEP, data, 1);
		if (ExComI2CWait(MAINMCU, &cmd, data)>=0)
			if (cmd==CMD_OK) break;
	}
	return cmd;
}
/*
int ExMcuDog(int delay)
{
	int i=3, cmd=-1;
	char data[1];

	data[0]=delay;
	while(i--)
	{
		ExComI2C(MAINMCU, MCU_CMD_DOG, data, 1);
		if (ExComI2CWait(MAINMCU, &cmd, data)>=0)
			if (cmd==CMD_OK) break;
	}
	return cmd;
}
*/
int ExMcuDog(int index,int delay)
{
	int i=3, cmd=-1;
	char data[1];

	data[0]=delay;
	while(i--)
	{
		ExComI2C(index, MCU_CMD_DOG, data, 1);
		if (ExComI2CWait(index, &cmd, data)>=0)
			if (cmd==CMD_OK) break;
	}
	return cmd;
}


int ExResetMainMCU()
{
	int i=3, cmd=-1;
	char data[1];

	while(i--)
	{
		ExComI2C(MAINMCU, MCU_CMD_RESETMAINMCU, 0, 0);
		if (ExComI2CWait(MAINMCU, &cmd, data)>=0)
			if (cmd==CMD_OK) break;
	}
	return cmd;
}

int ExResetMCU()
{
	int i=3, cmd=-1;
	char data[1];

	while(i--)
	{
		ExComI2C(MAINMCU, MCU_CMD_RESETMCU, 0, 0);
		if (ExComI2CWait(MAINMCU, &cmd, data)>=0)
			if (cmd==CMD_OK) break;
	}
	return cmd;
}





int ExSetButton(int addr, int index, int state)
{
	int i=3, cmd=-1;
	char data[2];

	data[0] = index%2;
	data[1] = state;

	while(i--)
	{
		ExComI2C(addr, MCU_CMD_SETBUTTON, data, 2);
		if (ExComI2CWait(MAINMCU, &cmd, data)>=0)
		{
			if (cmd == CMD_OK) break;
			else DBPRINTF("ExSetButton defail=%d\n", cmd);
		}
		else
			 DBPRINTF("ExComI2CWait defail\n");
	}
	return cmd;
}


int ExEnableClock(int Enable)
{
	int i=3, cmd=-1;
	char data[1];

	data[0] = Enable;
	while(i--)
	{
		ExComI2C(MAINMCU, MCU_CMD_CLOCK, data, 2);
		if (ExComI2CWait(MAINMCU, &cmd, data)>=0)
		{
			if (cmd == CMD_OK) break;
				else DBPRINTF("ExEnableClock  defail=%d\n", cmd);
		}
		else
			 DBPRINTF("ExComI2CWait defail\n");

	}
	return cmd;
}

//控制LCD背光，Enable为0X00，关，Enable为0XFF,开。
int ExEnableLCDLight(int Enable)
{
	int i=3, cmd=-1;
	char data[1];

	data[0] = Enable;
	while(i--)
	{
		ExComI2C(MAINMCU, MCU_CMD_LCD_LIGHT, data, 2);
		if (ExComI2CWait(MAINMCU, &cmd, data)>=0)
		{
			if (cmd == CMD_OK)
			{
				break;
			}
			else
			{
				DBPRINTF("ExEnableLCDLight  defail=%d\n", cmd);
			}
		}
		else
		{
			 DBPRINTF("ExComI2CWait defail\n");
		}

	}
	return cmd;
}

/***********
 * Function  for DateTime
*************/
int i2c_set_addr(unsigned int slave_addr)
{

//	printf("i2c_set_addr = 0x%02x\n",slave_addr);
	if (ioctl(dev_iic_fd, IOCTL_SET_SLAVE_ID, (unsigned int)&slave_addr) < 0)
	{
		return -1;
	}
	return 0;
}

int i2c_set_clk(unsigned int clk)
{
	if (ioctl(dev_iic_fd, IOCTL_SET_CLK, (unsigned int)&clk) < 0)
	{
		return -1;
	}
	return 0;
}

int i2c_readtime(int slave_addr, int reg, unsigned char* value)
{
	S_CFG s;

	if(!C4IIC_open())
	{
		DBPRINTF(">>>mcdebug:\twarning: C4IIC_open failed!\n");
		return 0;
	}

	if(i2c_set_addr(slave_addr)<0)
	{
		DBPRINTF(">>>mcdebug:\twarning: i2c_set_addr failed!\n");
		return 0;
	}

	s.reg = reg;
	s.val = 0;
	if (ioctl(dev_iic_fd, IOCTL_READ_REG, (unsigned long)&s) < 0)
	{
		DBPRINTF(">>>mcdebug:\twarning: read key char failed!\n");
		return 0;
	}
	*value = s.val;
	return 1;
}

int i2c_writetime(int slave_addr, int reg, unsigned char value)
{
	S_CFG s;

	if(!C4IIC_open())
	{
		DBPRINTF(">>>mcdebug:\twarning: C4IIC_open failed!\n");
		return 0;
	}

	if(i2c_set_addr(slave_addr)<0)
	{
		DBPRINTF(">>>mcdebug:\twarning: i2c_set_addr failed!\n");
		return 0;
	}
	s.reg = reg;
	s.val = value;
	if (ioctl(dev_iic_fd, IOCTL_WRITE_REG, (unsigned long)&s) < 0)
	{
		DBPRINTF(">>>mcdebug:\twarning: write key char failed!\n");
		return 0;
	}
	return 1;
}

char *str_dateandtime[] ={"year","month","weekday","day","hour","minute","second"};

#ifdef MACHINETYPE_C4
unsigned short GetDateOrTime(DATEANDTIME dt, int reg)
{
	unsigned char value1;

	if(!i2c_readtime(ERTC_RD_ADDR, reg, &value1))
	{
		DBPRINTF(">>>mcdebug:\tDs3231 get date or time fail\n");
		return 0;
	}

	switch(dt)
	{
		case eYEAR:
			break;
		case eMONTH:
			value1 &= 0x1F;
			break;
		case  eDAYOFWEEK:
			value1 &= 0x07;
			return value1-1;//c4的rtc 的weekday为1~7
		case eDAY:
			value1 &= 0x3F;
			break;
		case eHOUR:
			if(value1 & ERTC_12_24_BIT)
			{
				if(value1 & ERTC_AM_PM_BIT)
					value1 &= 0x1F;
				else
					value1 &= 0x0F;
			}
			else
				value1 &= 0x3F;
			break;
		case eMINUTE:
			value1 &= 0x7F;
			break;
		case eSECOND:
			value1 &= 0x7F;
			break;
		default:
			DBPRINTF(">>>mcdebug:\tds3231 unsupport parameter\n");
			break;
	}

	return Bcd2Dec(value1);
}

#else
unsigned short GetDateOrTime(DATEANDTIME dt, int reg)
{
	unsigned char value1,value2;

	if(!i2c_readtime(ERTC_RD_ADDR, reg, &value1))
	{
		DBPRINTF(">>>mcdebug:\tDs3231 get date or time fail\n");
		return 0;
	}

	switch(dt)
	{
		case eYEAR:
			break;
		case eMONTH:
			value1 &= 0x1F;
			break;
		case  eDAYOFWEEK:
			value1 &= 0x07;
//			return value1;
		case eDAY:
			value1 &= 0x3F;
			break;
		case eHOUR:
			DBPRINTF(">>>mcdebug:eHOUR  = %d\n",value1);
			if(value1 & ERTC_12_24_BIT)
			{
				value1 &= 0x3F;
				return Bcd2Dec(value1);
			}
			else
			{
				value2 = value1;
				if(value2 & ERTC_AM_PM_BIT)
				{
					value1 = value2 & 0x1F;
					return Bcd2Dec(value1)+ 12;
				}
				else
				{
					value1 = value2 & 0x1F;
					return Bcd2Dec(value1);
				}

			}
			break;
		case eMINUTE:
			value1 &= 0x7F;
			break;
		case eSECOND:
			value1 &= 0x7F;
			break;
		default:
			DBPRINTF(">>>mcdebug:\tds3231 unsupport parameter\n");
			break;
	}

	return Bcd2Dec(value1);
}

#endif


int GetExternelRTC(TTime *tm)
{
	TTime time;

	memset(&time, 0,sizeof(TTime));
	time.tm_sec = GetDateOrTime(eSECOND,ERTC_SECOND_REG);
	time.tm_min = GetDateOrTime(eMINUTE,ERTC_MINUTE_REG);
	time.tm_hour = GetDateOrTime(eHOUR,ERTC_HOUR_REG);
	time.tm_wday = GetDateOrTime(eDAYOFWEEK,ERTC_DAYOFWEEK_REG);
	time.tm_mday = GetDateOrTime(eDAY,ERTC_DAY_REG);
	time.tm_mon = GetDateOrTime(eMONTH,ERTC_MONTH_REG);
	time.tm_year = GetDateOrTime(eYEAR,ERTC_YEAR_REG);
	DBPRINTF("Rtc's time:-%04d-%02d-%02d %02d:%02d:%02d   ---%2d \n",
		time.tm_year+2000,time.tm_mon,time.tm_mday,time.tm_hour,time.tm_min,time.tm_sec,time.tm_wday);
	//linux时间中，月份为0~11,年为从1900开始
	time.tm_mon -=1;
	time.tm_year = (time.tm_year +2000)-1900;
	memcpy(tm, &time, sizeof(TTime));

	return 0;
}

BOOL IsLeapYear(int year)
{
	if (year%4 == 0) return TRUE;
	return FALSE;
}


//判断linux时间值正常不
int IsTimeStructureValid(TTime* tm)
{
	int mon_days;

	if(tm == NULL)
	{
		DBPRINTF(">>>mcdebug:\tertc\tInvalid Argument\n");
		return 0;
	}

	if(tm->tm_year>(2099 -1900))
	{
		DBPRINTF(">>>mcdebug:\tertc\tInvalid year:%d\n",tm->tm_year);
		return 0;
	}

	if ((tm->tm_mon<0) || (tm->tm_mon>11))
	{
		DBPRINTF(">>>mcdebug:\tertc\tInvalid month:%d\n",tm->tm_mon);
		return 0;
	}

	if( (tm->tm_wday<0) || (tm->tm_wday>6) )
	{
		DBPRINTF(">>>mcdebug:\tertc\tInvalid weekday:%d\n",tm->tm_wday);
		return 0;
	}

	if( (tm->tm_hour<0) || (tm->tm_hour>23) )
	{
		DBPRINTF(">>>mcdebug:\tertc\tInvalid hour:%d\n",tm->tm_hour);
		return 0;
	}

	if( (tm->tm_min<0) || (tm->tm_min>59) )
	{
		DBPRINTF(">>>mcdebug:\tertc\tInvalid minute:%d\n",tm->tm_min);
		return 0;
	}

	if( (tm->tm_sec<0) || (tm->tm_sec>59) )
	{
		DBPRINTF(">>>mcdebug:\tertc\tInvalid second:%d\n",tm->tm_sec);
		return 0;
	}
//linux月是实际月减1 // 1,3 5 7 8 10 12    46911
//	if ((tm->tm_mon==4)||(tm->tm_mon==6)||(tm->tm_mon==9)||(tm->tm_mon==11))
	if ((tm->tm_mon==3)||(tm->tm_mon==5)||(tm->tm_mon==8)||(tm->tm_mon==10))
		mon_days = 30;
	else if( tm->tm_mon==1)//linux月是实际月减1
		mon_days = (IsLeapYear(tm->tm_year)) ? 29:28;
	else mon_days = 31;

	if(tm->tm_mday>mon_days)
	{
		DBPRINTF(">>>mcdebug:\tertc\tInvalid days:%d\n",tm->tm_mday);
		DBPRINTF("tm->tm_mon = %d,mon_days=%d\n",tm->tm_mon,mon_days);
		return 0;
	}
	return 1;
}

//设置外部rtc时间  月份要加1，年份要加1900
int SetExternelRTC(TTime *tm)
{
	unsigned char value1,value2;

	char log_string[50];

	memset(log_string,0x00,50);
	sprintf(log_string,"settime %d-%d-%d,%d/%d/%d  - %d\n",tm->tm_year,tm->tm_mon,tm->tm_mday,tm->tm_hour,tm->tm_min,tm->tm_sec,tm->tm_wday);

	FDB_AddOPLog(log_string,strlen(log_string));

	DBPRINTF(">>>SetExternelRTC\n");
	if(!IsTimeStructureValid(tm)) return 0;

	value1 = tm->tm_mon+1;
	value2 = tm->tm_year+ 1900 - 2000;

#ifndef MACHINETYPE_C4
//修改时间值前，先改写控制寄存器，仅用于sd2403
//置寄存器CTRL_REG2 的 WRTC1=1
	if (!i2c_writetime(ERTC_WR_ADDR, CTRL_REG2, 0x80))	 return 0;
//置寄存器CTRL_REG1 的 置WRTC2,WRTC3=1
	if (!i2c_writetime(ERTC_WR_ADDR, CTRL_REG1, 0x84))	 return 0;
#endif

//	DBPRINTF(">>>mcdebug:\tSetExternelRTC year=%02d\n",tm->tm_year+ 1900 - 2000);
//	if (!i2c_writetime(ERTC_WR_ADDR, ERTC_YEAR_REG, Dec2Bcd(tm->tm_year+ 1900 - 2000)))	 return 0;
	DBPRINTF(">>>mcdebug:\tSetExternelRTC second=%02d\n",tm->tm_sec);
	if (!i2c_writetime(ERTC_WR_ADDR, ERTC_SECOND_REG, Dec2Bcd(tm->tm_sec))) return 0;

	DBPRINTF(">>>mcdebug:\tSetExternelRTC minute=%02d\n",tm->tm_min);
	if (!i2c_writetime(ERTC_WR_ADDR, ERTC_MINUTE_REG, Dec2Bcd(tm->tm_min))) return 0;

	DBPRINTF(">>>mcdebug:\tSetExternelRTC hour=%02d\n",tm->tm_hour);
	if (!i2c_writetime(ERTC_WR_ADDR, ERTC_HOUR_REG, (Dec2Bcd(tm->tm_hour) | 0x80)  ))
		return 0;

#ifndef MACHINETYPE_C4
	DBPRINTF(">>>mcdebug:\t   c3 SetExternelRTC weekday=%02d\n",tm->tm_wday);//c3的rtc为0~6,pc送下来的也为0~6
	if (!i2c_writetime(ERTC_WR_ADDR, ERTC_DAYOFWEEK_REG, Dec2Bcd(tm->tm_wday))) return 0;
#else
	DBPRINTF(">>>mcdebug:  c4  tSetExternelRTC weekday=%02d\n",tm->tm_wday + 1);//ds3231为1~7
	if (!i2c_writetime(ERTC_WR_ADDR, ERTC_DAYOFWEEK_REG, Dec2Bcd(tm->tm_wday + 1))) return 0;
#endif

	DBPRINTF(">>>mcdebug:\tSetExternelRTC day=%02d\n",tm->tm_mday);
	if (!i2c_writetime(ERTC_WR_ADDR, ERTC_DAY_REG, Dec2Bcd(tm->tm_mday))) return 0;

	DBPRINTF(">>>mcdebug:\tSetExternelRTC month=%02d\n",value1);
	if (!i2c_writetime(ERTC_WR_ADDR, ERTC_MONTH_REG, Dec2Bcd(value1))) return 0;

	DBPRINTF(">>>mcdebug:\tSetExternelRTC year=%02d\n",value2);
	if (!i2c_writetime(ERTC_WR_ADDR, ERTC_YEAR_REG, Dec2Bcd(value2)))	 return 0;

#ifndef MACHINETYPE_C4
//修改完，先改写控制寄存器，仅用于sd2403
//置寄存器CTRL_REG2 的 WRTC1=0
	if (!i2c_writetime(ERTC_WR_ADDR, CTRL_REG1, 0x00))	 return 0;
	if (!i2c_writetime(ERTC_WR_ADDR, CTRL_REG2, 0x00))	 return 0;
//置寄存器CTRL_REG1 的 置WRTC2,WRTC3=0

#endif

	return 1;
}

int CheckI2CState(int Addr, char *buffer)
{
	int  cmd=-1;//i=3,
	if(Addr > 0xff)
		return 1;
	if(!CheckC4State()) //低电平表示有信号
	{
//		printf("CheckI2CState :%d \n",Addr);

		ExComI2C(Addr, CMD_QUERY, NULL, 0);
		if (ExComI2CWait(Addr, &cmd, buffer)>=0)
		{
			if (cmd == CMD_OK) return 0;
		}
	}
	return 1;
}

int Req_Query(int Addr, int cmd)
{
	char data[1], buffer[10];
	data[0] = cmd;
	ExComI2C(Addr, CMD_QUERY_REQ, data, 1);
	if (ExComI2CWait(Addr, &cmd, buffer)>=0) return 0;
	return 1;
}

int Req_QueryOnoff(int Addr, int cmd, int no)
{
	char data[2], buffer[10];
	data[0] = cmd;
	data[1] = no;
//	printf("Req_QueryOnoff :0x%x,%x \n",data[0],data[1]);
	ExComI2C(Addr, CMD_QUERY_REQ, data, 2);
	if (ExComI2CWait(Addr, &cmd, buffer)>=0) return 0;
	return 1;
}


