//by oscarchang fo c3/c4  2010-01-15
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "flashdb.h"
#include "options.h"
#include "pollcard_logic.h"
#include "exfun.h"
#include "c4i2c.h"


extern TTime gCurTime;
extern Tpollcard pollcard_struct_variable;//刷卡变量结构
extern int CommKeepOpenSign[4];

//刷卡间隔判断 1，OK，间隔已经到，可以继续处理，0，停止处理
int process_interval(int *time_count,int *timer_Preset)
{
//	printf("time_count= %d, timer_Preset = %d,\n",*time_count, *timer_Preset);
	if(*time_count == 0)//说明刷卡间隔已经清0，可以刷卡了
	{
		*time_count = *timer_Preset;//更新值，为下次判断准备
		return 1;
	}
	else
		return 0;
}

//判断卡片有效时间，失效时间  0:失效 1有效
int process_card_valid_time(int start_time,int end_time,int cur_time)
{
//	printf("start_time= %d, end_time = %d,curr_time = %d\n",start_time, end_time, cur_time);
	if ( start_time == 0 &&  end_time == 0)//为0，表示没有启用此功能
		return 1;

	if(start_time <= cur_time && end_time >= cur_time)
		return 1;
	else
		return 0;
}


//门激活时间段判断 1 有效允许刷卡，0 无效，不允许刷卡
int process_door_valid_time(int timeID,TTime curtime)
{
	TC3Timeperiod Timeperiod;
	U32 hourMin;//,i;

	hourMin = curtime.tm_hour * 100 + curtime.tm_min;

	if(FDB_GetTimezone(timeID,curtime,&Timeperiod))
	{
		U16 StartTime1 = Timeperiod.time1 >>16 & 0xffff;
		U16 EndTime1 = Timeperiod.time1 & 0xffff;

		U16 StartTime2 = Timeperiod.time2 >>16 & 0xffff;
		U16 EndTime2 = Timeperiod.time2 & 0xffff;

		U16 StartTime3 = Timeperiod.time3 >>16 & 0xffff;
		U16 EndTime3 = Timeperiod.time3 & 0xffff;

		if((((EndTime1 - StartTime1) > 0) && (hourMin >= StartTime1) &&  (hourMin <= EndTime1))
		||(((EndTime2 - StartTime2) > 0) && (hourMin >= StartTime2) &&  (hourMin <= EndTime2))
		||(((EndTime3 - StartTime3) > 0) && (hourMin >= StartTime3) &&  (hourMin <= EndTime3)))
		{
			return 1;
		}

		else
		{
			printf("curhourmin= %d,time1=%d-%d,time1=%d-%d,time1=%d-%d \n",
				hourMin,StartTime1, EndTime1,StartTime2,EndTime2,StartTime3, EndTime3);

			return 0;
		}
	}
	else if(1 == timeID)  //默认时间段24小时，可以通过。
	{
		return 1;
	}

	return 0;
}
//时间反潜
int ProcessAPBLastTime(PLastTimeAPB CurLastTimeAPB,U32 PIN,int LastTimeAPB,time_t curTime)
{
	int i = 0;
	printf("curTime=%d,LastTimeAPB=%d\n",curTime,LastTimeAPB);
	if(LastTimeAPB == 0 || NULL == CurLastTimeAPB)
	{
		return 0;
	}

	for(i=0;i<gOptions.MaxUserCount*100;i++)
	{
		if(CurLastTimeAPB[i].PIN == PIN)
		{

			if(curTime - CurLastTimeAPB[i].LastTime < LastTimeAPB)
			{
				return -1;//距离上次时间过短，不能通过。
			}
			else
			{
				CurLastTimeAPB[i].LastTime = OldEncodeTime(&gCurTime);
				return 0;//可以通过
			}

		}
		if(CurLastTimeAPB[i].PIN == 0)//还没有该用户最后一次记录信息，需添加
		{
			CurLastTimeAPB[i].PIN = PIN;
			CurLastTimeAPB[i].LastTime = curTime;
			return 0;//可以通过
		}
	}
	return 0;//可以通过
}
//判断反潜回
// 0 不符合反潜回要求,1符合
int process_card_AntiPassback(PAlarmRec CurAlarmRec,PAlarmRec CurAlarmRec2,int PIN,U8 State)
{
	int CurPos=0,CurPos2=0;

	if(gOptions.AntiPassback == 0)//无反潜功能
	{
		return 1;
	}
	printf("AntiPassback mode = %d, PIN = %d,State = %d\n",
		gOptions.AntiPassback,PIN,State);

	//先作反潜类型3判断 且在3，4号读头刷卡的
	if(gOptions.AntiPassback == 3 && (State==3 || State == 4))
	{
		for(CurPos2=0;CurPos2<gOptions.MaxUserCount*100 ;CurPos2++)
		{

			if(CurAlarmRec2[CurPos2].PIN == PIN)
			{
				if(CurAlarmRec2[CurPos2].State  == State)
				{
					return 0;//不通过
				}
				else
				{
					CurAlarmRec2[CurPos2].State = State;
					return 1;
				}
			}
		}
		return 1;
	}

	for(CurPos=0;CurPos<gOptions.MaxUserCount*100 ;CurPos++)
	{

		if(CurAlarmRec[CurPos].PIN == PIN)
		{
			switch (gOptions.AntiPassback)
			{
				case 1://c3_400:12号门返潜  c3_200:1号门进出反潜
				{
					if(CurAlarmRec[CurPos].State == State && (State == 1 || State == 2))
					{
						return 0;
					}
					else
					{
						if(State == 1 || State == 2)
						{
							CurAlarmRec[CurPos].State = State;
						}
						return 1;
					}
				}
				break;
				case 2: //c3_400:34号门返潜  c3_200:2号门进出反潜
				{
					if(CurAlarmRec[CurPos].State == State && (State == 3 || State == 4))
					{
						return 0;
					}
					else
					{
						if(State == 3 || State == 4)
						{
							CurAlarmRec[CurPos].State = State;
						}
						return 1;
					}
				}
				break;
				case 3://c3_400:12号门返潜,34号门返潜  		c3_200:1,2号门各自进出反潜
				{
					if(CurAlarmRec[CurPos].State == State && (State == 1 || State == 2))//如果在12号读头上刷卡
					{
						return 0;
					}
					else
					{
						if(State == 1 || State == 2)
						{
							CurAlarmRec[CurPos].State = State;
						}
						return 1;
					}

				}
				break;
				case 4://c3_400:1/2与3/4号门返潜 		 c3_200:1,2号门间反潜
				{
					if(CurAlarmRec[CurPos].State  == 1 || CurAlarmRec[CurPos].State == 2)
					{
						if(State == 1 || State == 2)
						{
							return 0;
						}
						else
						{
							CurAlarmRec[CurPos].State = State;
							return 1;
						}
					}
					else
					{

						if(State == 3 || State == 4)
						{
							return 0;
						}
						else
						{
							CurAlarmRec[CurPos].State = State;
							return 1;
						}
					}
				}
				break;
				case 5://c3_400:1与2/3号门返潜	 	c3_200:无
				{
					if(CurAlarmRec[CurPos].State  == 1)
					{
						if(State == 1)
						{
							return 0;
						}
						else
						{
							CurAlarmRec[CurPos].State = State;
							return 1;
						}
					}
					else if(CurAlarmRec[CurPos].State  == 2
						|| CurAlarmRec[CurPos].State  == 3)
					{
						if(State == 2 || State == 3)
						{
							return 0;
						}
						else
						{
							CurAlarmRec[CurPos].State = State;
							return 1;
						}
					}
					else
						return 1;
				}
				break;
				case 6://c3_400: 1与2/3/4号门返潜		c3_200:无
				{
					if(CurAlarmRec[CurPos].State  == 1)
					{
						if(State == 1)
						{
							return 0;
						}
						else
						{
							CurAlarmRec[CurPos].State = State;
							return 1;
						}
					}
					else if(CurAlarmRec[CurPos].State  == 2
						|| CurAlarmRec[CurPos].State  == 3
						|| CurAlarmRec[CurPos].State  == 4)
					{
						if(State == 2 || State == 3 || State == 4)
						{
							return 0;
						}
						else
						{
							CurAlarmRec[CurPos].State = State;
							return 1;
						}
					}
					else
						return 1;
				}
				break;
			}
		}

	}

	return 1;
}

