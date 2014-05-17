#include "ZFTemplate.h"


#define ZFFEAT_BO_BACKGROUND 126
#define ZFFEAT_BO_EQUALS	0x80
#define ZFFEAT_BO_DIFFS		0xA0
#define ZFFEAT_BO_OFFx		0xC0
#define ZFFEAT_BO_OFFy		0xE0
#define ZFFEAT_BO_EOL		ZFFEAT_BO_OFFx
#define ZFFEAT_BO_EOI		ZFFEAT_BO_OFFy

#define ZFFeatBOToDir(d) ((BYTE)((BYTE)(d) == ZFFEAT_BO_BACKGROUND ? (BYTE)DIR_BACKGROUND : (d)))
#define ZFFeatBOIsValue(d) ((BOOL)(((BYTE)(d) & 0x80) == 0))
#define ZFFEAT_BO_CODE_MASK 0xE0
#define ZFFeatBOGetCode(d) ((BYTE)((BYTE)d & ZFFEAT_BO_CODE_MASK))
#define ZFFEAT_BO_N_MASK 0x1F
#define ZFFeatBOGetN(d) ((SBYTE)((BYTE)d & ZFFEAT_BO_N_MASK))

#define ZFFeatBOGetDiff1(d) ((SBYTE)((((BYTE)d & 0xF0) >> 4) - 8))
#define ZFFeatBOGetDiff2(d) ((SBYTE)(((BYTE)d & 0x0F) - 8))

#define ZFFeatBOMakeCode(code, n) ((BYTE)((code) | (n)))
#define ZFFeatBOMakeEquals(n) ZFFeatBOMakeCode(ZFFEAT_BO_EQUALS, n)
#define ZFFeatBOMakeDiffs(n) ZFFeatBOMakeCode(ZFFEAT_BO_DIFFS, n)
#define ZFFeatBOMakeOffX(n) ZFFeatBOMakeCode(ZFFEAT_BO_OFFx, n)
#define ZFFeatBOMakeOffY(n) ZFFeatBOMakeCode(ZFFEAT_BO_OFFy, n)

void SetFlagAndVersion(BYTE *ATemplate, int AVersion)
{
	WORD WordData;
	WordData = (('Z' - 65) << 11) | (('K' - 65) << 6) | AVersion;
	memcpy(ATemplate, &WordData, sizeof(WORD));
}

INT CompressBOLine(BYTE d, INT c, BYTE * l, BYTE * el)
{
	INT i, j;
	BYTE sl[ZF_MAX_BLOCKED_ORIENTS_DIMENSION];
	BYTE esl[ZF_MAX_BLOCKED_ORIENTS_DIMENSION];
	INT sc;
	INT ec = -1, esc;
	INT n;
	INT difs[ZF_MAX_BLOCKED_ORIENTS_DIMENSION];
	INT dif;
	INT m;

	if(c == 0) return 0;

	if(d < DIR_180)
	{
		for(i = 0; i < c;) if(l[i] != d) break; else i++;
		n = i > 32 ? 32 : i;
		if(i>1 && c<=32 && n==c)
		{
			el[0] = ZFFeatBOMakeEquals(n - 1);
			return 1;
		}
		else
		for(i = 2; i <= n; i++)
		{
			sc = c - i;
			for(j = 0; j < sc; j++) sl[j] = l[j + i];
			if(sc==0) esc=0; else
			esc = CompressBOLine(d, sc, sl, esl);
			if(ec == -1 || esc < ec - 1)
			{
				ec = esc + 1;
				for(j = 0; j < esc; j++) el[j + 1] = esl[j];
				el[0] = ZFFeatBOMakeEquals(i - 1);
			}
		}
		for(i = 0; i < c;)
		{
			dif = l[i] - (i == 0 ? d : l[i - 1]);
			if(dif >= DIR_90) dif -= DIR_180;
			if(dif < -DIR_90) dif += DIR_180;
			if(dif >= -8 && dif < 8)
			{
				difs[i] = dif + 8;
				i++;
			}
			else break;
		}
		n = i / 2;
		if(n > 32) n = 32;
		for(i = 2; i <= n; i++)
		{
			m = i * 2;
			sc = c - m;
			for(j = 0; j < sc; j++) sl[j] = l[j + m];
			if(sc==0) esc=0; else
			esc = CompressBOLine(l[m - 1], sc, sl, esl);
			if(ec == -1 || esc < ec - 1 - i)
			{
				ec = esc + 1 + i;
				for(j = 0; j < esc; j++) el[j + 1 + i] = esl[j];
				for(j = 0; j < i; j++) el[j + 1] = (BYTE)(((difs[j * 2] & 0x0F) << 4) | (difs[j * 2 + 1] & 0x0F));
				el[0] = ZFFeatBOMakeDiffs(i - 1);
			}
		}
	}
	sc = c - 1;
	for(j = 0; j < sc; j++) sl[j] = l[j + 1];
	if(sc==0) esc=0; else
	esc = CompressBOLine(l[0], sc, sl, esl);
	if(ec == -1 || esc < ec - 1)
	{
		ec = esc + 1;
		for(j = 0; j < esc; j++) el[j + 1] = esl[j];
		el[0] = l[0];
	}
	return ec;
}

