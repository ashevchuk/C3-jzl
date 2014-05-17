#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ccc.h"
#include "msg.h"
#include "utils.h"
#include "lcdmenu.h"
#include "lcm.h"
#include "locale.h"
#include "kb.h"
#include "options.h"
#include "flashdb.h"

int InputTimeOut=0;

#define MENU_ITEM_CHARWIDTH 32


extern unsigned char Icon_Menu_Option[];
extern unsigned char Icon_Menu_AttLog[];
extern unsigned char Icon_Menu_User[];
extern unsigned char Icon_Menu_Info[];
extern unsigned char Icon_Menu_Finger[];

char* MenuFmtStr(char *buf, int StrID, char *Value)
{
        return PadRightStrSID(buf, StrID, NULL, Value, MenuCharWidth);
}

char* MenuFmtInt(char *buf, int StrID, int Value)
{
        return PadRightIntSID(buf, StrID, NULL, Value, MenuCharWidth);
}


char* MenuFmtStrDef(char *buf, int StrID, const char *DefStr, char *Value)
{
        return PadRightStrSID(buf, StrID, DefStr, Value, MenuCharWidth);
}

char* MenuFmtIntDef(char *buf, int StrID, const char *DefStr, int Value)
{
        return PadRightIntSID(buf, StrID, DefStr, Value, MenuCharWidth);
}


char *MenuFmtStrStr(char *buf, int width, char *Value)
{
/*
       char *p=GetNextText(buf, (MenuCharWidth-width)*gLangDriver->CharWidth);
        if(p) *p=0;
        return PadRightStrStr(buf, buf, Value, MenuCharWidth);
*/
        if(gLangDriver->RightToLeft)
        {
                char *p=GetNextText(buf, width*gLangDriver->CharWidth);
                if(p) while(*p==' ') p++;
                return PadRightStrStr(buf, p, Value, MenuCharWidth);
        }
        else
        {
                char *p=GetNextText(buf, (MenuCharWidth-width)*gLangDriver->CharWidth);
                if(p) *p=0;
                return PadRightStrStr(buf, buf, Value, MenuCharWidth);
        }

}

char *MenuFmtStrInt(char *buf, int width, int Value)
{
/*
       char vbuf[20];
        char *p=GetNextText(buf, (MenuCharWidth-width)*gLangDriver->CharWidth);
        if(p) *p=0;
        sprintf(vbuf,"%d",Value);
        return PadRightStrStr(buf, buf, vbuf, MenuCharWidth);
*/
        char vbuf[20]={0};
        char *p;
        if(gLangDriver->RightToLeft)
        {
                p=GetNextText(buf, width*gLangDriver->CharWidth);
                if(p) while(*p==' ') p++;
	        sprintf(vbuf,"%d",Value);
                return PadRightStrStr(buf, p, vbuf, MenuCharWidth);
        }
        else
        {
                p=GetNextText(buf, (MenuCharWidth-width)*gLangDriver->CharWidth);
                if(p) *p=0;
                sprintf(vbuf,"%d",Value);
                return PadRightStrStr(buf, buf, vbuf, MenuCharWidth);
        }


}

int InputValueOfItem(PMsg p, int width, int minv, int maxv, int *OptionValue)
{
	PMenu menu=((PMenu)p->Object);
	int row=CalcMenuItemOffset(menu, menu->ItemIndex), ret, Value;
	int col=gLCDCharWidth-width;
	Value=*OptionValue;
	do
	{
		ret=InputNumber(row,col,width,&Value,minv,maxv,FALSE);
		if(News_CancelInput==ret || News_TimeOut==ret) return ret;
	}while(ret==News_ErrorInput);
	// News_CommitInput
	if(Value!=*OptionValue)
	{
		/*
                MenuFmtStrInt(menu->Items[menu->ItemIndex].Caption,width,Value);
		*OptionValue=Value;
		gOptions.Saved=FALSE;
		*/
                if(!gLangDriver->RightToLeft||
                        (gLangDriver->GetTextWidthFun(gLangDriver,menu->Items[menu->ItemIndex].Caption)/gLangDriver->CharWidth>=MenuCharWidth))
                        MenuFmtStrInt(menu->Items[menu->ItemIndex].Caption,width,Value);
                else
                {
                        char value[20]={0};
			/*int i;
			if(gOptions.CardkeyKeypad)
			{
				for(i=0;i<width;i++)
					value[i]="*";
			}
			else*/
				sprintf(value, "%d", Value);
                        PadRightStrStr(menu->Items[menu->ItemIndex].Caption, menu->Items[menu->ItemIndex].Caption, value, MenuCharWidth);
			DBPRINTF("PadRight\n");
                }
                *OptionValue=Value;
                gOptions.Saved=FALSE;

	}
	return ret;
}


int InputYesNoItem(PMsg p, int *OptionValue)
{
	char Items[MAX_CHAR_WIDTH];
	PMenu menu=((PMenu)p->Object);
	int row=CalcMenuItemOffset(menu, menu->ItemIndex), ret;
	int col=gLCDCharWidth-3, index, values[]={1,0};
	sprintf(Items, "%s:%s", LoadStrByID(HID_YES), LoadStrByID(HID_NO));
	index=*OptionValue;
	ret=LCDSelectItemValue(row,col,3,Items,values,&index);
	if(ret==News_CommitInput)
	{
		if(index!=*OptionValue)
		{
			*OptionValue=index;
			gOptions.Saved=FALSE;
			MenuFmtStrStr(menu->Items[menu->ItemIndex].Caption, 3, GetYesNoName(index));
		}
	}
	return ret;
}


int CalcMenuItemOffset(PMenu menu, int index)
{
	int j=0, cc;
	int ShowTitle=(menu->Title && ((menu->TopIndex<0) || (RowCount>2)));
	if(menu->Title && ShowTitle)
		j++;
	cc=menu->TopIndex;
	if(cc<0)cc=0;
	return index-cc+j;
}

#define LeftStart (MenuIndicatorWidth*gLangDriver->CharWidth/2)

int CheckMenuStyle(PMenu menu)
{
	if(MenuStyle_ICON==menu->Style)
	{
		if(gLCDHeight<64)
			menu->Style=MenuStyle_OLD;
		else
		{
			int i;
			for(i=0;i<menu->Count;i++)
				if(menu->Items[i].Icon==NULL)
				{
					menu->Style=MenuStyle_OLD; break;
				}
		}
	}
	return 0;
}

int DoShowNetIpInfo(void *p)
{
	int ret = 0;
	char *pIpInfo= NULL;

	LCDClear();

	pIpInfo = LoadStrOld("IPAddress");

	if(NULL == pIpInfo)
	{
		pIpInfo = LoadStrByID(MID_NO_SET);
	}

	LCDWriteLineStrID(0,MID_NET_IP);
	LCDWriteCenterStr(1,pIpInfo);
	LCDWriteCenterStrID(3,HID_CONTINUEESC);

	ret = InputLine(0,0,0,NULL);

	if(News_CommitInput == ret)
	{
		ret = News_CancelInput;
	}

	return ret;
}