//多门互锁判�
//0无1 :1/2号门互锁2 3/4号门互锁3 1/2/3号门互锁4.  1/2号门互锁2 3/4号门互锁 5.1/2/3/4号门互锁
//返回1，表示OK,    0, 互锁失败
//DoorID 为当前刷卡的门号
int process_mutiLock_linkage(int DoorID)
{
	int getstate=0,getstate2=0,getstate3=0;

// ExGetState      0x00表示断开，0xff闭合
//门磁 0 无，1常开(0X00,门闭合，oxff门打开)   2常闭(0Xff,门闭合ox00表示门打开)�
	printf("doorID = %d,mutuallockrule =%d,detectortype = %d,%d,%d,%d ,\n",
		DoorID,gOptions.InterLock,
		gOptions.DoorDetectortype[0],gOptions.DoorDetectortype[1],gOptions.DoorDetectortype[2],gOptions.DoorDetectortype[3]);
	switch(gOptions.InterLock)
	{
		case 0://0、	无
		{
			return 1;//
		}
		break;
		case 1://	1、2号门互锁
		{

			if(DoorID ==1)
			{
				if(0 == gOptions.DoorDetectortype[1])//无门磁
				{
					return 0;
				}
				//2号继电器的开关状态
				if(gOptions.MachineType == C3_200 || gOptions.MachineType == C3_400To_200)
				{
					getstate =  ExGetState(OUTPUT,MCU2,1);
				}
				else if(gOptions.MachineType == C3_400)
				{
					getstate =  ExGetState(OUTPUT,MCU1,2);
				}
				else if(gOptions.MachineType == C4 || gOptions.MachineType == C4_200)
				{
					getstate =  ExGetState(OUTPUT,MCU2,1);
				}
				else if(gOptions.MachineType == C4_400To_200)
				{
					getstate =  ExGetState(OUTPUT,MCU3,1);
				}

				printf("skui --------  getstate = 0x%x\n",getstate & 0xff);
				if(0 != getstate)
				{
					return 0;
				}
				// 2号门情况  (不同型号机型的门磁与cpuid不一样)
				if(gOptions.MachineType == C3_200)
				{
					getstate =  (gOptions.DoorDetectortype[1]==1)?  ExGetState(INPUT,MCU2,3) :!ExGetState(INPUT,MCU2,3);
				}
				else if(gOptions.MachineType == C3_400)
				{
					getstate =  (gOptions.DoorDetectortype[1]==1)?  ExGetState(INPUT,MCU1,4) :!ExGetState(INPUT,MCU1,4);
				}
				else if(gOptions.MachineType == C4 || gOptions.MachineType == C4_200)
				{
					getstate =  (gOptions.DoorDetectortype[1]==1)?  ExGetState(INPUT,MCU2,2) :!ExGetState(INPUT,MCU2,2);
				}
				else if(gOptions.MachineType == C4_400To_200)
				{
					getstate =  (gOptions.DoorDetectortype[1]==1)?  ExGetState(INPUT,MCU3,2) :!ExGetState(INPUT,MCU3,2);
				}
				else if(gOptions.MachineType == C3_400To_200)
				{
					getstate =  (gOptions.DoorDetectortype[1]==1)?  ExGetState(INPUT,MCU2,3) :!ExGetState(INPUT,MCU2,3);
				}
				printf("ExGetState  linktype is 1 dooridis 1  getstate = 0x%x\n",getstate & 0xff);
				if(getstate  == 0)//0 表示门没有开
				{
					return 1;
				}
				else
				{
					return 0;
				}
			}
			else if(DoorID ==2)
			{
				if(0 == gOptions.DoorDetectortype[0])//无门磁
				{
					return 0;
				}
				//1号继电器的开关状态
				getstate =  ExGetState(OUTPUT,MCU1,1);
				if(0 != getstate)
				{
					return 0;
				}

				// 1号门情况
				if(gOptions.MachineType == C3_200 || gOptions.MachineType == C3_400To_200)
				{
					getstate =  (gOptions.DoorDetectortype[0]==1)?  ExGetState(INPUT,MCU1,3) :!ExGetState(INPUT,MCU1,3);
				}
				else if(gOptions.MachineType == C3_400)
				{
					getstate =  (gOptions.DoorDetectortype[0]==1)?  ExGetState(INPUT,MCU1,3) :!ExGetState(INPUT,MCU1,3);
				}
				else if(gOptions.MachineType == C4 || gOptions.MachineType == C4_200 || gOptions.MachineType == C4_400To_200)
				{
					getstate =  (gOptions.DoorDetectortype[0]==1)?  ExGetState(INPUT,MCU1,2) :!ExGetState(INPUT,MCU1,2);
				}
				printf("ExGetState  linktype is 1 dooridis 2   getstate = 0x%x\n",getstate & 0xff);

				if(getstate  == 0)//0 表示门没有开
				{
					return 1;
				}
				else
				{
					return 0;
				}
			}
			else
			{
				return 1;
			}

		}
		break;
		case 2://	3、4号门互锁
		{

			if(DoorID ==3)
			{
				if(0 == gOptions.DoorDetectortype[3])//无门磁
				{
					return 0;
				}

				if(gOptions.MachineType == C3_400)
				{
					getstate =  ExGetState(OUTPUT,MCU2,2);
				}
				else if(gOptions.MachineType == C4)
				{
					getstate =  ExGetState(OUTPUT,MCU4,1);
				}
				if(0 != getstate)
				{
					return 0;
				}
				printf("skui --------  getstate = 0x%x\n",getstate & 0xff);
				// 4号门情况
				if(gOptions.MachineType == C3_400)
				{
					getstate =  (gOptions.DoorDetectortype[3]==1)?  ExGetState(INPUT,MCU2,4) :!ExGetState(INPUT,MCU2,4);
				}
				else if(gOptions.MachineType == C4)
				{
					getstate =  (gOptions.DoorDetectortype[3]==1)?  ExGetState(INPUT,MCU4,2) :!ExGetState(INPUT,MCU4,2);
				}
				printf("ExGetState  type is 2  DoorID =%d, getstate = 0x%x    MachineType = %d\n",DoorID,getstate & 0xff,gOptions.MachineType);
				if(getstate  == 0)//0 表示门没有开
				{
					return 1;
				}
				else
				{
					return 0;
				}
			}
			else if(DoorID ==4)
			{
			// 3号门情况
				if(0 == gOptions.DoorDetectortype[2])//无门磁
				{
					return 0;
				}

				if(gOptions.MachineType == C3_400)
				{
					getstate =  ExGetState(OUTPUT,MCU2,1);
				}
				else if(gOptions.MachineType == C4)
				{
					getstate =  ExGetState(OUTPUT,MCU3,1);
				}
				if(0 != getstate)
				{
					return 0;
				}

				if(gOptions.MachineType == C3_400)
				{
					getstate =  (gOptions.DoorDetectortype[2]==1)?  ExGetState(INPUT,MCU2,3) :!ExGetState(INPUT,MCU2,3);
				}
				else if(gOptions.MachineType == C4)
				{
					getstate =  (gOptions.DoorDetectortype[2]==1)?  ExGetState(INPUT,MCU3,2) :!ExGetState(INPUT,MCU3,2);
				}
				printf("ExGetState  type is 2  DoorID =%d, getstate = 0x%x    MachineType = %d\n",DoorID,getstate & 0xff,gOptions.MachineType);
				if(getstate  == 0)//0 表示门没有开
				{
					return 1;
				}
				else
				{
					return 0;
				}
			}
			else
			{
				return 1;
			}
		}
		break;
		case 3://	1,2,3号门互锁
		{


			if(DoorID ==1)
			{
				if(0 == gOptions.DoorDetectortype[1] || 0 == gOptions.DoorDetectortype[2])//无门磁
				{
					return 0;
				}
                //2、3号继电器的开关状态
				if(gOptions.MachineType == C3_400)
				{
					getstate =  ExGetState(OUTPUT,MCU1,2);
					getstate2 =  ExGetState(OUTPUT,MCU2,1);
				}
				else if(gOptions.MachineType == C4)
				{
					getstate =  ExGetState(OUTPUT,MCU2,1);
					getstate2 =  ExGetState(OUTPUT,MCU3,1);
				}
				if(0 != getstate || 0 != getstate2)
				{
					return 0;
				}

				//2、3号门磁的开关状态
				if(gOptions.MachineType == C3_400)
				{
					getstate =  (gOptions.DoorDetectortype[1]==1)?  ExGetState(INPUT,MCU1,4) :!ExGetState(INPUT,MCU1,4);
					getstate2 =  (gOptions.DoorDetectortype[2]==1)?  ExGetState(INPUT,MCU2,3) :!ExGetState(INPUT,MCU2,3);
				}
				else if(gOptions.MachineType == C4)
				{
					getstate =  (gOptions.DoorDetectortype[1]==1)?  ExGetState(INPUT,MCU2,2) :!ExGetState(INPUT,MCU2,2);
					getstate2 =  (gOptions.DoorDetectortype[2]==1)?  ExGetState(INPUT,MCU3,2) :!ExGetState(INPUT,MCU3,2);
				}

				printf("ExGetState  linktype is 3    doorid is 1 ,getstate = 0x%x,getstate2 = 0x%x\n",getstate & 0xff,getstate2 & 0xff);

				if(getstate == 0 &&  getstate2 == 0)//0 表示门没有开
				{
					return 1;
				}
				else
				{
					return 0;
				}
			}
			else if(DoorID ==2)
			{
				if(0 == gOptions.DoorDetectortype[0] || 0 == gOptions.DoorDetectortype[2])//无门磁
				{
					return 0;
				}
				 //1、3号继电器的开关状态
				if(gOptions.MachineType == C3_400)
				{
					getstate =  ExGetState(OUTPUT,MCU1,1);
					getstate2 =  ExGetState(OUTPUT,MCU2,1);
				}
				else if(gOptions.MachineType == C4)
				{
					getstate =  ExGetState(OUTPUT,MCU1,1);
					getstate2 =  ExGetState(OUTPUT,MCU3,1);
				}
				if(0 != getstate || 0 != getstate2)
				{
					return 0;
				}
				// 1、3号门情况

				if(gOptions.MachineType == C3_400)
				{
					getstate =  (gOptions.DoorDetectortype[0]==1)?  ExGetState(INPUT,MCU1,3) :!ExGetState(INPUT,MCU1,3);
					getstate2 =  (gOptions.DoorDetectortype[2]==1)?  ExGetState(INPUT,MCU2,3) :!ExGetState(INPUT,MCU2,3);
				}
				else if(gOptions.MachineType == C4)
				{
					getstate =  (gOptions.DoorDetectortype[0]==1)?  ExGetState(INPUT,MCU1,2) :!ExGetState(INPUT,MCU1,2);
					getstate2 =  (gOptions.DoorDetectortype[2]==1)?  ExGetState(INPUT,MCU3,2) :!ExGetState(INPUT,MCU3,2);
				}

				printf("ExGetState  linktype is 3    doorid is 2 ,getstate = 0x%x,getstate2 = 0x%x\n",getstate & 0xff,getstate2 & 0xff);

				if(getstate == 0 &&  getstate2 == 0)//0 表示门没有开
				{
					return 1;
				}
				else
				{
					return 0;
				}
			}
			else if(DoorID ==3)
			{
				if(0 == gOptions.DoorDetectortype[0] || 0 == gOptions.DoorDetectortype[1])//无门磁
				{
					return 0;
				}
				 //1、2号继电器的开关状态
				if(gOptions.MachineType == C3_400)
				{
					getstate =  ExGetState(OUTPUT,MCU1,1);
					getstate2 =  ExGetState(OUTPUT,MCU1,2);
				}
				else if(gOptions.MachineType == C4)
				{
					getstate =  ExGetState(OUTPUT,MCU1,1);
					getstate2 =  ExGetState(OUTPUT,MCU2,1);
				}
				if(0 != getstate || 0 != getstate2)
				{
					return 0;
				}
				// 1、2号门情况

				if(gOptions.MachineType == C3_400)
				{
					getstate =  (gOptions.DoorDetectortype[0]==1)?  ExGetState(INPUT,MCU1,3) :!ExGetState(INPUT,MCU1,3);
					getstate2 =  (gOptions.DoorDetectortype[1]==1)?  ExGetState(INPUT,MCU1,4) :!ExGetState(INPUT,MCU1,4);
				}
				else if(gOptions.MachineType == C4)
				{
					getstate =  (gOptions.DoorDetectortype[0]==1)?  ExGetState(INPUT,MCU1,2) :!ExGetState(INPUT,MCU1,2);
					getstate2 =  (gOptions.DoorDetectortype[1]==1)?  ExGetState(INPUT,MCU2,2) :!ExGetState(INPUT,MCU2,2);
				}

				printf("ExGetState  linktype is 3    doorid is 1 ,getstate = 0x%x,getstate2 = 0x%x\n",getstate & 0xff,getstate2 & 0xff);

				if(getstate == 0 &&  getstate2 == 0)//0 表示门没有开
				{
					return 1;
				}
				else
				{
					return 0;
				}
			}
			else
			{
				return 1;
			}
		}
		break;
		case 4://  1/2号门间互锁，或	3/4号门间互锁
		{

			if(DoorID ==1)
			{
				if(0 == gOptions.DoorDetectortype[1])//无门磁
				{
					return 0;
				}

				if(gOptions.MachineType == C3_400)
				{
					getstate =  ExGetState(OUTPUT,MCU1,2);
				}
				else if(gOptions.MachineType == C4)
				{
					getstate =  ExGetState(OUTPUT,MCU2,1);
				}

				printf("skui --------  getstate = 0x%x\n",getstate & 0xff);
				if(0 != getstate)
				{
					return 0;
				}
				// 2号门情况  (不同型号机型的门磁与cpuid不一样)

				if(gOptions.MachineType == C3_400)
				{
					getstate =  (gOptions.DoorDetectortype[1]==1)?  ExGetState(INPUT,MCU1,4) :!ExGetState(INPUT,MCU1,4);
				}
				else if(gOptions.MachineType == C4)
				{
					getstate =  (gOptions.DoorDetectortype[1]==1)?  ExGetState(INPUT,MCU2,2) :!ExGetState(INPUT,MCU2,2);
				}

				printf("ExGetState  linktype is 1 dooridis 1  getstate = 0x%x\n",getstate & 0xff);
				if(getstate  == 0)//0 表示门没有开
				{
					return 1;
				}
				else
				{
					return 0;
				}
			}
			else if(DoorID ==2)
			{

				if(0 == gOptions.DoorDetectortype[0])//无门磁
				{
					return 0;
				}
				getstate =  ExGetState(OUTPUT,MCU1,1);
				if(0 != getstate)
				{
					return 0;
				}
				// 1号门情况

				if(gOptions.MachineType == C3_400)
				{
					getstate =  (gOptions.DoorDetectortype[0]==1)?  ExGetState(INPUT,MCU1,3) :!ExGetState(INPUT,MCU1,3);
				}
				else if(gOptions.MachineType == C4)
				{
					getstate =  (gOptions.DoorDetectortype[0]==1)?  ExGetState(INPUT,MCU1,2) :!ExGetState(INPUT,MCU1,2);
				}
				printf("ExGetState  linktype is 1 dooridis 2   getstate = 0x%x\n",getstate & 0xff);

				if(getstate  == 0)//0 表示门没有开
				{
					return 1;
				}
				else
				{
					return 0;
				}
			}
			else if(DoorID ==3)
			{
			////4号门情况
				if(0 == gOptions.DoorDetectortype[3])//无门磁
				{
					return 0;
				}
				if(gOptions.MachineType == C3_400)
				{
					getstate =  ExGetState(OUTPUT,MCU2,2);
				}
				else if(gOptions.MachineType == C4)
				{
					getstate =  ExGetState(OUTPUT,MCU4,1);
				}
				if(0 != getstate)
				{
					return 0;
				}
				if(gOptions.MachineType == C3_400)
				{
					getstate =  (gOptions.DoorDetectortype[3]==1)?  ExGetState(INPUT,MCU2,4) :!ExGetState(INPUT,MCU2,4);
				}
				else if(gOptions.MachineType == C4)
				{
					getstate =  (gOptions.DoorDetectortype[3]==1)?  ExGetState(INPUT,MCU4,2) :!ExGetState(INPUT,MCU4,2);
				}
				printf("ExGetState  linktype is 2   getstate = 0x%x\n",getstate & 0xff);
				if(getstate  == 0)//0 表示门没有开
				{
					return 1;
				}
				else
				{
					return 0;
				}
			}
			else if(DoorID ==4)
			{
			////3号门情况
				if(0 == gOptions.DoorDetectortype[2])//无门磁
				{
					return 0;
				}
				if(gOptions.MachineType == C3_400)
				{
					getstate =  ExGetState(OUTPUT,MCU2,1);
				}
				else if(gOptions.MachineType == C4)
				{
					getstate =  ExGetState(OUTPUT,MCU3,1);
				}
				if(0 != getstate)
				{
					return 0;
				}
				if(gOptions.MachineType == C3_400)
				{
					getstate =  (gOptions.DoorDetectortype[2]==1)?  ExGetState(INPUT,MCU2,3) :!ExGetState(INPUT,MCU2,3);
				}
				else if(gOptions.MachineType == C4)
				{
					getstate =  (gOptions.DoorDetectortype[2]==1)?  ExGetState(INPUT,MCU3,2) :!ExGetState(INPUT,MCU3,2);
				}
				printf("ExGetState getstate = 0x%x,mcuaddr = 0x%x\n",getstate & 0xff,MCU1);
				if(getstate  == 0)//0 表示门没有开
				{
					return 1;
				}
				else
				{
					return 0;
				}
			}
			else
				return 1;

		}
		break;
		case 5://	1,2,3,4号门互锁
		{


			if(DoorID ==1)
			{
				if(0 == gOptions.DoorDetectortype[1] || 0 == gOptions.DoorDetectortype[2]
				           || 0 == gOptions.DoorDetectortype[3])//无门磁
				{
					return 0;
				}
				if(gOptions.MachineType == C3_400)
				{
					getstate =  ExGetState(OUTPUT,MCU1,2);
					getstate2 =  ExGetState(OUTPUT,MCU2,1);
					getstate3 =  ExGetState(OUTPUT,MCU2,2);
				}
				else if(gOptions.MachineType == C4)
				{
					getstate =  ExGetState(OUTPUT,MCU2,1);
					getstate2 =  ExGetState(OUTPUT,MCU3,1);
					getstate3 =  ExGetState(OUTPUT,MCU4,1);
				}
				if(0 != getstate || 0 != getstate2 || 0 != getstate3)
				{
					return 0;
				}

				// 2号门情况  (不同型号机型的门磁与cpuid不一样)

				if(gOptions.MachineType == C3_400)
				{
					getstate =  (gOptions.DoorDetectortype[1]==1)?  ExGetState(INPUT,MCU1,4) :!ExGetState(INPUT,MCU1,4);
					getstate2 =  (gOptions.DoorDetectortype[2]==1)?  ExGetState(INPUT,MCU2,3) :!ExGetState(INPUT,MCU2,3);
					getstate3 =  (gOptions.DoorDetectortype[3]==1)?  ExGetState(INPUT,MCU2,4) :!ExGetState(INPUT,MCU2,4);
				}
				else if(gOptions.MachineType == C4)
				{
					getstate =  (gOptions.DoorDetectortype[1]==1)?  ExGetState(INPUT,MCU2,2) :!ExGetState(INPUT,MCU2,2);
					getstate2 =  (gOptions.DoorDetectortype[2]==1)?  ExGetState(INPUT,MCU3,2) :!ExGetState(INPUT,MCU3,2);
					getstate3 =  (gOptions.DoorDetectortype[3]==1)?  ExGetState(INPUT,MCU4,2) :!ExGetState(INPUT,MCU4,2);
				}
				// 3号门情况

				if(getstate == 0 &&  getstate2 == 0  &&  getstate3 == 0)//0 表示门没有开
				{
					return 1;
				}
				else
				{
					return 0;
				}
			}
			else if(DoorID ==2)
			{
				if(0 == gOptions.DoorDetectortype[0] || 0 == gOptions.DoorDetectortype[2]
						|| 0 == gOptions.DoorDetectortype[3])//无门磁
				{
					return 0;
				}
				if(gOptions.MachineType == C3_400)
				{
					getstate =  ExGetState(OUTPUT,MCU1,1);
					getstate2 =  ExGetState(OUTPUT,MCU2,1);
					getstate3 =  ExGetState(OUTPUT,MCU2,2);
				}
				else if(gOptions.MachineType == C4)
				{
					getstate =  ExGetState(OUTPUT,MCU1,1);
					getstate2 =  ExGetState(OUTPUT,MCU3,1);
					getstate3 =  ExGetState(OUTPUT,MCU4,1);
				}
				if(0 != getstate || 0 != getstate2 || 0 != getstate3)
				{
					return 0;
				}

				// 1号门情况
				if(gOptions.MachineType == C3_400)
				{
					getstate =  (gOptions.DoorDetectortype[0]==1)?  ExGetState(INPUT,MCU1,3) :!ExGetState(INPUT,MCU1,3);
					getstate2 =  (gOptions.DoorDetectortype[2]==1)?  ExGetState(INPUT,MCU2,3) :!ExGetState(INPUT,MCU2,3);
					getstate3 =  (gOptions.DoorDetectortype[3]==1)?  ExGetState(INPUT,MCU2,4) :!ExGetState(INPUT,MCU2,4);
				}
				else if(gOptions.MachineType == C4)
				{
					getstate =  (gOptions.DoorDetectortype[0]==1)?  ExGetState(INPUT,MCU1,2) :!ExGetState(INPUT,MCU1,2);
					getstate2 =  (gOptions.DoorDetectortype[2]==1)?  ExGetState(INPUT,MCU3,2) :!ExGetState(INPUT,MCU3,2);
					getstate3 =  (gOptions.DoorDetectortype[3]==1)?  ExGetState(INPUT,MCU4,2) :!ExGetState(INPUT,MCU4,2);
				}
				// 3号门情况

				if(getstate == 0 &&  getstate2 == 0 &&  getstate3 == 0)
				{
					return 1;
				}
				else
				{
					return 0;
				}
			}
			else if(DoorID ==3)
			{
				if(0 == gOptions.DoorDetectortype[0] || 0 == gOptions.DoorDetectortype[1]
						|| 0 == gOptions.DoorDetectortype[3])//无门磁
				{
					return 0;
				}
				if(gOptions.MachineType == C3_400)
				{
					getstate =  ExGetState(OUTPUT,MCU1,1);
					getstate2 =  ExGetState(OUTPUT,MCU1,2);
					getstate3 =  ExGetState(OUTPUT,MCU2,2);
				}
				else if(gOptions.MachineType == C4)
				{
					getstate =  ExGetState(OUTPUT,MCU1,1);
					getstate2 =  ExGetState(OUTPUT,MCU2,1);
					getstate3 =  ExGetState(OUTPUT,MCU4,1);
				}
				if(0 != getstate || 0 != getstate2 || 0 != getstate3)
				{
					return 0;
				}


				if(gOptions.MachineType == C3_400)
				{
					getstate =  (gOptions.DoorDetectortype[0]==1)?  ExGetState(INPUT,MCU1,3) :!ExGetState(INPUT,MCU1,3);
					getstate2 =  (gOptions.DoorDetectortype[1]==1)?  ExGetState(INPUT,MCU1,4) :!ExGetState(INPUT,MCU1,4);
					getstate3 =  (gOptions.DoorDetectortype[3]==1)?  ExGetState(INPUT,MCU2,4) :!ExGetState(INPUT,MCU2,4);
				}
				else if(gOptions.MachineType == C4)
				{
					getstate =  (gOptions.DoorDetectortype[0]==1)?  ExGetState(INPUT,MCU1,2) :!ExGetState(INPUT,MCU1,2);
					getstate2 =  (gOptions.DoorDetectortype[1]==1)?  ExGetState(INPUT,MCU2,2) :!ExGetState(INPUT,MCU2,2);
					getstate3 =  (gOptions.DoorDetectortype[3]==1)?  ExGetState(INPUT,MCU4,2) :!ExGetState(INPUT,MCU4,2);
				}
				if(getstate == 0 &&  getstate2 == 0 &&  getstate3 == 0)
				{
					return 1;
				}
				else
				{
					return 0;
				}
			}
			else if(DoorID ==4)
			{
				if(0 == gOptions.DoorDetectortype[0] || 0 == gOptions.DoorDetectortype[1]
						|| 0 == gOptions.DoorDetectortype[2])//无门磁
				{
					return 0;
				}
				if(gOptions.MachineType == C3_400)
				{
					getstate =  ExGetState(OUTPUT,MCU1,1);
					getstate2 =  ExGetState(OUTPUT,MCU1,2);
					getstate3 =  ExGetState(OUTPUT,MCU2,1);
				}
				else if(gOptions.MachineType == C4)
				{
					getstate =  ExGetState(OUTPUT,MCU1,1);
					getstate2 =  ExGetState(OUTPUT,MCU2,1);
					getstate3 =  ExGetState(OUTPUT,MCU3,1);
				}
				if(0 != getstate || 0 != getstate2 || 0 != getstate3)
				{
					return 0;
				}
				// 1号门情况

				if(gOptions.MachineType == C3_400)
				{
					getstate =  (gOptions.DoorDetectortype[0]==1)?  ExGetState(INPUT,MCU1,3) :!ExGetState(INPUT,MCU1,3);
					getstate2 =  (gOptions.DoorDetectortype[1]==1)?  ExGetState(INPUT,MCU1,4) :!ExGetState(INPUT,MCU1,4);
					getstate3 =  (gOptions.DoorDetectortype[2]==1)?  ExGetState(INPUT,MCU2,3) :!ExGetState(INPUT,MCU2,3);
				}
				else if(gOptions.MachineType == C4)
				{
					getstate =  (gOptions.DoorDetectortype[0]==1)?  ExGetState(INPUT,MCU1,2) :!ExGetState(INPUT,MCU1,2);
					getstate2 =  (gOptions.DoorDetectortype[1]==1)?  ExGetState(INPUT,MCU2,2) :!ExGetState(INPUT,MCU2,2);
					getstate3 =  (gOptions.DoorDetectortype[2]==1)?  ExGetState(INPUT,MCU3,2) :!ExGetState(INPUT,MCU3,2);
				}

				if(getstate == 0 &&  getstate2 == 0 &&  getstate3 == 0)
				{
					return 1;
				}
				else
				{
					return 0;
				}
			}
			else
			{
				return 1;
			}

		}
		break;


	}
	return 1;
}
//加记录到每张卡最后一条记录中，作反潜用
int add_Rec_CurAlarmRec(PAlarmRec CurAlarmRec,PAlarmRec CurAlarmRec2,int PIN,time_t LastTime,U8  State,U32 workcode)
{
	int CurPos;
	BYTE bfind;

	switch (gOptions.AntiPassback)
	{
		case 0://无反潜
			return 1;
		break;
		case 1://c3_400:12号门返潜  c3_200:1号门进出反潜
		{
			if(State == 1 || State == 2)
			{
				for(CurPos=0,bfind = 0;CurPos<gOptions.MaxUserCount*100 && bfind != 1;CurPos++)
				{

					if(CurAlarmRec[CurPos].PIN == PIN)
					{
						CurAlarmRec[CurPos].PIN = PIN;
//						CurAlarmRec[CurPos].LastTime = LastTime;
						CurAlarmRec[CurPos].State = State;
						bfind = 1;
						return 1;
					}
				}
				if(!bfind)//说明记录buffer中没有当前卡的记录，就将
				{
					for(CurPos=0,bfind = 0;CurPos<gOptions.MaxUserCount*100 && bfind != 1;CurPos++)
					{

						if(CurAlarmRec[CurPos].PIN==0)
						{
							CurAlarmRec[CurPos].PIN = PIN;
//							CurAlarmRec[CurPos].LastTime = LastTime;
							CurAlarmRec[CurPos].State = State;
							bfind = 1;
							return 1;
						}
					}
				}
			}
		}
		break;
		case 2: //c3_400:34号门返潜  c3_200:2号门进出反潜
		{
			if(State == 3 || State == 4)
			{
				for(CurPos=0,bfind = 0;CurPos<gOptions.MaxUserCount*100 && bfind != 1;CurPos++)
				{

					if(CurAlarmRec[CurPos].PIN == PIN)
					{
						CurAlarmRec[CurPos].PIN = PIN;
//						CurAlarmRec[CurPos].LastTime = LastTime;
						CurAlarmRec[CurPos].State = State;
						bfind = 1;
						return 1;
					}
				}
				if(!bfind)//说明记录buffer中没有当前卡的记录，就将
				{
					for(CurPos=0,bfind = 0;CurPos<gOptions.MaxUserCount*100 && bfind != 1;CurPos++)
					{

						if(CurAlarmRec[CurPos].PIN==0)
						{
							CurAlarmRec[CurPos].PIN = PIN;
//							CurAlarmRec[CurPos].LastTime = LastTime;
							CurAlarmRec[CurPos].State = State;
							bfind = 1;
							return 1;
						}
					}
				}
			}
		}
		break;
		case 3://c3_400:12号门返潜,34号门返潜  		c3_200:1,2号门各自进出反潜
		{
			if(State == 1 || State == 2)
			{
				for(CurPos=0,bfind = 0;CurPos<gOptions.MaxUserCount*100 && bfind != 1;CurPos++)
				{

					if(CurAlarmRec[CurPos].PIN == PIN)
					{
						CurAlarmRec[CurPos].PIN = PIN;
//						CurAlarmRec[CurPos].LastTime = LastTime;
						CurAlarmRec[CurPos].State = State;
						bfind = 1;
						return 1;
					}
				}
				if(!bfind)//说明记录buffer中没有当前卡的记录，就将
				{
					for(CurPos=0,bfind = 0;CurPos<gOptions.MaxUserCount*100 && bfind != 1;CurPos++)
					{

						if(CurAlarmRec[CurPos].PIN==0)
						{
							CurAlarmRec[CurPos].PIN = PIN;
//							CurAlarmRec[CurPos].LastTime = LastTime;
							CurAlarmRec[CurPos].State = State;
							bfind = 1;
							return 1;
						}
					}
				}
			}
			else if(State == 3 || State == 4)
			{
				for(CurPos=0,bfind = 0;CurPos<gOptions.MaxUserCount*100 && bfind != 1;CurPos++)
				{

					if(CurAlarmRec2[CurPos].PIN == PIN)
					{
						CurAlarmRec2[CurPos].PIN = PIN;
//						CurAlarmRec2[CurPos].LastTime = LastTime;
						CurAlarmRec2[CurPos].State = State;
						bfind = 1;
						return 1;
					}
				}
				if(!bfind)//说明记录buffer中没有当前卡的记录，就将
				{
					for(CurPos=0,bfind = 0;CurPos<gOptions.MaxUserCount*100 && bfind != 1;CurPos++)
					{

						if(CurAlarmRec2[CurPos].PIN==0)
						{
							CurAlarmRec2[CurPos].PIN = PIN;
//							CurAlarmRec2[CurPos].LastTime = LastTime;
							CurAlarmRec2[CurPos].State = State;
							bfind = 1;
							return 1;
						}
					}
				}
			}
		}
		break;
		case 4://c3_400:1/2与3/4号门返潜 		 c3_200:1,2号门间反潜
		{
			for(CurPos=0,bfind = 0;CurPos<gOptions.MaxUserCount*100 && bfind != 1;CurPos++)
			{

				if(CurAlarmRec[CurPos].PIN == PIN)
				{
					CurAlarmRec[CurPos].PIN = PIN;
//					CurAlarmRec[CurPos].LastTime = LastTime;
					CurAlarmRec[CurPos].State = State;
					bfind = 1;
					return 1;
				}
			}
			if(!bfind)//说明记录buffer中没有当前卡的记录，就将
			{
				for(CurPos=0,bfind = 0;CurPos<gOptions.MaxUserCount*100 && bfind != 1;CurPos++)
				{

					if(CurAlarmRec[CurPos].PIN==0)
					{
						CurAlarmRec[CurPos].PIN = PIN;
//						CurAlarmRec[CurPos].LastTime = LastTime;
						CurAlarmRec[CurPos].State = State;
						bfind = 1;
						return 1;
					}
				}
			}
		}
		break;
		case 5://c3_400:1与2/3号门返潜	 	c3_200:无
		{
			if(State == 1 || State == 2 || State == 3)
			{
				for(CurPos=0,bfind = 0;CurPos<gOptions.MaxUserCount*100 && bfind != 1;CurPos++)
				{

					if(CurAlarmRec[CurPos].PIN == PIN)
					{
						CurAlarmRec[CurPos].PIN = PIN;
//						CurAlarmRec[CurPos].LastTime = LastTime;
						CurAlarmRec[CurPos].State = State;
						bfind = 1;
						return 1;
					}
				}
				if(!bfind)//说明记录buffer中没有当前卡的记录，就将
				{
					for(CurPos=0,bfind = 0;CurPos<gOptions.MaxUserCount*100 && bfind != 1;CurPos++)
					{

						if(CurAlarmRec[CurPos].PIN==0)
						{
							CurAlarmRec[CurPos].PIN = PIN;
//							CurAlarmRec[CurPos].LastTime = LastTime;
							CurAlarmRec[CurPos].State = State;
							bfind = 1;
							return 1;
						}
					}
				}
			}
		}
		break;
		case 6://c3_400: 1与2/3/4号门返潜		c3_200:无
		{
			for(CurPos=0,bfind = 0;CurPos<gOptions.MaxUserCount*100 && bfind != 1;CurPos++)
			{

				if(CurAlarmRec[CurPos].PIN == PIN)
				{
					CurAlarmRec[CurPos].PIN = PIN;
//					CurAlarmRec[CurPos].LastTime = LastTime;
					CurAlarmRec[CurPos].State = State;
					bfind = 1;
					return 1;
				}
			}
			if(!bfind)//说明记录buffer中没有当前卡的记录，就将
			{
				for(CurPos=0,bfind = 0;CurPos<gOptions.MaxUserCount*100 && bfind != 1;CurPos++)
				{

					if(CurAlarmRec[CurPos].PIN==0)
					{
						CurAlarmRec[CurPos].PIN = PIN;
//						CurAlarmRec[CurPos].LastTime = LastTime;
						CurAlarmRec[CurPos].State = State;
						bfind = 1;
						return 1;
					}
				}
			}
		}
		break;
	}

	return 1;
}


