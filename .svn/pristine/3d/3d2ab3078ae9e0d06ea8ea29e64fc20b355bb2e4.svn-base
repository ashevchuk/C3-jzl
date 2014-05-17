/*************************************************
                                           
 ZEM 200                                          
                                                    
 sdcard_helper.c support for sdcard FLASH PEN DRIVE
                                                      
 Copyright (C) 2010-2015, ZKSoftware Inc.
                                                      
*************************************************/

#include <stdio.h>
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/sysmacros.h>
#include <sys/mount.h>
#include <time.h>
#include <mntent.h>
#include <dirent.h>
#include <ctype.h>
#include <wait.h>
#include "options.h"
#include "sdcard_helper.h"



int ExecuteMount(const char *device)
{
	char * MountPoint = SDCARD_MOUNTPOINT;
	char MountCmd[128] = {0};
	int ret = 0;

	sprintf(MountCmd, "mount /dev/%s  %s > VerifySD.txt 2>&1",  device,MountPoint);

	printf("%s\n",MountCmd);

	ret = system(MountCmd);

	return ret;
}


int DoLoopMountSDCard(void)
{
	int loop = 0;
	int ret = -1;
        
	while(loop < 6)
	{

		if(0 == ExecuteMount("mmca1"))
		{
			DBPRINTF("Mount mmca1 sucessfully\n");
			ret = 0;
			return ret;
		}
		else if(0 == ExecuteMount("mmca2"))
		{
			DBPRINTF("Mount mmca2 sucessfully\n");
			ret = 0;
			return ret;
		}
		else if(0 == ExecuteMount("mmca3"))
		{
			DBPRINTF("Mount mmca3 sucessfully\n");
			ret = ret;
			return 1;
		}
		else if(0 == ExecuteMount("mmca"))
		{
			DBPRINTF("Mount mmca sucessfully\n");
			ret = 0;
			return ret;
		}
		else
		{
			DBPRINTF("Mount  failed cnd: %d\n", loop);
		}

		loop++;
	}
	return ret;
}	

int DoUmountSDCard(void)
{
	const char * MPoint = SDCARD_MOUNTPOINT;
	char UmountCmd[128] = {0};
	char Path[32] = {0};
	char FileBuf[128] = {0};
	char *pFind = NULL;
	int ret = -1;
	int RetSystem = 0;
	int loop = 0;
	int fd = -1;
	int len = 0;

	sprintf(UmountCmd, "umount -l  %s > VerifySD.txt 2>&1",MPoint);
	//sprintf(UmountCmd, "umount    %s",MPoint);

	printf("%s\n",UmountCmd);

	while(loop < 6)
	{
		RetSystem = system(UmountCmd);

		printf("DoUmountSDCard: RetSystem: %d\n ",RetSystem);
		if(WEXITSTATUS(RetSystem) == 0 &&WIFEXITED(RetSystem) )
		{
			printf("ok\n");
		}
		else
		{
			printf("error:%d\n",WEXITSTATUS(RetSystem));

		}

		if(256 == RetSystem || 0 == RetSystem)
		{
			memset(Path, 0, 128);

			fd=open(GetEnvFilePath("USERDATAPATH", "VerifySD.txt",Path), O_RDWR|O_CREAT|O_SYNC, S_IRWXU|S_IRWXG|S_IRWXO);
			printf("open fd: %d  %s\n",fd, Path);

			if (fd == -1)
			{
				printf("open failed: %s\n", Path);
			}
			else
			{
				len = lseek(fd, 0, SEEK_END);

				if(len != 0)
				{
					lseek(fd, 0, SEEK_SET);
					if(read(fd, FileBuf, len)==len)
					{
						printf("read VerifySD.txt:  %s", FileBuf);
						pFind = strstr(FileBuf,"busy");
						if(pFind == NULL)
						{
							ret = 0;//已经卸载或者路径参数不对，此时可以进行挂载。
							close(fd);
							return ret;

						}
						else
						{
							ret = -2; //umount: Couldn't umount /mnt/sdcard: Device or resource busy，此时挂载，mount进程僵死。
							close(fd);
						}

					}
				}
				else
				{
					printf("VerifySD.txt len: %d\n",len);
					ret = 0;
					close(fd);
					//return ret;
				}
			}
			close(fd);
		}
		loop++;
	}


	return  ret;
}

int DoMountSDCard(void)
{
	int ret=0;
	int RetDoUmountSDCard = 0;

	RetDoUmountSDCard = DoUmountSDCard();

	if(0 == RetDoUmountSDCard)
	{
		usleep(1000*100); //卸载之后，需要等待一段时间才能进行重新挂载，否则进程会僵死
		if(0 == DoLoopMountSDCard())
		{
			ret = 0;
			DBPRINTF("mount successful\n");
		}
		else
		{
			ret = -1;
			DBPRINTF("mounted failed\n");
		}
	}
	else
	{
		ret = -2;
		printf("DoUmountSDCard failed\n");
	}
       
	return ret;  
}