int DoShowNetMaskInfo(void *p)
{
	int ret = 0;
	char *pMaskInfo = NULL;

	LCDClear();
	pMaskInfo = LoadStrOld("NetMask");

	if(NULL == pMaskInfo)
	{
		pMaskInfo = LoadStrByID(MID_NO_SET);
	}

	LCDWriteLineStrID(0,MID_NETMASK_ADDR);
	LCDWriteCenterStr(1,pMaskInfo);
	LCDWriteCenterStrID(3,HID_CONTINUEESC);

	ret = InputLine(0,0,0,NULL);

	if(News_CommitInput == ret)
	{
		ret = News_CancelInput;
	}

	return ret;
}

int DoShowNetGateIpInfo(void *p)
{
	int ret = 0;
	char *pGateIpInfo = NULL;

	LCDClear();
	pGateIpInfo = LoadStrOld("GATEIPAddress");

	if(NULL == pGateIpInfo)
	{
		pGateIpInfo = LoadStrByID(MID_NO_SET);
	}

	LCDWriteLineStrID(0,MID_GATEWAY_IP);
	LCDWriteCenterStr(1,pGateIpInfo);
	LCDWriteCenterStrID(3,HID_CONTINUEESC);

	ret = InputLine(0,0,0,NULL);

	if(News_CommitInput == ret)
	{
		ret = News_CancelInput;
	}

	return ret;
}

int DoShowNetMACInfo(void *p)
{
	int ret = 0;
	char *pMACInfo = NULL;
	char MACInfoBuf[24] = {0};

	LCDClear();
	pMACInfo = LoadStrOld("MAC");

	LCDWriteLineStrID(0,MID_NET_MAC);
	if(NULL != pMACInfo)
	{
		memcpy(MACInfoBuf, pMACInfo, 9);
		LCDWriteCenterStr(1, MACInfoBuf);

		memset(MACInfoBuf, 0 , sizeof(MACInfoBuf));
		memcpy(MACInfoBuf, pMACInfo+9, 8);

		LCDWriteStr(2, 3, MACInfoBuf, 0);
	}
	else
	{
		pMACInfo = LoadStrByID(MID_NO_SET);
		LCDWriteCenterStr(1,pMACInfo);
	}

	LCDWriteCenterStrID(3,HID_CONTINUEESC);

	ret = InputLine(0,0,0,NULL);

	if(News_CommitInput == ret)
	{
		ret = News_CancelInput;
	}

	return ret;
}

int DoShowRs23ComInfo(void *p)
{
	int ret = 0;
	char BaudRateBuf[24] = {0};

	LCDClear();

	sprintf(BaudRateBuf, "%d", gOptions.RS232BaudRate);

	LCDWriteLineStrID(0,MID_OC_BAUDRATE);
	LCDWriteCenterStr(1, BaudRateBuf);
	LCDWriteCenterStrID(3,HID_CONTINUEESC);

	ret = InputLine(0,0,0,NULL);

	if(News_CommitInput == ret)
	{
		ret = News_CancelInput;
	}

	return ret;
}

int DoShowRSAddrss(void *p)
{
	int ret = 0;

	char DeviceIDBuf[24] = {0};

	LCDClear();

	sprintf(DeviceIDBuf, "%d", gOptions.DeviceID);

	LCDWriteLineStrID(0,MID_RS_ADDR);
	LCDWriteCenterStr(1, DeviceIDBuf);
	LCDWriteCenterStrID(3,HID_CONTINUEESC);

	ret = InputLine(0,0,0,NULL);

	if(News_CommitInput == ret)
	{
		ret = News_CancelInput;
	}

	return ret;
}

int DoSetRsAddr(void *p)
{
	int ret = 0;

	InputValueOfItem((PMsg)p, 3, 1, 254, &gOptions.DeviceID);

	return ret;
}

int DoShowMainMCUInfo(void *p)
{
	int ret = 0;
	char *pMainMCUInfo= NULL;
	char SubMCUBuf[10] = {0};

	LCDClear();

	pMainMCUInfo = LoadStrOld("MainMCUVer");

	if(NULL == pMainMCUInfo)
	{
		pMainMCUInfo = "";
	}

	sprintf(SubMCUBuf,"%s",LoadStrByID(MID_MCUVER));
	LCDWriteStr(0, 0, SubMCUBuf, 0);
	LCDWriteCenterStr(1,pMainMCUInfo);
	LCDWriteCenterStrID(3,HID_CONTINUEESC);

	ret = InputLine(0,0,0,NULL);

	if(News_CommitInput == ret)
	{
		ret = News_CancelInput;
	}

	return ret;
}

int DoShowSubMCUVer1Info(void *p)
{
	int ret = 0;
	char *pSubMCUInfo= NULL;
	char SubMCUBuf[10] = {0};

	LCDClear();

	pSubMCUInfo = LoadStrOld("SubMCUVer1");

	if(NULL == pSubMCUInfo)
	{
		pSubMCUInfo = "";
	}

	sprintf(SubMCUBuf,"%s1",LoadStrByID(MID_MCUVER));
	LCDWriteStr(0, 0, SubMCUBuf, 0);
	LCDWriteCenterStr(1,pSubMCUInfo);
	LCDWriteCenterStrID(3,HID_CONTINUEESC);

	ret = InputLine(0,0,0,NULL);

	if(News_CommitInput == ret)
	{
		ret = News_CancelInput;
	}

	return ret;
}

int DoShowSubMCUVer2Info(void *p)
{
	int ret = 0;
	char *pSubMCUInfo= NULL;
	char SubMCUBuf[10] = {0};

	LCDClear();

	pSubMCUInfo = LoadStrOld("SubMCUVer2");

	if(NULL == pSubMCUInfo)
	{
		pSubMCUInfo = "";
	}

	sprintf(SubMCUBuf,"%s2",LoadStrByID(MID_MCUVER));
	LCDWriteStr(0, 0, SubMCUBuf, 0);
	LCDWriteCenterStr(1,pSubMCUInfo);
	LCDWriteCenterStrID(3,HID_CONTINUEESC);

	ret = InputLine(0,0,0,NULL);

	if(News_CommitInput == ret)
	{
		ret = News_CancelInput;
	}

	return ret;
}

int DoShowSubMCUVer3Info(void *p)
{
	int ret = 0;
	char *pSubMCUInfo= NULL;
	char SubMCUBuf[10] = {0};

	LCDClear();

	pSubMCUInfo = LoadStrOld("SubMCUVer3");

	if(NULL == pSubMCUInfo)
	{
		pSubMCUInfo = "";
	}

	sprintf(SubMCUBuf,"%s3",LoadStrByID(MID_MCUVER));
	LCDWriteStr(0, 0, SubMCUBuf, 0);
	LCDWriteCenterStr(1,pSubMCUInfo);
	LCDWriteCenterStrID(3,HID_CONTINUEESC);

	ret = InputLine(0,0,0,NULL);

	if(News_CommitInput == ret)
	{
		ret = News_CancelInput;
	}

	return ret;
}

