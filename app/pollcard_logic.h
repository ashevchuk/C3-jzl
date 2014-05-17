//刷卡间隔判断 1，OK，间隔已经到，可以继续处理，0，停止处理


int process_interval(int *time_count,int *timer_Preset);
int process_card_valid_time(int start_time,int end_time,int cur_time);int process_door_valid_time(int timeID,TTime curtime);
int ProcessAPBLastTime(PLastTimeAPB CurLastTimeAPB,U32 PIN,int LastTimeAPB,time_t curTime);
int process_card_AntiPassback(PAlarmRec CurAlarmRec,PAlarmRec CurAlarmRec2,int PIN,U8 State);
int process_mutiLock_linkage(int state);
int add_Rec_CurAlarmRec(PAlarmRec CurAlarmRec,PAlarmRec CurAlarmRec2,int PIN,time_t LastTime,U8  State,U32 workcode);
int process_firstcardopendoor(int DoorID,int pin,TTime currtime,PC3doorkeepOpen c3doorkeepopen);
int process_multicardopendoor(int DoorID,int pin,int group,PC3Multicardopendoor CurCardgroup) ;
void LinkControl( BYTE EventType,BYTE InAddr);

int GetDoorState(void);
int DoRestoreDefaultOptions(void);
void GetDeviceInState(char *state);
//0为无门状态，1为关，2为开
BYTE GetOneDoorState(BYTE DoorID);
int GetRelayState(int doorID);


