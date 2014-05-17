////timer function by oscarchang   20100105
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <pthread.h>
#include "arca.h"
#include "main.h"
#include "timer.h"

#define TIMERNUM      5 						//最大定时器数量
unsigned int  cTnCounter[TIMERNUM];			// 扩展定时器计数器
unsigned int  cTnPreset[TIMERNUM];				// 扩展定时器预置值
unsigned int  cTCON[TIMERNUM];					// 扩展定时器控制标志

extern int dev_handle_timer;
unsigned int thread_count = 0;

/*
pthread_t a_thread;

void *thread_function(void *arg);

int open_timer(void)
{
	int res;

	if(dev_handle_timer >0)
		return 0;

	res = pthread_create(&a_thread,NULL,thread_function, NULL);
	if(res !=0)
		return 0;
	dev_handle_timer++;
	return dev_handle_timer;
}

//100ms定时
void *thread_function(void *arg)
{
	int i;

	while(1)
	{
		usleep(100*1000);
		thread_count++;

		for (i=0; i<TIMERNUM; i++)
		{
			if (ReloadTimer(i))				// 定时到
				OnTimer(i);					// 定时事件处理		
		}
	}
}
*/

int open_timer(void)
{
	if(dev_handle_timer >0)
		return 0;

	dev_handle_timer++;
	return dev_handle_timer;
}

void Timer_count()
{
	int i;

	for (i=0; i<TIMERNUM; i++)
	{
		if (ReloadTimer(i))				// 定时到
			OnTimer(i);					// 定时事件处理		
	}
}

////////////////////////////////////////////////////////////////////////////////
// 定时器扩展(预定扩展6个定时器)，每个扩展定时器都有一个溢出计数器
// 启动定时器		cID: 定时器号 nTime: 定时时间ms
void SetTimer(unsigned char cID, unsigned int nTime)
{
/*	if (cID >= TIMERNUM)							// 合法定时器号0～2
		return;
	if (nTime >= 100)					// 定时器最小定时为100ms
	{
		cTnCounter[cID] = cTnPreset[cID] = nTime / 100;	// 定时器以100ms为定时单位
		cTCON[cID] = 1;						// allow TimerN
	}
	else
		cTCON[cID] = 0;						// disable TimerN
*/
	if (cID >= TIMERNUM)							// 合法定时器号0～5
		return;
	if (nTime >= 100)					// 如果以100ms为单位
	{
		cTnCounter[cID] = cTnPreset[cID] = nTime / 100;	// 定时器以100ms为定时单位
		cTCON[cID] = 1;						// allow TimerN
	}
	else if (nTime < 100 && nTime > 0) 	//以秒为单位
	{
		cTnCounter[cID] = cTnPreset[cID] = nTime;
		cTCON[cID] = 1;						// allow TimerN		
	}
	else
		cTCON[cID] = 0;
}

// 关闭定时器 cID: 定时器号
void KillTimer(unsigned char cID)
{
	cTCON[cID] = 0;							// stop Timer0-5
}

// 重装溢出计数器
int ReloadTimer(unsigned char cID)
{
	if ((cTCON[cID]) && --cTnCounter[cID]==0)
	{
		cTnCounter[cID] = cTnPreset[cID];
		return 1;
	}
	return 0;
}