int DoShowSubMCUVer4Info(void *p)
{
	int ret = 0;
	char *pSubMCUInfo= NULL;
	char SubMCUBuf[10] = {0};

	LCDClear();

	pSubMCUInfo = LoadStrOld("SubMCUVer4");

	if(NULL == pSubMCUInfo)
	{
		pSubMCUInfo = "";
	}

	sprintf(SubMCUBuf,"%s4",LoadStrByID(MID_MCUVER));
	LCDWriteStr(0, 0, SubMCUBuf, 0);
	LCDWriteCenterStr(1,pSubMCUInfo);
	LCDWriteCenterStrID(3,HID_CONTINUEESC);

	ret = InputLine(0,0,0,NULL);

	if(News_CommitInput == ret)
	{
		ret = News_CancelInput;
	}

	return ret;
}

int DoShowMCUVerInfo(void *p)
{
	int ret = 0;
	char SubMCUBuf[10] = {0};

	LCDClear();

	PMenu pDevInfo=CreateMenu(LoadStrByID(MID_MCUVER),gOptions.MenuStyle,NULL);

	AddMenuItem(1,pDevInfo,LoadStrByID(MID_MCU_VER),DoShowMainMCUInfo,NULL);
	sprintf(SubMCUBuf,"%s1",LoadStrByID(MID_SUBMCU_VER));
	AddMenuItem(1,pDevInfo,SubMCUBuf,DoShowSubMCUVer1Info,NULL);

	memset(SubMCUBuf, 0, 10);
	sprintf(SubMCUBuf,"%s2",LoadStrByID(MID_SUBMCU_VER));
	AddMenuItem(1,pDevInfo,SubMCUBuf,DoShowSubMCUVer2Info,NULL);

	if(C4_200 != gOptions.MachineType)
	{
		memset(SubMCUBuf, 0, 10);
		sprintf(SubMCUBuf,"%s3",LoadStrByID(MID_SUBMCU_VER));
		AddMenuItem(1,pDevInfo,SubMCUBuf,DoShowSubMCUVer3Info,NULL);

		memset(SubMCUBuf, 0, 10);
		sprintf(SubMCUBuf,"%s4",LoadStrByID(MID_SUBMCU_VER));
		AddMenuItem(1,pDevInfo,SubMCUBuf,DoShowSubMCUVer4Info,NULL);
	}

	ret=RunMenu(pDevInfo);
	DestroyMenu(pDevInfo);

	return ret;
}

int DoShowUsrInfo(void *p)
{
	int ret = 0;
	int CntUsr = 0;
	char CntUsrBuf[24] = {0};
	char FreeUsrBuf[24] = {0};
	char UsedUsrBuf[24] = {0};

	CntUsr = FDB_CntUser();

	LCDClear();

	sprintf(CntUsrBuf, "%s %d", LoadStrByID(MID_CONTENT), gOptions.MaxUserCount*100);
	sprintf(UsedUsrBuf, "%s %d", LoadStrByID(MID_USED), CntUsr);
	sprintf(FreeUsrBuf, "%s %d", LoadStrByID(MID_FREE), gOptions.MaxUserCount*100 - CntUsr);

	LCDWriteStr(0, 0, CntUsrBuf, 0);
	LCDWriteStr(1, 0, UsedUsrBuf, 0);
	LCDWriteStr(2, 0, FreeUsrBuf, 0);

	LCDWriteCenterStrID(3,HID_CONTINUEESC);

	ret = InputLine(0,0,0,NULL);

	if(News_CommitInput == ret)
	{
		ret = News_CancelInput;
	}

	return ret;
}

int DoShowRecordInfo(void *p)
{
	int ret = 0;
	int CntRecord = 0;
	char CntRecordBuf[24] = {0};
	char FreeRecordBuf[24] = {0};
	char UsedRecordBuf[24] = {0};

	CntRecord = CurGuarderEventLogCount;

	LCDClear();

	sprintf(CntRecordBuf, "%s %d", LoadStrByID(MID_CONTENT), gOptions.MaxAttLogCount*10000);
	sprintf(UsedRecordBuf, "%s %d", LoadStrByID(MID_USED), CntRecord);
	sprintf(FreeRecordBuf, "%s %d", LoadStrByID(MID_FREE), gOptions.MaxAttLogCount*10000 - CntRecord);

	LCDWriteStr(0, 0, CntRecordBuf, 0);
	LCDWriteStr(1, 0, UsedRecordBuf, 0);
	LCDWriteStr(2, 0, FreeRecordBuf, 0);

	LCDWriteCenterStrID(3,HID_CONTINUEESC);
	ret = InputLine(0,0,0,NULL);

	if(News_CommitInput == ret)
	{
		ret = News_CancelInput;
	}

	return ret;
}


int DoShowProTime(void *p)
{
	int ret = 0;
	char buf[MAX_CHAR_WIDTH];
	int i;

	LCDClear();

	strcpy(buf, ProductTime);
        if(buf[10]==' ')
                i=10;
        else
                i=11;
        buf[i]=0;
	LCDWriteLineStrID(0,MID_OI_PT);
	LCDWriteCenterStr(1, buf);
	LCDWriteCenterStr(2, buf+i+1);
	LCDWriteCenterStrID(3,HID_CONTINUEESC);

	ret = InputLine(0,0,0,NULL);

	if(News_CommitInput == ret)
	{
		ret = News_CancelInput;
	}

	return ret;
}


int DoShowProSN(void *p)
{
	int ret = 0;
	char buf[MAX_CHAR_WIDTH];

	LCDClear();
	strcpy(buf, SerialNumber);
	LCDWriteLineStrID(0,MID_OI_SN);
	LCDWriteCenterStr(1, buf);
	if(strlen(buf)>16)
		LCDWriteCenterStr(2, buf+16);
	LCDWriteCenterStrID(3,HID_CONTINUEESC);

	ret = InputLine(0,0,0,NULL);

	if(News_CommitInput == ret)
	{
		ret = News_CancelInput;
	}

	return ret;
}

int DoShowProOEM(void *p)
{
	int ret = 0;

	LCDClear();
	LCDWriteLineStrID(0,MID_OI_OEM);
	LCDWriteCenterStr(1, OEMVendor);
	LCDWriteCenterStrID(3,HID_CONTINUEESC);

	ret = InputLine(0,0,0,NULL);

	if(News_CommitInput == ret)
	{
		ret = News_CancelInput;
	}

	return ret;
}

int DoShowProName(void *p)
{
	int ret =0 ;

	LCDClear();
	LCDWriteLineStrID(0,MID_OI_PN);
	LCDWriteCenterStr(1, DeviceName);
	LCDWriteCenterStrID(3,HID_CONTINUEESC);

	ret = InputLine(0,0,0,NULL);

	if(News_CommitInput == ret)
	{
		ret = News_CancelInput;
	}

	return ret;
}

