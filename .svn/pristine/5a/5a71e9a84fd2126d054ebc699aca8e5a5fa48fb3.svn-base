/*************************************************
  
 ZEM 200                                          
 
 kb.c scan the keypad and get keypad value                             
 
 Copyright (C) 2003-2005, ZKSoftware Inc.      		
 
*************************************************/

#include <string.h>
#include "arca.h"
#include "kb.h"
#include "exfun.h"
#include "options.h"
#include "utils.h"

#define MAXKEYBUFFERLEN 5

static int PressKeyBuffer[MAXKEYBUFFERLEN] = {-1, -1, -1, -1, -1};
static int CurrentPos = -1;

unsigned char KeyLayouts[4][FunKeyCount]={
	//0   1   2   3   4   5   6   7   8   9 MENU OK  ESC UP DOWN PWR OTI OTO TIN TOU IN OUT BELL DURESS
	{'0','1','2','3','4','5','6','7','8','9','D','#','A','B','C','*','3','2','1','B','I','J','J','C','K'},  	//BioClockI II
	{'0','1','2','3','4','5','6','7','8','9','#','B','A','C','D','*','A','B','C','D','*','#','J','D','K'}, 	//A5, K8, K9
	{'0','1','2','3','4','5','6','7','8','9','D','#','A','B','C','*','A','B','C','D','*','#','J','C','K'}, 	//F4+, F8
	{'0','1','2','3','4','5','6','7','8','9','H','I','E','G','F','B','D','C','#','A','*','B','J','F','K'} 	//A6
};	

unsigned char ConvertWiegandKeyValueToASCII(int WiegandValue)
{
	int MachineKeyPad=0;
	unsigned char WiegandKeyPad[2][FunKeyCount+1]={
		{0,'1','2','3','A','4','5','6','B','7','8','9','C','*','0','#','D','*','#','A','B','C','D'},  	//A5, K8, F4+, F8
		{0,'1','2','3','4','5','6','7','8','9','*','#','A','B','C','D','E','F','G','H','I','J','0'}  	//A6
	}; 
	if (WiegandValue>FunKeyCount) return 0;
	if (gOptions.KeyLayout==KeyLayout_A6) 
		MachineKeyPad=1;
	else
		MachineKeyPad=0;	    
	return (WiegandKeyPad[MachineKeyPad][WiegandValue]);    
}

#define MAX_KEY_INTERVAL  50*1000   //50ms

void WriteKeyToBuffer(unsigned char KeyValue)
{
	CurrentPos = CurrentPos + 1;
	if (CurrentPos >= MAXKEYBUFFERLEN)
		CurrentPos = 0;
	PressKeyBuffer[CurrentPos] = KeyValue;
	//printf("%c %d\n", KeyValue,gOptions.KeyLayout);
	if(gOptions.KeyPadBeep&&KeyValue) ExBeep(1);
}

void GetKeyFromWiegand(int WiegandKey)
{
	WriteKeyToBuffer(ConvertWiegandKeyValueToASCII(WiegandKey));
}

BOOL GetKeyFromBuffer(unsigned char *KeyValue)
{
	int i, j;
	
	j = CurrentPos + 1;
	for (i = 0; i < MAXKEYBUFFERLEN; i++){
		if (j >= MAXKEYBUFFERLEN)
			j = 0;
		if (PressKeyBuffer[j] != -1){
			*KeyValue = (unsigned char)PressKeyBuffer[j];  
			PressKeyBuffer[j] = -1;
			return TRUE; 
		}
		j++;
	}
	return FALSE;
}

//ת��ԭʼ���к���Ϊ������ ���д���
//Convert the origin number to key value according the following below
//Keypad use 4 pin GPIO for out and 4 pin GPIO for in (refer line 43-45 in arca.c)
//1   	2	3	A
//4	5 	6	B
//7	8	9 	C
//*	0      #     	D
unsigned char ConvertKeyValueToASCII(unsigned char bits)
{
	unsigned char KEYMAPTABLE[4][4] = {{'1', '2', '3', 'A'}, {'4', '5', '6', 'B'},
					   {'7', '8', '9', 'C'}, {'*', '0', '#', 'D'}};
	unsigned char biti[4]={0x70, 0xb0, 0xd0, 0xe0};
	int i, row, col;
	
	row = 0;
	for(i=0; i<4; i++)
		if ((bits&0xf0) == biti[i]){
		row = 3-i;
		break;
	}
	
	col = 0;
	for(i=0; i<4; i++)
		if ((bits&0x0f) == (biti[i]>>4)){
		col = i;
		break;
	}
	return KEYMAPTABLE[row][col];
}

