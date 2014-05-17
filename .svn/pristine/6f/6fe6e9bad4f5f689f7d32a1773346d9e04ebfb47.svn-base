#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "wiegand.h"
#include "arca.h"
#include "../public/options.h"

static TWiegandFmt WiegandFmt26 = {
	26,
	"pssssssssccccccccccccccccp",
	"eeeeeeeeeeeeeooooooooooooo"
};


/*FC:2-9 ID:10-25*/
static TWiegandFmt WiegandFmt26a = {
	26,
	"pffffffffccccccccccccccccp",
	"eeeeeeeeeeeeeooooooooooooo"
};

/*ID:2-24*/
static TWiegandFmt WiegandFmt26b = {
	26,
	"pccccccccccccccccccccccccp",
	"eeeeeeeeeeeeeooooooooooooo"
};

/*ID:1-26*/
static TWiegandFmt WiegandFmt26c = {
	26,
	"cccccccccccccccccccccccccc",
	"bbbbbbbbbbbbbbbbbbbbbbbbbb"
};

/*ID:14-29*/
static TWiegandFmt WiegandFmt30a = {
	30,
	"pffffffffffffccccccccccccccccp",
	"eeeeeeeeeeeeeeeooooooooooooooo"
};

/*ID:15-30*/
static TWiegandFmt WiegandFmt30b = {
	30,
	"pfffffffffffffcccccccccccccccc",
	"eeeeeeeeeeeeeeeeeeeeeeeeeeeeee"
};

/*ID:1-30*/
static TWiegandFmt WiegandFmt30c = {
	30,
	"cccccccccccccccccccccccccccccc",
	"bbbbbbbbbbbbbbbbbbbbbbbbbbbbbb"
};

/*ID:16-31*/
static TWiegandFmt WiegandFmt32a = {
	32,
	"pffffffffffffffccccccccccccccccp",
	"eeeeeeeeeeeeeeeeoooooooooooooooo"
};

/*ID:17-32*/
static TWiegandFmt WiegandFmt32b = {
	32,
	"pfffffffffffffffcccccccccccccccc",
	"eeeeeeeeeeeeeeeeeeeeeeeeeeeeeeee"
};

/*ID:1-32*/
static TWiegandFmt WiegandFmt32c = {
	32,
	"cccccccccccccccccccccccccccccccc",
	"bbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbb"
};


static TWiegandFmt WiegandFmt34 = {
	34,
	"pssssssssssssssssccccccccccccccccp",
	"eeeeeeeeeeeeeeeeeooooooooooooooooo"
};

/*FC:2-17 ID:18-33*/
static TWiegandFmt WiegandFmt34a = {
	34,
	"pffffffffffffffffccccccccccccccccp",
	"eeeeeeeeeeeeeeeeeooooooooooooooooo"
};

/*FC:2-18 ID:19-34*/
static TWiegandFmt WiegandFmt34b = {
	34,
	"pfffffffffffffffffcccccccccccccccc",
	"eeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeee"
};

/*ID:1-34*/
static TWiegandFmt WiegandFmt34c = {
	34,
	"cccccccccccccccccccccccccccccccccc",
	"bbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbb"
};

/*FC: 2-18 ID:19-34*/
static TWiegandFmt WiegandFmt35a = {
	35,
	"pfffffffffffffffffccccccccccccccccp",
	"eeeeeeeeeeeeeeeeeoooooooooooooooooo"
};

/*FC: 3-14 ID:15-34*/
static TWiegandFmt WiegandFmt35b = {
	35,
	"pbffffffffffffccccccccccccccccccccp",
	"eeeeeeeeeeeeeeeeeoooooooooooooooooo"
};

/*FC: 3-14 ID:16-34*/
static TWiegandFmt WiegandFmt35c = {
	35,
	"pbffffffffffffbcccccccccccccccccccp",
	"eeeeeeeeeeeeeeeeeoooooooooooooooooo"
};

/*ID:1-35*/
static TWiegandFmt WiegandFmt35d = {
	35,
	"ccccccccccccccccccccccccccccccccccc",
	"bbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbb"
};

static TWiegandFmt WiegandFmt36 = {
	36,
	"pssssssssssssssssccccccccccccccccccp",
	"oooooooooooooooeeeeeeeeeeeeeeeeeeeee"
};

