#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <unistd.h>
#include "ccc.h"
#include "arca.h"
#include "dataapi.h"
#include "dlc.h"

UINT32 buftovalue(UCHAR *buf,int size)
{
	if(size == 1)
		return buf[0] & 0xff;
	else if(size == 2)
		return buf[1] *256 + buf[0];
	else if(size == 3)
		return (buf[2] *256 + buf[1] )*256 +  buf[0];
	else if(size == 4)
		return (buf[3]*256*256*256 + buf[2]*256*256+  buf[1]*256+ buf[0]);
	else
		return 0;
}

int intvaluetobuf(UCHAR  *in_value,UCHAR *buff,DATA_TYPE datatype)
{
	UINT32 value;
	int i;
	
	switch(datatype)
	{
		 case CHAR_T:
		 {	buff[0] = in_value[0];
			return 1;
		 }
		 break;
		 case INT_T:
		 {
			if(in_value[1] & 0xff)
			{
				buff[0] = in_value[0];
				buff[1] = in_value[1];
				return 2;
			}
			else
			 {	
			 	buff[0] = in_value[0];
				return 1;
			 }
				
		 }
	 	break;
		case STRING_T:
		{
			i = 0;
			while(in_value[i])
			{	
				buff[i] = in_value[i];
				i++;
			}
			return i;
		}
		break;

		 case UINT_T:
		 case TIME_T:
			value =  buftovalue(in_value,4);
//			printf("intvaluetobuf value = %d \n",value);
									
			if(value & 0xff000000)
			{
				buff[0] = value & 0xff;
				buff[1] = (value  >> 8) & 0xff;
				buff[2] = (value  >> 16) & 0xff;
				buff[3] = (value  >> 24) & 0xff;
				return 4;
			}
			else if(value & 0xff0000)
			{
				buff[0] = value & 0xff;
				buff[1] = (value  >> 8) & 0xff;
				buff[2] = (value  >> 16) & 0xff;
				return 3;
			}
			else if(value & 0xff00)
			{
				buff[0] = value & 0xff;
				buff[1] = (value  >> 8) & 0xff;
				return 2;
			}
			else 
			{
				buff[0] = value & 0xff;
				return 1;
			}
		break;
	}

	return 0;
}


