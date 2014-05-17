#ifndef _TMP2FTR_H_
#define _TMP2FTR_H_

//Force C (not C++) names for API functions
#ifdef __cplusplus
extern "C"
{
#endif

#include "ZF_Sys.h"

#define BIOKEYVERSION30 0x18
#define BIOKEYVERSION31 0x19
#define BIOKEYVERSION32 0x1A
#define BIOKEYVERSION33 0x1B

#define  BIOKEYVERSION40 0x20
#define  BIOKEYVERSION41 0x21

#define  MaxFeatureNumber 63
#define  NewMaxFeatureNumber (256*4-1)
//最大局部特征点个数

#define  MaxPatternNumber 5
#define  NewMaxPatternNumber 31
//最大全局特征点个数

#define  MaxXY  512
//最大图像大小
#define  MaxDirection  360
//最大方向值
#define  MaxCurvature 128
//最大切线角度
#define  MaxYValue  128
#define  MaxPatternType  8
//全局特征类型个数
#define  MaxBlockTemplateSize  1024
//最大模板大小
#define  MaxBlockValue  255
//最大方向块值

  //模板长度
#define  Tl_Standard  1024
#define  Tl_Lite       256
#define  Tl_NoLimit   65535

#define  GPos  6
#define  TemplateType  2
#define  SavedBlock  3
#define  FeatureNum  5

  //模板类型
#define  Tt_Verification     0
#define  Tt_Registration     1
#define  Tt_All              2

int BiokeyTemplateToFeature(BYTE* ATemplate, FEATURES *AFeatures, int ATemplateType);
int BiokeyFeatureToTemplate(int ATemplateType, BOOL ABlock, int AVersion,
    FEATURES *AFeatures, BYTE *AResult);
int BiokeyTemplateToFeature41(BYTE* ATemplate, FEATURES *AFeatures);
int BiokeyFeatureToTemplate41(BOOL ABlock, int AVersion,
    FEATURES *AFeatures, BYTE *AResult);
int OldBiokeyTemplateToFeature(BYTE* ATemplate, FEATURES *AFeatures);

int BiokeyTemplateLength(BYTE* ATemplate);
//转换成41模板
int TestTemplate41(BYTE* ATemplate, BYTE* AResult);
//转换成40模板
int TestTemplate(BYTE* ATemplate, BYTE* AResult);
//转换成<=指定长度的41模板
int SetBiokeyTemplateLen(BYTE* ATemplate, int Len);

#ifdef __cplusplus
}
#endif

#endif