//首卡开门判断
//0 不符合要求，1.符合要求
int process_firstcardopendoor(int DoorID,int pin,TTime currtime,PC3doorkeepOpen c3doorkeepopen)
{
	TC3Firstcardopendoor firstcardinfo;
	int starttime,endtime;

	if(!gOptions.DoorFirstCardOpenDoor[DoorID-1]) //没启用首卡开门功能
		return 0;

	printf("process_firstcardopendoor...................\n");

	TSearchHandle sh;
	char buf[100];

	//一个个比较，找到最后一个为止，必须门ID相等，cardno相等，且当前时间正在此时区
	sh.ContentType=FCT_C3FIRSTCARD;
	sh.buffer = buf;
	SearchFirst(&sh);
	while(!SearchNext(&sh))
	{
//		printf("Firstcard cardno = %d , timezone = %d doorid = %d \n ",
//			((PC3Firstcardopendoor)sh.buffer)->cardno,
//			((PC3Firstcardopendoor)sh.buffer)->timezoneid,
//			((PC3Firstcardopendoor)sh.buffer)->doorID);

		if((sh.datalen>0)
			&& ((PC3Firstcardopendoor)sh.buffer)->pin == pin
			&& ((PC3Firstcardopendoor)sh.buffer)->doorID == DoorID)
		{
			printf("cardno ok DoorID ok   %d,%d \n",pin,DoorID);
			memcpy(&firstcardinfo, (PC3Firstcardopendoor)sh.buffer,sizeof(TC3Firstcardopendoor));
			if(GetTimefromtimezone(firstcardinfo.timezoneid,currtime,&starttime,&endtime))
			{
				c3doorkeepopen->keepOpenSign = 2;
				c3doorkeepopen->keepopenreason = FIRSTCARD;
				c3doorkeepopen->starttime = starttime;
				c3doorkeepopen->endtime = endtime;
				return 1;
			}
		}
	}

	return 0;

}


