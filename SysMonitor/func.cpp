#include "func.h"

int GetCurrentPath(char buf[], char *pFileName)
{
#ifdef WIN32
	GetModuleFileName(NULL, buf, MAX_PATH);
#else
	char pidfile[64];
	int bytes;
	int fd;
	sprintf(pidfile, "/proc/%d/cmdline", getpid());

	fd = open(pidfile, O_RDONLY, 0);
	bytes = read(fd, buf, 256);
	close(fd);
	buf[MAX_PATH] = '\0';
#endif
	char * p = &buf[strlen(buf)];
	do{
		*p = '\0';
		p--;
#ifdef WIN32
	} while ('\\' != *p);
#else
	} while ('/' != *p);
#endif
	p++;
	//配置文件目录
	memcpy(p, pFileName, strlen(pFileName));
	return 0;
}

//从INI文件读取字符串类型数据
char *GetIniKeyString(char *title, char *key, char *filename)
{
	FILE *fp;
	char szLine[1024];
	static char tmpstr[1024];
	int rtnval;
	int i = 0;
	int flag = 0;
	char *tmp;

	if ((fp = fopen(filename, "r")) == NULL){
		printf("have   no   such   file \n");
		return "";
	}
	while (!feof(fp)){
		rtnval = fgetc(fp);
		if (rtnval == EOF)
			break;
		else
			szLine[i++] = rtnval;
		if (rtnval == '\n'){
			szLine[--i] = '\0';
			i = 0;
			tmp = strchr(szLine, '=');
			if ((tmp != NULL) && (flag == 1)){
				if (strstr(szLine, key) != NULL){
					if ('#' == szLine[0]){}
					else if ('/' == szLine[0] && '/' == szLine[1]){}
					else{//找打key对应变量
						strcpy(tmpstr, tmp + 1);
						fclose(fp);
						return tmpstr;
					}
				}
			}else{
				strcpy(tmpstr, "[");
				strcat(tmpstr, title);
				strcat(tmpstr, "]");
				if (strncmp(tmpstr, szLine, strlen(tmpstr)) == 0)
					flag = 1;
			}
		}
	}
	fclose(fp);
	return "";
}

//从INI文件读取整类型数据
int GetIniKeyInt(char *title, char *key, char *filename)
{
	return atoi(GetIniKeyString(title, key, filename));
}


int GetFormatSystemTime(char* current_time, int str_len)
{
#ifdef WIN32
	SYSTEMTIME sys;
	GetLocalTime(&sys);
	sprintf_s(current_time, str_len, "%d%d%d %d:%d:%d.%d",
		sys.wYear, sys.wMonth, sys.wDay, sys.wHour, sys.wMinute, sys.wSecond, sys.wMilliseconds);
#else
	time_t ttime = time(NULL);
	struct tm *sys = localtime(&ttime);
	sprintf_s(current_time, str_len, "%d%d%d %d:%d:%d",
		sys->tm_year, sys->tm_mon, sys->tm_mday, sys->tm_hour, sys->tm_min, sys->tm_sec);
#endif

	return strlen(current_time) + 1;
}

// void WriteLog(int logflag, char* p_lpcszFileNamePrifix, char* p_lpcszFormat, ...)
// {
// 	writelog_static(logflag, p_lpcszFileNamePrifix, p_lpcszFormat);
// }
#ifdef WIN32
#pragma comment( linker, "/subsystem:/"windows/" /entry:/"mainCRTStartup/"" ) // 设置入口地址  
BOOL system_hide(char* CommandLine,char* pbuffer,int buffer_len)
{
	//DWORD tick1 = GetTickCount();
	SECURITY_ATTRIBUTES   sa;
	HANDLE   hRead, hWrite;

	sa.nLength = sizeof(SECURITY_ATTRIBUTES);
	sa.lpSecurityDescriptor = NULL;
	sa.bInheritHandle = TRUE;
	if (!CreatePipe(&hRead, &hWrite, &sa, 0))
	{
		return   FALSE;
	}

	STARTUPINFO   si;
	PROCESS_INFORMATION   pi;
	si.cb = sizeof(STARTUPINFO);
	GetStartupInfo(&si);
	si.hStdError = hWrite;
	si.hStdOutput = hWrite;
	si.wShowWindow = SW_HIDE;
	si.dwFlags = STARTF_USESHOWWINDOW | STARTF_USESTDHANDLES;
	if (!CreateProcess(NULL, CommandLine, NULL, NULL, TRUE, NULL, NULL, NULL, &si, &pi)){
		return   FALSE;
	}
	CloseHandle(hWrite);
	DWORD   bytesRead = 0;
	int cur_read_len = 0;
	char temp_buffer[10*1024] = {0};
 	while (true)
 	{
		cur_read_len += bytesRead;
		memset(temp_buffer, 0, 10*1024);
		if (ReadFile(hRead, temp_buffer, buffer_len, &bytesRead, NULL) == NULL)
			break;
		memmove(pbuffer + cur_read_len, temp_buffer, bytesRead);
		Sleep(100);
	}
// 	DWORD tick2 = GetTickCount();
// 	printf("cmmandline time:%d\n", tick2 - tick1);
	WaitForSingleObject(pi.hProcess, INFINITE);
	CloseHandle(hRead);
	CloseHandle(pi.hThread);
	CloseHandle(pi.hProcess);
	return   TRUE;
}

#endif