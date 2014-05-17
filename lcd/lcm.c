/*************************************************

 ZEM 200

 lcm.c LCD output functions support multi-language

 Copyright (C) 2003-2006, ZKSoftware Inc.

*************************************************/

#include <stdio.h>
#include <string.h>
#include <time.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>
#include <stdlib.h>
#include "lcm.h"
#include "exfun.h"
#include "utils.h"
#include "locale.h"
#include "c4i2c.h"

#define BIDI

#define MAX_CHAR_WIDTH 100

#define LCD_BUFFER_ON   1
#define LCD_BUFFER_OFF  0


//COMMAND
#define  LCD_DISPLAY_ON          0x01
#define  LCD_DISPLAY_OFF         0x02
#define  LCD_RESET               0x03
#define  LCD_DISPLAY_LINE_START  0x04
#define  LCD_WRITE_DATA          0x05
#define  LCD_READ_DATA           0x06
#define  LCD_SET_PAGE            0x07
#define  LCD_SET_ADDRESS         0x08
#define  LCD_READ_STATE          0x09

static int LCDBuffered=LCD_BUFFER_OFF;

static BYTE **LCDBuffer=NULL;

int gLCDWidth=128, gLCDHeight=64, gLCDCharWidth=16, gLCDRowCount=4, gRowHeight=16;
int gLCDData;

int LCD_GetHeight(){return gLCDHeight;}
int LCD_GetWidth(){return gLCDWidth;}
int LCD_GetCharWidth(){return gLCDCharWidth;}

void LCD_ShowImg(int row, int col, int bytes, unsigned char *Buffer)
{
//	LCD_OutDots16(row, col*bytes, Buffer, bytes);
	LCD_OutDotsX(row, col*bytes, Buffer, bytes);
}

int LCDBufferStart(int OnOff)
{
	if(LCDBuffered!=OnOff)
	{
		//printf("if(LCDBuffered!=OnOff)  ----- LCDBuffered: %d OnOff: %d\n",LCDBuffered, OnOff);
		LCDBuffered=OnOff;
		if(OnOff==LCD_BUFFER_OFF)
		{
			LCDInvalid();
		}
	}
	else
		return OnOff;

	return 1;
}

void ExLCDShowDot(BOOL b_Second)
{
	BYTE Dot32[]={
		0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
		0x00,0x00,0x00,0x00,0x0c,0x1e,0x1e,0x0c,0x00,0x00,
		0x00,0x00,0x00,0x00,0x80,0xc0,0xc0,0x80,0x00,0x00,
		0x00,0x00,0x00,0x00,0x01,0x03,0x03,0x01,0x00, //};
		0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00};
	LCDBufferStart(LCD_BUFFER_ON);
	if(b_Second)
	{
		LCD_ShowImg(3, 7, 8, (Dot32+8-2));
		LCD_ShowImg(4, 7, 8, (Dot32+16));
		LCD_ShowImg(5, 7, 8, (Dot32+24+2));
		LCD_ShowImg(3, 8, 8, (Dot32+8-2+8));
		LCD_ShowImg(4, 8, 8, (Dot32+16+8));
		LCD_ShowImg(5, 8, 8, (Dot32+24+2+8));
	}
	else
	{
		BYTE *buf=Dot32;

		LCD_ShowImg(3,7,8,buf);buf=Dot32;
		LCD_ShowImg(4,7,8,buf);buf=Dot32;
		LCD_ShowImg(5,7,8,buf);buf=Dot32;
		LCD_ShowImg(3,8,8,buf);buf=Dot32;
		LCD_ShowImg(4,8,8,buf);buf=Dot32;
		LCD_ShowImg(5,8,8,buf);
	}
	LCDBufferStart(LCD_BUFFER_OFF);
}

void LCD_Final(void)
{
	if(LCDBuffer)
		free(LCDBuffer);
	LCDBuffer=NULL;
}