int DoShowFWVer(void *p)
{
	int ret = 0;
	char buf[32]={0};

	LCDClear();
	strncpy(buf, MAINVERSION, 12); buf[12]=0;
	printf("FirmVer: %s\n", buf);
	if(gLCDRowCount>2)
	{
			LCDWriteLineStrID(0,MID_OI_FWVER);
			LCDWriteCenterStr(1, buf);
			LCDWriteCenterStr(2, MAINVERSION+12);
	}
	else
	{
			LCDWriteCenterStr(0, buf);
			LCDWriteCenterStr(1, MAINVERSION+12);
	}
	LCDWriteCenterStrID(3,HID_CONTINUEESC);

	ret = InputLine(0,0,0,NULL);

	if(News_CommitInput == ret)
	{
		ret = News_CancelInput;
	}

	return ret;
}


int DoShowDevInfo(void *p)
{
	int ret = 0;

	LCDClear();
	PMenu pDevInfo=CreateMenu(LoadStrByID(MID_INFO_DEV),gOptions.MenuStyle,NULL);

	AddMenuItem(1,pDevInfo,LoadStrByID(MID_OI_PT),DoShowProTime,NULL);
	AddMenuItem(1,pDevInfo,LoadStrByID(MID_OI_SN),DoShowProSN,NULL);
	if('?'!=*(OEMVendor))
	{
		AddMenuItem(1,pDevInfo,LoadStrByID(MID_OI_OEM),DoShowProOEM,NULL);
	}
	AddMenuItem(1,pDevInfo,LoadStrByID(MID_OI_PN),DoShowProName,NULL);
	AddMenuItem(1,pDevInfo,LoadStrByID(MID_OI_FWVER),DoShowFWVer,NULL);
	AddMenuItem(1,pDevInfo,LoadStrByID(MID_MCUVER),DoShowMCUVerInfo,NULL);

	ret=RunMenu(pDevInfo);
	DestroyMenu(pDevInfo);

	return ret;
}

extern U32 CurGuarderEventLogCount;
int DoShowSysInfo(void *p)
{//��ʾ����������¼������ǰ��¼��
	char buf[30];

	LCDClear();
	sprintf(buf,"MaxRec:%8d",gOptions.MaxAttLogCount*10000);
	LCDWriteStr(0,0,buf,0);
	sprintf(buf,"CurRec:%8d",CurGuarderEventLogCount);
	LCDWriteStr(1,0,buf,0);
	sprintf(buf,"MaxUser:%7d",gOptions.MaxUserCount*100);
	LCDWriteStr(2,0,buf,0);

//	LCDWriteCenterStrID(3,HID_CONTINUEESC);
	LCDWriteCenterStrID(3,HID_ESC);
	return InputLine(0,0,0,NULL);
}



int DoMainMenu(void)
{
	char buf[3][MAX_CHAR_WIDTH] = {0};
	PMenu MainMenu = NULL;
        PMenu CommInfoMenu = NULL;
        PMenu SysInsoMenu = NULL;
	int ret;

	LCDClear();

	MainMenu=CreateMenu(LoadStrByID(MID_MENU),gOptions.MenuStyle,NULL);

	printf("ItemIndex=%d,Count=%d,Title=%s \n",MainMenu->ItemIndex,MainMenu->Count,MainMenu->Title);

	//comm Info
	CommInfoMenu = CreateMenu(LoadStrByID(MID_COMM_INFO), gOptions.MenuStyle, MainMenu);
	AddMenuItem(1, CommInfoMenu, LoadStrByID(MID_NET_IP), DoShowNetIpInfo, NULL);
	AddMenuItem(1, CommInfoMenu, LoadStrByID(MID_NETMASK_ADDR), DoShowNetMaskInfo, NULL);
	AddMenuItem(1, CommInfoMenu, LoadStrByID(MID_GATEWAY_IP), DoShowNetGateIpInfo, NULL);
	AddMenuItem(1, CommInfoMenu, LoadStrByID(MID_NET_MAC), DoShowNetMACInfo, NULL);
	AddMenuItem(1, CommInfoMenu, LoadStrByID(MID_RS_ADDR), DoShowRSAddrss, NULL);
	AddMenuItem(1, CommInfoMenu, LoadStrByID(MID_OC_BAUDRATE), DoShowRs23ComInfo, NULL);
	AddMenuItem(1, MainMenu, LoadStrByID(MID_COMM_INFO), NULL, CommInfoMenu);//->Icon=Icon_Menu_User;

	SysInsoMenu = CreateMenu(LoadStrByID(MID_SYSINFO),gOptions.MenuStyle,MainMenu);
	AddMenuItem(1, SysInsoMenu, LoadStrByID(MID_USER_INFO), DoShowUsrInfo, NULL);
	AddMenuItem(1, SysInsoMenu, LoadStrByID(MID_RECORD_INFO), DoShowRecordInfo, NULL);
	AddMenuItem(1, SysInsoMenu, LoadStrByID(MID_INFO_DEV), DoShowDevInfo, NULL);
	AddMenuItem(1, MainMenu, LoadStrByID(MID_SYSINFO), NULL, SysInsoMenu);

	AddMenuItem(0, MainMenu, MenuFmtInt(buf[0], MID_RS_ADRR_SET, gOptions.DeviceID), DoSetRsAddr, NULL);

	//AddMenuItem(1, MainMenu, LoadStrByID(MID_INFO_DEV), DoShowMachineInfo, NULL);
	//AddMenuItem(1, MainMenu, LoadStrByID(MID_SYSINFO), DoShowSysInfo, NULL);


	gOptions.Saved=TRUE;

	ret=RunMenu(MainMenu);

	if (CommInfoMenu)
	{
		 DestroyMenu(CommInfoMenu);
	}

	if (SysInsoMenu)
	{
		 DestroyMenu(SysInsoMenu);
	}

	if (MainMenu)
	{
		DestroyMenu(MainMenu);
	}

	if(!gOptions.Saved && ret!=News_TimeOut)
	{
		SaveOptions(&gOptions);
		/*
		ret=LCDSelectOK(LoadStrByID(MID_RS_ADRR_SET),LoadStrByID(HID_SAVEQ), LoadStrByID(HID_SAVECANCEL));
		if(News_CommitInput==ret)
		{
			ExSetPowerSleepTime(gOptions.IdleMinute); //setup idle time again
			SaveOptions(&gOptions);
		}
		else
		{
			LoadOptions(&gOptions);
		}
		*/
	}

	return 1;
}