/*
void TestBOLine(void)
{
	BYTE tl[100], ttl[100];
	int i, sc;
	for(i=2;i<40;i++)
	{
		memset(tl, 10, i);
		sc=CompressBOLine(10, i, tl, ttl);
		sprintf(tl, "%2d: [%02X %02X %02X]", sc, ttl[0],ttl[1],ttl[2]);
		tl[0]=0;
	}
}
*/

BYTE * CompressBO(const BLOCKEDORIENTS * bo, BYTE * features)
{
	INT w = bo->Width;
	INT h = bo->Height;
	INT i, j, ii, jj;
	INT c, ec;
	BYTE l[ZF_MAX_BLOCKED_ORIENTS_DIMENSION];
	BYTE el[ZF_MAX_BLOCKED_ORIENTS_DIMENSION];
	BYTE cur_dir;
	BYTE d;

//	TestBOLine();

	if(w < 0) w = 0;
	if(w > 512/16) w = ZF_MAX_BLOCKED_ORIENTS_DIMENSION;
	if(h < 0) h = 0;
	if(h > 512/16) h = ZF_MAX_BLOCKED_ORIENTS_DIMENSION;
	*features = (BYTE)(w - 1); features++;
	*features = (BYTE)(h - 1); features++;
	for(i = 0; i < h; i++)
		for(j = 0, cur_dir = ZFFEAT_BO_BACKGROUND; j < w; j++, features++)
		{
			for(jj = j; jj < w; jj++)
				if(bo->Bits[i][jj] != DIR_BACKGROUND) break;
			if(jj == w)
			{
				BOOL stop = FALSE;
				for(ii = i + 1; ii < h && !stop; ii += stop ? 0 : 1)
					for(jj = 0; jj < w; jj++)
						if(bo->Bits[ii][jj] != DIR_BACKGROUND)
						{
							stop = TRUE;
							break;
						}
				if(!stop) // End Of Image
				{
					*features = ZFFEAT_BO_EOI;
					j = w - 1;
					i = h - 1;
				}
				else
				{
					INT oy = ii - i;
					if(oy > 1) // Offset Y
					{
						oy--;
						if(oy > 31) oy = 31;
						*features = ZFFeatBOMakeOffY(oy);
						j = w - 1;
						i += oy;
					}
					else // End Of Line
					{
						*features = ZFFEAT_BO_EOL;
						j = w - 1;
					}
				}
			}
			else
			{
				INT ox = jj - j;
				if(ox > 1) // Offset X
				{
					ox--;
					if(ox > 31) ox = 31;
					*features = ZFFeatBOMakeOffX(ox);
					j += ox;
					cur_dir = ZFFEAT_BO_BACKGROUND;
				}
				else
				{
					c = 0;
					for(jj = j; jj < w; jj++)
					{
						d = bo->Bits[i][jj];
						if(d >= DIR_180) break;
						else
						{
							l[c] = d;
							c++;
						}
					}
					if(c == 0)
					{
						d = bo->Bits[i][j];
						if(d == DIR_BACKGROUND) d = ZFFEAT_BO_BACKGROUND;
						else if(d > DIR_180) d = DIR_UNKNOWN;
						*features = d;
						cur_dir = d;
					}
					else
					{
						ec = CompressBOLine(cur_dir, c, l, el);
						for(jj = 0; jj < ec; jj++, features++) *features = el[jj];
						j += c - 1;
						cur_dir = bo->Bits[i][j];
						features--;
					}
				}
			}
		}
	return features;
}

BYTE * DecompressBO(BYTE * features, BLOCKEDORIENTS * bo)
{
	INT w, h;
	INT i, j, k;
	INT cur_dir = ZFFEAT_BO_BACKGROUND;
	BYTE *f=features;
	bo->Width = 0;
	bo->Height = 0;
	w = *features + 1; features++;
	h = *features + 1; features++;
	if(w>512/16) return f;
	if(h>512/16) return f;
	bo->Width = w;
	bo->Height = h;
	FillMem(bo->Bits, DIR_BACKGROUND, ZF_MAX_BLOCKED_ORIENTS_DIMENSION * ZF_MAX_BLOCKED_ORIENTS_DIMENSION);
	i = 0; j = 0;
	do
	{
		BYTE b = *features;
		if(ZFFeatBOIsValue(b))
		{
			bo->Bits[i][j++] = ZFFeatBOToDir(b);
			cur_dir = b;
		}
		else
		{
			INT c = ZFFeatBOGetCode(b);
			INT n = ZFFeatBOGetN(b);
			if(n>w) 
				return f;
			switch(c)
			{
				case ZFFEAT_BO_OFFy:
					if(n == 0)
					{
						j = w;
						i = h - 1;
					}
					else
					{
						j = w;
						i += n;
					}
					break;
				case ZFFEAT_BO_OFFx:
					if(n == 0)
					{
						j = w;
					}
					else
					{
						j += n + 1;
						cur_dir = ZFFEAT_BO_BACKGROUND;
					}
					break;
				case ZFFEAT_BO_EQUALS:
					for(k = 0; k <= n; k++)
						bo->Bits[i][j++] = (BYTE)cur_dir;
					break;
				case ZFFEAT_BO_DIFFS:
					for(k = 0; k <= n; k++)
					{
						features++;
						cur_dir += ZFFeatBOGetDiff1(*features);
						if(cur_dir >= DIR_180) cur_dir -= DIR_180;
						if(cur_dir < DIR_0) cur_dir += DIR_180;
						bo->Bits[i][j++] = (BYTE)cur_dir;
						cur_dir += ZFFeatBOGetDiff2(*features);
						if(cur_dir >= DIR_180) cur_dir -= DIR_180;
						if(cur_dir < DIR_0) cur_dir += DIR_180;
						bo->Bits[i][j++] = (BYTE)cur_dir;
					}
					break;
				default:
					j = w;
					i = h - 1;
					break;
			}
		}
		if(j >= w)
		{
			j = 0;
			i++;
			cur_dir = ZFFEAT_BO_BACKGROUND;
		}
		features++;
		if(features>f+w*h) 
			return f;
		if(i>h) 
			return f;
	} while(i < h);
	return features;
}

