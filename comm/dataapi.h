
//
// 以下为字段名回调函数
//
//	get 类型
typedef int (*FldValueFunT)(void *data, char *valueBuffer, int bufferSize);
//	set 类型
typedef int (*FldSetFunT)(void *data, char *valueBuffer, int bufferSize);
//	check 类型
typedef int (*FldCheckFunT)(void *data);

//
// 以下为表使用回调函数
//
//the callback function for test a rowData is taged deleted(return 1) or not(return 0)
typedef int (*TblIsDeletedFunT)(void *rowData);
//the callback function for doing delete operation for a rowData in a table.
//return 1 for overwrite old record, return 0 for not overwrite
typedef int (*TblDelRowFunT)(void *rowData);
typedef int (*TblRepRecord)(void *data, int flag, int primaryKey);
typedef int (*TblGetAllPK)(void *dataIn, void *dataOut, int *size);
typedef int (*TblSetInit)(void *data);

#define TRANSACTION_TABLE_ID 5
//
//	使用数据类型运算符类型
//
typedef enum {CHAR_T, STRING_T, INT_T, UINT_T, TIME_T} DATA_TYPE;
typedef enum {NO_OP, NOT, AND, OR}OPT;

#define FIELD_WIDTH_MAX	 2*1024

typedef struct 
{	
	int 			fieldID;		//段ID
	char			*name;				//字段名
	int				width;				
	int 			offset;				
	int				isPrimaryKey;
	void			*foriegnKey;		
	FldValueFunT	cbGetFun;			//读取该字段值时使用
	FldSetFunT		cbSetFun;			//设置该字段值时使用
	FldCheckFunT	cbCheckVal;			//检查该字段值时使用
	DATA_TYPE		type;				
	char			buffer[2048];		//暂存的缓冲1
	char			newData[1024];		//暂存的缓冲2

//过滤条件参数
	int len;//条件内容长度
	char condition_buffer[100];//具体内容
}C3FIELD_DESC;

typedef struct 
{
	int 					tableID;			//表ID
	char					*name;			//表名
	int						fd;				//文件fd
	int						FCT;			//内部使用的表类型，如指纹?用户？
	int						fieldCount;		//字段的个数
	int						rowDataWidth;	//一行记录的字节数
	C3FIELD_DESC				*fieldDesc;		//指向一个字段描述结构
	TblIsDeletedFunT		testDelFun;		//该记录是否被删除过得
	TblDelRowFunT			doDelFun;		//删除该记录回调函数
	TblGetAllPK				cbGetAllPKFun;	//返回一个组合，表示唯一
	TblSetInit				cbSetInit;
}C3TABLE_DESC;


typedef struct 
{
//    unsigned char buffer[6000];
    unsigned char *buffer;
    int bufferSize;
}Tdatabuf;

typedef struct 
{
	Tdatabuf 		buffer;			//用于接收返回数据的缓冲区，返回的数据是文本格式的，可能是多条记录，
	C3TABLE_DESC		*C3tableDesc;
	int 				select_field_count;		//显示需要的字段
	C3FIELD_DESC 	select_field_desc[50];	//字段内容
	int				condition_field_count;		//条件字段数量
	C3FIELD_DESC	condition_field_desc[50];		//		

	int table_para_num;							//表参数数量
	int table_para;								//表参数内容  目前最多1 个，所以没有定义为数组
}C3DPARAM;



int c3CbQuery(void *data, int idx, void *param);

int GetTableCount();

C3TABLE_DESC *getc3TableDesc(int TableID);






