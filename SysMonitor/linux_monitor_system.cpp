#ifndef WIN32
#include "monitor_system.h"
#include <stdio.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>

int 
CLinuxSysinfo::write(int fd, char *buf)
{

	return 0;
}


void CLinuxSysinfo::get_loadavg(Value& json_value){
	int f = 0;
	char buffer[80] = "";                         /* 定义字符串并初始化为'\0' */
	char buf[5][10];
	char *file = "/proc/loadavg";
	f = open(file, O_RDONLY);
	if (f == 0)
	{
		printf("error to open: %s\n", file);
		exit(EXIT_FAILURE);
	}
	read(f, (void *)buffer, 80);
	sscanf(buffer, "%s %s %s %s %s",            /* sscanf()拆分成多个字符串 */
		&buf[0], &buf[1], &buf[2], &buf[3], &buf[4]);
	json_value["loadavg_one"] = buf[0];
	json_value["loadavg_five"] = buf[1];
	json_value["loadavg_quarter"] = buf[2];
	json_value["time_interval"] = buf[3];
	json_value["maxthreadnum"] = buf[4];
	close(f);
}
void CLinuxSysinfo::get_systemtime(Value& json_value){
	int f = 0;
	char buffer[80] = "";
	char buf[2][10];
	float seconds = 0;
	float secondr = 0;
	char *file = "/proc/uptime";
	f = open(file, O_RDONLY);
	if (f == 0)
	{
		printf("error to open: %s\n", file);
		exit(EXIT_FAILURE);
	}
	read(f, (void *)buffer, 80);
	sscanf(buffer, "%s %s", &buf[0], &buf[1]);
	close(f);
	json_value["system_run"] = buf[0];
	json_value["system_idle"] = buf[1];
	close(f);
}

void CLinuxSysinfo::get_kernel_version(Value& json_value){
	int f = 0;
	char buffer[80] = "";
	char *file = "/proc/sys/kernel/version";
	f = open(file, O_RDONLY);
	if (f == 0)
	{
		printf("error to open: %s\n", file);
		exit(EXIT_FAILURE);
	}
	read(f, (void *)buffer, 80);
	buffer[strlen(buffer) - 1] = 0;                 /* 简单实现tr()函数的功能 */
	json_value["on_version"] = buffer;
	close(f);
}

void CLinuxSysinfo::get_os_release(Value& json_value){
	int f = 0;
	char buffer[80] = "";
	char *file = "/proc/sys/kernel/osrelease";
	f = open(file, O_RDONLY);
	if (f == 0)
	{
		printf("error to open: %s\n", file);
		exit(EXIT_FAILURE);
	}
	read(f, (void *)buffer, 80);
	buffer[strlen(buffer) - 1] = 0;
	json_value["os_release_version"] = buffer;
	close(f);
}

void CLinuxSysinfo::get_os_type(Value& json_value){
	int f = 0;
	char buffer[80] = "";
	char *file = "/proc/sys/kernel/ostype";
	f = open(file, O_RDONLY);
	if (f == 0)
	{
		printf("error to open: %s\n", file);
		exit(EXIT_FAILURE);
	}
	read(f, (void *)buffer, 80);
	buffer[strlen(buffer) - 1] = 0;
	json_value["os_type"] = buffer;
	close(f);
}
void CLinuxSysinfo::get_diskinfo(Value& json_value){
	FILE *fp;
	ssize_t len = 0;
	int nread = 0;
	char buf[4][32];
	char *file = "/proc/partitions";
	fp = fopen(file, "rb");
	if (fp == NULL)
	{
		printf("error to open: %s\n", file);
		exit(EXIT_FAILURE);
	}
	while ((nread = getline(&buffer, &len, fp)) != -1) { /* 简单实现读行的功能 */
		sscanf(buffer, "%s %s %s %s",&buf[0],&buf[1],&buf[2],&buf[3]);
		if (strstr(buf[3],"da")!=NULL){
		}
	}
}
void CLinuxSysinfo::get_disk_stat(Value& json_value){
	
	FILE *fp;
	int nread = 0;
	ssize_t len = 0;
	char *buffer = NULL;
	char buf[20][32];
	char *file = "/proc/diskstats";
	char *p;
	fp = fopen(file, "rb");
	if (fp == NULL)
	{
		printf("error to open: %s\n", file);
		exit(EXIT_FAILURE);
	}
	//printf("  磁盘  读次数  写次数\n");
	while ((nread = getline(&buffer, &len, fp)) != -1) { /* 简单实现读行的功能 */
		sscanf(buffer, "%04s%08s%s %s %s %s %s %s %s %s %s %s %s %s",
			&buf[0], &buf[1], &buf[2], &buf[3], &buf[4], &buf[5], &buf[6],
			&buf[7], &buf[8], &buf[9], &buf[10], &buf[11], &buf[12], &buf[13]);
		if ((p = strstr(buf[2], "da")) != NULL)
		{
			string key = buf[2];
			string keyread = key + "disk_read_";
			json_value[keyread.c_str()] = &buf[3];                                  /* loop本地回路不作操作 */
			string keywrite = key + "disk_read_";
			json_value[keywrite.c_str()] = &buf[7];
		}
		else {
			printf("%06s%08s%08s\n",
				&buf[2], &buf[3], &buf[7]);
		}
	}
	if (buffer)
		free(buffer);
	fclose(fp);
}

