/********************************************************\
|*                                                      *|
|*                  BiokeyEM 2.1                        *|
|*                                                      *|
|*  ZF_Sys.h                                            *|
|*    Header file for System module                     *|
|*                                                      *|
|*  Copyright (C) 1998-2003 ZKSoftware Inc.             *|
|*                                                      *|
\********************************************************/


#ifndef __ZF_SYS_INCLUDED__
#define __ZF_SYS_INCLUDED__


#ifdef __cplusplus
extern "C++"
{
#endif

#include <stdlib.h>
#include <string.h>
#include <math.h>

#ifdef __cplusplus
}
#endif


//#define ZFINGER_API __stdcall
#define ZFINGER_API
#ifndef RESOLUTION_HIGH
#ifndef RESOLUTION_LOW
#define RESOLUTION_HIGH 
//#define RESOLUTION_LOW
#endif
#endif

#ifndef VOID
typedef void VOID;
#endif
typedef unsigned char BYTE;
typedef signed char SBYTE;
typedef unsigned short WORD;
typedef unsigned short USHORT;
typedef short SHORT;
typedef int INT;
typedef unsigned int UINT;
typedef unsigned long DWORD;
typedef char CHAR;
typedef int BOOL;
typedef float FLOAT;
typedef double DOUBLE;

#ifndef NULL
#define NULL 0
#endif

#ifndef FALSE
#define FALSE 0
#endif
#ifndef TRUE
#define TRUE 1
#endif

#if defined(__GNU__)
	#define PACKED
	#define GCC_PACKED __attribute__((packed))
	#define GCC_ALIGN0 __attribute__((aligned(1)))
	#define ALIGN0
#elif defined(__arm)
	#define PACKED __packed
	#define GCC_PACKED
	#define ALIGN0 __packed
	#define GCC_ALIGN0
#elif defined(__DCC__)
	#define PACKED  __packed__
	#define GCC_PACKED
	#define ALIGN0
	#define GCC_ALIGN0
#else
  	#define PACKED 
	#define GCC_PACKED
	#define ALIGN0
	#define GCC_ALIGN0
#endif

#define BLOCK_SIZE 16
#define BLOCK_SIZE_F BLOCK_SIZE
#define ZF_IMAGE_RESOLUTION 500

#define ZF_B_WIDTH (ZF_WIDTH/BLOCK_SIZE)
#define ZF_B_HEIGHT (ZF_HEIGHT/BLOCK_SIZE)

#define MAX_SINGULAR_POINT_COUNT 64

typedef struct _SINGULARPOINTS {
	INT Count;
	INT X[MAX_SINGULAR_POINT_COUNT];
	INT Y[MAX_SINGULAR_POINT_COUNT];
	INT D[MAX_SINGULAR_POINT_COUNT];
	INT T[MAX_SINGULAR_POINT_COUNT];
} SINGULARPOINTS, *PSINGULARPOINTS;


#define MAX_MINUTIA_COUNT 512

typedef struct _MINUTIAE {
	INT Count;
	INT X[MAX_MINUTIA_COUNT];
	INT Y[MAX_MINUTIA_COUNT];
	INT D[MAX_MINUTIA_COUNT];	//Direction
	INT T[MAX_MINUTIA_COUNT];	//Minutiae type
	INT C[MAX_MINUTIA_COUNT];	//curvature
} MINUTIAE, *PMINUTIAE;

#define ZF_MAX_IMAGE_DIMENSION 1024
//((ZF_HEIGHT>ZF_WIDTH)?ZF_HEIGHT:ZF_WIDTH)
#define ZF_MAX_FEATURES_DIMENSION ZF_MAX_IMAGE_DIMENSION

#define ZF_MAX_BLOCKED_ORIENTS_DIMENSION (ZF_MAX_FEATURES_DIMENSION/8)

typedef struct _BLOCKEDORIENTS
{
	INT Width;
	INT Height;
	BYTE Bits[ZF_MAX_BLOCKED_ORIENTS_DIMENSION][ZF_MAX_BLOCKED_ORIENTS_DIMENSION];
} BLOCKEDORIENTS, * PBLOCKEDORIENTS;