int LCDInit(void)
{
	int i, RowCount,LCDWidth, LCDHeight;

	LCDWidth = 128;
	LCDHeight = 64;

	gLCDWidth=LCDWidth;//lcd点阵宽
	gLCDHeight=LCDHeight;//lcd点阵高
	gRowHeight=  16;//DEFAULT_ROW_HEIGHT = 16;//每个字的高
	gLCDRowCount=LCDHeight/DEFAULT_ROW_HEIGHT;//整屏最多有4行字
	gLCDCharWidth=LCDWidth/8;//整屏每行可显示16个asii
	RowCount=LCDHeight/8;//   8

	if(LCDBuffer)
		free(LCDBuffer);
	LCDBuffer=NULL;

	LCDBuffer=(BYTE**)malloc(RowCount*sizeof(BYTE*)+RowCount*2*LCDWidth);
	LCDBuffer[0]=(BYTE*)LCDBuffer+RowCount*sizeof(BYTE*);
	memset(LCDBuffer[0],0,RowCount*LCDWidth*2);
	for(i=1;i<RowCount;i++)
	{
		LCDBuffer[i]=LCDBuffer[i-1]+2*LCDWidth;
	}
	ExClearLCD();
	return TRUE;

}

int ExLCDOpen(void)
{
	LCDInit();
	ExInit();
	return 1;
}

void ExLCDClose(void)
{
	LCD_Final();
}

void LCDClear(void)		//直接清除缓冲区和LCD
{
	LCD_ClearFullBuff();
	ExClearLCD();
}

void LCDInvalid(void)	//重新绘制缓冲区数据，使得LCD的显示反映缓冲区真实内容
{
	int i, j, LCDCharWidth, RowCount;
	BYTE **LCDGrid;
	if(LCDBuffer==NULL) return;

#ifndef MACHINETYPE_C4
	return;
#endif


	//printf("LCDInvalid !---------------------  \n");

	LCDCharWidth=(gLCDWidth+7)/8;
	RowCount=gLCDHeight/8;
	LCDGrid=(BYTE**)malloc(RowCount*sizeof(BYTE*)+LCDCharWidth*RowCount);
	LCDGrid[0]=(BYTE*)LCDGrid+RowCount*sizeof(BYTE*);
	for(i=1;i<RowCount;i++)
		LCDGrid[i]=LCDGrid[i-1]+LCDCharWidth;
	memset(LCDGrid[0],0,LCDCharWidth*RowCount);
	for(i=0;i<RowCount;i++)
	{
		for(j=0;j<gLCDWidth;j++)
			if(LCDBuffer[i][j]!=LCDBuffer[i][j+gLCDWidth])
				LCDGrid[i][j/8]=1;
	}
	for(i=0;i<(RowCount)/2;i++)
	{
		for(j=0;j<gLCDWidth/8-1;j++)
		{
			if((LCDGrid[i*2][j] && LCDGrid[i*2+1][j+1]) ||
				(LCDGrid[i*2][j+1] && LCDGrid[i*2+1][j]))
			{
				unsigned char Dots[32];
				memcpy(Dots, LCDBuffer[i*2]+j*8, 16);
				memcpy(Dots+16, LCDBuffer[i*2+1]+j*8, 16);
				ExPutPixelBuffer32(i,j,Dots);
				LCDGrid[i*2][j]=0;
				LCDGrid[i*2][j+1]=0;
				LCDGrid[i*2+1][j]=0;
				LCDGrid[i*2+1][j+1]=0;
			}
		}
	}
	for(i=0;i<RowCount-1;i++)
	{
		for(j=0;j<LCDCharWidth;j++)
		{
			if(LCDGrid[i][j] || LCDGrid[i+1][j])
			{
				unsigned char Dots[32];
				memcpy(Dots, LCDBuffer[i]+j*8, 8);
				memcpy(Dots+8, LCDBuffer[i+1]+j*8, 8);
				ExPutPixelBuffer16(i,j,Dots);
				LCDGrid[i][j]=0;
				LCDGrid[i+1][j]=0;
			}
		}
	}
	for(i=0;i<RowCount;i++)
	{
		for(j=0;j<LCDCharWidth-1;j++)
		{
			if(LCDGrid[i][j] || (((j==LCDCharWidth-2)||(j==LCDCharWidth/2-2)) && LCDGrid[i][j+1]))
			{
				ExPutPixelBuffer16H(i,j,(unsigned char*)LCDBuffer[i]+j*8);
				LCDGrid[i][j]=0;
				LCDGrid[i][j+1]=0;
			}
		}
	}

	free(LCDGrid);
	for(i=0;i<RowCount;i++)
		memcpy(LCDBuffer[i]+gLCDWidth, LCDBuffer[i], gLCDWidth);
}

