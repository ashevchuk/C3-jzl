/*************************************************

 ZEM 200

 lcm.h the header file for LCD

 Copyright (C) 2003-2004, ZKSoftware Inc.

*************************************************/
#ifndef _LCM_H_
#define _LCM_H_

#include "ccc.h"
#include "locale.h"

#define DEFAULT_ROW_HEIGHT 16
#define LCD_BUFFER_ON   1
#define LCD_BUFFER_OFF  0

#define TriDir_Left 	0
#define TriDir_Right    1
#define TriDir_Top 	2
#define TriDir_Bottom 	3

#define AUX_ON  0xFF
#define AUX_OFF 0

#define MAX_CHAR_WIDTH 100


extern int gLCDWidth, gLCDHeight, gLCDCharWidth, gLCDRowCount, gRowHeight;


//operate lcd by hardware command
int  ExLCDOpen(void);
void ExLCDClose(void);
void ExClearLCD(void);
void ExPutPixelBuffer32(int row, int col, unsigned char *Buffer);
void ExPutPixelBuffer16(int row, int col, unsigned char *Buffer);
void ExPutPixelBuffer16H(int row, int col, unsigned char *Buffer);
//显示时间冒号
void ExLCDShowDot(BOOL b_Second);

//buffer function
int LCDInit(void);
void LCD_Final(void);
void LCDClear(void);            //直接清除缓冲区和LCD

void LCDInvalid(void);
//下面的函数只针对缓冲区操作，完成后，需要用LCDInvalid才能真正显示到LCD上
int LCD_OutDots16(int Row, int x, BYTE *Dots, int CharWidth);
int LCD_OutDotsX(int Row, int x, BYTE *Dots, int Width);

//动态界面显示使用
int LCD_OutBMP1Bit(int StartX, int StartY, BYTE *BMPData, int x1, int y1, int With, int Height, int Reverse);
#define LCD_BMP1Bit(x,y,bmp) LCD_OutBMP1Bit(x,y,bmp,0,0,-1,-1,0);
#define LCD_BMP1BitRev(x,y,bmp) LCD_OutBMP1Bit(x,y,bmp,0,0,-1,-1,1);
extern BYTE numBmpData[];
extern BYTE AmBmpData[];//modify by dsl 2007.6.27
extern BYTE PmBmpData[];
int LCD_Bar(int x1, int y1, int x2, int y2, int clear);
int LCD_ClearBar(int x1, int y1, int x2, int y2);
int LCD_DrawProgress(int x, int y, int width, int height, int count, int progress, int showBox, int clear);
int LCDBufferStart(int OnOff);

int LCD_SetPixel(int x, int y);
int LCD_ClearPixel(int x, int y);
int LCD_Line(int x1, int y1, int x2, int y2, int clear);
int LCD_Rectangle(int x1, int y1, int x2, int y2, int clear);
int LCD_Ellipse(int x1, int y1, int x2, int y2, int clear);
int LCD_Circle(int cx, int cy, int r, int clear);
int LCD_Triangle(int TopX, int TopY, int Height, int clear, int Direction);
void LCD_ClearFullBuff(void);
void LCD_ClearHalfBuff(void);

#define CHLB0   0x12
#define CHLB1   0x13
#define CHLT0   0x14
#define CHLT1   0x15
#define CHLL    0x16
#define CHTL    0x17

//行清屏
void LCDClearLine(int line);
//输出字符串相关函数
void LCDWritef(int line, char * fmt, ...);

//显示标志FLAG
#define LCD_WRAP        1 //折行
#define LCD_BOTTOM_LINE 2 //下划线
#define LCD_TOP_LINE    4 //上划线
#define LCD_HIGH_LIGHT  8 //反色高亮显示
#define LCD_LEFT_LINE   16
#define LCD_RIGHT_LINE  32

#define LCD_WRITE_ERROR_LNG -1
#define LCD_WRITE_ERROR_ROW -2
#define LCD_WRITE_OK		0
#define LCD_WRITE_FULL_COL	1
#define LCD_WRITE_FULL_ROW	2

#define REVERSE_DIAGONAL	2
#define REVERSE_COLOR		1

void LCDWriteStr(int row, int col, char *s, int flag);
void LCDWriteStrID(int row, int col, int StrID);
void LCDWriteCenterStrID(int row, int StrID);
void LCDWriteLineStrID(int row, int StrID);
int LCDWriteCenterStr(int row, const char *str);
//void LCDWriteCenter(int Row, const char *Text);
void LCDWriteLine(int row, char *Str);
void LCDFullALine(int row, char *hint);
char* PadRightStrStr(char *buf, char *Str, char *Value, int Width);
char* PadRightStrSID(char *buf, int StrID, const char *DefStr, char *Value, int Width);
char* PadRightIntSID(char *buf, int StrID, const char *DefStr, int Value, int Width);

//Display black/white image according to threshold
void DrawImage(char *image, int width, int height, int WhiteThreshold);

int LCD_DrawAnt(); //zsliu add 2008-10-08

//显示设备图标 MIFARE AND ETHERNET
void ExShowDevIcon(int Row, int Col);

int LCD_GetHeight();
int LCD_GetWidth();
int LCD_GetCharWidth();

void _LCD_OutBMP1BitData(int StartX, int StartY, BYTE *data, int bmpHeight, int bmpRowByte, int x1, int y1, int Width, int Height, int Reverse);
void DrawImage(char *image, int width, int height, int WhiteThreshold);
int LCD_OutBMP1Bit(int StartX, int StartY, BYTE *BMPData, int x1, int y1, int Width, int Height, int Reverse);

int lcdTextOut(int x, int y, char *data);
int lcdTextOutRect(int x, int y, int width, int height, char *data, int alignment);
int lcdGetTextExtent(char *data, int *size);
int lcdTextSelectFont(const char *fontName, int size, int properties, const char *charSet);

int LCDPutPixelBuffer16(int row, int col, char *cps, int flag, int CharWidth);
int LCDPutPixelBuffer(int row, int col, char *cps, int flag, int CharWidth);

void _LCDWriteStrLngDelay(PLangDriver Lng, int row, int col, char *string, int flag);


int LCDPutPixelBuffer8(int row, int col, char *cps, int flag, int CharWidth);
void LCDWriteStrLng(int row, int col, char *s, int flag);

void GetTimePixel(char *line1, char *line2, int Hour, int Minute);

char* PadMidStrStr(char *buf, char *Str, char *Value, int Width);
#endif