typedef struct _FEATURES
{
	BYTE G;				//纹理密度
	MINUTIAE M;			//Minutiae
	SINGULARPOINTS SP;	//core point
	BLOCKEDORIENTS BO;	//Block orient
} FEATURES, * PFEATURES;


#define Min(a, b) ((a) < (b) ? (a) : (b))
#define Max(a, b) ((a) > (b) ? (a) : (b))
#define Abs(x) ((x)>=0?(x):-(x))
#define ZFAbs(x) abs(x)
#define ZFMax(a, b) ((b) > (a) ? (b) : (a))
#define ZFMin(a, b) ((b) < (a) ? (b) : (a))
#define ZFRound(x) ((INT)((x) >= 0 ? (x) + 0.5 : (x) - 0.5))
#define ZFRoundP(x) ((INT)((x) + 0.5))
#define ZFSqr(x) ((x) * (x))

extern INT ZFSqr255[256];

#define ZF_SQRT_8_MAX (1 << 8)
#define ZF_SQRT_10_MAX (1 << 10)
#define ZF_SQRT_12_MAX (1 << 12)
#define ZF_SQRT_14_MAX (1 << 14)
#define ZF_SQRT_16_MAX (1 << 16)
#define ZF_SQRT_10_SHIFT 4
#define ZF_SQRT_12_SHIFT 5
#define ZF_SQRT_14_SHIFT 6
#define ZF_SQRT_16_SHIFT 7
#define ZF_SQRT_10_DELTA (1 << ZF_SQRT_10_SHIFT)
#define ZF_SQRT_12_DELTA (1 << ZF_SQRT_12_SHIFT)
#define ZF_SQRT_14_DELTA (1 << ZF_SQRT_14_SHIFT)
#define ZF_SQRT_16_DELTA (1 << ZF_SQRT_16_SHIFT)
#define ZF_SQRT_8_LENGTH ZF_SQRT_8_MAX
#define ZF_SQRT_10_LENGTH ((ZF_SQRT_10_MAX - ZF_SQRT_8_MAX) >> ZF_SQRT_10_SHIFT)
#define ZF_SQRT_12_LENGTH ((ZF_SQRT_12_MAX - ZF_SQRT_10_MAX) >> ZF_SQRT_12_SHIFT)
#define ZF_SQRT_14_LENGTH ((ZF_SQRT_14_MAX - ZF_SQRT_12_MAX) >> ZF_SQRT_14_SHIFT)
#define ZF_SQRT_16_LENGTH ((ZF_SQRT_16_MAX - ZF_SQRT_14_MAX) >> ZF_SQRT_16_SHIFT)
#define ZF_SQRT_8_INDEX(x) (x)
#define ZF_SQRT_10_INDEX(x) ((x >> ZF_SQRT_10_SHIFT) - ZF_SQRT_10_DELTA)
#define ZF_SQRT_12_INDEX(x) ((x >> ZF_SQRT_12_SHIFT) - ZF_SQRT_12_DELTA)
#define ZF_SQRT_14_INDEX(x) ((x >> ZF_SQRT_14_SHIFT) - ZF_SQRT_14_DELTA)
#define ZF_SQRT_16_INDEX(x) ((x >> ZF_SQRT_16_SHIFT) - ZF_SQRT_16_DELTA)
extern BYTE ZFSqrt8[ZF_SQRT_8_LENGTH];
extern BYTE ZFSqrt10[ZF_SQRT_10_LENGTH];
extern BYTE ZFSqrt12[ZF_SQRT_12_LENGTH];
extern BYTE ZFSqrt14[ZF_SQRT_14_LENGTH];
extern BYTE ZFSqrt16[ZF_SQRT_16_LENGTH];

#define ZFSqr255(x) ((x) < 0 ? 0 : (x) > 255 ? ZFSqr(255) : ZFSqr255[x])

