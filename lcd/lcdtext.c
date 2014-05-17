#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "locale.h"
#include "lcm.h"
//#include "bidi.h"
#include "utils.h"

#define LCD_WRAP        1 //折行
#define LCD_BOTTOM_LINE 2 //下划线
#define LCD_TOP_LINE    4 //上划线
#define LCD_HIGH_LIGHT  8 //反色高亮显示
#define LCD_LEFT_LINE   16
#define LCD_RIGHT_LINE  32

typedef struct {
	char *name;
	int id;
}TLngName;

static TLngName lngNames[]={
	{"EN_US", LID_ISO8859_1},
	{"ZH_CN", LID_GB23122},
	{"ZH_TW", LID_BIG5},
};

static TLngName fontNames[]={
	{"System", 	LID_ISO8859_1},
	{"Arial", 	LID_ISO8859_1},
	{"Courier New", LID_ISO8859_1},
	{"宋体", 	LID_GB23122}, 
	{"细明体", 	LID_BIG5}
};

static int searchLngID(const char *lngName, TLngName *lngNames, int count)
{
	int i;
	for(i=0;i<count;i++)
		if(strcasecmp(lngName, lngNames[i].name)==0)
		{
			return lngNames[i].id;
		}
	return -1;
}

int getLngID(const char *lngName)
{
	int c=sizeof(lngNames)/sizeof(lngNames[0]);
	return searchLngID(lngName, lngNames, c);
}

int getFontLngID(const char *fontName)
{
	int c=sizeof(fontName)/sizeof(fontNames[0]);
	return searchLngID(fontName, fontNames, c);
}

int lcdTextSelectFont(const char *fontName, int size, int properties, const char *charSet)
{
	int id=-1;
	if(charSet && *charSet)
	{
		id=getLngID(charSet);
	}
	if(id==-1)
	{
		if(fontName==NULL || 0==*fontName) //no font name
		{
			return -1;
		}
		//from font name to charset
		id=getFontLngID(fontName);
	}
	if(id==-1) return id;
	if(0==SetDefaultLanguage(id, size)) return -2;
	printf("%s OK\n", __FUNCTION__);
	return 0;
}

