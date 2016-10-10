#ifndef FUNC_H
#define FUNC_H
#include <stdio.h>
#include <stdarg.h>
#ifdef WIN32
#include <string>
#include <Windows.h>
#else
#define  MAX_PATH 260
#include <unistd.h>
#include <fcntl.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#define _vsnprintf vnsprintf
#define sprintf_s  snprintf
#endif


int GetCurrentPath(char buf[], char *pFileName);
char *GetIniKeyString(char *title, char *key, char *filename);
int GetIniKeyInt(char *title, char *key, char *filename);


static void WriteLog(char* p_lpcszFileNamePrifix, char* p_lpcszFormat, ...)
{
#ifdef _DEBUG
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
	__time64_t ttime = _time64(NULL);
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

	sprintf_s(szFileName, sizeof(szFileName), "%s%04d%02d%02d.log", szPrefix, stNow.wYear, stNow.wMonth, stNow.wDay);
	GetCurrentPath(path, szFileName);
	strcpy(szFileName, path);
#endif

	FILE	*pFile = fopen(szFileName, "ab");
	if (pFile != NULL)
	{
		DWORD	dwThreadID = GetCurrentThreadId();
		int	iRet = sprintf_s(szMessage, sizeof(szMessage)-2, "%04d-%02d-%02d/%02d:%02d:%02d.%03d[%08X]--%s",
			stNow.wYear, stNow.wMonth, stNow.wDay, stNow.wHour, stNow.wMinute, stNow.wSecond, stNow.wMilliseconds,
			dwThreadID, szValue);
		szMessage[iRet] = '\r';
		szMessage[iRet + 1] = '\n';
		szMessage[iRet + 2] = '\0';
		fwrite(szMessage, 1, strlen(szMessage), pFile);
		fclose(pFile);
		pFile = NULL;
	}

#endif // _DEBUG
}

#endif