int LCD_OutDotsX(int Row, int x, BYTE *Dots, int Width)
{
	if(LCDBuffer==NULL) return 0;
	if(Row>=0 && Row<gLCDHeight/8 && x>=0)
	{
		int c=gLCDWidth-x;
		if(c>Width) c=Width;
		if(c>0)
		{
			memcpy(LCDBuffer[Row]+x,Dots,c);
			return TRUE;
		}
	}
	return FALSE;
}

int LCD_OutDots16(int Row, int x, BYTE *Dots, int CharWidth)
{
	if(LCD_OutDotsX(Row, x, Dots, CharWidth))
		return LCD_OutDotsX(Row+1, x, Dots+CharWidth, CharWidth);
	else
		return FALSE;
}

static int RightToLeft=0;

#define ClearPixelLng(x,y)        do{\
        if(RightToLeft)\
                LCD_ClearPixel(gLCDWidth-(x),y); \
        else\
                LCD_ClearPixel(x,y); }while(0)

#define _SetPixelLng(x,y)        do{\
        if(RightToLeft)\
                LCD_SetPixel(gLCDWidth-(x),y); \
        else\
                LCD_SetPixel(x,y); }while(0)

#define SetPixelLng(x,y,clear)  do{ if(clear) ClearPixelLng(x,y); else _SetPixelLng(x,y);}while(0)

int LCD_Line(int x1, int y1, int x2, int y2, int clear)
{
        const int p=100;
        const int pHalf=p / 2;
        int   i,j,dx,dy,d,di,dj,Result;
        di = y2 - y1;
        if(di<0)
        {
                di=y2;y2=y1;y1=di;di=y2-y1;
	}
        dj = x2 - x1;
        if(dj<0)
        {
                dj=x2;x2=x1;x1=dj;dj=x2-x1;
        }

        Result = 0;
        if((di == 0) && (dj == 0)) return Result;

        if(abs(dj) > abs(di))
        {
                if(dj>0) dx=1; else dx=-1;
                d = di * p / abs(dj);
                i = y1;
                dy = 0;
                j = x1;
                while(j <= x2)
                {
                        SetPixelLng(j, i, clear);
                        Result++;
                        dy += d;
                        if(dy >= pHalf) { dy -= p; i++; }
                        if(dy <= -pHalf) { dy += p; i--; }
                        j += dx;
                }
        }
        else
        {
                if(di>0) dy=1; else dy=-1;
                d = dj * p / abs(di);
                j = x1;
                dx = 0;
                i = y1;
                while(i <= y2)
                {
                        SetPixelLng(j, i, clear);
                        Result++;
                        dx += d;
                        if(dx >= pHalf) { dx -= p; j++; }
                        if(dx <= -pHalf) { dx += p; j--; }
                        i += dy;
                }
        }
        return Result;
}

int LCD_Rectangle(int x1, int y1, int x2, int y2, int clear)
{
	LCD_Line(x1,y1,x2,y1,clear);
	LCD_Line(x2,y1,x2,y2,clear);
	LCD_Line(x2,y2,x1,y2,clear);
	LCD_Line(x1,y2,x1,y1,clear);
	return TRUE;
}

void fourPoint(int cx, int cy, int x, int y)
{
	LCD_SetPixel(cx+x,cy+y);
	LCD_SetPixel(cx+x,cy-y);
	LCD_SetPixel(cx-x,cy+y);
	LCD_SetPixel(cx-x,cy-y);
}