static TWiegandFmt WiegandFmt37 = {
	37,
	"pmmmffffffffffssssssccccccccccccccccp",
	"eeeeeeeeeeeeeeeeeeooooooooooooooooooo"
};

static TWiegandFmt WiegandFmt37a = {
	37,
	"pmmmmsssssssssssscccccccccccccccccccp",
	"oeobeobeobeobeobeobeobeobeobeobeobeoe"
};

/*FC: 2-17 ID:18-36*/
static TWiegandFmt WiegandFmt37b = {
	37,
	"pffffffffffffffffcccccccccccccccccccp",
	"eeeeeeeeeeeeeeeeeeooooooooooooooooooo"
};

/*ID: 2-36*/
static TWiegandFmt WiegandFmt37c = {
	37,
	"pcccccccccccccccccccccccccccccccccccp",
	"eeeeeeeeeeeeeeeeeeooooooooooooooooooo"
};

static TWiegandFmt WiegandFmt50 = {
	50,
	"pssssssssssssssssccccccccccccccccccccccccccccccccp",
	"eeeeeeeeeeeeeeeeeeeeeeeeeooooooooooooooooooooooooo"
};

/*FC: 2-17 ID:18-49*/
static TWiegandFmt WiegandFmt50a = {
	50,
	"pffffffffffffffffccccccccccccccccccccccccccccccccp",
	"eeeeeeeeeeeeeeeeeeeeeeeeeooooooooooooooooooooooooo"
};

/*ID:1-50*/
static TWiegandFmt WiegandFmt50b = {
	50,
	"cccccccccccccccccccccccccccccccccccccccccccccccccc",
	"bbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbb"
};

static TWiegandFmt WiegandFmt66 = {
	66,
	"pccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccp",
	"eeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeooooooooooooooooooooooooooooooooo"
};

