//---------------------------------------
/*
新版通讯协议命令定义
*/

#ifndef _PROC_COMMFUN_H_
#define _PROC_COMMFUN_H_

int SaveFile(char *buf,int inbuflen);
int BroadModiyIP(char *p, int inbuflen);
int SaveTableContent(int FCT,char *saveBuf);
#endif