void circlePoint(int cx, int cy, int x, int y)
{
	fourPoint(cx, cy, x, y);
	fourPoint(cx, cy, y, x);
}

int LCD_Circle(int cx, int cy, int r, int clear)
{
	int d,x,y;
	if(r<=0) return FALSE;
	d=1-r;
	x=0;
	y=r;
	while(x<=y)
	{
		circlePoint(cx,cy,x,y);
		if(d<=0)
			d+=2*x+3;
		else
		{
			d+=2*(x-y)+5;
			y--;
		}
		x++;
	}
	return TRUE;
}

int LCD_Triangle(int TopX, int TopY, int Height, int clear, int Direction)
{
	int i, startx, starty, endx, endy, dx, dy;
	switch(Direction)
	{
	case TriDir_Left:	dx=-1;dy=0;	break;
	case TriDir_Right:	dx=1;dy=0;	break;
	case TriDir_Top	:	dx=0;dy=-1;	break;
	default:			dx=0;dy=1;	break;
	}
	startx=TopX; starty=TopY; endx=TopX; endy=TopY;
	for(i=0;i<Height;i++)
	{
		int x=startx, y=starty;
		while(1)
		{
			//DBPRINTF("----X=%d,y=%d,",x,y);
			SetPixelLng(x,y, clear);
//			LCD_SetPixel(x,y);
			if(x==endx && y==endy) break;
			y-=dx; x-=dy;
		}
		startx+=dx+dy;endx+=dx-dy;
		starty+=dx+dy;endy+=dy-dx;
	}
	return TRUE;
}

int LCD_Ellipse(int x1, int y1, int x2, int y2, int clear)
{
	int d1,d2;
	int x=0,y;
	int a,b,cx,cy;
	cx=(x1+x2)/2;
	cy=(y1+y2)/2;
	a=abs(x2-x1)/2;
	b=abs(y2-y1)/2;
	y=b;
	d1=4*b*b+a*a*(1-4*b);
	fourPoint(cx,cy,x,y);
	while( b*b*(x+1)*2 < a*a*(2*y-1) )
	{
		if(d1<=0)
		{
			d1+=4*b*b*(2*x+3);
			x++;
		}
		else
		{
			d1+=4*b*b*(2*x+3)+4*a*a*(2-2*y);
			x++;
			y--;
		}
		fourPoint(cx,cy,x,y);
	}
	d2=b*b*(4*x+1)*(4*x+1)+16*a*a*(y-1)*(y-1)-16*a*a*b*b;
	while(y>0)
	{
		if(d2<=0)
		{
			d2+=16*b*b*(2*x+2)+16*a*a*(3-2*y);
			x++;
			y--;
		}
		else
		{
			d2+=16*a*a*(3-2*y);
			y--;
		}
		fourPoint(cx,cy,x,y);
	}

	return TRUE;
}

void LCD_ClearFullBuff(void)
{
	int i;
	if(LCDBuffer)
	for(i=0;i<gLCDHeight/8;i++)
	{
		memset(LCDBuffer[i],0,2*gLCDWidth);
	}
}

void LCD_ClearHalfBuff(void)
{
	int i;
	if(LCDBuffer)
	for(i=0;i<gLCDHeight/8;i++)
	{
		memset(LCDBuffer[i],0,gLCDWidth);
	}
}

int XY2Index(int x, int y)
{
	int row=y/8;
	y-=row*8;
#ifdef ZEM500
	x=(row*128*2+x)*8+y;	//在F7上出现菜单中箭头显示问题，故改
#else
	x=(row*gLCDWidth*2+x)*8+y;
#endif
	return x;
}

int LCD_SetPixel(int x, int y)
{
	if(LCDBuffer==NULL) return 0;
	int Index=XY2Index(x,y);
	if(Index>=0 && Index<gLCDWidth*gLCDHeight*2)
	{
		SetBit(LCDBuffer[0], Index);
		return TRUE;
	}
	else
		return FALSE;
}

