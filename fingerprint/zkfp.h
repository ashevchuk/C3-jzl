#ifndef _ZKFINGER_H_
#define _ZKFINGER_H_

//Force C (not C++) names for VeriFinger API functions
#ifdef __cplusplus
extern "C"
{
#endif

#ifdef ZK_FP_DLL
	#ifdef ZKFINGER_EXPORTS
		#define BIOKEY_API __declspec(dllexport)
	#else
		#define BIOKEY_API __declspec(dllimport)
	#endif
#else
	#define BIOKEY_API
#endif

#define BYTE unsigned char
#define WORD unsigned short
typedef void * HANDLE;

#define SPEED_LOW 0
#define SPEED_HIGH 1

#define EXTRACT_FOR_ENROLLMENT		0
#define EXTRACT_FOR_IDENTIFICATION	1
#define EXTRACT_FOR_VERIFICATION	2

#define CORRECT_NONE	0
#define CORRECT_REVERSE	1
#define CORRECT_ROTATION 2

//采用序列化参数初始化指纹识别系统，用于嵌入式系统，
//包括了进行光学畸变/分辨率校正和实时检查按压手指的参数。
// Param - 为一指向各种参数的指针，这些参数在调用前须被初始化
// ImageBuffer - 为指向采集指纹的数据区的指针，该处存放着原始的指纹图像
BIOKEY_API HANDLE BIOKEY_INIT(int License, WORD *isize, BYTE *Params, BYTE *Buffer, int ImageFlag);

//采用一般的指纹图像参数初始化指纹识别系统
// width - 指纹图像的宽度
// height - 指纹图像的高度
// ImageBuffer - 指纹图像数据区，该数据区应该在调用前初始化
BIOKEY_API HANDLE BIOKEY_INIT_SIMPLE(int License, int width, int height, BYTE *Buffer);

//关闭指纹识别系统
BIOKEY_API int BIOKEY_CLOSE(HANDLE Handle);

//提取当前缓冲区中指纹图像的特征模板，其结果放入Template所指的缓冲区，
//Template缓冲区的大小须预设为2K
//若提取成功，返回指纹模板的实际大小，否则返回值<=0
BIOKEY_API int BIOKEY_EXTRACT(HANDLE Handle, BYTE* PixelsBuffer, BYTE *Template, int PurposeMode);
BIOKEY_API int BIOKEY_EXTRACT_SIMPLE(HANDLE Handle, BYTE** PixelsBuffer, BYTE *Template, int PurposeMode);

//由3个指纹特征模板生成一个登记模板
BIOKEY_API int BIOKEY_GENTEMPLATE(HANDLE Handle, BYTE *Templates[], int TmpCount, BYTE *GTemplate);

//比对两个特征模板，返回它们的相似程度，0表示完全不同，100表示完全相同
BIOKEY_API int BIOKEY_VERIFY(HANDLE Handle,  BYTE *Template1, BYTE *Template2);

BIOKEY_API int BIOKEY_MATCHINGPARAM(HANDLE Handle,  int speed, int threshold);
////////////////////////////////////////////
//
//  Identification 相关函数
//
//清除数据库中指纹模板
BIOKEY_API int BIOKEY_DB_CLEAR(HANDLE Handle);

//把指纹模板加入数据库
// TID - 是指纹模板的标识号。每个指纹模板有一个唯一的标识号
// Template - 指纹模板
//成功时返回1, 失败时返回0
BIOKEY_API int BIOKEY_DB_ADD(HANDLE Handle,  int TID, int TempLength, BYTE *Template);

//从数据库中移除一个指纹模板
// TID - 是要移除的指纹模板的标识号。
// Template - 指纹模板
//成功时返回1, 失败时返回0
BIOKEY_API int BIOKEY_DB_DEL(HANDLE Handle,  int TID);

//应用一个过滤函数过滤数据库，只有该过滤函数返回非零时，该TID对应的指纹模板才会参与Identify
typedef int (*FilteFun)(int TID);
BIOKEY_API int BIOKEY_DB_FILTERID(HANDLE Handle,  FilteFun Filter);
BIOKEY_API int BIOKEY_DB_FILTERID_ALL(HANDLE Handle);
BIOKEY_API int BIOKEY_DB_FILTERID_NONE(HANDLE Handle);

//识别指纹
// Template - 要识别的指纹模板
// TID - 识别的结果，与之相匹配的数据库中的指纹所对应的标识号
// Score - 作为输入参数，表示立即确认的匹配相似度，
//         作为输出，表示Template与TID对应指纹的实际相似度
//         为NULL时忽略该项
// 识别成功返回1，否则返回0
BIOKEY_API int BIOKEY_IDENTIFYTEMP(HANDLE Handle, BYTE *Template, int *TID, int *Score);

//识别当前采集指纹图像
// TID - 识别的结果，与之相匹配的数据库中的指纹所对应的标识号
// Score - 作为输入参数，表示立即确认的匹配相似度，
//         作为输出，表示Template与TID对应指纹的实际相似度
//         为NULL时忽略该项
// 识别成功返回1，否则返回0
BIOKEY_API int BIOKEY_IDENTIFY(HANDLE Handle, BYTE *ImageBuffer, int *TID, int *Score);
BIOKEY_API int BIOKEY_IDENTIFY_SIMPLE(HANDLE Handle, BYTE **ImageBuffer, int *TID, int *Score);

BIOKEY_API int BIOKEY_TEMPLATELEN(BYTE *Template);
BIOKEY_API int BIOKEY_SETTEMPLATELEN(BYTE *Template, int NewLength);

BIOKEY_API int BIOKEY_SETNOISETHRESHOLD(HANDLE Handle, int NoiseValue, int MinMinutiae, int MaxTempLen, int ExtractScaleDPI);

int BIOKEY_CHECKIMAGE(HANDLE Handle, BYTE * Finger);
int ZFSearchValidArea(BYTE* FullImage, int  IWidth, int IHeight,
        int  MaxWidth, int MaxHeight, int *Corners, int *VHeight, int *VWidth, int *GridValue);

////////////////////////////////////////////
//
//  嵌入式系统相关
//
//矫正当前缓冲区的指纹，返回畸变校正后的指纹图像
BIOKEY_API void BIOKEY_GETFINGERLINEAR(HANDLE Handle, BYTE *ImageBuffer, BYTE *Finger);

//取指纹处理系统参数
BIOKEY_API int BIOKEY_GETPARAM(HANDLE Handle, int *DPI, int *width, int *height);

#ifdef __cplusplus
extern "C"
}
#endif

#endif