#define ZFFEAT_HAS_BLOCKED_ORIENTATIONS	0x80

int ReversFeature(FEATURES *AFeatures)
{
	FEATURES f;
	int i,j,maxx,x;
	return 1;
	memcpy(&f, AFeatures, sizeof(FEATURES));
	maxx=f.BO.Width*BLOCK_SIZE;
	for(i=0;i<f.M.Count;i++)
	{
		x=maxx-f.M.X[i];
		if(x<0)x=0;
		AFeatures->M.X[i]=x;
		x=DIR_180-f.M.D[i];
		if(x<0)x+=DIR_360;
		AFeatures->M.D[i]=x;
	}
	for(i=0;i<f.SP.Count;i++)
	{
		x=maxx-f.SP.X[i];
		if(x<0)x=0;
		AFeatures->SP.X[i]=x;
		x=DIR_180-f.SP.D[i];
		if(x<0)x+=DIR_360;
		AFeatures->SP.D[i]=x;
	}
	for(i=0;i<f.BO.Width;i++)
		for(j=0;j<f.BO.Height;j++)
		{
			x=f.BO.Bits[j][f.BO.Width-1-i];
			if(x==DIR_UNKNOWN)
				AFeatures->BO.Bits[j][i]=DIR_UNKNOWN;
			else if(x==DIR_BACKGROUND)
				AFeatures->BO.Bits[j][i]=DIR_BACKGROUND;
			else
			{
				x=DIR_180-x;
				if(x<0)x+=DIR_360;
				AFeatures->BO.Bits[j][i]=x;
			}
		}
	return 1;
}

int BiokeyFeatureToTemplate41(BOOL ABlock, int AVersion,
		FEATURES *AFeatures, BYTE *AResult)
{
	BYTE bTemp, *p;
	WORD WordData;
	int iTemp, MaxFeatureCnt, MaxPatternCnt, I;
	long LongWordData;

	p = AResult;
	//头信息
	//Flag&Version
	if(AVersion==BIOKEYVERSION40)
		return BiokeyFeatureToTemplate(Tt_All,ABlock,AVersion,AFeatures,AResult);
	if(AVersion!=BIOKEYVERSION41) return 0;
	SetFlagAndVersion(p++, AVersion); p++;
	//Character
	MaxFeatureCnt = AFeatures->M.Count;
	if( MaxFeatureCnt > NewMaxFeatureNumber )
		MaxFeatureCnt = NewMaxFeatureNumber;
	MaxPatternCnt = AFeatures->SP.Count;
	if( MaxPatternCnt > NewMaxPatternNumber )
		MaxPatternCnt = NewMaxPatternNumber;
	*p++ = MaxFeatureCnt & 0xff;
	if(ABlock) 
	{
		ABlock=(AFeatures->BO.Width>0) && (AFeatures->BO.Height>0)
			&& (AFeatures->BO.Width*BLOCK_SIZE<=512/*ZF_MAX_IMAGE_DIMENSION*/) 
			&& (AFeatures->BO.Height*BLOCK_SIZE<=512/*ZF_MAX_IMAGE_DIMENSION*/);
	}

	*p++ = (ABlock?ZFFEAT_HAS_BLOCKED_ORIENTATIONS:0)|
		MaxPatternCnt | (((MaxFeatureCnt >> 8) &3) << 5);

	//Ridge density
	*p++ = AFeatures->G;
	//Feature Data
	bTemp = 0;
	if( AFeatures->M.Count > MaxFeatureCnt )
		bTemp = (AFeatures->M.Count - MaxFeatureCnt) / 2;
	//开始保存数据
	for(I = bTemp; I<bTemp + MaxFeatureCnt; I++) {
		//检查数据有效性
		if( AFeatures->M.X[I] >= MaxXY )
			AFeatures->M.X[I] = MaxXY - 1;
		if( AFeatures->M.Y[I] >=  MaxXY )
			AFeatures->M.Y[I] = MaxXY - 1;
		//X 9Bit, Y 7 Bit
		WordData=(AFeatures->M.X[I] << 7) | (AFeatures->M.Y[I] >> 2);
		memcpy(p, &WordData, 2); p++;p++;

		//Y 2Bit T 6Bit
		*p++ = ((AFeatures->M.Y[I] & 3) << 6) + AFeatures->M.T[I] + 1;

		WordData=(AFeatures->M.D[I] << 8) | AFeatures->M.C[I];
		memcpy(p,&WordData,2); p+=2;

	}
	//Pattern data
	for(I = 0; I<MaxPatternCnt; I++) {
		//检查数据有效性
		if( AFeatures->SP.X[I] >= MaxXY )
			AFeatures->SP.X[I] = MaxXY - 1;
		if( AFeatures->SP.Y[I] >= MaxXY )
			AFeatures->SP.Y[I] = MaxXY - 1;
		AFeatures->SP.T[I] = AFeatures->SP.T[I] + 1;
		if( AFeatures->SP.T[I] >= MaxPatternType )
			AFeatures->SP.T[I] = MaxPatternType - 1;
		if( AFeatures->SP.D[I] >= MaxDirection )
			AFeatures->SP.D[I] = MaxDirection - 1;
		LongWordData = (AFeatures->SP.X[I] << 21) | (AFeatures->SP.Y[I] << 12) |
			(AFeatures->SP.T[I] << 9) | AFeatures->SP.D[I];
		memcpy(p, &LongWordData,4); p+=4;
	}
	//Block data
	iTemp = 0;
	if( ABlock ) {
		p=CompressBO(&AFeatures->BO, p);
	}
	return (int)(p - AResult);
}