int LCD_ClearPixel(int x, int y)
{
	if(LCDBuffer==NULL) return 0;
	int Index=XY2Index(x,y);
	if(Index>=0 && Index<gLCDWidth*gLCDHeight*2)
	{
		ClearBit(LCDBuffer[0], Index);
		return TRUE;
	}
	else
		return FALSE;
}

int LCDPutPixelBuffer(int row, int col, char *cps, int flag, int CharWidth)
{
	int i;
	if(flag & LCD_BOTTOM_LINE)
	{
		for(i=CharWidth*2;i<CharWidth*4;i++)
			cps[i]|=0x80;
	}
	if(flag & LCD_TOP_LINE)
	{
		for(i=0;i<CharWidth*2;i++)
			cps[i]|=1;
	}
	if(flag & LCD_RIGHT_LINE)
	{
		cps[CharWidth*2-1]=(char)0xff;cps[CharWidth*4-1]=(char)0xff;
	}
	if(flag & LCD_LEFT_LINE)
	{
		cps[0]=(char)0xff;cps[CharWidth*2]=(char)0xff;
	}
	if(flag & LCD_HIGH_LIGHT)
	{
		for(i=0;i<CharWidth*4;i++)
			cps[i]=~cps[i];
	}
	LCD_OutDots16(row, col*CharWidth, (BYTE*)cps, CharWidth*2);
	return 1;
}

int LCDPutPixelBuffer8(int row, int col, char *cps, int flag, int CharWidth)
{
	int i;
	if(flag & LCD_BOTTOM_LINE)
	{
		for(i=0;i<CharWidth;i++)
			cps[i]|=0x80;
	}
	if(flag & LCD_TOP_LINE)
	{
		for(i=0;i<CharWidth;i++)
			cps[i]|=1;
	}
	if(flag & LCD_RIGHT_LINE)
	{
		cps[CharWidth-1]=(char)0xff;
	}
	if(flag & LCD_LEFT_LINE)
	{
		cps[0]=(char)0xff;
	}
	if(flag & LCD_HIGH_LIGHT)
	{
		for(i=0;i<8;i++)
			cps[i]=~cps[i];
	}
	LCD_OutDotsX(row, col*CharWidth, (BYTE*)cps, CharWidth);
	return 1;
}

int LCDPutPixelBuffer16(int row, int col, char *cps, int flag, int CharWidth)
{
	int i;
	if(flag & LCD_BOTTOM_LINE)
	{
		for(i=CharWidth;i<CharWidth*2;i++)
			cps[i]|=0x80;
	}
	if(flag & LCD_TOP_LINE)
	{
		for(i=0;i<CharWidth;i++)
			cps[i]|=1;
	}
	if(flag & LCD_RIGHT_LINE)
	{
		cps[CharWidth-1]=(char)0xff;cps[CharWidth*2-1]=(char)0xff;
	}
	if(flag & LCD_LEFT_LINE)
	{
		cps[0]=(char)0xff;cps[CharWidth]=(char)0xff;
	}
	if(flag & LCD_HIGH_LIGHT)
	{
		for(i=0;i<CharWidth*2;i++)
			cps[i]=~cps[i];
	}
	LCD_OutDots16(row, col*CharWidth, (BYTE*)cps, CharWidth);
	return 1;
}

char* PadRightStrStr(char *buf, char *Str, char *Value, int Width)
{
        if(gLangDriver)
        {
                if(!gLangDriver->RightToLeft)
                        PadMidStrStr(buf, Str, Value, Width);
                else
                        PadMidStrStr(buf, Value, Str, Width);
        }
        return buf;
}


char* PadRightStrSID(char *buf, int StrID, const char *DefStr, char *Value, int Width)
{
	return PadRightStrStr(buf, LoadStrByIDDef(StrID, DefStr), Value, Width);
}

char* PadRightIntSID(char *buf, int StrID, const char *DefStr, int Value, int Width)
{
	char StrValue[100];
	sprintf(StrValue, "%d", Value);
	return PadRightStrSID(buf, StrID, DefStr, StrValue, Width);
}

