#ifndef _ISOTEMPLATE_H_
#define _ISOTEMPLATE_H_

//Force C (not C++) names for API functions
#ifdef __cplusplus
extern "C"
{
#endif

#define EXT_TYPE_ZK_CURVATURE	0x1300
#define EXT_TYPE_ZK_BO			0x1400

#define FMR_FORMAT_ID 		"FMR"
#define FMR_FORMAT_ID_LEN 	4

// The version number
#define FMR_SPEC_VERSION	"20"
#define FMR_SPEC_VERSION_LEN	4

#define	FMR_MAX_NUM_MINUTIAE	255
#define FMR_MAX_NUM_EXTENDEDDATA	255
#define FMR_MAX_LENGTH_EXTENDEDDATA	255


typedef struct _CORE_DATA {
	int count;
	unsigned short			type[MAX_SINGULAR_POINT_COUNT];
	unsigned short			x_coord[MAX_SINGULAR_POINT_COUNT];
	unsigned short			y_coord[MAX_SINGULAR_POINT_COUNT];
	unsigned char			angle[MAX_SINGULAR_POINT_COUNT];	
}CORE_DATA, *PCORE_DATA;

typedef struct _DELTA_DATA{
	int count;
	unsigned short type[MAX_SINGULAR_POINT_COUNT];
	unsigned short x_coord[MAX_SINGULAR_POINT_COUNT];
	unsigned short y_coord[MAX_SINGULAR_POINT_COUNT];
	unsigned char			angle[MAX_SINGULAR_POINT_COUNT];	
}DELTA_DATA, *PDELTA_DATA;

typedef struct _EXTENDED_DATA{
	unsigned short				type_id;
	unsigned short				length;
	CORE_DATA	cddb;
	DELTA_DATA	dddb;
} EXTENDED_DATA, *PEXETENDED_DATA;

typedef struct _EXT_ZK_CURVATURE{
	unsigned short				type_id;
	unsigned short				length;
	BYTE						data[FMR_MAX_NUM_MINUTIAE];
} EXT_ZK_CURVATURE, *PEXT_ZK_CURVATURE;

typedef struct _EXT_ZK_BO{
	unsigned short				type_id;
	unsigned short				length;
	BYTE						data[1024];
} EXT_ZK_BO, *PEXT_ZK_BO;

typedef struct _EXTENDED{
	int BlockLength;
	int count;
	EXTENDED_DATA extended_data;
	EXT_ZK_CURVATURE	zkcurdb;
	EXT_ZK_BO			zkbodb;
} EXTENDED, *PEXTENDED;

typedef struct _MINUTIAEDATA{
	int count;
	WORD TypeAndX[FMR_MAX_NUM_MINUTIAE];	//Minutia type,X location
	WORD RFUAndY[FMR_MAX_NUM_MINUTIAE];	//RFU(inoutstate),Y location
	BYTE Angle[FMR_MAX_NUM_MINUTIAE];
	BYTE Quality[FMR_MAX_NUM_MINUTIAE];
} MINUTIAEDATA, *PMINUTIAEDATA;

typedef struct _FVMR{
	BYTE FingerPosition;
	BYTE ViewNumAndImpressionType;
	BYTE FingerQuality;
	BYTE NumMinutiae;
	MINUTIAEDATA minutiae;
} FVMR, *PFVMR;

typedef struct _ISOFMR {
	char FormatID[4];
	char SpecVersion[4];
	INT RecordLength;
	WORD CaptureEquipment;	//capture equipment compliance,capture equipment ID
	WORD XImageSize;
	WORD YImageSize;
	WORD XResolution;
	WORD YResolution;
	BYTE NumberFinger;
	BYTE Reserved;
	FVMR fvmr;
	EXTENDED extended;
} ISOFMR, *PISOFMR;





//ANSI ±ê×¼

typedef struct _ANSIEXT_ZK_CURVATURE{
	unsigned short				type_id;
	unsigned short				length;
	BYTE						data[FMR_MAX_NUM_MINUTIAE];
} ANSIEXT_ZK_CURVATURE, *PANSIEXT_ZK_CURVATURE;

typedef struct _ANSIEXT_ZK_BO{
	unsigned short				type_id;
	unsigned short				length;
	BYTE						data[1024];
} ANSIEXT_ZK_BO, *PANSIEXT_ZK_BO;

typedef struct _ANSICORE_DATA {
	int count;
	unsigned short			type[MAX_SINGULAR_POINT_COUNT];
	unsigned short			x_coord[MAX_SINGULAR_POINT_COUNT];
	unsigned short			y_coord[MAX_SINGULAR_POINT_COUNT];
	unsigned char			angle[MAX_SINGULAR_POINT_COUNT];	
}ANSICORE_DATA, *PANSICORE_DATA;

typedef struct _ANSIDELTA_DATA{
	int count;
	unsigned short type[MAX_SINGULAR_POINT_COUNT];
	unsigned short x_coord[MAX_SINGULAR_POINT_COUNT];
	unsigned short y_coord[MAX_SINGULAR_POINT_COUNT];
	unsigned char			angle[MAX_SINGULAR_POINT_COUNT];	
}ANSIDELTA_DATA, *PANSIDELTA_DATA;



typedef struct _ANSIEXTENDED_DATA{
	unsigned short				type_id;
	unsigned short				length;
	CORE_DATA	cddb;
	DELTA_DATA	dddb;
} ANSIEXTENDED_DATA, *PANSIEXETENDED_DATA;

typedef struct _ANSIEXTENDED{
	int BlockLength;
	int count;
	EXTENDED_DATA extended_data;
	EXT_ZK_CURVATURE	zkcurdb;
	EXT_ZK_BO			zkbodb;
} ANSIEXTENDED, *PANSIEXTENDED;


typedef struct _ANSIMINUTIAEDATA{
	int count;
	WORD TypeAndX[FMR_MAX_NUM_MINUTIAE];	//Minutia type,X location
	WORD RFUAndY[FMR_MAX_NUM_MINUTIAE];	//RFU(inoutstate),Y location
	BYTE Angle[FMR_MAX_NUM_MINUTIAE];
	BYTE Quality[FMR_MAX_NUM_MINUTIAE];
} ANSIMINUTIAEDATA, *PANSIMINUTIAEDATA;

typedef struct _ANSIFVMR{
	BYTE FingerPosition;
	BYTE ViewNumAndImpressionType;
	BYTE FingerQuality;
	BYTE NumMinutiae;
	ANSIMINUTIAEDATA minutiae;
} ANSIFVMR, *PANSIFVMR;

typedef struct _ANSIFMR {
	char FormatID[4];
	char SpecVersion[4];
	WORD RecordLength;
	DWORD CBEFFPID;	//capture equipment compliance,capture equipment ID
	WORD CaptureEquipment;
	WORD XImageSize;
	WORD YImageSize;
	WORD XResolution;
	WORD YResolution;
	BYTE NumberFinger;
	BYTE Reserved;
	ANSIFVMR fvmr;
	ANSIEXTENDED extended;
} ANSIFMR, *PASNIFMR;

#ifdef __cplusplus
}
#endif

#endif