//比较两个组是否一样, 一样返回1,不一样返回0
int compare_AsemData(TC3Multicardassemble Cardinfo,TC3Multicardassemble Tableinfo)
{
	int i,j,m;

//	将比较的两组数据先排序，大的在前  再比较
	//数据库中内容先排序
	for(i = 0;i< 5;i++)
	{
		for(j = i+1;j< 5;j++)
		{
			if(Tableinfo.group[i] < Tableinfo.group[j])
			{
				m = Tableinfo.group[i];
				Tableinfo.group[i] = Tableinfo.group[j];
				Tableinfo.group[j] = m;
			}
		}
	}

	//	刷卡组中内容排序
	for(i = 0;i< 5;i++)
	{
		for(j = i+1;j< 5;j++)
		{
			if(Cardinfo.group[i] < Cardinfo.group[j])
			{
				m = Cardinfo.group[i];
				Cardinfo.group[i] = Cardinfo.group[j];
				Cardinfo.group[j] = m;
			}
		}
	}

	//两个先排序后再比较，如果比较到第5组或数据库中n项开始为0表示符合多组规则
	for(i = 0;i< 5;i++)
	{
		if(Tableinfo.group[i] != Cardinfo.group[i])
			return 0;
		if(Tableinfo.group[i] == 0)
			break;
	}
	return 1;

}


