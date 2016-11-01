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

static void writelog_static(int logflag, char* p_lpcszFileNamePrifix, char* p_lpcszFormat, ...)
{
	//#ifdef _DEBUG
	if (logflag > 0){
		char	szValue[100 * 1024] = { 0 }, szMessage[100 * 1024] = { 0 }, szFileName[MAX_PATH] = { 0 };
		va_list args;
		va_start(args, p_lpcszFormat);
		int nReturnCode = _vsnprintf(szValue, sizeof(szValue), p_lpcszFormat, args);
		if ((nReturnCode < 0) || (nReturnCode == sizeof(szValue)))
		{
			szValue[sizeof(szValue)-1] = '\0';
			nReturnCode = sizeof(szValue)-1;
		}
		va_end(args);
#ifdef WIN32
		SYSTEMTIME	stNow;
		GetLocalTime(&stNow);

#else
		time_t ttime = time(NULL);
		struct tm *stNow = localtime(&ttime);

#endif
		char	szPrefix[32] = "DB";
		if (p_lpcszFileNamePrifix[0] != '\0')
		{
			sprintf_s(szPrefix, sizeof(szPrefix), "%s", p_lpcszFileNamePrifix);
		}
		char path[256] = "";
#ifdef WIN32
		GetModuleFileName(NULL, path, sizeof(path));
		long nLen = strlen(path);
		for (long loop = (nLen - 1); loop >= 0; loop--)
		{
			if (path[loop] == '\\')
			{
				path[loop] = 0;
				break;
			}
		}
		sprintf_s(szFileName, sizeof(szFileName), "%s\\%s%04d%02d%02d.log", path, szPrefix, stNow.wYear, stNow.wMonth, stNow.wDay);
#else

		sprintf_s(szFileName, sizeof(szFileName), "%s%04d%02d%02d.log", szPrefix, stNow->tm_year, stNow->tm_mon, stNow->tm_mday);
		GetCurrentPath(path, szFileName);
		strcpy(szFileName, path);
#endif

		FILE	*pFile = fopen(szFileName, "ab");
		if (pFile != NULL)
		{
#ifdef WIN32
			DWORD	dwThreadID = GetCurrentThreadId();
			int	iRet = sprintf_s(szMessage, sizeof(szMessage)-2, "%04d-%02d-%02d/%02d:%02d:%02d.%03d[%08X]--%s",
				stNow.wYear, stNow.wMonth, stNow.wDay, stNow.wHour, stNow.wMinute, stNow.wSecond, stNow.wMilliseconds,
				dwThreadID, szValue);
#else
			int	iRet = sprintf_s(szMessage, sizeof(szMessage)-2, "%04d-%02d-%02d/%02d:%02d:%02d[%u]--%s",
				stNow->tm_year, stNow->tm_mon, stNow->tm_mday, stNow->tm_hour, stNow->tm_min, stNow->tm_sec,
				(int)pthread_self(), szValue);
#endif
			szMessage[iRet] = '\r';
			szMessage[iRet + 1] = '\n';
			szMessage[iRet + 2] = '\0';
			fwrite(szMessage, 1, strlen(szMessage), pFile);
			fclose(pFile);
			pFile = NULL;
		}

	}
}

void WriteLog(int logflag, char* p_lpcszFileNamePrifix, char* p_lpcszFormat, ...)
{
	writelog_static(logflag, p_lpcszFileNamePrifix, p_lpcszFormat);
}
