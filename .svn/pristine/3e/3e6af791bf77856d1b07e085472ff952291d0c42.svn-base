#ifndef __WIEGAND_H__
#define __WIEGAND_H__

#include "arca.h"

#define MAX_WG_BITS 128
#define ERROR_PARITY_EVEN	-10091
#define ERROR_PARITY_ODD	-10092
#define ERROR_OK		0

typedef struct _WiegandFmt_
{
	int count;
	char code[MAX_WG_BITS];
	char parity[MAX_WG_BITS];
}TWiegandFmt;

typedef struct _WiegandData_
{
	U32 manufactureCode;
	U32 facilityCode;
	U32 siteCode;
	U32 cardNumber;
}TWiegandData;

int wiegandKeyDecode(unsigned char *data, int *key);
int wiegandKeyDecode2(unsigned char *data, int *key);

int wiegandDecode(char *bitStr, unsigned char *data, U32 *manufactureCode, U32 *facilityCode, U32 *siteCode, U32 *cardNumber);

#endif