//TC3Multicardopendoor CurCardgroup;//最近5张刷卡分组组合信息供多卡开门用
//达到多卡开门要求，返回1,否则为0
int process_multicardopendoor(int DoorID,int pin,int group,PC3Multicardopendoor CurCardgroup)
{
	int i = 0;
	int ret = -1;

	TC3Multicardassemble Cardinfo;
	TC3Multicardassemble Tableinfo;

    if(0 == group)
    {
    	return ret;
    }
	//重新计时
	CurCardgroup->MultiCardTimeCount = 10;

//过滤相同卡重复刷卡
	for(i = 0;i< 5;i++)
	{
		if(CurCardgroup->CardGroup[i].PIN == pin &&  CurCardgroup->CardGroup[i].group == group)
			return 0;
	}

	//刷卡数据信息进队列
	for(i = 0;i< 4;i++)			//旧数据被数据覆盖
		memcpy(&CurCardgroup->CardGroup[i],&CurCardgroup->CardGroup[i+1],sizeof(TC3Cardgroup));
	//当前新数据进来
	CurCardgroup->CardGroup[4].PIN = pin;
	CurCardgroup->CardGroup[4].group = group;


	//获取比较组信息
	for(i = 0;i< 5;i++)
		Cardinfo.	group[i] = CurCardgroup->CardGroup[i].group;

	printf("DoorID=%d,Cardinfo->group:%d,%d,%d,%d,%d\n ",DoorID,
		Cardinfo.	group[0],Cardinfo.group[1],	Cardinfo.	group[2],Cardinfo.group[3],	Cardinfo.	group[4]);

	TSearchHandle sh;
	char buf[100];

	sh.ContentType=FCT_C3MULTICARDOPENDOOR;
	sh.buffer = buf;
	SearchFirst(&sh);
	while(!SearchNext(&sh))
	{

		if(sh.datalen>0 && ((PC3Multicardassemble)sh.buffer)->doorID == DoorID)
		{
			printf("Table->group:  index=%d,DoorID=%d, %d,%d,%d,%d,%d\n ",
				((PC3Multicardassemble)sh.buffer)->index,
				((PC3Multicardassemble)sh.buffer)->doorID,
				((PC3Multicardassemble)sh.buffer)->group[0],
				((PC3Multicardassemble)sh.buffer)->group[1],
				((PC3Multicardassemble)sh.buffer)->group[2],
				((PC3Multicardassemble)sh.buffer)->group[3],
				((PC3Multicardassemble)sh.buffer)->group[4]);
			printf("-----------------------------group = %d\n",group);
			if(group == ((PC3Multicardassemble)sh.buffer)->group[0] || group == ((PC3Multicardassemble)sh.buffer)->group[1]
			                || group == ((PC3Multicardassemble)sh.buffer)->group[2] || group == ((PC3Multicardassemble)sh.buffer)->group[3]
			                    || group == ((PC3Multicardassemble)sh.buffer)->group[4])
			{
				ret = 0;
			}

			memcpy(&Tableinfo,(PC3Multicardassemble)sh.buffer,sizeof(TC3Multicardassemble));
			if(compare_AsemData(Cardinfo,Tableinfo))
			{
				return 1;
			}
		}
	}

	return ret;
}