void ShowMenu(PMenu menu)
{
	int i,j,c, ShowTitle=0, cc;
	char line[32];


	if(MenuStyle_ICON==menu->Style)
		c=gLCDWidth/32;
	else
	{
		ShowTitle=(menu->Title && ((menu->TopIndex<0) || (RowCount>2)));
		if(ShowTitle) c=RowCount-1; else c=RowCount;
		if(menu->Count<=menu->TopIndex) menu->TopIndex=menu->Count-c;
		if(menu->ItemIndex<menu->TopIndex) menu->TopIndex=menu->ItemIndex;
	}

	printf("ShowMenu 001   menu->Styl =%d   menu->TopIndex =%d\n",menu->Style, menu->TopIndex);

	LCDBufferStart(LCD_BUFFER_ON);
	LCD_ClearHalfBuff();
	j=0;
	if(MenuStyle_ICON==menu->Style)
	{
		cc=menu->TopIndex;
		if(cc<0)cc=0;

		for(i=cc;i<menu->Count && i<cc+c; i++,j++)
		{
			if(i==menu->ItemIndex)
			{
				LCD_Bar(j*32, 3, j*32+31, 6+32+3, COLOR_FG);
				LCD_OutBMP1Bit(j*32, 6, menu->Items[i].Icon, 0,0,32,32,1);
			}
			else
				LCD_OutBMP1Bit(j*32, 6, menu->Items[i].Icon, 0,0,32,32,0);
			LCD_Line(0, gLCDHeight-gRowHeight-3, gLCDWidth, gLCDHeight-gRowHeight-3, COLOR_FG);
			LCD_Line(0, gLCDHeight-gRowHeight-1, gLCDWidth, gLCDHeight-gRowHeight-1, COLOR_FG);
		}
		LCDWriteStr(gLCDRowCount-1, 0, menu->Items[menu->ItemIndex].Caption, LCD_HIGH_LIGHT);

	}
	else if(MenuStyle_STD==menu->Style)
	{
/*		if(menu->Title && ShowTitle)
		{
			for(i=1;i<gLCDCharWidth/2;i++)
			line[i*2]=(char)CHLB0, line[i*2+1]=(char)CHTL;	//horized line
			line[0]=(char)CHLB0;
			if(menu->TopIndex>0)	//���ǴӲ˵���һ�ʼ��ʾ��
			line[1]=(char)CHLT1;			//arrowed lefttop corner
			else
			line[1]=(char)CHLT0;			//lefttop corner
			LCDWriteStrLng(gSymbolDriver, 0, 0, line,0);

			LCD_Line(LeftStart,gRowHeight/2,gLCDWidth-1,gRowHeight/2, COLOR_FG);
			LCD_Line(LeftStart,gRowHeight/2,LeftStart, gRowHeight, COLOR_FG);
			if(menu->TopIndex>0)
				LCD_Triangle(LeftStart,gRowHeight/2+1, 4, TriDir_Bottom, COLOR_FG);

			LCDWriteCenter(j++,menu->Title);
		}*/
		cc=menu->TopIndex;
		if(cc<0)cc=0;
		for(i=cc;i<menu->Count && i<cc+c; i++,j++)
		{
			int len=strlen(menu->Items[i].Caption);
			if(len>32) len=32;
			memset(line,' ',32);
			memcpy(line,menu->Items[i].Caption,len);

			if(i==menu->ItemIndex)
			{
				LCDWriteStr(j,MenuIndicatorWidth,line,LCD_HIGH_LIGHT);
			    printf("Menu  = %s\n",line);
			}
			else if(i==menu->Count-1) //�˵������һ��
			{
				printf("Menu  = %s\n",line);
				LCDWriteStr(j,1,line,LCD_BOTTOM_LINE);
			}
			else
				LCDWriteStr(j,MenuIndicatorWidth,line,0);
			line[1]=' ';
			line[MenuIndicatorWidth]=0;

			LCD_Line(LeftStart, j*gRowHeight, LeftStart, j*gRowHeight+gRowHeight, COLOR_FG);
			if(i==menu->Count-1) //�˵������һ��
			{
				LCD_Line(LeftStart, j*gRowHeight+gRowHeight-1, gLCDWidth, j*gRowHeight+gRowHeight-1, COLOR_FG);
			}
			else if(i==menu->TopIndex+c-1) //���в˵���û����ʾ��4
			{
				LCD_Triangle(LeftStart,gLCDHeight-1, 4, COLOR_FG, TriDir_Top);
			}
			else
			{
				LCD_Line(LeftStart, j*gRowHeight, LeftStart, j*gRowHeight+gRowHeight, COLOR_FG);
			}
		}
	}
	else if(MenuStyle_OLD==menu->Style)
	{

//		printf("ShowMenu 002  --0\n");
		if(menu->Title && ShowTitle)
		{
			memset(line,' ',32);
			memcpy(line,menu->Title,strlen(menu->Title));
			LCDWriteStr(j++,0,line,0);
		}
		cc=menu->TopIndex;
		if(cc<0) cc++;

//		printf("ShowMenu 002  --1   menu->TopIndex=%d, cc = %d menu->Count=%d\n", menu->TopIndex,cc,menu->Count);

		for(i=cc;i<menu->Count && i<cc+c; i++,j++)
		{
			memset(line,' ',32);
			memcpy(line+MenuIndicatorWidth,menu->Items[i].Caption,strlen(menu->Items[i].Caption));
			LCDWriteStr(j,0,line,0);
			if(i==menu->ItemIndex)
			{
				LCD_Triangle(7, j*gRowHeight+gRowHeight/2, 6, TriDir_Left, COLOR_FG);
			}
		}

//		printf("ShowMenu 003    \n");
		if(menu->TopIndex>0 || c+menu->TopIndex<menu->Count)
		{
			LCDWriteStr(0, gLCDCharWidth-1, " ", 0);
			LCD_Line(gLCDWidth-5, 2, gLCDWidth-5, gRowHeight-2, COLOR_FG);
			if(menu->TopIndex>0 && c+menu->TopIndex<menu->Count)
			{//Up and Down Arrow
				LCD_Triangle(gLCDWidth-5, 2, 4, TriDir_Bottom, COLOR_FG);
				LCD_Triangle(gLCDWidth-5, gRowHeight-2, 4, TriDir_Top, COLOR_FG);
			}
			else if(menu->TopIndex>0)
			{//Up Arrow
				LCD_Triangle(gLCDWidth-5, 2, 4, COLOR_FG, TriDir_Bottom);
			}
			else if(c+menu->TopIndex<menu->Count)
			{//Down Arrow
				LCD_Triangle(gLCDWidth-5, gRowHeight-2, 4, TriDir_Top, COLOR_FG);
			}
		}
	}

/*	//������
	if(MenuStyle_ICON!=menu->Style)
	for(;j<RowCount;j++)
	{
		memset(line,' ',gLCDCharWidth);
		LCDWriteStr(j,0,line,0);
	}
*/
//	LCDInvalid();

	LCDBufferStart(LCD_BUFFER_OFF);
}