int BiokeyTemplateToFeature41(BYTE* ATemplate, FEATURES *AFeatures)
{
	BYTE *p;
	WORD WordData;
	DWORD LongWordData;
	int I, HasBlock, HasRidge;
	p = ATemplate;
	//处理头信息
	//Flag&Version
	memcpy(&WordData, p, 2);
	if(WordData==0x432a) return OldBiokeyTemplateToFeature(ATemplate, AFeatures);
	if(WordData==0xca9a) return BiokeyTemplateToFeature(ATemplate, AFeatures, Tt_All);
	if(WordData >> 8 !=0xCA) return 0;
	if((WordData & 0x007f)<BIOKEYVERSION41) return BiokeyTemplateToFeature(ATemplate, AFeatures, Tt_All);
	if(((('Z' - 65) << 5) | ('K' - 65)) == (WordData >> 6)==0) return 0;
	p++; p++;
	//初始化Block数组
	memset(AFeatures, 0, sizeof(FEATURES));
	//Character
	AFeatures->M.Count = *p++;
	AFeatures->SP.Count = *p++;
	AFeatures->M.Count = ((AFeatures->SP.Count & 0x60) >> 5 << 8) | AFeatures->M.Count;
	if(AFeatures->M.Count>250) return 0;
	HasBlock=(AFeatures->SP.Count & ZFFEAT_HAS_BLOCKED_ORIENTATIONS)!=0;
	HasRidge=(AFeatures->SP.Count & (1<<4))!=0;
	AFeatures->SP.Count=AFeatures->SP.Count & 0x7;
	if(AFeatures->SP.Count>8) return 0;
	//Ridge density
	AFeatures->G = *p++;
	//处理特征数据
	//X Y D C T
	for(I = 0;I<AFeatures->M.Count;I++) {
		AFeatures->M.X[I] = (p[0]+256*p[1])>>7;	//pm.W1 >> 7;
		AFeatures->M.Y[I] = (p[0]&127)<<2;	//(pm.W1&127) << 2;
		AFeatures->M.Y[I] = AFeatures->M.Y[I] + (p[2]>>6);	//(pm.B[0] >> 6);
		AFeatures->M.T[I] = (p[2]&0x3F)-1;	//(pm.B[0]&0x3F) - 1;
		AFeatures->M.D[I] = p[4];	//pm.B[2];
		AFeatures->M.C[I] = p[3];	//pm.B[1];
		p+=5;
	}
	//Pattern data
	for(I = 0;I<AFeatures->SP.Count;I++)
	{
		memcpy(&LongWordData, p, 4); p+=4;
		AFeatures->SP.X[I] = LongWordData >> 21;
		AFeatures->SP.Y[I] = (LongWordData >> 12)&511;
		AFeatures->SP.T[I] = (LongWordData >> 9)&7 ;
		AFeatures->SP.D[I] = LongWordData&511;
		AFeatures->SP.T[I] = AFeatures->SP.T[I] - 1;
	}
	if(HasBlock)
		p=DecompressBO(p, &AFeatures->BO);
	return p-ATemplate;
}


