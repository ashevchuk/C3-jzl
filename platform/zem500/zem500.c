/*************************************************

 ZEM 200

 jz4730.c init funtions for GPIO/AC97 MUTE/USB POWER

 Copyright (C) 2003-2006, ZKSoftware Inc.

*************************************************/

#include <stdio.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>
#include <time.h>
#include <sys/time.h>
#include "arca.h"
#include "gpio.h"
#include "flash.h"

static int fdJzIO = -1;

static int BASELOOPTESTCNT = 20;
int GPIOCvtTbl[]={	33,100,101,0,0,0,105,104,0,0,
			0,0,0,0,0,0,0,0,0,0,
			0,0,0,0,0,0,0,0,0,0,
			103,0,0,0,0,0,0,0,0,0,
			0,0,0,0,0,0,0,0,0,103};
//because the minmize interval on linux is 10ms, use it when need
//it maybe not exatctly
void CalibrationTimeBaseValue(void)
{
	int i, j, k=0;
    	struct timeval tv;
    	struct timezone tz;
    	unsigned int s_msec, e_msec;
    	int DelayTimes=200*1000;

	gettimeofday(&tv, &tz);
    	s_msec = tv.tv_sec*1000*1000 + tv.tv_usec;

    	for (i=0; i<DelayTimes; i++)
		for(j=0; j<BASELOOPTESTCNT; j++) k++;

    	gettimeofday(&tv, &tz);
    	e_msec = tv.tv_sec*1000*1000 + tv.tv_usec;

    	DBPRINTF("DELAY US=%d BASECNT=%f\n", e_msec-s_msec, ((float)DelayTimes/(e_msec-s_msec))*BASELOOPTESTCNT);
    	BASELOOPTESTCNT=(int)(((float)DelayTimes/(e_msec-s_msec))*BASELOOPTESTCNT+1);

	#if 0
    	gettimeofday(&tv, &tz);
    	s_msec = tv.tv_sec*1000*1000 + tv.tv_usec;

    	DelayUS(1000*1000);

    	gettimeofday(&tv, &tz);
    	e_msec = tv.tv_sec*1000*1000 + tv.tv_usec;

	DBPRINTF("DELAY US=%d\n", e_msec-s_msec);
	#endif
}

void DelayUS(int us)
{
    	int i, j, k;

	for (i=0; i<us; i++)
		for(j=0; j<BASELOOPTESTCNT; j++) k++;
}

void DelayMS(int ms)
{
	//DBPRINTF("DelayMS zem500.c\n");
	DelayUS(1000*ms);
}

void DelayNS(long ns)
{
        struct timespec req;

        req.tv_sec=0;
        req.tv_nsec=ns;
        nanosleep(&req, NULL);
}

void mmsleep(int mms)
{
    struct timespec time;
    time.tv_sec = mms/1000;
    time.tv_nsec = (mms % 1000000) * 1000;
    //nanosleep(&time,NULL);
    select(0,NULL,NULL,NULL,&time);
}

BOOL GPIO_IO_Init(void)
{
	int i;
	fdJzIO = open ("/dev/mem", O_RDWR);
    	if (fdJzIO < 0)
		return FALSE;
    	else
	{
		//mmap jz register
		jz_reg = (unsigned char *) mmap(0, 0x100000, PROT_READ|PROT_WRITE,
					  MAP_SHARED, fdJzIO, JZ_REG_BASE);
		if (jz_reg == NULL)
		{
	   		close(fdJzIO);
	   		return FALSE;
       		}

		//jz gpio base

		jz_gpio_base=(unsigned char *)(jz_reg + JZ_GPIO_REG);
		gpio_reg = (jz_gpio_t *)(jz_reg + JZ_GPIO_REG);


		__gpio_as_output(RS485_SEND);    /* gpio pin as output */
//		__gpio_as_output(SHUT_KEY);	//SHUT_KEY = 103 is Eth/232 switch
		__gpio_as_output(IO_WIEGAND_OUT_D1);//	//2
		__gpio_as_output(IO_WIEGAND_OUT_D0);// 	//3

		__gpio_as_input(IO_C4_STATE);    /* gpio pin as input */

		//__gpio_as_input(SDCARD_CHECK);    /* gpio pin as input */

		__gpio_clear_pin(RS485_SEND);      /* output high level */ //485 �͵�ƽΪ��
	   //	__gpio_set_pin(SHUT_KEY);	 /* output high level */ none for ZEM500
		__gpio_clear_pin(IO_WIEGAND_OUT_D1);//	//2
		__gpio_clear_pin(IO_WIEGAND_OUT_D0);// 	//3


		//flash 4M Bytes
//		FlashBaseAddr = (REG16 *)mmap(0, 0x400000, PROT_READ|PROT_WRITE,
//					      MAP_SHARED, fdJzIO, FLASH_BASE);
		FlashBaseAddr = (REG16 *)mmap(0x100000, 0x400000,
		PROT_READ|PROT_WRITE,
		                                             MAP_SHARED, fdJzIO, FLASH_BASE);
		if (FlashBaseAddr == NULL)
		{
			close(fdJzIO);
			return FALSE;
		}
		gFlash16 = FlashBaseAddr + 0x100000; //2M BYTES OFFSET
		//�����TOP BOOT, ���޸���ʼƫ�Ƶ�ַ 1��SECTOR
		if (GetFlashID() == FLASH_TOP_BOOT)
		{
			gFlash16 -= STD_SECTOR_SIZE; //64KBytes
			DBPRINTF("FLASH IS TOP BOOT TYPE!    zem500 \n");
		}

		for(i = 0; i < FLASH_SECTOR_COUNT; i++)
			FlashSectorSizes[i] = STD_SECTOR_SIZE;

		DBPRINTF("GPIO_IO_Init OK!\n");
		return TRUE;
   	}
}