//联动控制
void LinkControl( BYTE EventType,BYTE InAddr)
{
	TSearchHandle sh;
	char buf[100];




	printf("LinkControl EventType=%d,InAddr=%d\n ",EventType & 0xff, InAddr & 0xff);

	sh.ContentType=FCT_C3INOUTFUN;
	sh.buffer = buf;
	SearchFirst(&sh);

	while(!SearchNext(&sh))
	{
			printf("INOUTFUN:Index=%d,EventType=%d,InAddr=%d,OutType=%d,OutAddr=%d,OutTime=%d,%d\n ",
			((PC3InOutFunDefine)sh.buffer)->index,
			((PC3InOutFunDefine)sh.buffer)->EventType,
			((PC3InOutFunDefine)sh.buffer)->InAddr,
			((PC3InOutFunDefine)sh.buffer)->OutType,
			((PC3InOutFunDefine)sh.buffer)->OutAddr,
			((PC3InOutFunDefine)sh.buffer)->OutTime,
			((PC3InOutFunDefine)sh.buffer)->Reserved);

			if(sh.datalen>0 && ((PC3InOutFunDefine)sh.buffer)->EventType == EventType
			   && (((PC3InOutFunDefine)sh.buffer)->InAddr == 0 || ((PC3InOutFunDefine)sh.buffer)->InAddr == InAddr))
		//事件类型 和 事件地点都相等    事件地点为0 表示所有
		    {
			//保存记录到数据库, 触发联动事件类型附用验证方式,联动ID附用到卡号
			   FDB_AddAcessEvenLog(((PC3InOutFunDefine)sh.buffer)->index,0,EventType,InAddr,EVENT_LINKCONTROL,2,OldEncodeTime(&gCurTime));
			//实时记录
			   AppendRTLogBuff(((PC3InOutFunDefine)sh.buffer)->index,0,EventType,InAddr,EVENT_LINKCONTROL,2,OldEncodeTime(&gCurTime));

			   printf("OK find one LinkControl-----------------------\n ");
			   if(((PC3InOutFunDefine)sh.buffer)->OutType == 0)//输出类型  0为门锁，1为辅助输出
			   {
				   if(0 == CommKeepOpenSign[((PC3InOutFunDefine)sh.buffer)->OutAddr-1] &&
						   (0 == pollcard_struct_variable.c3doorkeepopen[((PC3InOutFunDefine)sh.buffer)->OutAddr-1].keepOpenSign))
				   {
					   Ex_ConctrolLock(LINKCON,gOptions.MachineType,((PC3InOutFunDefine)sh.buffer)->OutAddr,((PC3InOutFunDefine)sh.buffer)->OutTime);//驱动电锁
				   }
				   if(255 == ((PC3InOutFunDefine)sh.buffer)->OutTime)
				   {
					   CommKeepOpenSign[((PC3InOutFunDefine)sh.buffer)->OutAddr-1] = 1;
					   pollcard_struct_variable.c3doorkeepopen[((PC3InOutFunDefine)sh.buffer)->OutAddr-1].keepOpenSign = 1;//常开标记
				   }
				   else if(0 == ((PC3InOutFunDefine)sh.buffer)->OutTime && 0 == CommKeepOpenSign[((PC3InOutFunDefine)sh.buffer)->OutAddr-1])
				   {
					   Ex_ConctrolLock(LINKCON,gOptions.MachineType,((PC3InOutFunDefine)sh.buffer)->OutAddr,((PC3InOutFunDefine)sh.buffer)->OutTime);//驱动电锁
					   pollcard_struct_variable.c3doorkeepopen[((PC3InOutFunDefine)sh.buffer)->OutAddr-1].keepOpenSign = 0;
				   }

			   }
			   else
			   {
				   Ex_ConctrolAlarm(gOptions.MachineType,
				 	 ((PC3InOutFunDefine)sh.buffer)->OutAddr,
					 ((PC3InOutFunDefine)sh.buffer)->OutTime);
			   }


		   }
	}
}


