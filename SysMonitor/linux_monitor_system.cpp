#include "monitor_system.h"
#ifndef WIN32
#include <stdio.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#endif
int 
CLinuxSysinfo::write(int fd, char *buf)
{
	FastWriter json_write;
	Value  json_value;
	string jsonstr;
	get_loadavg(json_value);
	get_systemtime(json_value);
	get_kernel_version(json_value);
	get_os_release( json_value);
	get_os_type(json_value);
	get_diskinfo(json_value);
	get_disk_stat(json_value);
	get_cpu_rate(json_value);
	get_meminfo(json_value);
	jsonstr = json_write.write(json_value);
	memcpy(buf, jsonstr.c_str(), jsonstr.length() + 1);
	printf("%s\n", buf);
	return jsonstr.length() + 1;
	return 0;
}


void CLinuxSysinfo::get_loadavg(Value& json_value){//cpuƽ������
#ifndef WIN32
	int f = 0;
	char buffer[80] = "";                         /* �����ַ�������ʼ��Ϊ'\0' */
	char buf[5][10];
	char *file = "/proc/loadavg";
	f = open(file, O_RDONLY);
	if (f == 0)
	{
		printf("error to open: %s\n", file);
		exit(EXIT_FAILURE);
	}

	read(f, (void *)buffer, 80);
	sscanf(buffer, "%s %s %s %s %s",            /* sscanf()��ֳɶ���ַ��� */
		&buf[0], &buf[1], &buf[2], &buf[3], &buf[4]);
	json_value["loadavg_one"] = buf[0];
	json_value["loadavg_five"] = buf[1];
	json_value["loadavg_quarter"] = buf[2];
	json_value["time_interval"] = buf[3];
	json_value["maxthreadnum"] = buf[4];
	close(f);
#endif
}
void CLinuxSysinfo::get_systemtime(Value& json_value){
#ifndef WIN32
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
#endif
}

void CLinuxSysinfo::get_kernel_version(Value& json_value){
#ifndef WIN32
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
	buffer[strlen(buffer) - 1] = 0;                 /* ��ʵ��tr()�����Ĺ��� */
	json_value["on_version"] = buffer;
	close(f);
#endif
}

void CLinuxSysinfo::get_os_release(Value& json_value){
#ifndef WIN32
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
#endif
}

void CLinuxSysinfo::get_os_type(Value& json_value){
#ifndef WIN32
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
#endif
}
void CLinuxSysinfo::get_diskinfo(Value& json_value){
#ifndef WIN32
	FILE *fp;
	size_t len = 0;
	int nread = 0;
	char *buffer = NULL;
	char buf[4][32];
	char *file = "/proc/partitions";
	fp = fopen(file, "rb");
	if (fp == NULL)
	{
		printf("error to open: %s\n", file);
		exit(EXIT_FAILURE);
	}
	while ((nread = getline(&buffer, &len, fp)) != -1) { /* ��ʵ�ֶ��еĹ��� */
		sscanf(buffer, "%s %s %s %s",&buf[0],&buf[1],&buf[2],&buf[3]);
		if (strstr(buf[3],"da")!=NULL){
			string keystr = "disk_kb_";
			keystr += buf[3];
			json_value[keystr] = buf[2];
		}
	}
	if (buffer)
		free(buffer);
	fclose(fp);
#endif
}
void CLinuxSysinfo::get_disk_stat(Value& json_value){
#ifndef WIN32
	FILE *fp;
	int nread = 0;
	size_t len = 0;
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
	//printf("  ����  ������  д����\n");
	while ((nread = getline(&buffer, &len, fp)) != -1) { /* ��ʵ�ֶ��еĹ��� */
		sscanf(buffer, "%04s%08s%s %s %s %s %s %s %s %s %s %s %s %s",
			&buf[0], &buf[1], &buf[2], &buf[3], &buf[4], &buf[5], &buf[6],
			&buf[7], &buf[8], &buf[9], &buf[10], &buf[11], &buf[12], &buf[13]);
		if ((p = strstr(buf[2], "da")) != NULL)
		{
			string key = buf[2];
			string keyread = key + "disk_read_";
			json_value[keyread.c_str()] = buf[3];                                  /* loop���ػ�·�������� */
			string keywrite = key + "disk_write_";
			json_value[keywrite.c_str()] = buf[7];
		}
	}
	if (buffer)
		free(buffer);
	fclose(fp);
#endif
}

