#ifndef FUNC_H
#define FUNC_H
#include "sys_config.h"
//#include <stdio.h>
#include <stdarg.h>
//#include <stdlib.h>
#ifdef WIN32
#include <Windows.h>
#else
#define  MAX_PATH 260
#include <pthread.h>
#include <time.h>
#include <unistd.h>
#include <fcntl.h>
//#include <stdarg.h>

#define _vsnprintf vsnprintf
#define sprintf_s  snprintf
#endif


int GetCurrentPath(char buf[], char *pFileName);
char *GetIniKeyString(char *title, char *key, char *filename);
int GetIniKeyInt(char *title, char *key, char *filename);
int GetFormatSystemTime(char* current_time,int str_len);
static void WriteLog(int logflag, char* p_lpcszFileNamePrifix, char* p_lpcszFormat, ...)
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
		char folder_path[256] = "";
		sprintf_s(folder_path,"%s\\log",path);
		CreateDirectory(folder_path,NULL);
		sprintf_s(szFileName, sizeof(szFileName), "%s\\log\\%s%04d%02d%02d.log", path, szPrefix, stNow.wYear, stNow.wMonth, stNow.wDay);
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

//void WriteLog(int logflag, char* p_lpcszFileNamePrifix, char* p_lpcszFormat, ...);
#endif