int LCDWriteStr_Auto(PLangDriver Lng, int x, int y, int width, int height, char *string, int mode)
{
	BYTE cps[32];
	char S[10*1024], *s=S,*p=s;
	unsigned short *ucs=NULL;
	int startx, startw, gWidth=LCD_GetWidth(), gHeight=LCD_GetHeight();
	if(Lng==NULL) return LCD_WRITE_ERROR_LNG;
	strncpy(S,string,sizeof(S));
	if(x+width>gWidth) width=gWidth-x;
	if(y+height>gHeight) height=gHeight-y;
	startx=x; startw=width;
	if(Lng->RightToLeft)
	{
		int len;
		p=Lng->GetNextTextFun(Lng, s, width);
		if(p)
		{
			int len=p-s;
			char *ps=string+len-1, *sp=s;
			*p=0;
			while(p>sp)
			{
				if(' '==*ps--)
					*sp++=' ';
				else
					break;
			}
			ps=string;
			while(ps<string+len)
				if(*ps!=' ')
					break;
				else
					ps++;

			while((sp<p) && (ps<string+len))
			{
				*sp++=*ps++;
			}
			while(sp<p)
				*sp++=' ';
		}
		p=s;
		len=Lng->GetTextWidthFun(Lng, s);
		x+=(width-len);
	}
	mode|=REVERSE_COLOR|REVERSE_DIAGONAL;
	if(Lng->GetTextDotsFun==NULL)
	{//To UCS2
		unsigned short *tmpUCS=StrToUCS2(Lng->LanguageID, s);
		if(Lng->Bidi)
		{
			ucs=bidi_l2v(tmpUCS,1);
		}
		else
			ucs=tmpUCS;
	}
	
	while(1)
	{
		int DotsSize=32;
		int ByteCount;
		unsigned short *new_ucs=ucs;
		char *new_s=s;
		if(ucs)
			new_ucs=GetTextDots_UCS2(ucs, (char*)cps, &DotsSize, &ByteCount);
		else
			new_s=Lng->GetTextDotsFun(Lng, s, (char*)cps, &DotsSize, &ByteCount);
		if(ByteCount<=0) break;
		DotsSize/=16;

		if(DotsSize==0)
		{
			_LCD_OutBMP1BitData(x, y, (BYTE*)cps, 8, 1, 0, 0, Lng->CharWidth, 8, mode);
			x+=Lng->CharWidth;
			width-=Lng->CharWidth;
		}
		else if(DotsSize==1)
		{
			_LCD_OutBMP1BitData(x, y, (BYTE*)cps, 8, 1, 0, 0, Lng->CharWidth, 8, mode);
			_LCD_OutBMP1BitData(x, y+8, (BYTE*)cps+8, 8, 1, 0, 0, Lng->CharWidth, 8, mode);
			x+=Lng->CharWidth;
			width-=Lng->CharWidth;
		}
		else
		{
			if(width>=2*Lng->CharWidth)
			{
				_LCD_OutBMP1BitData(x, y, (BYTE*)cps, 8, 1, 0, 0, Lng->CharWidth, 8, mode);
				_LCD_OutBMP1BitData(x+8, y, (BYTE*)cps+8, 8, 1, 0, 0, Lng->CharWidth, 8, mode);
				_LCD_OutBMP1BitData(x, y+8, (BYTE*)cps+16, 8, 1, 0, 0, Lng->CharWidth, 8, mode);
				_LCD_OutBMP1BitData(x+8, y+8, (BYTE*)cps+24, 8, 1, 0, 0, Lng->CharWidth, 8, mode);
			}
			else
			{
				new_ucs=ucs;
				new_s=s;
			}
			x+=2*Lng->CharWidth;
			width-=2*Lng->CharWidth;
		}
		ucs=new_ucs;
		s=new_s;
		if(width<=0)
		{
			x=startx; width=startw;
			y+=Lng->CharHeight;
			height-=Lng->CharHeight;
			if(height<=0) break;
		}
	}

	return LCD_WRITE_OK;

}

int LCDWriteCenter(int row, const char *str)
{
	 return LCDWriteCenterStr(row, str);
}

int LCDWriteCenterStr(int row, const char *str)
{
	//return lcdTextOutRect(0, row*gLangDriver->CharHeight, LCD_GetWidth(), gLangDriver->CharHeight, (char *)str, 0);
	if(gLangDriver)
	{
		int i=gLangDriver->GetTextWidthFun(gLangDriver, str)/gLangDriver->CharWidth;
		i=(LCD_GetCharWidth()-2-i)/2+1;
		LCDWriteStr(row, i, str, 0);
	}
	return 0;
}

void LCDFullALine(int row, char *hint)
{
	int c=0;
	char p1[MAX_CHAR_WIDTH], *p2=NULL;
	strcpy(p1,hint);
	TrimStr(p1);
	c=strlen(p1);
	while(c--)
	{
		char x=hint[c];
		if(x==0x20)
		{
			p2=p1+c+1;
			break;
		}
	}
	if(p2==NULL)
		LCDWriteCenterStr(row, hint);
	else
	{
		char p3[MAX_CHAR_WIDTH];
		p1[c]=0;
		strcpy(p3,p2);
//		PadRightStrStr(p1, p1, p3, LCD_GetWidth()/gLangDriver->CharWidth);
		LCDWriteCenterStr(row, p1);
	}
}

void LCDClearLine(int line)
{
	LCD_ClearBar(0, line*gLangDriver->CharHeight, LCD_GetWidth(), (line+1)*gLangDriver->CharHeight);
}

int lcdTextOut(int x, int y, char *data)
{
	return LCDWriteStr_Auto(gLangDriver, x, y, 128, 64, data, 0);
}

int lcdTextOutRect(int x, int y, int width, int height, char *data, int alignment)
{
	return LCDWriteStr_Auto(gLangDriver, x, y, width, height, data, 0);
}

int lcdGetTextExtent(char *data, int *size)
{
	size[0]=gLangDriver->GetTextWidthFun(gLangDriver, data);
	size[1]=gLangDriver->GetTextHeightFun(gLangDriver, data);
	return 0;
}