int BiokeyFeatureToTemplate(int ATemplateType, BOOL ABlock, int AVersion,
		FEATURES *AFeatures, BYTE *AResult)
{
	BYTE bTemp, *p, sPos, ePos, *pBlockLine;
	WORD WordData;
	int iTemp, MaxFeatureCnt, MaxPatternCnt, I, J,
		Low, High, CurCnt, iMaxBlockCount,Result;
	char *pBlockRow;
	long LongWordData;

	p = AResult;
	//头信息
	//Flag&Version
	if(AVersion!=BIOKEYVERSION40) return 0;
	SetFlagAndVersion(p++, AVersion);
	p++;
	//Block number
	iTemp = AFeatures->BO.Height;
	if( AFeatures->BO.Width > iTemp )
		iTemp = AFeatures->BO.Width;
	iMaxBlockCount = iTemp;
	*p++ = (iTemp << 2) | ATemplateType;
	//Template length & Saved block number
	/*  pBlockLine = (WORD*)p++;
	 *pBlockLine = 0;
	 p++;*/
	pBlockLine=p;
	*p++=0;
	*p++=0;
	//Character
	MaxFeatureCnt = AFeatures->M.Count;
	if( MaxFeatureCnt > MaxFeatureNumber )
		MaxFeatureCnt = MaxFeatureNumber;
	MaxPatternCnt = AFeatures->SP.Count;
	if( MaxPatternCnt > MaxPatternNumber )
		MaxPatternCnt = MaxPatternNumber;
	*p++ = (MaxFeatureCnt << 2) | MaxPatternCnt;
	//Ridge density
	*p++ = AFeatures->G;
	//Feature Data
	/*pWordData = (WORD*)p;*/
	bTemp = 0;
	if( AFeatures->M.Count > MaxFeatureNumber )
		bTemp = (AFeatures->M.Count - MaxFeatureNumber) / 2;
	//开始保存数据
	for(I = bTemp; I<bTemp + MaxFeatureCnt; I++) {
		//检查数据有效性
		if( AFeatures->M.X[I] >= MaxXY )
			AFeatures->M.X[I] = MaxXY - 1;
		if( AFeatures->M.Y[I] >=  MaxXY )
			AFeatures->M.Y[I] = MaxXY - 1;
		//X 9Bit, Y 7 Bit
		/**pWordData++ = (AFeatures->M.X[I] << 7) | (AFeatures->M.Y[I] >> 2);
		  p = (BYTE*)pWordData;*/
		WordData=(AFeatures->M.X[I] << 7) | (AFeatures->M.Y[I] >> 2);
		memcpy(p, &WordData, 2); p++;p++;

		//Y 2Bit T 6Bit
		*p++ = ((AFeatures->M.Y[I] & 3) << 6) + AFeatures->M.T[I] + 1;

		/*pWordData = (WORD*)p;
		//D, C
		 *pWordData++ = (AFeatures->M.D[I] << 8) | AFeatures->M.C[I];*/

		WordData=(AFeatures->M.D[I] << 8) | AFeatures->M.C[I];
		memcpy(p,&WordData,2); p+=2;

	}
	//Pattern data
	/*pLongWordData = (long*)pWordData;*/
	for(I = 0; I<MaxPatternCnt; I++) {
		//检查数据有效性
		if( AFeatures->SP.X[I] >= MaxXY )
			AFeatures->SP.X[I] = MaxXY - 1;
		if( AFeatures->SP.Y[I] >= MaxXY )
			AFeatures->SP.Y[I] = MaxXY - 1;
		AFeatures->SP.T[I] = AFeatures->SP.T[I] + 1;
		if( AFeatures->SP.T[I] >= MaxPatternType )
			AFeatures->SP.T[I] = MaxPatternType - 1;
		if( AFeatures->SP.D[I] >= MaxDirection )
			AFeatures->SP.D[I] = MaxDirection - 1;
		/**pLongWordData++ = (AFeatures->SP.X[I] << 21) | (AFeatures->SP.Y[I] << 12) |
		  (AFeatures->SP.T[I] << 9) | AFeatures->SP.D[I];*/
		LongWordData = (AFeatures->SP.X[I] << 21) | (AFeatures->SP.Y[I] << 12) |
			(AFeatures->SP.T[I] << 9) | AFeatures->SP.D[I];
		memcpy(p, &LongWordData,4); p+=4;
	}
	//Block data Max 256Byte (p - AResult) = 实际字节数
	/*p = (BYTE*)pLongWordData;*/
	iTemp = 0;
	if( ABlock ) {
		Low = iMaxBlockCount / 2;
		High = Low + 1;
		I = 1;
		while ((p - AResult + 3) <= MaxBlockTemplateSize) {
			if( I % 2 == 1 ) {
				CurCnt = Low;
				Low = Low - 1;
			} else {
				CurCnt = High;
				High = High + 1;
			}
			if( I > (iMaxBlockCount + 1) ) break;
			I++;
			if( (CurCnt >= 1) && (CurCnt <= iMaxBlockCount) ) {
				pBlockRow = (char*)p++; p++;
				//保存数据
				sPos = 0;
				ePos = 0;
				//locate ePos
				for(J = iMaxBlockCount - 1; J>=0; J--)
					if( !(AFeatures->BO.Bits[CurCnt - 1][J] == MaxBlockValue) ) {
						ePos = J;
						break;
					}
				//locate sPos
				for(J = 0; J<=ePos; J++)
					if( !(AFeatures->BO.Bits[CurCnt - 1][J] == MaxBlockValue) ) {
						sPos = J;
						break;
					}
				//save block data
				for(J = sPos; J<=ePos; J++)
					if( (p - AResult + 1) <= MaxBlockTemplateSize )
						*p++ = AFeatures->BO.Bits[CurCnt - 1][J];
					else {
						ePos = J;
						break;
					}
				//保存位置
				*pBlockRow = sPos;
				*(pBlockRow + 1) = ePos;
				iTemp++;
			}
		}
	}
	Result = (p - AResult);
	/**pBlockLine = (Result << 6) | iTemp;*/
	WordData=(Result << 6) | iTemp;
	memcpy(pBlockLine, &WordData, 2);
	return Result;
}