void CLinuxSysinfo::get_cpu_rate(Value& json_value)//��ȡCPU�����ʽ�����
{
#ifndef WIN32
	FILE *fp;
	int nread = 0;
	size_t len = 0;
	char *buf = NULL;
	char *buffer = NULL;
	char *file = "/proc/stat";
	char cpu[5]; 
	int user,nice,sys,idle,iowait,irq,softirq;
	int all1,all2,idle1,idle2;
	fp = fopen(file, "rb");
	if (fp == NULL)
	{
		printf("error to open: %s\n", file);
		exit(EXIT_FAILURE);
	}

	getline(&buffer,&len,fp);
	sscanf(buffer,"%s%d%d%d%d%d%d%d",cpu,&user,&nice,&sys,&idle,&iowait,&irq,&softirq);
	all1 = user+nice+sys+idle+iowait+irq+softirq;  
	idle1 = idle;
	while ((nread = getline(&buffer, &len, fp)) != -1) {
		if ((buf = strstr(buffer, "processes")) != NULL)  /* ��ʵ��grep�Ĺ��� */
			break;
	}
	buffer[strlen(buffer) - 1] = 0;                 /* ��ʵ��tr()�����Ĺ��� */
	char count[16] = "";
	sscanf(buffer, "%s%s", count, count);
	json_value["processes"] = count;
	
	rewind(fp);
	sleep(1);
	memset(buffer,0,sizeof(buffer)); 
	cpu[0] = '\0';  
	user=nice=sys=idle=iowait=irq=softirq=0;  
	getline(&buffer,&len,fp);
	sscanf(buffer,"%s%d%d%d%d%d%d%d",cpu,&user,&nice,&sys,&idle,&iowait,&irq,&softirq);
	all2 = user+nice+sys+idle+iowait+irq+softirq;  
	idle2 = idle;  
	char str_usage[10];
	sprintf_s(str_usage,sizeof(str_usage),"%.2f",(float)(all2-all1-(idle2-idle1)) / (all2-all1)*100 ); 
	json_value["cpu_usage"] = str_usage;
	if (buffer)
		free(buffer);
	fclose(fp);
#endif
}

void CLinuxSysinfo::get_meminfo(Value& json_value)
{
#ifndef WIN32
	FILE *fp;
	int nread = 0;
	size_t len = 0;
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
		if ((buf = strstr(buffer, "MemTotal")) != NULL)  /* ��ʵ��grep�Ĺ��� */
		{
			buffer[strlen(buffer) - 1] = 0;             /* ��ʵ��tr()�����Ĺ��� */
			sscanf(buffer, "%s%s", content, content);
			int memtotal_kb = (int)(strtof(content, NULL));
			json_value["MemTotalKB"] = memtotal_kb;
		}
		if ((buf = strstr(buffer, "MemFree")) != NULL)  /* ��ʵ��grep�Ĺ��� */
		{
			buffer[strlen(buffer) - 1] = 0;                 /* ��ʵ��tr()�����Ĺ��� */
			sscanf(buffer, "%s%s", content, content);
			int memfree_kb = (int)(strtof(content, NULL));
			json_value["MemFreeKB"] = memfree_kb ;
		}
		if ((buf = strstr(buffer, "SwapTotal")) != NULL)  /* ��ʵ��grep�Ĺ��� */
		{
			buffer[strlen(buffer) - 1] = 0;                 /* ��ʵ��tr()�����Ĺ��� */
			sscanf(buffer, "%s%s", content, content);
			int swaptotal_kb = (int)(strtof(content, NULL));
			json_value["SwapTotalKB"] = swaptotal_kb;
		}
		if ((buf = strstr(buffer, "SwapFree")) != NULL)  /* ��ʵ��grep�Ĺ��� */
		{
			buffer[strlen(buffer) - 1] = 0;                 /* ��ʵ��tr()�����Ĺ��� */
			sscanf(buffer, "%s%s", content, content);
			int swapfree_kb = (int)(strtof(content, NULL));
			json_value["SwapFreeKB"] = swapfree_kb;
		}
		if ((buf = strstr(buffer, "VmallocTotal")) != NULL)  /* ��ʵ��grep�Ĺ��� */
		{
			buffer[strlen(buffer) - 1] = 0;                 /* ��ʵ��tr()�����Ĺ��� */
			sscanf(buffer, "%s%s", content, content);
			Int64 vmalloctotal_kb = (Int64)(strtof(content, NULL));
			json_value["VmallocTotalKB"] = vmalloctotal_kb ;
		}
		if ((buf = strstr(buffer, "VmallocUsed")) != NULL)  /* ��ʵ��grep�Ĺ��� */
		{
			buffer[strlen(buffer) - 1] = 0;                 /* ��ʵ��tr()�����Ĺ��� */
			sscanf(buffer, "%s%s", content, content);
			int vmallocused_kb = (int)(strtof(content, NULL));
			json_value["VmallocUsedKB"] = vmallocused_kb;
		}
	}
	if (buffer)
		free(buffer);
	fclose(fp);
#endif
}
