#include <string.h>
#include <stdlib.h>
#include "flashdb.h"
#include "ccc.h"
#include "dataapi.h"

//卡号信息表结构
C3FIELD_DESC C3UserInfoFields[]=
{
//ID FieldName    width offset pk fk cbGetFun cbSetFun cbCheckVal type buffer newData */
{1, "CardNo", 	  4, 0, 0, NULL, NULL, NULL, NULL, UINT_T, {0}, {0},0,{0}},
{2, "Pin", 	      4, 4, 1, NULL, NULL, NULL, NULL, UINT_T, {0}, {0},0,{0}},
{3, "Password",   8, 8, 0, NULL, NULL, NULL, NULL, STRING_T, {0}, {0},0,{0}},
{4, "Group", 	  4, 16, 0, NULL, NULL, NULL, NULL, UINT_T, {0}, {0},0,{0}},
{5, "StartTime",  4, 20, 0, NULL, NULL, NULL, NULL, UINT_T, {0}, {0},0,{0}},
{6, "EndTime",    4, 24, 0, NULL, NULL, NULL, NULL, UINT_T, {0}, {0},0,{0}},
{7, "SuperAuthorize",  4, 28, 0, NULL, NULL, NULL, NULL, UINT_T, {0}, {0},0,{0}}
};
//pin授权表,PK>1表示为主键
C3FIELD_DESC C3UserAuthorizeFields[]=
{
// filed name width offset pk fk cbGetFun cbSetFun cbCheckVal type buffer newData */
{1, "Pin",			4, 0, 2, NULL, NULL, NULL, NULL, UINT_T, {0}, {0},0,{0}},
{2, "AuthorizeTimezoneId", 	4, 4, 1, NULL, NULL, NULL, NULL, UINT_T, {0}, {0},0,{0}},
{3, "AuthorizeDoorId", 		4, 8, 3, NULL, NULL, NULL, NULL, UINT_T, {0}, {0},0,{0}}
};
//假日表
C3FIELD_DESC C3HolidayFields[]=
{
// filed name width offset pk fk cbGetFun cbSetFun cbCheckVal type buffer newData */
{1, "Holiday", 	4, 0, 1, NULL, NULL, NULL, NULL, UINT_T, {0}, {0},0,{0}},
{2, "HolidayType", 	4, 4, 0, NULL, NULL, NULL, NULL, UINT_T, {0}, {0},0,{0}},
{3, "Loop", 	4, 8, 0, NULL, NULL, NULL, NULL, UINT_T, {0}, {0},0,{0}}
};
//时区表
//suntime1 :  例8:30 ~12:00   值 为8301200  即0x7eaa90
C3FIELD_DESC C3TimezoneFields[]=
{
// filed name width offset pk fk cbGetFun cbSetFun cbCheckVal type buffer newData */
{1, "TimezoneId", 	4, 0, 1, NULL, NULL, NULL, NULL, UINT_T, {0}, {0},0,{0}},
{2, "SunTime1", 	4, 4, 0, NULL, NULL, NULL, NULL, UINT_T, {0}, {0},0,{0}},
{3, "SunTime2", 	4, 8, 0, NULL, NULL, NULL, NULL, UINT_T, {0}, {0},0,{0}},
{4, "SunTime3", 	4, 12, 0, NULL, NULL, NULL, NULL, UINT_T, {0}, {0},0,{0}},
{5, "MonTime1", 	4, 16, 0, NULL, NULL, NULL, NULL, UINT_T, {0}, {0},0,{0}},
{6, "MonTime2", 4, 20, 0, NULL, NULL, NULL, NULL, UINT_T, {0}, {0},0,{0}},
{7, "MonTime3", 4, 24, 0, NULL, NULL, NULL, NULL, UINT_T, {0}, {0},0,{0}},
{8, "TueTime1", 4, 28, 0, NULL, NULL, NULL, NULL, UINT_T, {0}, {0},0,{0}},
{9, "TueTime2", 4, 32, 0, NULL, NULL, NULL, NULL, UINT_T, {0}, {0},0,{0}},
{10, "TueTime3", 4, 36, 0, NULL, NULL, NULL, NULL, UINT_T, {0}, {0},0,{0}},
{11, "WedTime1", 4, 40, 0, NULL, NULL, NULL, NULL, UINT_T, {0}, {0},0,{0}},
{12, "WedTime2", 4, 44, 0, NULL, NULL, NULL, NULL, UINT_T, {0}, {0},0,{0}},
{13, "WedTime3", 4, 48, 0, NULL, NULL, NULL, NULL, UINT_T, {0}, {0},0,{0}},
{14, "ThuTime1", 4, 52, 0, NULL, NULL, NULL, NULL, UINT_T, {0}, {0},0,{0}},
{15, "ThuTime2", 4, 56, 0, NULL, NULL, NULL, NULL, UINT_T, {0}, {0},0,{0}},
{16, "ThuTime3", 4, 60, 0, NULL, NULL, NULL, NULL, UINT_T, {0}, {0},0,{0}},
{17, "FriTime1", 4, 64, 0, NULL, NULL, NULL, NULL, UINT_T, {0}, {0},0,{0}},
{18, "FriTime2", 4, 68, 0, NULL, NULL, NULL, NULL, UINT_T, {0}, {0},0,{0}},
{19, "FriTime3", 4, 72, 0, NULL, NULL, NULL, NULL, UINT_T, {0}, {0},0,{0}},
{20, "SatTime1", 4, 76, 0, NULL, NULL, NULL, NULL, UINT_T, {0}, {0},0,{0}},
{21, "SatTime2", 4, 80, 0, NULL, NULL, NULL, NULL, UINT_T, {0}, {0},0,{0}},
{22, "SatTime3", 4, 84, 0, NULL, NULL, NULL, NULL, UINT_T, {0}, {0},0,{0}},
{23, "Hol1Time1", 4, 88, 0, NULL, NULL, NULL, NULL, UINT_T, {0}, {0},0,{0}},
{24, "Hol1Time2", 4, 92, 0, NULL, NULL, NULL, NULL, UINT_T, {0}, {0},0,{0}},
{25, "Hol1Time3", 4, 96, 0, NULL, NULL, NULL, NULL, UINT_T, {0}, {0},0,{0}},
{26, "Hol2Time1", 4, 100, 0, NULL, NULL, NULL, NULL, UINT_T, {0}, {0},0,{0}},
{27, "Hol2Time2", 4, 104, 0, NULL, NULL, NULL, NULL, UINT_T, {0}, {0},0,{0}},
{28, "Hol2Time3", 4, 108, 0, NULL, NULL, NULL, NULL, UINT_T, {0}, {0},0,{0}},
{29, "Hol3Time1", 4, 112, 0, NULL, NULL, NULL, NULL, UINT_T, {0}, {0},0,{0}},
{30, "Hol3Time2", 4, 116, 0, NULL, NULL, NULL, NULL, UINT_T, {0}, {0},0,{0}},
{31, "Hol3Time3", 4, 120, 0, NULL, NULL, NULL, NULL, UINT_T, {0}, {0},0,{0}}
};