BOOL OldBiokeyTemplateToFeature(BYTE* ATemplate, FEATURES *AFeatures)
{
	BYTE *p, sPos, ePos, *pY;
	WORD WordData;
	DWORD LongWordData;
	int I, J, Low, High, CurCnt, iBlockRows, iTemp, Result, Y128;
	p = ATemplate;
	memcpy(&WordData, p, 2);

	//初始化Block数组
	memset(AFeatures, 0, sizeof(FEATURES));
	memset(&AFeatures->BO.Bits[0][0], 255,
			ZF_MAX_BLOCKED_ORIENTS_DIMENSION*ZF_MAX_BLOCKED_ORIENTS_DIMENSION);
	//处理头信息
	//Flag&Version
	p++; p++;
	//Block number
	AFeatures->BO.Width  = *p >> 2;
	AFeatures->BO.Height = *p >> 2;
	p++;
	//Character
	AFeatures->SP.Count = *p&3;
	AFeatures->M.Count  = *p >> 2;
	p++;
	//Ridge density
	AFeatures->G = 255 & *(int*)p;
	p++;
	//处理特征数据
	//X Y D C
	pY=p;
	Y128=0;
	p+=5;
	for(I = 0;I<AFeatures->M.Count;I++) {
		WORD pmW1, pmW2;
		pmW1=p[0]+256*p[1]; pmW2=p[2]+256*p[3];
		p+=4;
		AFeatures->M.X[I] = pmW1 >> 7;
		AFeatures->M.Y[I] = (pmW1&127);
		if(I+1==(*(pY+Y128)))
		{
			AFeatures->M.Y[I] |= ((*(pY+4) >> (Y128*2)) & 3) << 7;
			Y128++;
		}
		if(I>0) AFeatures->M.Y[I] +=AFeatures->M.Y[I-1];
		AFeatures->M.C[I] = pmW2 & 127;
		AFeatures->M.D[I] = pmW2 >> 7;
		AFeatures->M.D[I] = 120-AFeatures->M.D[I]*120/180;
		if(AFeatures->M.D[I]<0) AFeatures->M.D[I]+=240;
	}
	//Pattern data
	for(I = 0;I<AFeatures->SP.Count;I++)
	{
		memcpy(&LongWordData, p, 4); p+=4;
		AFeatures->SP.X[I] = LongWordData >> 21;
		AFeatures->SP.Y[I] = (LongWordData >> 12)&511;
		AFeatures->SP.T[I] = (LongWordData >> 9)&7 ;
		AFeatures->SP.T[I] = AFeatures->SP.T[I] - 1;
		AFeatures->SP.D[I] = LongWordData&511;
		AFeatures->SP.D[I] = 120-AFeatures->SP.D[I]*120/180;
		if(AFeatures->SP.D[I]<0) AFeatures->SP.D[I]+=240;
	}
	if(AFeatures->BO.Width == 0 ) return TRUE;
	I = 1;
	Low = AFeatures->BO.Width / 2;
	High = Low + 1;
	iTemp = 0;
	iBlockRows=*p;
	p++;
	while (1) { //包含Block
		if(I % 2 == 1 )
			CurCnt = Low--;
		else
			CurCnt = High++;
		if(I > (AFeatures->BO.Width + 1) ) break;
		I++;
		if((CurCnt >= 1)&&(CurCnt <= AFeatures->BO.Width) ) {
			if(iTemp < iBlockRows ) {
				//读取Block行信息
				sPos = *p++;
				ePos = *p++;
				for(J = 0;J<AFeatures->BO.Width;J++) {
					if((J >= sPos)&&(J <= ePos ) )
						AFeatures->BO.Bits[CurCnt - 1][J] = *p++;
					else
						AFeatures->BO.Bits[CurCnt - 1][J] = MaxBlockValue;
				}
				iTemp++;
			}
			else {
				for(J = 0;J<AFeatures->BO.Width;J++)
					AFeatures->BO.Bits[CurCnt - 1][J] = MaxBlockValue;
			}
		}
	}
	Result = TRUE;
	return Result;
}

