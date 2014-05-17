//data link layer process by oscar
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
#include "dlc.h"
#include "options.h"
#include "utils.h"
#include "commfun.h"
#include "proc_commfun.h"

int *LoadFirmware(char *FirmwareFile, char *Version, int *Length, char **filebuf)
{
        FILE *fh = NULL;
        char line[4000] = {0}, linename[20] = {0};
        int i=0, dataline=0, checksum=0, position=0;
        char *fdata=NULL;
        char platform[60] = {0};
        char platformvalue[20] = {0};
        int CmpPlatformCnt = 2; //比对平台的最多次数
        int CmpRet = -1;   //平台比对的返回结果

        memset(platform,0,sizeof(platform));
        memset(platformvalue,0,sizeof(platformvalue));

        if (LoadStr(PLATFORM,platformvalue))
        {
           sprintf(platform,"%s%s",platformvalue,"_FirmwareVersion=");
        }
        else
        {
        	printf("device not set platform \n");
        	return ERR_CMD_NO_PLATFORME_ERROR;
        }

        fh=fopen(FirmwareFile, "r");
        if(fh!=NULL)
        {
                sprintf(linename,"Data%d=",dataline);
                while(fgets(line, 4000, fh))
                {
                	CmpRet = strncmp(line,platform, strlen(platform));
                	if(0 == i && CmpRet != 0)
                	{
                		printf("CmpPlatformCnt: %d LocalPlatform: %s FromPCPlatform: %s\n", CmpPlatformCnt, platform, line);
                		if(CmpPlatformCnt == 0)
                		{
                			printf("cmp platform failed\n");
                			return ERR_CMD_DIFF_PLATFORM_ERROR; //比对平台不一样，则升级失败
                		}
                		else
                		{
                			CmpPlatformCnt--;
                		}
                	}
                        if(0==i && 0 == CmpRet)
                        {
                        	strcpy(Version, line+strlen(platform));
                        }
                        else if(strncmp(line, "FirmwareLength=",strlen("FirmwareLength="))==0)
                        {
                                *Length=atoi(line+strlen("FirmwareLength="));
                                fdata=malloc(4000+*Length);
                                i++;
                        }
                        else if(strncmp(line, "FirmwareCheckSum=",strlen("FirmwareCheckSum="))==0)
                        {
                                checksum=atoi(line+strlen("FirmwareCheckSum="));
                                i++;
                        }
                        else if(i==2)
                        {
                                if(strncmp(line, linename, strlen(linename))==0)
                                {
                                        line[strlen(line)-1]='\0';
                                        position+=Decode16(line+strlen(linename), fdata+position);
                                        sprintf(linename,"Data%d=",++dataline);
                                        //printf("----------------------- position: %d *Length: %d\n",position,*Length);
                                        if(position==*Length) break;
                                }
                        }
                }
                fclose(fh);
        }

        if(position==*Length)
        {
                if(position)
                {
                        if(in_chksum((unsigned char*)fdata, position)==checksum)
                        {
                        	*filebuf = fdata;
                            return CMD_SUCCESS;
                        }
                        else
                        {
                        	printf("LoadFirmware: in_chksum failed\n");
				//printf("%d,%d\n",checksum,in_chksum((unsigned char*)fdata, position));
                            if(fdata)
                            {
                            	free(fdata);
                            }
                        	return ERR_CMD_DATA_CHKSUM_ERROR;
                        }
                }
        }
        else
        {
        	printf("file's length is err! position: %d *Length: %d\n",position, *Length);
        	return ERR_CMD_DATA_LEN_ERROR;
        }
}