#define ZFSqrt255Value(x, result) \
	(result) = ((x) < 0 ? 0\
		: (x) < ZF_SQRT_8_MAX ? ZFSqrt8[ZF_SQRT_8_INDEX(x)]\
		: (x) < ZF_SQRT_10_MAX ? ZFSqrt10[ZF_SQRT_10_INDEX(x)]\
		: (x) < ZF_SQRT_12_MAX ? ZFSqrt12[ZF_SQRT_12_INDEX(x)]\
		: (x) < ZF_SQRT_14_MAX ? ZFSqrt14[ZF_SQRT_14_INDEX(x)]\
		: (x) < ZF_SQRT_16_MAX ? ZFSqrt16[ZF_SQRT_16_INDEX(x)]\
		: 255)

#define ZFSqrt255(x, result) \
{\
	ZFSqrt255Value(x, result);\
	if(ZFSqr255[(result) + 1] - (x) < (x) - ZFSqr255[result]) ++(result);\
}

#define ZFSqrt255NR(x, result) \
{\
	ZFSqrt255Value(x, result);\
	if(ZFSqr255[result] > x) --(result);\
}

#define ZFSqrt255CD(x, result) \
{\
	ZFSqrt255NR(x, result)\
	if(((result) & 0x01) == 0) ++(result);\
}

#define Round(x) ((INT)((x) >= 0 ? (x) + 0.5 : (x) - 0.5))

#define CopyMem(dst, src, count, size) memcpy(dst, src, count * size)
#define MoveMem(dst, src, count, size) memmove(dst, src, count * size)
#define FillMem(data, c, count) memset(data, c, count)

INT CalcLineDir(INT x1, INT y1, INT x2, INT y2);
INT ComputeDistance(INT dx, INT dy);
INT ComputeDistance3D(INT dx, INT dy, INT dz);

VOID CopyFeatures(FEATURES * target, const FEATURES * source);

#define GRAY_BLACK	0
#define GRAY_WHITE	255

#define DIR_0			0
#define DIR_45			30
#define DIR_90			(DIR_45 * 2)
#define DIR_135			(DIR_45 * 3)
#define DIR_180			(DIR_45 * 4)
#define DIR_225			(DIR_45 * 5)
#define DIR_270			(DIR_45 * 6)
#define DIR_315			(DIR_45 * 7)
#define DIR_360			(DIR_45 * 8)
#define DIR_UNKNOWN		127
#define DIR_BACKGROUND	255

#define IsBadArea(dir) ((dir) & 128)
#define IsGoodArea(dir) (!((dir) & 128))
#define TheDir(dir) ((dir) & 127)
#define MakeBadArea(dir) ((dir) |= 128)
#define MakeGoodArea(dir) ((dir) &= 127)
#define ZFMakeBadArea(orient) ((orient) |= 128)
#define ZFMakeGoodArea(orient) ((orient) &= 127)
#define AssignBadArea(dir, bad_area) ((bad_area) ? MakeBadArea(dir) : MakeGoodArea(dir))
#define AssignGoodArea(dir, good_area) ((good_area) ? MakeGoodArea(dir) : MakeBadArea(dir))

#define NOISE_MAX 255

#define G_MAX 255

#define ZFCOH_MAX 255

#define RAD_TO_DIR_K 38.197186342054880584532103209403
#define DIR_TO_RAD_K 0.026179938779914943653855361527329
#define RadToDirF(r) ((r) * RAD_TO_DIR_K)
#define RadToDir(r) Round(RadToDirF(r))
#define DirToRad(d) ((d) * DIR_TO_RAD_K)

#define UDIV8(dividend, divisor) _udiv16(dividend, divisor)
#define UDIV16(dividend, divisor) _udiv16(dividend, divisor)
#define UDIV32(dividend, divisor) _udiv32(dividend, divisor)
#define DIV32(dividend, divisor) _div32(dividend, divisor)
#define DIV(dividend, divisor) ((dividend)/(divisor))
#define DIVCONST(dividend, divisor, c) ((dividend)*(((c)+(divisor)/2)/(divisor))+(c)/2) / (c)