int BiokeyTemplateToFeature(BYTE* ATemplate, FEATURES *AFeatures, int ATemplateType)
{
	BYTE *p, sPos, ePos;
	WORD WordData;
	DWORD LongWordData;
	int I, J, Low, High, CurCnt, iBlockRows, iTemp, Result;
	p = ATemplate;
	memcpy(&WordData, p, 2);
	if(WordData >> 8 !=0xca) return 0;
	//  {
	//	if(((('Z' - 65) << 5) | ('K' - 65)) == (WordData >> 6)==0) return FALSE;
	//	if((WordData & 0x007f)!=BIOKEYVERSION40) return FALSE;
	//  }
	//初始化Block数组
	memset(AFeatures, 0, sizeof(FEATURES));
	memset(&AFeatures->BO.Bits[0][0], 255,
			ZF_MAX_BLOCKED_ORIENTS_DIMENSION*ZF_MAX_BLOCKED_ORIENTS_DIMENSION);
	//处理头信息
	//Flag&Version
	p++; p++;
	//Block number
	AFeatures->BO.Width  = *p >> 2;
	AFeatures->BO.Height = *p >> 2;
	Result = (*p&3) == ATemplateType;
	if(ATemplateType == Tt_All ) //如果设为全部类型
		Result = TRUE;
	if(! Result ) return 0;
	p++;
	//Template length & Saved block number
	memcpy(&WordData, p,2);
	iBlockRows = WordData&63;
	p++;p++;
	//Character
	AFeatures->SP.Count = *p&3;
	AFeatures->M.Count  = *p >> 2;
	p++;
	//Ridge density
	AFeatures->G = *p;
	p++;
	//处理特征数据
	//X Y D C T
	for(I = 0;I<AFeatures->M.Count;I++) {
		AFeatures->M.X[I] = (p[0]+256*p[1])>>7;	//pm.W1 >> 7;
		AFeatures->M.Y[I] = (p[0]&127)<<2;	//(pm.W1&127) << 2;
		AFeatures->M.Y[I] = AFeatures->M.Y[I]+(p[2]>>6);	//AFeatures->M.Y[I] + (pm.B[0] >> 6);
		AFeatures->M.T[I] = (p[2]&0x3F)-1;	//(pm.B[0]&0x3F) - 1;
		AFeatures->M.D[I] = p[4];	//pm.B[2];
		AFeatures->M.C[I] = p[3];	//pm.B[1];
		p+=5;
	}
	//Pattern data
	for(I = 0;I<AFeatures->SP.Count;I++)
	{
		memcpy(&LongWordData, p, 4); p+=4;
		AFeatures->SP.X[I] = LongWordData >> 21;
		AFeatures->SP.Y[I] = (LongWordData >> 12)&511;
		AFeatures->SP.T[I] = (LongWordData >> 9)&7 ;
		AFeatures->SP.D[I] = LongWordData&511;
		AFeatures->SP.T[I] = AFeatures->SP.T[I] - 1;
	}
	if(AFeatures->BO.Width == 0 ) return p-ATemplate;
	I = 1;
	Low = AFeatures->BO.Width / 2;
	High = Low + 1;
	iTemp = 0;
	while (1) { //包含Block
		if(I % 2 == 1 )
			CurCnt = Low--;
		else
			CurCnt = High++;
		if(I > (AFeatures->BO.Width + 1) ) break;
		I++;
		if((CurCnt >= 1)&&(CurCnt <= AFeatures->BO.Width) ) {
			if(iTemp < iBlockRows ) {
				//读取Block行信息
				sPos = *p++;
				ePos = *p++;
				for(J = 0;J<AFeatures->BO.Width;J++) {
					if((J >= sPos)&&(J <= ePos ) )
						AFeatures->BO.Bits[CurCnt - 1][J] = *p++;
					else
						AFeatures->BO.Bits[CurCnt - 1][J] = MaxBlockValue;
				}
				iTemp++;
			}
			else {
				for(J = 0;J<AFeatures->BO.Width;J++)
					AFeatures->BO.Bits[CurCnt - 1][J] = MaxBlockValue;
			}
		}
	}
	Result = TRUE;
	return p-ATemplate;
}

int BiokeyTemplateLength(BYTE* ATemplate)
{
	FEATURES f;
	return BiokeyTemplateToFeature41(ATemplate, &f);
}

int DelFeature(FEATURES *f, int DelCount)
{
	FEATURES nf;
	int Distance[1024];
	int i, DIndex[1024], minx, miny, maxx, maxy, sumx, sumy;
	if(DelCount<=0) return f->M.Count;
	if(DelCount>=f->M.Count) return 0;

	//找到特征点的中心位置
	minx=f->M.X[0];
	miny=f->M.Y[0];
	maxx=f->M.X[0];
	maxy=f->M.Y[0];
	sumx=f->M.X[0];
	sumy=f->M.Y[0];
	for(i=1;i<f->M.Count;i++)
	{
		sumx+=f->M.X[i];
		sumy+=f->M.Y[i];
		if(minx>f->M.X[i]) minx=f->M.X[i];
		if(miny>f->M.Y[i]) miny=f->M.Y[i];
		if(maxx<f->M.X[i]) maxx=f->M.X[i];
		if(maxy<f->M.Y[i]) maxy=f->M.Y[i];
	}
	sumx/=f->M.Count;
	sumy/=f->M.Count;
	minx=(maxx+minx)/2;
	miny=(maxy+miny)/2;
	sumx=(sumx+minx)/2;
	sumy=(sumy+miny)/2;

	//计算各个特征点到中心点的距离
	for(i=0;i<f->M.Count;i++)
	{
		Distance[i]=(f->M.X[i]-sumx)*(f->M.X[i]-sumx)+(f->M.Y[i]-sumy)*(f->M.Y[i]-sumy);
		DIndex[i]=i;
	}

	//排序，选出离中心点最远的若干个特征点，然后去掉它们
	for(i=0;i<DelCount;i++)
	{
		int j;
		for(j=1;j<f->M.Count-i;j++)
		{
			if(Distance[DIndex[j-1]]>Distance[DIndex[j]])
			{
				int swap=DIndex[j];
				DIndex[j]=DIndex[j-1];
				DIndex[j-1]=swap;
			}
		}
	}
	nf=*f;
	for(i=0;i<f->M.Count-DelCount;i++)
	{
		f->M.X[i]=nf.M.X[DIndex[i]];
		f->M.Y[i]=nf.M.Y[DIndex[i]];
		f->M.D[i]=nf.M.D[DIndex[i]];
		f->M.T[i]=nf.M.T[DIndex[i]];
		f->M.C[i]=nf.M.C[DIndex[i]];
	}
	return f->M.Count-=DelCount;
}