//��Ⲣ���ص�ǰ�ļ�ֵ �ɿ�����ʱ���ؼ�ֵ=0
//detect and return the current keypad value, when release the key, the result is 0 
BOOL CheckKeyPad(void)
{
	unsigned char biti[4]={0x70, 0xb0, 0xd0, 0xe0};
	int i;
	unsigned char highbits;
	static unsigned char LastestKeyValue = 0;
	
	//û�а��� 
	//there is no key pressed
	if (LastestKeyValue == 0){
		for(i=0; i<4; i++){
			GPIO_KEY_SET(biti[i]);
			DelayUS(1000);	
			//����Ƿ��м���
			//check whether key press or not
			if ((highbits = GPIO_KEY_GET()&0x0f)!=0x0f){
				//��ֹ���� 
				DelayUS(MAX_KEY_INTERVAL);
				if ((highbits = GPIO_KEY_GET()&0x0f)==0x0f) break; 
				LastestKeyValue = ConvertKeyValueToASCII((unsigned char)highbits + biti[i]);
				WriteKeyToBuffer(LastestKeyValue);
				return TRUE;
			}
		}
	}else{ //����Ƿ��ɿ����� 
		//detect whether release the key or not
		DelayUS(1000);
		if ((GPIO_KEY_GET()&0x0f)==0x0f){
			LastestKeyValue = 0;
			//֪ͨ������Ѿ��ɿ����� 
			//key have released and then notify main procedure
			WriteKeyToBuffer(LastestKeyValue);
			return TRUE;
		}
	}
	return FALSE;
}

int GetKeyPadValue(unsigned char *KeyValue)
{
	return GetKeyFromBuffer(KeyValue);
}

int GetStateKeyCode(int keychar)
{
	int i;
	int key;
	int MaxState;
	if(gOptions.ShowCheckIn) MaxState=IKeyOut; else MaxState=IKeyTOut;
	if(keychar>='0') keychar-='0';
	if(keychar<FunKeyCount)  
	{
		key=KeyLayouts[gOptions.KeyLayout][keychar];
		for(i=IKeyOTIn;i<=MaxState;i++)
			if(key==KeyLayouts[gOptions.KeyLayout][i])
				return i;
	}
	return 0;
}

//�ú���ذ���Ķ�Ӧ���ܣ�SecondFun���صڶ�����
int GetKeyChar(int key, int *SecondFun)
{
        int i,ret=key,j;
        *SecondFun=0;
        for(i=0;i<FunKeyCount;i++)
                if(KeyLayouts[gOptions.KeyLayout][i]==key)
                {
                        if(i<10)
                                ret='0'+i;
                        else
                                ret=i;
                        for(j=(i>9?i+1:10);j<FunKeyCount;j++)
                                if(KeyLayouts[gOptions.KeyLayout][j]==key)
                                if(j!=IKeyDuress || gOptions.DuressHelpKeyOn)
                                if((gOptions.ShowState && (gOptions.ShowCheckIn||(j<IKeyIn))) || j>IKeyOut || j<IKeyOTIn )
                                {
		   			if(gOptions.PageState==1 || gOptions.PageState==3)
	 				{
						if(j==IKeyIn)
						{
							switch(gOptions.AttState)
							{
								case -1:
									*SecondFun=IKeyIn;
									break;
								case 0:
									*SecondFun=IKeyOut;
									break;
								case 1:
									*SecondFun=IKeyOTIn;
									break;
								case 2:
									*SecondFun=IKeyOTOut;
									break;
								case 3:
									*SecondFun=-1;
									break;
								default:
									*SecondFun=j;
									break;
							}
						}
						else if(j==IKeyOut)
						{
							switch(gOptions.AttState)
							{
								case -1:
									*SecondFun=IKeyOTOut;
									break;
								case 0:
									*SecondFun=-1 ;
									break;
								case 1:
									*SecondFun=IKeyIn;
									break;
								case 2:
									*SecondFun=IKeyOut;
									break;
								case 3:
									*SecondFun=IKeyOTIn;
									break;
						
								default:
									*SecondFun=j;
									break;
							}
				
						}
						else
							*SecondFun=j;
		
					}
					else if(gOptions.PageState==2)
					{
						BYTE States[6]={IKeyIn, IKeyOut, IKeyOTIn, IKeyOTOut, IKeyTIn, IKeyTOut};
						int state=j;
						if(j==IKeyIn)
							state=(gOptions.AttState+2)%7-1;
						else if(j==IKeyOut)
							state=(gOptions.AttState+7)%7-1;
						if(state>=0 && state<6)
							state=States[state];
						*SecondFun=state;
					}else
						*SecondFun=j;
					break;
				}
			break;
		}
	if(*SecondFun && gMachineState!=STA_MENU)
        {
		//DBPRINTF("ret=%d\n",ret);
                //��Դ��ͨ���ǵڶ����ܣ����ǵ�һ����
                if(IKeyPower==ret)
                {
                        ret=*SecondFun;
                        *SecondFun=IKeyPower;
                }
                //�˵���ͨ���ǵڶ����ܣ����ǵ�һ����
                else if(IKeyMenu==ret)
                {
                        ret=*SecondFun;
                        *SecondFun=IKeyMenu;
                }
        }
	DBPRINTF("return ret=%d,SecondFun=%d\n",ret,*SecondFun);
        return ret;
}

int GetPowerKeyCode(void)
{
	return KeyLayouts[gOptions.KeyLayout][IKeyPower];
}

void SetKeyLayouts(char *Buffer)
{
	int index, j;
	index=0;
	
	for(j=0;j<(int)strlen(Buffer);j+=2)
		KeyLayouts[gOptions.KeyLayout][index++]=Hex2Char(Buffer+j)*16+Hex2Char(Buffer+j+1);
}

