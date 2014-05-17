#ifndef	__PY_IME_h__
#define	__PY_IME_h__
typedef struct
{
   const char *T9;
   const char *PY;
   const char *MB;
}T9PY_IDX;

unsigned int T9PY_GetPY(const unsigned char *p_PadInput, T9PY_IDX **p_PY_List);
unsigned int T9PY_Ime(const T9PY_IDX *p_PY_Idx, unsigned int HZ_Offset, unsigned int HZ_Num, unsigned char *HZ_Buf);

#endif