//压缩指纹模板到指定大小
int SetBiokeyTemplateLen(BYTE* ATemplate, int Len)
{
	FEATURES f;
	BYTE tmp[2048];
	if(BiokeyTemplateToFeature41(ATemplate, &f))
	{
		int len;
		len=BiokeyFeatureToTemplate41(TRUE, BIOKEYVERSION41, &f, tmp);
		if(len>Len)
		{
			len=BiokeyFeatureToTemplate41(FALSE, BIOKEYVERSION41, &f, tmp);
			if(len>Len)
			{
				if(0==DelFeature(&f, (len-Len+4)/5)) return 0;
				len=BiokeyFeatureToTemplate41(FALSE, BIOKEYVERSION41, &f, tmp);
			}
		}
		memcpy(ATemplate, tmp, len);
		return len;
	}
	else
		return 0;
}

int TestTemplate41(BYTE* ATemplate, BYTE* AResult)
{
	FEATURES f;
	if(BiokeyTemplateToFeature41(ATemplate, &f))
	{
		//		if(*(WORD*)ATemplate==0xcaa0)
		//			ReversFeature(&f);
		return BiokeyFeatureToTemplate41(TRUE, BIOKEYVERSION41, &f, AResult);
	}
	else
		return 0;
}

int TestTemplate(BYTE* ATemplate, BYTE* AResult)
{
	FEATURES f;
	if(BiokeyTemplateToFeature41(ATemplate, &f))
	{
		//		ReversFeature(&f);
		return BiokeyFeatureToTemplate(Tt_Registration,TRUE, BIOKEYVERSION40, &f, AResult);
	}
	else
		return 0;
}

/*
   BOOL BiokeyTemplateToFeature(BYTE* ATemplate, FEATURES *AFeatures, int ATemplateType)
   {
   BYTE *p, sPos, ePos;
   WORD WordData;
   long LongWordData;
   int I, J, Low, High, CurCnt, iBlockRows, iTemp, Result;
   p = ATemplate;

   if(!(ATemplate[0]==0xa0 && ATemplate[1]==0xca)) return 0;
//初始化Block数组
memset(AFeatures, sizeof(FEATURES), 0);
memset(AFeatures->BO.Bits,
ZF_MAX_BLOCKED_ORIENTS_DIMENSION*ZF_MAX_BLOCKED_ORIENTS_DIMENSION, 255);
//处理头信息
//Flag&Version
p++; p++;
//Block number
AFeatures->BO.Width  = *p >> 2;
AFeatures->BO.Height = *p >> 2;
Result = (*p&3) == ATemplateType;
if(ATemplateType == Tt_All ) //如果设为全部类型
Result = TRUE;
if(! Result ) return 0;
p++;
//Template length & Saved block number
iBlockRows = *p&63;
p++;p++;
//Character
AFeatures->SP.Count = *p&3;
AFeatures->M.Count  = *p >> 2;
p++;
//Ridge density
AFeatures->G = *p;
p++;
//处理特征数据
WordData = *p; *(((BYTE*)&WordData)+1)=p[1];
//X Y D C T
for(I = 0;I<AFeatures->M.Count;I++) {
AFeatures->M.X[I] = WordData >> 7;
AFeatures->M.Y[I] = (WordData&127) << 2;
p++;p++;
AFeatures->M.Y[I] = AFeatures->M.Y[I] + (*p >> 6);
AFeatures->M.T[I] = *p++&0x3F - 1;
WordData = *p; *(((BYTE*)&WordData)+1)=p[1];
AFeatures->M.D[I] = WordData >> 8;
AFeatures->M.C[I] = WordData&0xFF;
p++;p++;
WordData = *p; *(((BYTE*)&WordData)+1)=p[1];
}
for(I = 0;I<AFeatures->SP.Count;I++)
{
//Pattern data
LongWordData = *p++;*(((BYTE*)&LongWordData)+1)=*p++;
 *(((BYTE*)&LongWordData)+2)=*p++;*(((BYTE*)&LongWordData)+3)=*p++;
 AFeatures->SP.X[I] = LongWordData >> 21;
 AFeatures->SP.Y[I] = (LongWordData >> 12)&511;
 AFeatures->SP.T[I] = (LongWordData >> 9)&7 ;
 AFeatures->SP.D[I] = LongWordData&511;
 AFeatures->SP.T[I] = AFeatures->SP.T[I] - 1;
 }
 if(AFeatures->BO.Width == 0 ) return Result;
 I = 1;
 Low = AFeatures->BO.Width / 2;
 High = Low + 1;
 iTemp = 0;
 while (1) { //包含Block
 if(I % 2 == 1 )
 CurCnt = Low--;
 else
 CurCnt = High++;
if(I > (AFeatures->BO.Width + 1) ) break;
I++;
if((CurCnt >= 1)&&(CurCnt <= AFeatures->BO.Width) ) {
	if(iTemp < iBlockRows ) {
		//读取Block行信息
		sPos = *p++;
		ePos = *p++;
		for(J = 0;J<AFeatures->BO.Width;J++) {
			if((J >= sPos)&&(J <= ePos ) )
				AFeatures->BO.Bits[CurCnt - 1][J] = *p++;
			else
				AFeatures->BO.Bits[CurCnt - 1][J] = MaxBlockValue;
		}
		iTemp++;
	}
	else {
		for(J = 0;J<AFeatures->BO.Width;J++)
			AFeatures->BO.Bits[CurCnt - 1][J] = MaxBlockValue;
	}
}
}
Result = TRUE;
return Result;
}

*/