C3FIELD_DESC AcessLogFields[] =
{
// filed name 	width offset pk fk cbGetFun cbSetFun cbCheckVal type buffer newData */
{1, "Cardno",		4, 0, 1, NULL, NULL, NULL, NULL, UINT_T, {0}, {0},0,{0}},
{2, "Pin",			4, 4, 0, NULL, NULL, NULL, NULL, UINT_T, {0}, {0},0,{0}},
{3, "Verified", 	1, 8, 0, NULL, NULL, NULL, NULL, CHAR_T, {0}, {0},0,{0}},
{4, "DoorID", 		1, 9, 0, NULL, NULL, NULL, NULL, CHAR_T, {0}, {0},0,{0}},
{5, "EventType", 	1, 10, 0, NULL, NULL, NULL, NULL, CHAR_T,  {0}, {0},0,{0}},
{6, "InOutState", 	1, 11, 0, NULL, NULL, NULL, NULL, CHAR_T, {0}, {0},0,{0}},
{7, "Time_second", 4, 12, 0, NULL, NULL, NULL, NULL, UINT_T, {0}, {0},0,{0}}
};

//首卡开门表
C3FIELD_DESC FirstCardFields[] =
{
// filed name 	width offset pk fk cbGetFun cbSetFun cbCheckVal type buffer newData */
{1, "Pin",		4, 0, 2, NULL, NULL, NULL, NULL, UINT_T, {0}, {0},0,{0}},
{2, "DoorID",		4, 4, 1, NULL, NULL, NULL, NULL, UINT_T, {0}, {0},0,{0}},
{3, "TimezoneID",	4, 8, 0, NULL, NULL, NULL, NULL, UINT_T, {0}, {0},0,{0}}
};