TWiegandFmt *wgCreateFmt(char *fmtstr)
{
	TWiegandFmt *fmt;
	char *p;
	int i, pIndex=0;
	if (fmtstr==NULL) fmtstr = "26";

	if (strcmp(fmtstr, "WIEGAND26")==0  || strcmp(fmtstr, "Wiegand26")==0)	return &WiegandFmt26;
	if (strcmp(fmtstr, "WIEGAND26a")==0 || strcmp(fmtstr, "Wiegand26a")==0)	return &WiegandFmt26a;
	if (strcmp(fmtstr, "WIEGAND26b")==0 || strcmp(fmtstr, "Wiegand26b")==0)	return &WiegandFmt26b;
	if (strcmp(fmtstr, "WIEGAND26c")==0 || strcmp(fmtstr, "Wiegand26c")==0)	return &WiegandFmt26c;
	if (strcmp(fmtstr, "WIEGAND30a")==0 || strcmp(fmtstr, "Wiegand30a")==0)	return &WiegandFmt30a;
	if (strcmp(fmtstr, "WIEGAND30b")==0 || strcmp(fmtstr, "Wiegand30b")==0)	return &WiegandFmt30b;
	if (strcmp(fmtstr, "WIEGAND30c")==0 || strcmp(fmtstr, "Wiegand30c")==0)	return &WiegandFmt30c;
	if (strcmp(fmtstr, "WIEGAND32a")==0 || strcmp(fmtstr, "Wiegand32a")==0)	return &WiegandFmt32a;
	if (strcmp(fmtstr, "WIEGAND32b")==0 || strcmp(fmtstr, "Wiegand32b")==0)	return &WiegandFmt32b;
	if (strcmp(fmtstr, "WIEGAND32c")==0 || strcmp(fmtstr, "Wiegand32c")==0)	return &WiegandFmt32c;
	if (strcmp(fmtstr, "WIEGAND34")==0  || strcmp(fmtstr, "Wiegand34")==0)  return &WiegandFmt34;
	if (strcmp(fmtstr, "WIEGAND34a")==0 || strcmp(fmtstr, "Wiegand34a")==0) return &WiegandFmt34a;
	if (strcmp(fmtstr, "WIEGAND34b")==0 || strcmp(fmtstr, "Wiegand34b")==0) return &WiegandFmt34b;
	if (strcmp(fmtstr, "WIEGAND34c")==0 || strcmp(fmtstr, "Wiegand34c")==0) return &WiegandFmt34c;
	if (strcmp(fmtstr, "WIEGAND35a")==0 || strcmp(fmtstr, "Wiegand35a")==0) return &WiegandFmt35a;
	if (strcmp(fmtstr, "WIEGAND35b")==0 || strcmp(fmtstr, "Wiegand35b")==0) return &WiegandFmt35b;
	if (strcmp(fmtstr, "WIEGAND35c")==0 || strcmp(fmtstr, "Wiegand35c")==0) return &WiegandFmt35c;
	if (strcmp(fmtstr, "WIEGAND35d")==0 || strcmp(fmtstr, "Wiegand35d")==0) return &WiegandFmt35d;
	if (strcmp(fmtstr, "WIEGAND36")==0  || strcmp(fmtstr, "Wiegand36")==0)  return &WiegandFmt36;
	if (strcmp(fmtstr, "WIEGAND37")==0  || strcmp(fmtstr, "Wiegand37")==0)	return &WiegandFmt37;
	if (strcmp(fmtstr, "WIEGAND37a")==0 || strcmp(fmtstr, "Wiegand37a")==0) return &WiegandFmt37a;
	if (strcmp(fmtstr, "WIEGAND37b")==0 || strcmp(fmtstr, "Wiegand37b")==0) return &WiegandFmt37b;
	if (strcmp(fmtstr, "WIEGAND37c")==0 || strcmp(fmtstr, "Wiegand37c")==0) return &WiegandFmt37c;
	if (strcmp(fmtstr, "WIEGAND50")==0  || strcmp(fmtstr, "Wiegand50")==0)	return &WiegandFmt50;
	if (strcmp(fmtstr, "WIEGAND50a")==0 || strcmp(fmtstr, "Wiegand50a")==0)	return &WiegandFmt50a;
	if (strcmp(fmtstr, "WIEGAND50b")==0 || strcmp(fmtstr, "Wiegand50b")==0)	return &WiegandFmt50b;
	if (strcmp(fmtstr, "WIEGAND66")==0  || strcmp(fmtstr, "Wiegand66")==0)	return &WiegandFmt66;

	fmt = (TWiegandFmt*)malloc(sizeof(TWiegandFmt));
	memset((void*)fmt, 0, sizeof(TWiegandFmt));
	p=fmt->code;
	for (i=0; i<MAX_WG_BITS*2+1; i++)
	{
		char code = fmtstr[i];
		p[pIndex] = code;
		if (code==0) break;

		if (code==':')
		{
			p[pIndex]=0;
			if (p==fmt->parity) break;
			fmt->count=pIndex;
			p=fmt->parity;
			pIndex=0;
		}
		else pIndex++;
	}

	if (p==fmt->code)
	{
		fmt->count = pIndex;
		if (pIndex<3 || (!(p[0]=='p' && p[pIndex-1]=='p')))
		{
			free(fmt);
			return NULL;
		}
		for (i=0; i<pIndex; i++)
		{
			if (i<pIndex/2)
				fmt->parity[i]='e';
			else
				fmt->parity[i]='o';
		}
	}
	else
	{
		for (i=0; i<fmt->count; i++)
		{
			if (fmt->code[i]=='p' && !(fmt->parity[i]=='o' || fmt->parity[i]=='e'))
			{
				free(fmt);
				return NULL;
			}
		}
	}
	return fmt;
}

U32 wgGetValue(char *bits, char *fmt, char key)
{
	U32 value = 0;
	int i;
	for (i=0; i<MAX_WG_BITS; i++)
	{
		if (fmt[i]==0) break;
		if (fmt[i]==key)
		{
			value<<=1;
			if (bits[i]=='1')
			{
				value |= 1;
			}
		}
	}
	return value;
}

U32 wgGetBit1Count(char *bits, char *fmt, char key)
{
	int c=0;
	int i;
	for (i=0; i<MAX_WG_BITS; i++)
	{
		if (fmt[i]==0) break;
		if (fmt[i]==key)
		{
			if (bits[i]=='1')
			{
				c++;
			}

		}

	}
	return c;
}

U32 OldDecodeWiegandIn(char *bits, int bitsCount)
{
        U32 data=0;
	int i;
	U32 va;
	for (i=1; i<bitsCount-1; i++)
	{
		if(bits[i] == '1') va = 1;
		else       	va = 0;
		data |= va << (bitsCount-2-i);
	}
	return data;
}