int c3CbQuery(void *data, int idx, void *param)
{	
	int i, j;
	int result;
	UINT32 value,value2;
	C3DPARAM *p=param;
	char buff[FIELD_WIDTH_MAX] = {0};
	char *outdata;
	int out_field_len;

//	printf("c3CbQuery   &&  p->condition_field_count = %d\n",p->condition_field_count);
//	printf("c3CbQuery in buff : name = %s  width = %d \n",p->C3tableDesc->name,
//		p->C3tableDesc->rowDataWidth);
//	DEBUGTRACE(data,p->C3tableDesc->rowDataWidth);
	if(p->condition_field_count == 0)//无条件，所有数据合格
		goto QueryOK;

	for(i = 0;i< (p->condition_field_count);i++)//
	{	
		j = p->condition_field_desc[i].fieldID -1;
//		printf("condition_field_desc[ %d ], fieldID = %d,/\n",i,j+1);
//		printf("name = %s,offset = %d ,width = %d\n",(p->C3tableDesc->fieldDesc +j)->name,
//			(p->C3tableDesc->fieldDesc +j)->offset,(p->C3tableDesc->fieldDesc +j)->width);
		memcpy(buff,data +(p->C3tableDesc->fieldDesc + j)->offset,(p->C3tableDesc->fieldDesc+j)->width);
//		printf("p->C3tableDesc->fieldDesc->type = %d \n",p->C3tableDesc->fieldDesc->type);
		switch((p->C3tableDesc->fieldDesc + j)->type)
		{
			 case CHAR_T:
			 case INT_T:
			 case UINT_T:
			 case TIME_T:
//			 	printf("Table:0x%x,0x%x,0x%x,0x%x,",buff[0]&0xff,buff[1]&0xff,buff[2]&0xff,buff[3]&0xff);
//				printf("buff:0x%x,0x%x,0x%x,0x%x",p->condition_field_desc[i].condition_buffer[0] & 0xff,
//													p->condition_field_desc[i].condition_buffer[1] & 0xff,
//													p->condition_field_desc[i].condition_buffer[2] & 0xff,
//													p->condition_field_desc[i].condition_buffer[3]& 0xff) ;
				value = buftovalue(p->condition_field_desc[i].condition_buffer,p->condition_field_desc[i].len);
				value2 = buftovalue(buff,(p->C3tableDesc->fieldDesc+j)->width);
//				printf("    value=%d, value2=%d\n",value,value2);
//				result = strncasecmp(buff,p->condition_field_desc[i].condition_buffer,
//					p->condition_field_desc[i].len);
				result = (value == value2) ? 0 :1;
				if(result == 0)
			 	{
					printf("strncasecmp rsult: %d,len = %d\n",result,p->condition_field_desc[i].len);
					break;
				}
				else
					return 0;
				
				break;
			 case STRING_T:
//			 	printf("Table_s: %s,",buff);
//				printf("buff_s: %s   \n",p->condition_field_desc[i].condition_buffer);
			 	if(strcmp(buff,p->condition_field_desc[i].condition_buffer) == 0)
					break;
				else
					return 0;
				break;
			default:
				return 0;
		}

	}


QueryOK:	
	
	outdata = p->buffer.buffer +  p->buffer.bufferSize;
	out_field_len = 0;

	
	for(i = 0,j = 0;i< p->select_field_count;i++)
	{
//		printf("i = %d,j = %d  field_count =%d\n",i,j,p->select_field_count);
		memset(buff,0x00,FIELD_WIDTH_MAX);
		if(p->select_field_desc[i].fieldID > p->C3tableDesc->fieldCount)
			outdata[j++] = 0;
		else
		{
//			printf("id = %d,name =%s,offset = %d,width = %d\n",
//				(p->C3tableDesc->fieldDesc + p->select_field_desc[i].fieldID-1)->fieldID,
//				(p->C3tableDesc->fieldDesc + p->select_field_desc[i].fieldID-1)->name,
//				(p->C3tableDesc->fieldDesc + p->select_field_desc[i].fieldID-1)->offset,
//				(p->C3tableDesc->fieldDesc + p->select_field_desc[i].fieldID-1)->width);
			if((p->C3tableDesc->fieldDesc + p->select_field_desc[i].fieldID-1)->type == CHAR_T)
			{
				memcpy(buff, data +  (p->C3tableDesc->fieldDesc + p->select_field_desc[i].fieldID-1)->offset,sizeof(char));
				out_field_len = 1;
			}
			else if((p->C3tableDesc->fieldDesc + p->select_field_desc[i].fieldID-1)->type == INT_T)
			{
				value =  buftovalue(data +  (p->C3tableDesc->fieldDesc + p->select_field_desc[i].fieldID-1)->offset,2);

				out_field_len =  intvaluetobuf(data +  (p->C3tableDesc->fieldDesc + p->select_field_desc[i].fieldID-1)->offset,
										buff,INT_T);
//				printf("Uint_type buf_len = %d  \n",out_field_len);

			}
			else if((p->C3tableDesc->fieldDesc + p->select_field_desc[i].fieldID-1)->type == UINT_T)
			{
				value =  buftovalue(data +  (p->C3tableDesc->fieldDesc + p->select_field_desc[i].fieldID-1)->offset,4);

				out_field_len =  intvaluetobuf(data +  (p->C3tableDesc->fieldDesc + p->select_field_desc[i].fieldID-1)->offset,
										buff,UINT_T);
//				printf("Uint_type buf_len = %d  \n",out_field_len);

			}
			else if((p->C3tableDesc->fieldDesc + p->select_field_desc[i].fieldID-1)->type == STRING_T)
			{
				memcpy(buff,data +  (p->C3tableDesc->fieldDesc + p->select_field_desc[i].fieldID-1)->offset,
							(p->C3tableDesc->fieldDesc + p->select_field_desc[i].fieldID-1)->width);
				out_field_len = strlen(buff);
//				printf("string_type string_len = %d  \n",out_field_len);
			}				
//			DEBUGTRACE(data +  (p->C3tableDesc->fieldDesc + p->select_field_desc[i].fieldID-1)->offset,4);
			outdata[j++] = out_field_len;
			memcpy(outdata +j,buff,out_field_len);
//			DEBUGTRACE(outdata +j,out_field_len);
			j = j + out_field_len;
		}
		
	}

	 p->buffer.bufferSize += j;

	return 1;	
}