#define _udiv16(d1,d2) (WORD)(d1/d2)
#define _udiv32(d1,d2) (UINT)(d1/d2)

#define MAX_ARCTANGENTS 50
extern INT atans[MAX_ARCTANGENTS][MAX_ARCTANGENTS];

#define ZF_BLOCK_SIZE BLOCK_SIZE
#define ZFClearMem(data, size) FillMem(data, 0, size)
#define ZFGRAY_WHITE GRAY_WHITE
#define ZFGRAY_BLACK GRAY_BLACK
#define ZFCopyMem(d,s,size)	CopyMem(d,s,1,size)
#define ZFCopyByteRow(dstRow, row, length) CopyByteRow(row, dstRow, length)

#define CopyByteRow(row, dstRow, length) CopyMem(dstRow, row, length, sizeof(BYTE))
#define CopyIntRow(row, dstRow, length) CopyMem(dstRow, row, length, sizeof(INT))

BYTE * * AllocByteImage(BYTE *StartP, INT width, INT height, INT *Len);
BYTE * * CAllocByteImage(BYTE *StartP, INT width, INT height, INT *Len);
VOID FreeImage(VOID * * image, INT height);
VOID CopyFPImage(INT width, INT height, BYTE * * image, BYTE * * dstImage);

void SysInit(void);

#ifdef ZK_FP_DLL
	#define DM_Printf(v1,...)
	#define PrintHex(v1,v2,v3) 
	#define Printf(v1,...)
#else
	void DM_Printf(char * fmt, ...);
	void PrintHex(char *Desc, BYTE *p, int c);
	int Printf(char * fmt, ...);
#endif

#define SINMUL	16384
#define COSMUL	SINMUL

extern	INT SinValue[DIR_360];
extern	INT CosValue[DIR_360]; 

#define ZFE_OK					 0
#define ZFE_FAILED				-1
#define ZFE_OUT_OF_MEMORY		-2
#define ZFE_NOT_INITIALIZED		-3
#define ZFE_ARGUMENT_NULL		-4
#define ZFE_INVALID_ARGUMENT	-5

#define ZFFailed(result) ((result) < 0)
#define ZFSucceeded(result) ((result) >= 0)

#define ZFFullWindow(window) ((window) * 2 + 1)

extern int ZF_WIDTH;
extern int ZF_HEIGHT;

#ifdef WIN32
#ifdef _DEBUG
#include <crtdbg.h>
#define ZK_DEBUG_SAVEIMAGE
#endif
#endif

#ifndef _RPT0
#define _CRT_WARN 0
#define _RPT0(rpttype, msgformat)
#define _RPT1(rpttype, msgformat, v1)
#define _RPT2(rpttype, msgformat, v1, v2)
#define _RPT3(rpttype, msgformat, v1, v2,v3)
#define _RPT4(rpttype, msgformat, v1, v2,v3,v4)
#endif


#ifdef ZK_DEBUG_SAVEIMAGE
extern int findex;
#define SAVEDATA(name, image)/*
*/{/*
*/	char fn[50];/*
*/	sprintf(fn, "D:\\temp\\test\\ttt%02d(%s).bmp", findex++,name);/*
*/	WriteBitmap(image[0], ZF_WIDTH, ZF_HEIGHT, fn);/*
*/}
#define SAVEBMP(w, h, name, image)/*
*/{/*
*/	char fn[50];/*
*/	sprintf(fn, "D:\\temp\\test\\bmp%02d(%s).bmp", findex++,name);/*
*/	WriteBitmap(image[0], w, h, fn);/*
*/}
#define SAVEBMP1(w, h, name, image)/*
*/{/*
*/	char fn[50];/*
*/	sprintf(fn, "D:\\temp\\test\\bmp%02d(%s).bmp", findex++,name);/*
*/	WriteBitmap(image, w, h, fn);/*
*/}
#else
#define SAVEDATA(name, image)
#define SAVEBMP(w, h, name, image)
#endif

#endif //!__ZF_SYS_INCLUDED__