int GetDoorState(void)
{
	U8 getstate[4];
	int i,DoorState;

	// 1号门情况
	if(gOptions.MachineType == C3_200 || gOptions.MachineType == C3_100 || gOptions.MachineType == C3_400To_200)
	{
		getstate[0] =  (gOptions.DoorDetectortype[0]==1)?  ExGetState(INPUT,MCU1,3) :!ExGetState(INPUT,MCU1,3);
	}
	else if(gOptions.MachineType == C3_400)
	{
		getstate[0] =  (gOptions.DoorDetectortype[0]==1)?  ExGetState(INPUT,MCU1,3) :!ExGetState(INPUT,MCU1,3);
	}
	else if(gOptions.MachineType == C4 || gOptions.MachineType == C4_200 || gOptions.MachineType == C4_400To_200)
	{
		getstate[0] =  (gOptions.DoorDetectortype[0]==1)?  ExGetState(INPUT,MCU1,2) :!ExGetState(INPUT,MCU1,2);
	}
	// 2号门情况
	if(gOptions.MachineType == C3_200)
	{
		getstate[1] =  (gOptions.DoorDetectortype[1]==1)?  ExGetState(INPUT,MCU2,3) :!ExGetState(INPUT,MCU2,3);
	}
	else if(gOptions.MachineType == C3_400)
	{
		getstate[1] =  (gOptions.DoorDetectortype[1]==1)?  ExGetState(INPUT,MCU1,4) :!ExGetState(INPUT,MCU1,4);
	}
	else if(gOptions.MachineType == C4 || gOptions.MachineType == C4_200)
	{
		getstate[1] =  (gOptions.DoorDetectortype[1]==1)?  ExGetState(INPUT,MCU2,2) :!ExGetState(INPUT,MCU2,2);
	}
	else if(gOptions.MachineType == C3_400To_200)
	{
		getstate[1] =  (gOptions.DoorDetectortype[1]==1)?  ExGetState(INPUT,MCU2,3) :!ExGetState(INPUT,MCU2,3);
	}
	else if(gOptions.MachineType == C4_400To_200)
	{
		getstate[1] =  (gOptions.DoorDetectortype[1]==1)?  ExGetState(INPUT,MCU3,2) :!ExGetState(INPUT,MCU3,2);
	}
	// 3号门情况
	if(gOptions.MachineType == C3_400)
	{
		getstate[2] =  (gOptions.DoorDetectortype[2]==1)?  ExGetState(INPUT,MCU2,3) :!ExGetState(INPUT,MCU2,3);
	}
	else if(gOptions.MachineType == C4)
	{
		getstate[2] =  (gOptions.DoorDetectortype[2]==1)?  ExGetState(INPUT,MCU3,2) :!ExGetState(INPUT,MCU3,2);
	}
	// 4号门情况
	if(gOptions.MachineType == C3_400)
	{
		getstate[3] =  (gOptions.DoorDetectortype[3]==1)?  ExGetState(INPUT,MCU2,4) :!ExGetState(INPUT,MCU2,4);
	}
	else if(gOptions.MachineType == C4)
	{
		getstate[3] =  (gOptions.DoorDetectortype[3]==1)?  ExGetState(INPUT,MCU4,2) :!ExGetState(INPUT,MCU4,2);
	}

	for(i = 0;i< 4;i++)
	{
		//DATA[0]=继电器编号（从1开始编号）；DATA[1]=继电器的动作，0x00表示关，0xff表示开。
		if(getstate[i])//将开状态0xff转化为1
		{
			getstate[i]= 1;
		}
		getstate[i]= getstate[i] +1;//0为无门磁状态，1为关，2为开

		if(gOptions.DoorDetectortype[i] == 0 )//没有门磁
		{
			getstate[i] = 0;
		}
	}

	DoorState = getstate[0] | (getstate[1] << 8) | (getstate[2] << 16)  | (getstate[3] << 24) ;

	/*printf("DoorSensorType:  0x%x,0x%x,0x%x,0x%x\n",
		gOptions.DoorDetectortype[0]&0xff,
		gOptions.DoorDetectortype[1]&0xff,
		gOptions.DoorDetectortype[2]&0xff,
		gOptions.DoorDetectortype[3]&0xff);*/

	printf("sa 0x%x,0x%x,0x%x,0x%x,DoorState = 0x%08x\n",getstate[0]&0xff,
		getstate[1]&0xff,getstate[2]&0xff,getstate[3]&0xff,DoorState);

	return DoorState;
}