int wgDecode(TWiegandFmt *fmt, char *bits, TWiegandData *data)
{
	int oParity = wgGetBit1Count(bits, fmt->parity, 'o');
	int eParity = wgGetBit1Count(bits, fmt->parity, 'e');
	printf("oParity=%d, eParity=%d\n", oParity, eParity);
	//if (oParity%2==0) return ERROR_PARITY_ODD;
	//if (eParity%2==1) return ERROR_PARITY_EVEN;
	char cardBuf[128] = {0};
	int cardLen = 0;

	if (data)
	{
		if(1== gOptions.ShortCardFunOn)
		{
			data->cardNumber = wgGetValue(bits, fmt->code, 'c');
		}
		else if(2== gOptions.ShortCardFunOn)
		{
			U32 siteCode = wgGetValue(bits, fmt->code, 's');
			U32 cardNO = wgGetValue(bits, fmt->code, 'c');
			sprintf(cardBuf,"%u",cardNO);
			if(36 == fmt->count)
			{
				cardLen = 6;
			}
			else
			{
				cardLen = 5;
			}

			if(strlen(cardBuf) < cardLen)
			{
				memset(cardBuf,0x00,sizeof(cardBuf));
				sprintf(cardBuf,"%u0%u",siteCode,cardNO);
			}
			else
			{
				memset(cardBuf,0x00,sizeof(cardBuf));
				sprintf(cardBuf,"%u%u",siteCode,cardNO);
			}
			data->cardNumber = atoi(cardBuf);
		}
		else
		{
			//printf("%s\n",bits);
			data->cardNumber = OldDecodeWiegandIn(bits, fmt->count);
		/*	memcpy(cardBuf,&data->cardNumber,4);
			cardBuf[4] = cardBuf[3];
			cardBuf[5] = cardBuf[2];
			cardBuf[6] = cardBuf[1];
			cardBuf[7] = cardBuf[0];
			memcpy(&data->cardNumber,cardBuf+4,4);*/
		}
	}

	return 	ERROR_OK;
}

void buf2bits64(char *buf, char *bits, int bitsCount)
{
	int i;
	//printf("bitscount=%d\n", bitsCount);
	for (i=0; i<bitsCount; i++)
	{
		//if(buf[i/8] & (0x01<<(i%8)))
		if(buf[i/8] & (0x80>>(i%8)))
			bits[i]='1';
		else
			bits[i]='0';
		//printf("%c", bits[i]);
	}
	//printf("\n");
	bits[bitsCount]=0;
}

int wiegandDecode(char *bitStr, unsigned char *data, U32 *manufactureCode, U32 *facilityCode, U32 *siteCode, U32 *cardNumber)
{
	TWiegandFmt *fmt;
	TWiegandData wieganddata;
	char bitString[MAX_WG_BITS] = {0};
	int ret;

	if(data[0] == 32)
	{
		buf2bits64(data+1, bitString+1, data[0]);
		printf("%s\n",bitString);
		*cardNumber = OldDecodeWiegandIn(bitString,34);
		return 0;
	}
	printf("bitStr = %s\n",bitStr);
	fmt = wgCreateFmt(bitStr);
	printf("wiegandDecode wg_len=%d,data[0] = %d\n",fmt->count, data[0]);
	if (data[0] != fmt->count) return 0;	//no bitcount
	buf2bits64(data+1, bitString, fmt->count);


	ret = wgDecode(fmt, bitString, &wieganddata);
//	printf("wiegandDecode ret %d\n",ret);
	if (ret == ERROR_OK)
	{
		if (manufactureCode) *manufactureCode = wieganddata.manufactureCode;
		if (siteCode) *siteCode = wieganddata.siteCode;
		if (facilityCode) *facilityCode = wieganddata.facilityCode;
		if (cardNumber)
		{
			*cardNumber = wieganddata.cardNumber;
		}

	}
	return ret;
}

int wiegandKeyDecode(unsigned char *data, int *key)
{
	unsigned char bit1, bit2;

	bit1 = data[0]>>4;
	bit2 = (data[0] & 0x0F);
	//printf("bit1=%x, bit2=%x\n", bit1, bit2);
	if ((bit1 + bit2) == 0x0F)   *key = bit2;
	else *key=-1;
	//printf("key=%d\n", *key);
	return *key;
}

int wiegandKeyDecode2(unsigned char *data, int *key)
{
	unsigned char bit1;
	bit1 = data[0]>>4;
	*key = bit1;
	return *key;
}