int RunMenuMsg(PMsg msg)
{
	int oldkey,i;
	PMenu menu;
	menu=(PMenu)msg->Object;
	//printf("RunMenuMsg. message = 0x%x   Param1 =0x%x,Param2=0x%x\n",msg->Message,msg->Param1,msg->Param2);

	if(MSG_TYPE_TIMER==msg->Message && InputTimeOut>=0)
	{
		msg->Message=0;
		msg->Param1=0;
		if(++InputTimeOut>=InputTimeOutSec)
		{
			ConstructMSG(msg, MSG_TYPE_CMD, News_Exit_Input, News_TimeOut);
		}
		return 1;
	}
	else if(!(MSG_TYPE_BUTTON==msg->Message))
		return 0;
	oldkey=msg->Param1;
	msg->Param1=0;
	if(InputTimeOut>=0) InputTimeOut=0;


	if(C4_OK==oldkey)
	{
		if(menu->Items[menu->ItemIndex].Action)
		{
			int ret;
			if(menu->Style==MenuStyle_STD)
			{
				int row=CalcMenuItemOffset(menu, menu->ItemIndex);
				LCDWriteStr(row, MenuIndicatorWidth, "     ", 0);
				LCDWriteStr(row, MenuIndicatorWidth, menu->Items[menu->ItemIndex].Caption, 0);

			}
			ret=menu->Items[menu->ItemIndex].Action(msg);
			if(News_TimeOut==ret || News_CommitInput==ret)
			{
				printf("RunMenuMsg  News_TimeOut\n");
//				ConstructMSG(msg, MSG_TYPE_CMD, News_Exit_Input, ret);
//				ConstructMSG(msg, MSG_TYPE_BUTTON, C4_ESC, 0);
				return 1;
			}
		}
		if(menu->Items[menu->ItemIndex].SubMenu)
		{
			menu=menu->Items[menu->ItemIndex].SubMenu;
			msg->Object=menu;
			CheckMenuStyle(menu);
		}
		ShowMenu(menu);
	}
	else if(C4_ESC==oldkey)
	{
		if(menu->Parents)
		{
			menu=menu->Parents;
			msg->Object=menu;
			ShowMenu(menu);
		}
		else
		{
			ConstructMSG(msg, MSG_TYPE_CMD, News_Exit_Input, 0);
			return 1;
		}

	}
	else if(C4_UP==oldkey)
	{
		if(menu->ItemIndex>0)
		{
			menu->ItemIndex--;
			if(menu->ItemIndex<menu->TopIndex) menu->TopIndex=menu->ItemIndex;
			ShowMenu(menu);
		}
		else if(menu->ItemIndex==0 && menu->Title && (RowCount<=2))
		{
			menu->TopIndex=-1;
			ShowMenu(menu);
		}
	}
	else if(C4_DOWN==oldkey)
	{
		if(menu->ItemIndex<menu->Count-1)
		{
			menu->ItemIndex++;
			//if(menu->Title && RowCount>2) i=RowCount-1; else i=RowCount;
			if(menu->Title && RowCount>2 && (MenuStyle_ICON!=menu->Style)) i=RowCount-1; else i=RowCount;
			if(menu->ItemIndex>=menu->TopIndex+i) menu->TopIndex++;
			ShowMenu(menu);
		}

	}
	msg->Message=0;
	msg->Param1=0;
	return 1;
}

int RunMenu(PMenu menu)
{
	int ret,i=RegMsgProc(RunMenuMsg);

//	printf("start DoMsgProcess 001\n");

	U32 mm=SelectNewMsgMask(MSG_TYPE_BUTTON|MSG_TYPE_TIMER);
	if(menu->Title && (gLCDRowCount<=2)) menu->TopIndex=-1;

//	printf("start DoMsgProcess 002\n");

	CheckMenuStyle(menu);

//	printf("start DoMsgProcess 003  menu->TopIndex=%d\n", menu->TopIndex);

	ShowMenu(menu);
	InputTimeOut=0;

//	printf("start DoMsgProcess 004\n");

	ret=DoMsgProcess(menu, News_Exit_Input);

//	printf("start DoMsgProcess 005\n");

	SelectNewMsgMask(mm);
	UnRegMsgProc(i);
//	printf("RunMenu Over\n");
	return ret;
}

void ShowInput(PInputBox box)
{
	char s[32],tmp[32];
	int i,Align, Theflag;
	Align=box->Alignment;
	if(Align==Alignment_Auto)
	{
		if(box->Style==InputStyle_Number || box->Style==InputStyle_ANumber || box->Style==InputStyle_Number2)
			Align=Alignment_Right;
		else
			Align=Alignment_Left;
	}
	memset(s,' ',32);
	if(box->SelectStart<box->TopIndex)
		box->TopIndex=box->SelectStart;
	strcpy(tmp,box->Text+box->TopIndex);
	i=box->SelectStart-box->TopIndex;
	tmp[i]=0;
	if(i>0 && box->PasswordChar) memset(tmp,box->PasswordChar,i);
	if(Align==Alignment_Left)  //gLCDCharWidth
		memcpy(s,tmp,i);
	else if(Align==Alignment_Center)
		SPadCenterStr(s, box->Width, tmp);
	else
		SPadRightStr(s, box->Width, tmp);
	s[box->Width]=0;
	//Theflag=LCD_RIGHT_LINE|LCD_TOP_LINE|LCD_LEFT_LINE|LCD_RIGHT_LINE;
	Theflag=LCD_HIGH_LIGHT;
	LCDWriteStr(box->Row,box->Col,s,Theflag);
	if(i<box->Width)
	{
		if(box->SelectLength)
		{
			if(box->PasswordChar)
				memset(tmp,box->PasswordChar,32);
			else
				strcpy(tmp,box->Text+box->SelectStart);
			tmp[box->Width-i]=0;
			if(Align==Alignment_Left)
				LCDWriteStr(box->Row,box->Col+i,tmp,LCD_BOTTOM_LINE|Theflag);
			else if(i==0)
			{
				if(Align==Alignment_Center)
					LCDWriteStr(box->Row,box->Col+(box->Width-box->SelectLength)/2,
							tmp,LCD_BOTTOM_LINE|Theflag);
				else
					LCDWriteStr(box->Row,box->Col+box->Width-box->SelectLength,
							tmp,LCD_BOTTOM_LINE|Theflag);
			}
		}
		else if(Align==Alignment_Left)
		{
			s[i]='_';
			LCDWriteStr(box->Row,box->Col+i,s+i,Theflag);
		}
	}
	LCDInvalid();
}

#define ReturnMsg(NEWS,p1) msg->Message=MSG_TYPE_CMD;\
					msg->Param1=NEWS;\
msg->Param2=p1;\
return 1;

