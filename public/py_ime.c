//==========================================================================//
//	文件名称：	PY_Input.c  												//	
//	功能描述：	T9拼音输入法模块 											//	
//	维护记录：	2006-04-20	v1.0											//
#include "py_mb.h"
#include "PY_Index.h"

unsigned int T9PY_GetPY(const unsigned char *p_PadInput, const T9PY_IDX **p_PY_List)
{
	const T9PY_IDX *p_PY_CurIdx;
	const T9PY_IDX **p_PY_Item;
	unsigned int i, MatchFlag, MatchNum;
	
	if(p_PadInput[0]=='\0')return 0;

	p_PY_CurIdx = t9PY_index;
	p_PY_Item = p_PY_List;
	MatchNum = 0;
	while(p_PY_CurIdx->T9[0] != '\0')
	{
		i = 0;
		MatchFlag = 1;
		while(p_PadInput[i] != '\0')
		{
			if(p_PadInput[i] != p_PY_CurIdx->T9[i])
			{
				MatchFlag = 0;
				break;
			}
			i++;
		}
		if(p_PY_CurIdx->T9[i]!='\0')
		{
			MatchFlag = 0;
		}
		if(MatchFlag == 1)
		{
			*p_PY_Item++ = p_PY_CurIdx;
			MatchNum++;
		}
		p_PY_CurIdx++;
	}
	return MatchNum;
}

//========================================================================
//========================================================================
unsigned int T9PY_Ime(const T9PY_IDX *p_PY_Idx, unsigned int HZ_Offset, unsigned int HZ_Num, unsigned char *HZ_Buf)
{
	unsigned int i, HZ_Remain;
	unsigned char *p_HZ_Buf;
	
	i = 0;
	HZ_Remain = 0;
	p_HZ_Buf = HZ_Buf;
	
	while(p_PY_Idx->MB[i] != '\0')
	{
		if(i>=HZ_Offset && i<HZ_Offset+HZ_Num)
		{
			*p_HZ_Buf++ = p_PY_Idx->MB[i];
		}
		else if(i>=HZ_Offset+HZ_Num)
		{
			HZ_Remain++;
		}
     			
		i++;
	}
	if(i==0)
		HZ_Remain = 0xffff;
	*p_HZ_Buf = '\0';
	return HZ_Remain;
}