void CLinuxSysinfo::get_processes(void)
{
	FILE *fp;
	int nread = 0;
	ssize_t len = 0;
	char *buf = NULL;
	char *buffer = NULL;
	char *file = "/proc/stat";
	fp = fopen(file, "rb");
	if (fp == NULL)
	{
		printf("error to open: %s\n", file);
		exit(EXIT_FAILURE);
	}
	while ((nread = getline(&buffer, &len, fp)) != -1) {
		if ((buf = strstr(buffer, "processes")) != NULL)  /* 简单实现grep的功能 */
			break;
	}
	buffer[strlen(buffer) - 1] = 0;                 /* 简单实现tr()函数的功能 */
	char count[16] = "";
	sscanf(buffer, "%s%s", count, count);
	printf("执行线程数目:\t%s\n", count);
	if (buffer)
		free(buffer);
	fclose(fp);
}

void CLinuxSysinfo::get_meminfo()
{
	FILE *fp;
	int nread = 0;
	ssize_t len = 0;
	char *buf = NULL;
	char *buffer = NULL;
	char *file = "/proc/meminfo";
	char content[16] = "";
	fp = fopen(file, "rb");
	if (fp == NULL)
	{
		printf("error to open: %s\n", file);
		exit(EXIT_FAILURE);
	}
	while ((nread = getline(&buffer, &len, fp)) != -1) {
		if ((buf = strstr(buffer, "MemTotal")) != NULL)  /* 简单实现grep的功能 */
		{
			buffer[strlen(buffer) - 1] = 0;             /* 简单实现tr()函数的功能 */
			sscanf(buffer, "%s%s", content, content);
			int memtotal_kb = (int)(strtof(content, NULL));
			printf("内存总容量:\t%dG%4dM %4dK\n",
				memtotal_kb / (1024 * 1024),           /* Gb */
				(memtotal_kb / (1024)) % 1024,         /* Mb */
				(memtotal_kb % (1024 * 1024)) % 1024);   /* Kb */
		}
		if ((buf = strstr(buffer, "MemFree")) != NULL)  /* 简单实现grep的功能 */
		{
			buffer[strlen(buffer) - 1] = 0;                 /* 简单实现tr()函数的功能 */
			sscanf(buffer, "%s%s", content, content);
			int memfree_kb = (int)(strtof(content, NULL));
			printf("内存可用容量:\t%dG%4dM %4dK\n",
				memfree_kb / (1024 * 1024),           /* Gb */
				(memfree_kb / (1024)) % 1024,         /* Mb */
				(memfree_kb % (1024 * 1024)) % 1024);   /* Kb */
		}
		if ((buf = strstr(buffer, "SwapTotal")) != NULL)  /* 简单实现grep的功能 */
		{
			buffer[strlen(buffer) - 1] = 0;                 /* 简单实现tr()函数的功能 */
			sscanf(buffer, "%s%s", content, content);
			int swaptotal_kb = (int)(strtof(content, NULL));
			printf("SWAP总容量:\t%dG%4dM %4dK\n",
				swaptotal_kb / (1024 * 1024),           /* Gb */
				(swaptotal_kb / (1024)) % 1024,         /* Mb */
				(swaptotal_kb % (1024 * 1024)) % 1024);   /* Kb */
		}
		if ((buf = strstr(buffer, "SwapFree")) != NULL)  /* 简单实现grep的功能 */
		{
			buffer[strlen(buffer) - 1] = 0;                 /* 简单实现tr()函数的功能 */
			sscanf(buffer, "%s%s", content, content);
			int swapfree_kb = (int)(strtof(content, NULL));
			printf("SWAP可用容量:\t%dG%4dM %4dK\n",
				swapfree_kb / (1024 * 1024),           /* Gb */
				(swapfree_kb / (1024)) % 1024,         /* Mb */
				(swapfree_kb % (1024 * 1024)) % 1024);   /* Kb */
			break;                              /* 所需的信息已满足，退出循环 */
		}
	}
	if (buffer)
		free(buffer);
	fclose(fp);
}
#endif