void LCDWriteLineStrID(int row, int StrID)
{
	char buf[100];
	char *tmp=LoadStrByID(StrID);
	if(tmp)
		sprintf(buf, "%-80s", tmp);
	else
		sprintf(buf, "%-80s", " ");
	LCDWriteStr(row, 0, buf, 0);
}


//Display black/white image according to threshold
void DrawImage(char *image, int width, int height, int WhiteThreshold)
{
	BYTE ibuf[128*64/8], pbuf[32];
	int w,h,i,j, hist[256], row;

	//calculate the threshold
	memset(hist, 0, sizeof(int)*256);
	memset(ibuf, 0xff, sizeof(ibuf));
	h=0;
	for(i=0;i<height;i++)
		for(j=0;j<width;j++)
			hist[(BYTE)image[h++]]++;
	h=0;w=width*height;
	for(i=0;i<256;i++)
	{
		h+=hist[i];
		if(h*100>w*WhiteThreshold)
		{
			WhiteThreshold=i;
			break;
		}
	}

	//calculate the real size
	w=gLCDWidth/2;
	h=gLCDHeight;
	if(w*height<h*width)
		h=height*w/width;
	else
		w=width*h/height;

	//take a black/white image
	for(i=0;i<h;i++)
	{
		int k,W;
		BYTE *p=ibuf, *img=(BYTE*)image;
		int H=i*height/h;
		p+=i*gLCDWidth/8;
		img+=width*H;
		for(j=0;j<(w+7)/8;j++)
		{
			*p=0;
			for(k=0;k<8;k++)
			{
				W=(j*8+k)*width/w;
				if(W<width)
				{
					if(img[W]>WhiteThreshold)
						*p|=(0x80>>k);
				}
				else
					*p|=(0x80>>k);
			}
			p++;
		}
	}

	//render the image to LCD
	for(row=0;row<(h+15)/16;row++)
	{
		int x=0,CharIndex;
		BYTE *(p[8]), b;
		for(i=0;i<8;i++)
			((int *)pbuf)[i]=0;
		for(CharIndex=0;CharIndex<(w+15)/16;CharIndex++)
		{
			for(i=0;i<8;i++)
				p[i] = ibuf+(i+row*16)*gLCDWidth/8+x;
			for(i=0;i<8;i++)
			{
				b = 0x80 >> i;
				pbuf[i] = 0;
				for(j=0;j<8;j++)
					if(0==(b & *(p[j])))
						pbuf[i] |= (1 << j);
				pbuf[i+8] = 0;
				for(j=0;j<8;j++)
					if(0==(*(p[j]+1) & b))
						pbuf[i+8] |= (1 << j);
			}
			for(i=0;i<8;i++)
				p[i] = ibuf+(i+row*16+8)*gLCDWidth/8+x;
			for(i=0;i<8;i++)
			{
				b = 0x80 >> i;
				pbuf[i+16] = 0;
				for(j=0;j<8;j++)
					if((b & *p[j])==0)
						pbuf[i+16] |= (1 << j);
				pbuf[i+24] = 0;
				for(j=0;j<8;j++)
					if(0==(*(p[j]+1) & b))
						pbuf[i+24] |= (1 << j);
			}
			ExPutPixelBuffer32(row*2, x, (unsigned char*)pbuf);
			x+=2;
		}
	}
}

//use for new mainform
int LCD_Bar(int x1, int y1, int x2, int y2, int clear)
{
        while(y1<=y2)
        {
                LCD_Line(x1,y1,x2,y1,clear); y1++;
        }
        return TRUE;
}

int LCD_ClearBar(int x1, int y1, int x2, int y2)
{
        int x,y;
        if(RightToLeft)
        {
                x=x1;
                x1=gLCDWidth-x2;
                x2=gLCDWidth-x;
        }
        for(x=x1;x<x2;x++)
        for(y=y1;y<y2;y++)
        {
                LCD_ClearPixel(x,y);
        }
        return TRUE;
}