int UpdateFirmware(char *filename)
{
        int fd = -1;
        char *filebuf = NULL;
        char buf[128] = {0};
        char version[128] = {0};
        char sFirmwareFiles[128] = {0};
        int filebuflen = 0;
        int res = 0;
        char iMainVersion[64] = {0};
        char iFWVersion[64] = {0};

        int RetLoadFirmware = 0;

        memset(buf,0,sizeof(buf));
        memset(iFWVersion,0,sizeof(iFWVersion));
        sprintf(buf, "%s",  filename);

        RetLoadFirmware = LoadFirmware(buf, version, &filebuflen, &filebuf);

        if((CMD_SUCCESS == RetLoadFirmware) && (NULL != filebuf))
        {
            if(strcmp(ConvertMonth(version, iFWVersion), ConvertMonth(MAINVERSION, iMainVersion))>=0)
            {
                if((((BYTE *)filebuf)[0]==0x1b)&&(((BYTE *)filebuf)[1]==0x55))
                {
                        sprintf(buf,"%supdate.tgz",RAMPATH);
                        //save to file
                        fd=open(buf, O_CREAT|O_WRONLY|O_TRUNC|O_SYNC, S_IRWXU|S_IRWXG|S_IRWXO);
                        if (fd>0)
                        {
                                printf("ExtractPakage \n");
                                zkfp_ExtractPackage(filebuf,NULL,(int*)filebuflen);
                                write(fd, filebuf, filebuflen);
                                close(fd);

                                sprintf(sFirmwareFiles, "tar xvzf %s -C %s && sync && rm %s -rf",buf,"/mnt/mtdblock/",buf);

                                printf("sFirmwareFiles: %s\n",sFirmwareFiles);
                                if (system(sFirmwareFiles)==EXIT_SUCCESS)
                                {
                                    res = CMD_SUCCESS;
                                }
                                else
                                {
                                	printf("UpdateFirmware: system failed\n");
                                	res = ERR_CMD_PROCESS_ERROR;
                                }
                        }
                        else
                        {
                        	printf("open failed: %s\n",buf);
                        	res = ERR_CMD_FILE_NOTEXIST;
                        }
                }
                else
                {
                	printf("NOT (((BYTE *)filebuf)[0]==0x1b)&&(((BYTE *)filebuf)[1]==0x55)\n");
                	res = ERR_CMD_UPDATE_FILEFLG_ERROR;
                }
            }
            else
            {
            	printf("ConvertMonth failed: version: %s  iFWVersion: %s, MAINVERSION: %s   iMainVersion: %s\n", version, iFWVersion, MAINVERSION, iMainVersion);
            	res = ERR_CMD_UPDATE_OLDVER_ERROR;
            }

        	if(NULL != filebuf)
        	{
        		free(filebuf);
        	}
        }
        else
        {
        	printf("UpdateFirmware: LoadFirmware failed!\n");
        	if(NULL != filebuf)
        	{
        		free(filebuf);
        	}
        	res = RetLoadFirmware;
        }

        memset(sFirmwareFiles, 0, sizeof(sFirmwareFiles));
        sprintf(sFirmwareFiles, "rm %s -rf",filename);
        system(sFirmwareFiles); //把升级文件删除，没有判断system(sFirmwareFiles)的返回值，原因：删除目的是节省机器空间。
                                //无论删除成功与否，不妨碍升级成功。
        printf("----------res :%d\n",res);
        return res;
}

//从PC->设备传来的文件，按照路径进行保存。
int SaveFile(char *buf,int inbuflen)
{
	char filename[100] = {0};
	char TmpFileName[100] = {0};
	U32 filenamelen = 0;
	int RetUpdateFirmware = 0;

	filenamelen = buf[0];
	memcpy(TmpFileName,&buf[1],filenamelen);
	GetEnvFilePath("USERDATAPATH",TmpFileName, filename);
	printf("filenamelen = %d,filename =%s \n",filenamelen,filename);

	int fd=open(filename, O_CREAT|O_RDWR|O_TRUNC|O_SYNC, S_IRWXU|S_IRWXG|S_IRWXO);
	DBPRINTF("fd: %d\n",fd);
	if (fd>0)
	{
		if(write(fd, &buf[filenamelen+1],inbuflen - filenamelen -1 ))
		{
			DBPRINTF("Write file ok\n");
			close(fd);

			if(strcmp(TmpFileName, "emfw.cfg") == 0)
			{
				RetUpdateFirmware = UpdateFirmware(filename);
				return RetUpdateFirmware;
			}
			else
			{
				printf("rec file'name: %s is not emfw.cfg\n",TmpFileName);
				return ERR_CMD_FILENAME_ERROR;
			}

		}
		else
		{
			close(fd);
			DBPRINTF("Write file error\n");
			return ERR_CMD_WFILE_ERROR;
		}
	}
	else
	{
		return ERR_CMD_FILE_NOTEXIST;
	}
}

int BroadModiyIP(char *p, int inbuflen)
{
	char *value = NULL;
	char rec_macinfo[50] = {0};
	char my_macinfo[50] = {0};
	char temp_buf[50] = {0};
	char ComPwd[24] = {0};
	char len = 0;
	int MACNameLen = 0;

	if(NULL == p)
	{
		return ERR_CMD_ERROR_CMD;
	}

	printf("set para2 is : %s\n",p);
	p[inbuflen] = '\0';
	value = p;
	while(*value)
	{
		if(','==*value)
		{
			break;
		}

		rec_macinfo[MACNameLen++] = *value++;
		if(MACNameLen > inbuflen)
		{
			break;
		}
	}

	if(MACNameLen >= inbuflen || MACNameLen < 1)
	{
		return ERR_CMD_DATA_LEN_ERROR;
	}
	else
	{
		sprintf(temp_buf,"%s","MAC");
		len = strlen(temp_buf);
		printf("len=%d,databuf:%s\n",len,temp_buf);
		GetOptionNameAndValue(temp_buf, 3,my_macinfo);
		printf("my_mac:%s,rec_mac:%s \n",my_macinfo,rec_macinfo);

		if(strcmp(my_macinfo,rec_macinfo) == 0 )//比较命令参数中的mac是否与本机mac相等
		{
			GetOptionNameAndValue("ComPwd", 6, ComPwd);

			if(strlen(ComPwd) > 7)
			{
				printf("Device has set CommKey, don't modify IP by broad -- strlen(ComPwd): %d\n",strlen(ComPwd));

				return ERR_CMD_COMKEY_ERROR;
			}

			printf("OK!..........\n");

			if(BroadSetOptionNameAndValue(value+1, inbuflen - MACNameLen))
			{
				return CMD_SUCCESS;
			}
			else
			{
				return ERR_CMD_PROCESS_ERROR;
			}
		}
		else
		{
			return ERR_CMD_DATA_PARA_ERROR;
		}
	}
}