int RunInputMsg(PMsg msg)
{
	int oldkey,i;

	PInputBox box=(PInputBox)msg->Object;
	if(MSG_TYPE_TIMER==msg->Message && InputTimeOut>=0)
	{
		msg->Message=0;
		msg->Param1=0;

		//printf("RunInputMsg type is timer InputTimeOut = %d\n",InputTimeOut);
		if(++InputTimeOut>=InputTimeOutSec)
		{
			ConstructMSG(msg, MSG_TYPE_CMD, News_Exit_Input, News_TimeOut);
		}
		return 1;
	}
	if(!(MSG_TYPE_BUTTON==msg->Message)) return 0;
	oldkey=msg->Param1;
	msg->Param1=0;
	if(InputTimeOut>=0) InputTimeOut=0;
	if(oldkey==IKeyOK)
	{
		box->Text[box->SelectStart+box->SelectLength]=0;
		if(box->ValidFun)
		{
			if(!box->ValidFun(box->Text))
			{
				ConstructMSG(msg, MSG_TYPE_CMD, News_Exit_Input, News_ErrorInput);
				return 1;
			}
		}
		ConstructMSG(msg, MSG_TYPE_CMD, News_Exit_Input, News_CommitInput);
		return 1;
	}
	else if(oldkey==IKeyESC)
	{
		ConstructMSG(msg, MSG_TYPE_CMD, News_Exit_Input, News_CancelInput);
		return 1;
	}
	else if(oldkey==IKeyDown || oldkey==IKeyUp)
	{
		int NewValue=0;
		if (box->Style==InputStyle_Number2)
			return 1;
		if(box->Items && box->ItemCount)
		{
			i=SearchIndex(box->Items,box->Text,box->ItemCount);
			if(oldkey==IKeyUp) i-=1; else i+=1;
			if(i<0) i=box->ItemCount-1; else if(i>=box->ItemCount) i=0;
			strcpy(box->Text,box->Items[i]);
			box->SelectStart=0;
			box->SelectLength=strlen(box->Text);
			NewValue=1;
		}
		else if(box->AllowNav)
		{
			if(oldkey==IKeyDown)
				ConstructMSG(msg, MSG_TYPE_CMD, News_Exit_Input, News_NextInput);
			else
				ConstructMSG(msg, MSG_TYPE_CMD, News_Exit_Input, News_PrevInput);
			return 1;
		}
		else if(box->Style==InputStyle_Number || box->Style==InputStyle_ANumber)
		{
			int v,v1;
			char tmp[40];
			i=0;
			while(box->Text[i]) if(box->Text[i]>'0') break; else i++;
			if(!(strtou32(box->Text+i,(U32*)&v)==0)) v=0;
			if(oldkey==IKeyDown) i=-1;else i=1;
			v+=i;v1=v;
			if(box->ValidFun && (box->MaxValue-box->MinValue<0x10000))
				do{
					if(box->Style==InputStyle_ANumber)
						Pad0Num(tmp, box->MaxLength, v);
					else
						sprintf(tmp, "%d", v);
					if(box->ValidFun(tmp)) break;
					v+=i;
					if(box->MaxValue && v>box->MaxValue) v=box->MinValue;
					if(box->MinValue && v<box->MinValue) v=box->MaxValue;
					if(v==v1) break;
				}
				while(1);

			if(box->MaxValue && v>box->MaxValue) v=box->MinValue;
			else if(v<box->MinValue) v=box->MaxValue;
			if(box->Style==InputStyle_ANumber)
				Pad0Num(box->Text, box->MaxLength, v);
			else
				sprintf(box->Text, "%d", v);
			NewValue=1;
		}
		if(NewValue)
		{
			box->TopIndex=0;
			box->SelectStart=0;
			box->SelectLength=strlen(box->Text);
			ShowInput(box);
		}
	}
	else if(oldkey>0x20 && oldkey<0x7F && box->Style!=InputStyle_Select) //ascii
	{
		box->SelectLength=0;
		if(box->Style==InputStyle_Number || box->Style==InputStyle_ANumber || box->Style==InputStyle_Number2)
		{
			if(oldkey>'9' || oldkey<'0') return 1;
		}
		if(box->SelectStart==(box->MaxLength)  && oldkey>='0' && oldkey<='9' && box->Text[0]=='0')
		{
			for(i=0;i<box->SelectStart;i++)
				box->Text[i]=box->Text[i+1];
			box->SelectStart--;
		}
		//2006.10.11 ��ӵ��������ֵĳ���=���ʱ�����ٽ�����ֵ����box->Text����
		if (box->MaxLength != box->SelectStart)
			box->Text[box->SelectStart]=oldkey;
		if(!box->MaxLength || box->MaxLength>box->SelectStart)
		{
			box->SelectStart++;
			box->Text[box->SelectStart]=0;
		}
		else
		{
			box->Text[box->SelectStart-1]=oldkey;
		}
		if(box->SelectStart-box->TopIndex>box->Width)
			box->TopIndex++;
		if(box->Style==InputStyle_ANumber)
		{
			U32 v;
			i=0;
			while(box->Text[i]=='0') {i++;}
			if(!(strtou32(box->Text+i,(U32*)&v)==0)) v=0;
			Pad0Num(box->Text, box->MaxLength, v);
			box->SelectStart=box->MaxLength;
		}
		if(box->AutoCommit && box->SelectStart>=box->MaxLength)
		{
			ConstructMSG(msg, MSG_TYPE_CMD, News_Exit_Input, News_CommitInput);
			return 1;
		}
		ShowInput(box);
	}
	msg->Message=0;
	return 1;
}

int RunInput(PInputBox box)
{
	int ret,i=RegMsgProc(RunInputMsg);

	U32 mm=SelectNewMsgMask(MSG_TYPE_BUTTON|MSG_TYPE_TIMER);

	if(box->ValidFun && box->Text[0] && (box->MaxValue-box->MinValue<0x10000))
	{
		//�ı��ʼֵΪ��һ����Ч��ֵ
		U32 v;
		while(!box->ValidFun(box->Text))
		{
			strtou32(box->Text, &v);
			v++;
			if((int)v>box->MaxValue)	v=box->MinValue;
			if(box->Style==InputStyle_ANumber)
				Pad0Num(box->Text, box->MaxLength, v);
			else
				sprintf(box->Text, "%d", v);
		}
	}
	ShowInput(box);
	InputTimeOut=0;
	ret=DoMsgProcess(box, News_Exit_Input);

	SelectNewMsgMask(mm);
	UnRegMsgProc(i);
	return ret;
}

PMenu CreateMenu(char *Title, int Style, PMenu Parents)
{
	PMenu m;
	if(Title)
	{
		m=(PMenu)malloc(sizeof(TMenu));
		memset(m,0,sizeof(TMenu));
		m->Parents=Parents;
		m->Items=(PMenuItem)malloc(sizeof(TMenuItem)*100);
		memset(m->Items,0,100*sizeof(TMenuItem));
		m->Title=(char *)malloc(strlen(Title)+1);
		strcpy(m->Title, Title);
		m->Style=Style;
		return m;
	}
	else
		return NULL;
}

int DestroyMenu(PMenu menu)
{
	int i;
	for(i=0;i<menu->Count;i++)
		if ((menu->Items[i]).MemStyle) free((menu->Items[i]).Caption);
	free(menu->Title);
	free(menu->Items);
	free(menu);
	return 0;
}