void GPIO_IO_Free(void)
{
    	munmap((void *)jz_reg, 0x100000);
	munmap((void *)FlashBaseAddr, 0x400000);
    	if (fdJzIO > 0)
		close(fdJzIO);
}

void GPIO_PIN_CTL_SET(int IsWiegandKeyPad, int IsNetSwitch)
{

	if (IsWiegandKeyPad)
	{
		;
	}
	else //
	{
		DBPRINTF("--ZEM500 GPIO init\n");
		__gpio_as_output(IO_DOOR_SENSOR); 	//no need for ZEM500
		__gpio_as_output(IO_DOOR_BUTTON); 	//no need for ZEM500
		__gpio_as_output(IO_ALARM_STRIP); 	//no need for ZEM500
			//
		__gpio_as_output(IO_WIEGAND_OUT_D1);//	//2
		__gpio_as_output(IO_WIEGAND_OUT_D0);// 	//3

		__gpio_as_output(IO_LOCK); 		//5
		__gpio_as_output(IO_RED_LED); 		//6
//		__gpio_as_output(IO_GREEN_LED); 	//7
		__gpio_as_output(I2S_CTL); 	//7
			//DBPRINTF("--------------gpio_as_output:%d\n",IO_RED_LED);
			//setup initial status for gpio pins
			//
		__gpio_set_pin(I2S_CTL); //enable is2
        	gpio_reg->gpio_gpdr1 |= 0x37;
        	gpio_reg->gpio_gpdr1 &= 0xffffff37;
	}
}
//H/W exist some problems, so don't use it, we will fixed it on the next version
void GPIO_AC97_Mute(BOOL Switch)
{
	if (Switch)
		__gpio_clear_pin(AC97_CTL);
	else
		__gpio_set_pin(AC97_CTL);
}
//control SENSOR ON or OFF
void GPIO_HY7131_Power(BOOL Switch)
{
	if (Switch)
		__gpio_clear_pin(USB_POWER_PIN); /* output low level */
	else
		__gpio_set_pin(USB_POWER_PIN); /* output high level */
}
//control the USB0 power/LCD backgound light
void GPIO_LCD_USB0_Power(BOOL Switch)
{
	if (Switch)
		__gpio_clear_pin(LCM_BACK_LIGHT); /* output low level */
	else
		__gpio_set_pin(LCM_BACK_LIGHT); /* output high level */
}

//// 485�շ�����	�͵�ƽ��,�ߵ�ƽ��(x = 1,�� x= 0,��)
void GPIO_RS485_Status(U32 SendMode)
{
	if (SendMode)
		__gpio_set_pin(RS485_SEND);      /* output high level */
	else
	{
		__gpio_clear_pin(RS485_SEND);      /* output low level */
	}
	DelayUS(1000);
}

//// net �շ����ָʾ  ����}���Ϊ����ָʾ��
//#define  IO_WIEGAND_OUT_D0	100	//ZEM500
//#define  IO_WIEGAND_OUT_D1	101	//ZEM500
void GPIO_netled_active_Status(U32 status)
{
	if (status)
	{
	__gpio_set_pin(IO_WIEGAND_OUT_D0);      /* output high level */
	}
	else
	{
	__gpio_clear_pin(IO_WIEGAND_OUT_D0);      /* output low level */
	}
}


void GPIO_netled_link_Status(U32 status)
{
	if (status)
	{
	__gpio_set_pin(IO_WIEGAND_OUT_D1);      /* output high level */
	}
	else
	{
	__gpio_clear_pin(IO_WIEGAND_OUT_D1);      /* output low level */
	}
}




BOOL GPIOGetLevel(BYTE IOPIN)
{
	return __gpio_get_pin(GPIOCvtTbl[IOPIN]);
}

void GPIOSetLevel(BYTE IOPIN, int High)
{
	if (High)
		__gpio_clear_pin(GPIOCvtTbl[IOPIN]);
	else
		__gpio_set_pin(GPIOCvtTbl[IOPIN]);
}

BOOL ExCheckGPI(BYTE GPIOPIN)
{
	int i=200,c=0;
	while(--i)
	{
		if(!GPIOGetLevel(GPIOPIN)) if(++c>20) return FALSE;
		DelayUS(5);
	}
	return TRUE;
}

void GPIO_SYS_POWER(void)
{
//	__gpio_clear_pin(SHUT_KEY);
}

void GPIO_WIEGAND_LED(BYTE GPIOPIN, BOOL Light)
{
	if (Light)
		__gpio_set_pin(GPIOPIN);
	else
		__gpio_clear_pin(GPIOPIN);
}

//It is used for A5 keypad scanning
void GPIO_KEY_SET(BYTE Value)
{
	gpio_reg->gpio_gpdr1&=0xffffff00;
	gpio_reg->gpio_gpdr1|=Value;
}
int GPIO_KEY_GET(void)
{
	return gpio_reg->gpio_gpdr1;
}

#if 0
int main(int argc, char **argv)
{
	CalibrationTimeBaseValue();
}
#endif