//多卡开门组合表
C3FIELD_DESC  MultiCardFields[] =
{
// filed name 	width offset pk fk cbGetFun cbSetFun cbCheckVal type buffer newData */
{1, "Index", 		4, 0, 1, NULL, NULL, NULL, NULL, UINT_T, {0}, {0},0,{0}},
{2, "DoorId", 		4, 4, 0, NULL, NULL, NULL, NULL, UINT_T, {0}, {0},0,{0}},
{3, "Group1",		4, 8, 0, NULL, NULL, NULL, NULL, UINT_T, {0}, {0},0,{0}},
{4, "Group2", 		4, 12, 0, NULL, NULL, NULL, NULL, UINT_T, {0}, {0},0,{0}},
{5, "Group3", 		4, 16, 0, NULL, NULL, NULL, NULL, UINT_T, {0}, {0},0,{0}},
{6, "Group4", 		4, 20, 0, NULL, NULL, NULL, NULL, UINT_T, {0}, {0},0,{0}},
{7, "Group5", 		4, 24, 0, NULL, NULL, NULL, NULL, UINT_T, {0}, {0},0,{0}}
};

//联动控制i/o表
C3FIELD_DESC  InOutFunFields[] =
{
// filed name 	width offset pk fk cbGetFun cbSetFun cbCheckVal type buffer newData
{1, "Index", 		2, 0, 1, NULL, NULL, NULL, NULL, INT_T, {0}, {0},0,{0}},
{2, "EventType", 	1, 2, 0, NULL, NULL, NULL, NULL, CHAR_T, {0}, {0},0,{0}},
{3, "InAddr",		1, 3, 0, NULL, NULL, NULL, NULL, CHAR_T, {0}, {0},0,{0}},
{4, "OutType",	1, 4, 0, NULL, NULL, NULL, NULL, CHAR_T, {0}, {0},0,{0}},
{5, "OutAddr",		1, 5, 0, NULL, NULL, NULL, NULL, CHAR_T, {0}, {0},0,{0}},
{6, "OutTime",	1, 6, 0, NULL, NULL, NULL, NULL, CHAR_T, {0}, {0},0,{0}},
{7, "Reserved",	1, 7, 0, NULL, NULL, NULL, NULL, CHAR_T, {0}, {0},0,{0}},
};

C3TABLE_DESC c3table_descs[]=
{
//tableID name   fd FCT fieldCount rowDataWidth FIELD_DESC TblIsDeletedFunT TblDelRowFunT cbGetAllPKFun cbSetInit*/
{1,"user", 			0, FCT_C3USER, 7, sizeof(TC3User), C3UserInfoFields, NULL, NULL, NULL, NULL},
{2,"userauthorize", 	0, FCT_C3CARDAUTHORIZE, 3, sizeof(TC3authorize), C3UserAuthorizeFields, NULL, NULL, NULL, NULL},
{3,"holiday", 		 	0, FCT_C3HOLIDAY, 3, sizeof(TC3Holiday), C3HolidayFields, NULL, NULL, NULL, NULL},
{4,"timezone", 		0, FCT_C3TIMEZONE, 31, sizeof(TC3Timezone), C3TimezoneFields, NULL, NULL, NULL, NULL},
{5,"transaction", 		0, FCT_C3GUARDEREVENTLOG, 7, sizeof(TC3AcessLog), AcessLogFields, NULL, NULL, NULL, NULL},
{6,"firstcard", 			0, FCT_C3FIRSTCARD, 3, sizeof(TC3Firstcardopendoor), FirstCardFields, NULL, NULL, NULL, NULL},
{7,"multimcard", 		0, FCT_C3MULTICARDOPENDOOR, 7, sizeof(TC3Multicardassemble), MultiCardFields, NULL, NULL, NULL, NULL},
{8,"inoutfun",	 		0, FCT_C3INOUTFUN, 7, sizeof(TC3InOutFunDefine), InOutFunFields, NULL, NULL, NULL, NULL}
};

int GetTableCount()
{
	return sizeof(c3table_descs)/sizeof(c3table_descs[0]);
}

C3TABLE_DESC *getTableFromFCT(int FCT)
{
	int i;
	for(i=0;i<sizeof(c3table_descs)/sizeof(c3table_descs[0]);i++)
	{
		if(FCT==c3table_descs[i].FCT)
		{
			return c3table_descs+i;
		}
	}
	return NULL;
}

C3TABLE_DESC *getc3TableDesc(int TableID)
{
	int i;
	for(i=0;i<sizeof(c3table_descs)/sizeof(c3table_descs[0]);i++)
	{
		if(TableID==c3table_descs[i].tableID)
		{
			return c3table_descs+i;
		}
	}
	return NULL;
}