PMenuItem AddMenuItem(char MemStyle, PMenu menu, char *Caption, ActionFunc Action, PMenu SubMenu)
{
	PMenuItem mi;
	if (Caption==NULL) return NULL;
	if (strstr(Caption, "(null)")) return NULL;
	menu->Count++;
	mi=&(menu->Items[menu->Count-1]);
	memset(mi,0,sizeof(TMenuItem));
	if (MemStyle)
	{
		mi->Caption=(char *)malloc(MENU_ITEM_CHARWIDTH);
		strcpy(mi->Caption, Caption);
	}
	else
		mi->Caption=Caption;
	mi->Action=Action;
	mi->SubMenu=SubMenu;
	mi->MemStyle=MemStyle;
	return mi;
}

int InputLine(int row, int col, int width, char *text)
{
	int ret,i;
	PInputBox box;
	box=(PInputBox)malloc(sizeof(TInputBox));
	memset(box,0,sizeof(TInputBox));
	if(text)
	{
		box->SelectLength=strlen(text);
		strcpy(box->Text,text);
	}
	else
		box->SelectLength=0;
	box->MaxLength=width;
	box->Row=row;
	box->Col=col;
	box->Width=width;
	ret=RunInput(box);
	if(News_CommitInput==ret)
	{
		i=box->SelectStart;
		if(text)
		{
			nstrcpy(text,box->Text,i);
			text[i]=0;
		}
	}
	free(box);
	return ret;
}

int InputNumber(int row, int col, int width, int *value, int minv, int maxv, int nav)
{
	int ret,i;
	PInputBox box;
	box=(PInputBox)malloc(sizeof(TInputBox));
	memset(box,0,sizeof(TInputBox));
	box->MaxLength=width;
	box->Row=row;
	box->Col=col;
	box->MinValue=minv;
	box->MaxValue=maxv;
	box->Alignment=Alignment_Right;
	if(value)
		sprintf(box->Text,"%d",*value);
	else
		sprintf(box->Text,"%d",minv);
	box->SelectLength=strlen(box->Text);
	box->Width=width;
	box->Style=InputStyle_Number;
	box->AllowNav=nav;
	ret=RunInput(box);
	if(News_CommitInput==ret || News_NextInput==ret || News_PrevInput==ret)
	{
		i=box->SelectStart+box->SelectLength;
		box->Text[i]=0;
		if(i>0 && strtou32(box->Text,(U32*)&i)==0)
		{
			if(i<minv || i>maxv) ret=News_ErrorInput;
			if(value) *value=i;
		}
		else
			ret=News_ErrorInput;
	}
	free(box);
	return ret;
}

int InputTextNumber(int row, int col, int width, int *value, int minv, int maxv, U8 style)
{
	int ret,i,v;
	PInputBox box;
	box=(PInputBox)malloc(sizeof(TInputBox));
	memset(box,0,sizeof(TInputBox));
	box->MaxLength=width;
	box->Row=row;
	box->Col=col;
	box->MinValue=minv;
	box->MaxValue=maxv;
	box->Alignment=Alignment_Right;
	if(value)
		v=*value;
	else
		v=minv;
	if(style==InputStyle_ANumber)
		Pad0Num(box->Text, box->MaxLength, v);
	else
		sprintf(box->Text,"%d",v);
	box->SelectLength=strlen(box->Text);
	box->Width=width;
	box->Style=style;
	box->AllowNav=FALSE;
	ret=RunInput(box);
	if(News_CommitInput==ret || News_NextInput==ret || News_PrevInput==ret)
	{
		i=box->SelectStart+box->SelectLength;
		box->Text[i]=0;
		if(i>0 && strtou32(box->Text,(U32*)&i)==0)
		{
			if(i<minv || i>maxv) ret=News_ErrorInput;
			if(value) *value=i;
		}
		else
			ret=News_ErrorInput;
	}
	free(box);
	return ret;
}

int InputNumberAt(int row,int col, int width, int *number, int minv, int maxv)
{
	char buf[20],buf2[10];
	int ret;
	ret=InputNumber(row,col,width,number,minv,maxv,TRUE);
	sprintf(buf2, "%%.%dd", width);
	sprintf(buf, buf2, *number);
	LCDWriteStr(row,col,buf,0);
	return ret;
}

//Repeat input a number until no error
int RepeatInputNumber(int row,int col, int width, int *number, int minv, int maxv)
{
	int ret;
	while((ret=InputNumber(row,col,width,number,minv,maxv,FALSE))==News_ErrorInput);
	return ret;
}


void LCDInfo(char *info, int DelaySec)
{
	LCDInfoShow(NULL, info);
	mmsleep(DelaySec*1000);
}

void LCDInfoShow(char *title, char *info)
{
	LCD_ClearHalfBuff();
	if(title) LCDWriteStr(0, 0, title, 0);
	LCDWriteCenterStr(1, info);
}

int LCDSelectOK(char *title, char *info, char *hint)
{
	LCD_ClearHalfBuff();
	if(title && gLCDRowCount>2) LCDWriteStr(0, 0, title, 0);
	if(info) LCDWriteCenterStr(gLCDRowCount/2, info);
	LCDWriteStr(gLCDRowCount-1, 0, hint, 0);
	return InputLine(0,0,0,NULL);
}

int LCDSelectItem(int row, int col, int width, char **items, int itemcount, int *index)
{
	int ret,i=0;
	PInputBox box;
	box=(PInputBox)malloc(sizeof(TInputBox));
	memset(box,0,sizeof(TInputBox));
	box->MaxLength=width;
	box->Row=row;
	box->Col=col;
	if(items)
	{
		box->Items=items;
		box->ItemCount=itemcount;
		strcpy(box->Text,items[*index]);
		box->Width=width;
		box->SelectLength=strlen(items[*index]);
	}
	box->Style=InputStyle_Select;
	ret=RunInput(box);
	if(News_CommitInput==ret)
	{

		if(items) i=SearchIndex(items, box->Text, itemcount);
		DBPRINTF("box->text= %s, index=%d\n", box->Text,i);
		if(i>=0)
		{
			if(index) *index=i;
		}
		else
		{
			ret=News_ErrorInput;
		}
	}
	free(box);
	return ret;
}

#define MAXSELITEM	100

int LCDSelectItemValue(int row, int col, int width, char *items, int *values, int *value)
{
	char *(sitem[MAXSELITEM]), itembuf[MAXSELITEM*20];
	int i=0, count=0, j, ret;
	count=0;
	do
	{
		sitem[count]=itembuf+i;
		if(*items==0) break;
		j=SCopyStrFrom(sitem[count], items, 0);
		if(j==0)
		{
			items++;
		}
		else
		{
			count++;
			items+=j;
			i+=j+1;
		}
	}while(count<=MAXSELITEM);
	i=0;
	while(i<count)
		if(*value==values[i]) break; else i++;
	if(i>=count) i=0;
	do
	{
		ret=LCDSelectItem(row,col,width,sitem,count,&i);
		if(News_CancelInput==ret || News_TimeOut==ret) break;
		if(i<count)
		{
			*value=values[i];
			break;
		}
	}while(1);
	return ret;
}