void _LCD_OutBMP1BitData(int StartX, int StartY, BYTE *data, int bmpHeight, int bmpRowByte, int x1, int y1, int Width, int Height, int Reverse)
{
	int y,x,rdiagonal=Reverse&REVERSE_DIAGONAL;
        for(y=0;y<Height;y++)
        for(x=0;x<Width;x++)
        {
                int i,j,rcolor=Reverse&REVERSE_COLOR;
                i=(x+x1)/8+(bmpHeight-1-(y+y1))*bmpRowByte;
                j=7-((x+x1)%8);
		if(rdiagonal)
		{
			int r=i;
			i=7-j;
			j=7-r;
		}
                if(((0!=(data[i] & (1<<j))) && !rcolor) ||((0==(data[i] & (1<<j))) && rcolor))
                        LCD_ClearPixel(StartX+x, StartY+y);
                else
                        LCD_SetPixel(StartX+x, StartY+y);
        }
 }

int _LCD_OutBMP1Bit(int StartX, int StartY, BYTE *BMPData, int x1, int y1, int Width, int Height, int Reverse)
{
        BYTE *data=BMPData+BMPData[0xa]+256*BMPData[0xb];
        int bmpWidth, bmpHeight, bmpRowByte;
        bmpWidth=BMPData[0x12]+256*BMPData[0x13];
        bmpHeight=BMPData[0x16]+256*BMPData[0x17];
        if(Width==-1) Width=bmpWidth;
        if(Height==-1) Height=bmpHeight;
        bmpRowByte=((bmpWidth+7)/8+3)/4*4;
        if(BMPData[2]+256*BMPData[3]!=BMPData[0xa]+256*BMPData[0xb]+
                bmpRowByte*bmpHeight) return FALSE;
	_LCD_OutBMP1BitData(StartX, StartY, data, bmpHeight, bmpRowByte, x1, y1, Width, Height, Reverse);
        return TRUE;
}
int LCD_OutBMP1Bit(int StartX, int StartY, BYTE *BMPData, int x1, int y1, int Width, int Height, int Reverse)
{

        if(RightToLeft)
                StartX=gLCDWidth+1-StartX-(Width==-1?32:Width);
        return _LCD_OutBMP1Bit(StartX, StartY, BMPData, x1, y1, Width, Height, Reverse);
}

int LCD_DrawProgress(int x, int y, int width, int height, int count, int progress, int showBox, int clear)
{
        int barWidth=(width-3)/count-1;
        int i;
        width=barWidth*count+3;
        LCD_Rectangle(x,y,x+width-1,y+height-1, clear);
        x+=2;
        for(i=0;i<count;i++)
        {
                if(i<progress)
                        LCD_Bar(x, y+2, x+barWidth-2, y+height-3, clear);
                else if(showBox)
                        LCD_Rectangle(x, y+2, x+barWidth-2, y+height-3, clear);
                x+=barWidth;
        }
        return TRUE;
}