//0为无门状态，1为关，2为开
BYTE GetOneDoorState(BYTE DoorID)
{
	int AllDoorState;
	BYTE OneDoorState;
	AllDoorState = GetDoorState();

	OneDoorState = (AllDoorState >> ((DoorID-1) * 8) ) & 0xff;
	printf("GetOneDoorState: DoorID = %d,state = %d\n",DoorID & 0xff,OneDoorState & 0xff);

	return OneDoorState;
}
//获取继电器状态
int GetRelayState(int doorID)
{
	int state = 0;
	if(gOptions.MachineType == C3_200 || gOptions.MachineType == C3_400To_200)
	{
		state = ExGetState(OUTPUT,(MCU1 + doorID -1),1);
	}
	else if(gOptions.MachineType == C3_400 || gOptions.MachineType == C3_100)
	{
		state = ExGetState(OUTPUT,(MCU1 + (doorID -1)/2),1 + (doorID -1)%2);
	}
	else if(gOptions.MachineType == C4 || gOptions.MachineType == C4_200)
	{
		state =  ExGetState(OUTPUT,(MCU1 + doorID -1),1);
	}
	else if(gOptions.MachineType == C4_400To_200)
	{
		state =  ExGetState(OUTPUT,(MCU1 + (doorID -1)*2),1);
	}
	return state;
}

void GetDeviceInState(char *state)//获取各个in口的状态
{
	U8 Door[4],OpenDoorKey[4],AuxIn[4];

	if(gOptions.MachineType == C3_200 || gOptions.MachineType == C3_100)
	{
		//C3_200: 2个单片机   双门双向      In  index: 3为门磁  1为按键开门   5 扩展in
		Door[0] = ExGetState(INPUT,MCU1,3);
		Door[1] = ExGetState(INPUT,MCU2,3);
		Door[2] = 0;
		Door[3] = 0;

		OpenDoorKey[0] = ExGetState(INPUT,MCU1,1);
		OpenDoorKey[1] = ExGetState(INPUT,MCU2,1);
		OpenDoorKey[2] = 0;
		OpenDoorKey[3] = 0;

		AuxIn[0] = ExGetState(INPUT,MCU1,5);
		AuxIn[1] = ExGetState(INPUT,MCU2,5);
		AuxIn[2] = 0;
		AuxIn[3] = 0;
	}
	else if(gOptions.MachineType == C3_400To_200)
	{
		//C3_400To_200: 2个单片机  双门双向   In  index: 3 为门磁  1 为按键开门    5  6扩展in
		Door[0] = ExGetState(INPUT,MCU1,3);
		Door[1] = ExGetState(INPUT,MCU2,3);
		Door[2] = 0;
		Door[3] = 0;

		OpenDoorKey[0] = ExGetState(INPUT,MCU1,1);
		OpenDoorKey[1] = ExGetState(INPUT,MCU2,1);
		OpenDoorKey[2] = 0;
		OpenDoorKey[3] = 0;

		AuxIn[0] = ExGetState(INPUT,MCU1,5);
		AuxIn[1] = ExGetState(INPUT,MCU1,6);
		AuxIn[2] = ExGetState(INPUT,MCU2,5);
		AuxIn[3] = ExGetState(INPUT,MCU2,6);
	}
	else if(gOptions.MachineType == C3_400)
	{
		//C3_400: 2个单片机  4门单向In  index: 3 4为门磁  1 2为按键开门    5  6扩展in
		Door[0] = ExGetState(INPUT,MCU1,3);
		Door[1] = ExGetState(INPUT,MCU1,4);
		Door[2] = ExGetState(INPUT,MCU2,3);
		Door[3] = ExGetState(INPUT,MCU2,4);

		OpenDoorKey[0] = ExGetState(INPUT,MCU1,1);
		OpenDoorKey[1] = ExGetState(INPUT,MCU1,2);
		OpenDoorKey[2] = ExGetState(INPUT,MCU2,1);
		OpenDoorKey[3] = ExGetState(INPUT,MCU2,2);

		AuxIn[0] = ExGetState(INPUT,MCU1,5);
		AuxIn[1] = ExGetState(INPUT,MCU1,6);
		AuxIn[2] = ExGetState(INPUT,MCU2,5);
		AuxIn[3] = ExGetState(INPUT,MCU2,6);
	}
	else if(gOptions.MachineType == C4)
	{
		//C4: 5个单片机   4门单向    其中一个用作ui驱动显示      In  index: 1为按键开门，  2为门磁
		Door[0] = ExGetState(INPUT,MCU1,2);
		Door[1] = ExGetState(INPUT,MCU2,2);
		Door[2] = ExGetState(INPUT,MCU3,2);
		Door[3] = ExGetState(INPUT,MCU4,2);

		OpenDoorKey[0] = ExGetState(INPUT,MCU1,1);
		OpenDoorKey[1] = ExGetState(INPUT,MCU2,1);
		OpenDoorKey[2] = ExGetState(INPUT,MCU3,1);
		OpenDoorKey[3] = ExGetState(INPUT,MCU4,1);
		//好像单片机取值有些问题
		AuxIn[0] = ExGetState(INPUT,MAINMCU,1);
		AuxIn[1] = ExGetState(INPUT,MAINMCU,2);
		AuxIn[2] = ExGetState(INPUT,MAINMCU,3);
		AuxIn[3] = ExGetState(INPUT,MAINMCU,4);
	}
	else if(gOptions.MachineType == C4_400To_200)
	{
		//C4: 5个单片机   4门单向    其中一个用作ui驱动显示      In  index: 1为按键开门，  2为门磁
		Door[0] = ExGetState(INPUT,MCU1,2);
		Door[1] = ExGetState(INPUT,MCU3,2);
		Door[2] = 0;
		Door[3] = 0;

		OpenDoorKey[0] = ExGetState(INPUT,MCU1,1);
		OpenDoorKey[1] = ExGetState(INPUT,MCU3,1);
		OpenDoorKey[2] = 0;
		OpenDoorKey[3] = 0;
		//好像单片机取值有些问题
		AuxIn[0] = ExGetState(INPUT,MAINMCU,1);
		AuxIn[1] = ExGetState(INPUT,MAINMCU,2);
		AuxIn[2] = ExGetState(INPUT,MAINMCU,3);
		AuxIn[3] = ExGetState(INPUT,MAINMCU,4);
	}
	else if(gOptions.MachineType == C4_200)
	{
		//C4: 5个单片机   4门单向    其中一个用作ui驱动显示      In  index: 1为按键开门，  2为门磁
		Door[0] = ExGetState(INPUT,MCU1,2);
		Door[1] = ExGetState(INPUT,MCU2,2);
		Door[2] = 0;
		Door[3] = 0;

		OpenDoorKey[0] = ExGetState(INPUT,MCU1,1);
		OpenDoorKey[1] = ExGetState(INPUT,MCU2,1);
		OpenDoorKey[2] = 0;
		OpenDoorKey[3] = 0;
		//好像单片机取值有些问题
		AuxIn[0] = ExGetState(INPUT,MAINMCU,1);
		AuxIn[1] = ExGetState(INPUT,MAINMCU,2);
		AuxIn[2] = ExGetState(INPUT,MAINMCU,3);
		AuxIn[3] = ExGetState(INPUT,MAINMCU,4);
	}

	sprintf(state,"sensor:%d-%d-%d-%d,Button:%d-%d-%d-%d,AuxIn:%d-%d-%d-%d",
		Door[0] & 0xff,Door[1] & 0xff,Door[2] & 0xff,Door[3] & 0xff,
		OpenDoorKey[0] & 0xff,OpenDoorKey[1] & 0xff,OpenDoorKey[2] & 0xff,OpenDoorKey[3] & 0xff,
		AuxIn[0] & 0xff,AuxIn[1] & 0xff,AuxIn[2] & 0xff,AuxIn[3] & 0xff);
}
//初始化参数
int DoRestoreDefaultOptions(void)
{
        SaveOptions(GetDefaultOptions(&gOptions));
        ClearOptionItem("NONE");
        LoadOptions(&gOptions);
        //mmsleep(3*1000);
        //ShowMenu((PMenu)((PMsg)p)->Object);
        return 1;
}


