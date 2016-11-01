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

void WriteLog(int logflag, char* p_lpcszFileNamePrifix, char* p_lpcszFormat, ...);
#endif