void _LCDWriteStrLngDelay(PLangDriver Lng, int row, int col, char *string, int flag)
{
        int Theflag, r, mcol, colc;
        BYTE cps[32];
        char S[10*1024]={0}, *s=S, *p=s;
        unsigned short *ucs=NULL;

        if(Lng==NULL)
        {
                return ;
        }

	colc=gLCDWidth/Lng->CharWidth;
        strncpy(S, string, sizeof(S));

        if(gLCDHeight/Lng->CharHeight<=row)   return;

        if(Lng->GetTextDotsFun==NULL)
	{
		unsigned short *tmpUCS=StrToUCS2(Lng->LanguageID, s);
		unsigned short *bidi_l2v(const unsigned short *uscbuf, int orientation);
		if(Lng->Bidi)
			ucs=bidi_l2v(tmpUCS,1);
		else
			ucs=tmpUCS;
	}
	r=row*Lng->CharHeight/8;

	while(1)
	{
		int DotsSize=32;
		int ByteCount;
		unsigned short *new_ucs=ucs;
		char *new_s=s;

		Theflag=flag & ~(LCD_LEFT_LINE | LCD_RIGHT_LINE);
		if(s==p)
			Theflag=Theflag | (LCD_LEFT_LINE & flag);
		if(s[1]==0)
			Theflag=Theflag | (LCD_RIGHT_LINE & flag);

		if(ucs)
		{
			new_ucs=GetTextDots_UCS2(ucs, (char*)cps, &DotsSize, &ByteCount);
		}
		else
		{
			new_s=Lng->GetTextDotsFun(Lng, s, (char*)cps, &DotsSize, &ByteCount);
		}

		//printf("DotsSize=%d	ByteCount=%d  Theflag=%d\n", DotsSize, ByteCount, Theflag);
		if(ByteCount<=0) break;
		DotsSize/=16;
		mcol=col;
		if(DotsSize==0)
		{
			LCDPutPixelBuffer8(r,mcol,(char*)cps, Theflag, Lng->CharWidth);
			col++;
		}
		else if(DotsSize==1)
		{
			LCDPutPixelBuffer16(r,mcol,(char*)cps, Theflag, Lng->CharWidth);
			col++;
		}
		else
		{
			if(!(flag & LCD_WRAP) || col+2<=colc)
				LCDPutPixelBuffer(r,mcol,(char*)cps, Theflag, Lng->CharWidth);
			else
			{
				new_ucs=ucs;
				new_s=s;
			}
			col+=2;
		}

		ucs=new_ucs;
		s=new_s;
		if(col>=colc)
		{
			if(!(flag & LCD_WRAP)) return ;
			col=0;
			row++;
			r=row*Lng->CharHeight/8;
			if(row==gLCDHeight/Lng->CharHeight) return ;
		}
	}
	return ;
}

void LCDWriteStr(int row, int col, char *s, int flag)
{
	_LCDWriteStrLngDelay(gLangDriver,  row, col, s, flag);
	if(LCDBuffered==LCD_BUFFER_OFF)	LCDInvalid();
}

void LCDWriteCenterStrID(int row, int StrID)
{
	LCDWriteCenterStr(row, LoadStrByID(StrID));
}
void LCDWriteStrLng(int row, int col, char *s, int flag)
{
	_LCDWriteStrLngDelay(gSymbolDriver, row, col, s, flag);
	if(LCDBuffered==LCD_BUFFER_OFF) LCDInvalid();
}

void GetNumberChar(char *line1, char *line2, int Number)
{
	*line1=(char)254;
	line1[1]=(Number*2+161);
	*line2=(char)254;
	line2[1]=(Number*2+162);
}

void GetTimePixel(char *line1, char *line2, int Hour, int Minute)
{
	memset(line1, 32, gLCDCharWidth);
	memset(line2, 32, gLCDCharWidth);
	GetNumberChar(line1+3, line2+3, Hour/10);
	GetNumberChar(line1+5, line2+5, Hour%10);
	line1[7]=32;line1[8]=32;line2[7]=32;line2[8]=32;//靠縋osition
	GetNumberChar(line1+9, line2+9, Minute/10);
	GetNumberChar(line1+11, line2+11, Minute%10);
}
/*
void LCDWriteCenter(int Row, char *Text)
{
	if(gLangDriver)
	{
		int i=gLangDriver->GetTextWidthFun(gLangDriver, Text)/gLangDriver->CharWidth;
		i=(gLCDCharWidth-2-i)/2+1;
		LCDWriteStr(Row, i, Text, 0);
	}
}
*/
char* PadMidStrStr(char *buf, char *Str, char *Value, int Width)
{
	char *p, vbuf[40];
        int vwidth;
        int lwidth;

	printf("str=%s, value=%s\n", Str, Value);
	if(Value)	strcpy(vbuf,Value);
        vwidth=gLangDriver->GetTextWidthFun(gLangDriver,TrimRightStr(vbuf))/gLangDriver->CharWidth;
        sprintf(buf, "%s%20s", Str, " ");
        lwidth=Width-vwidth;
        if(lwidth<0) lwidth=0;
        p=gLangDriver->GetNextTextFun(gLangDriver, buf, lwidth*gLangDriver->CharWidth);
        if(p && Value)
	                sprintf(p,"%s", vbuf);
	printf("buf=%s\n", buf);
        return buf;
}
