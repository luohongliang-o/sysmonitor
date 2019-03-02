#ifndef SYS_CONFIG_H
#define SYS_CONFIG_H
#define LOGFILENAME "sysmonitor"
#ifdef WIN32
#define _WIN32_WINNT 0x0501 
//#include <targetver.h>
#include <afx.h>
#else

#if !defined(SSIZE_T)
typedef int SSIZE_T;
#endif

#endif

#include <limits.h>         // So we can set the bounds of our types
#include <stddef.h>         // For size_t


#include <map>
#include <vector>
#include <list>
using namespace std;
#include <stdio.h>
#include "port.h"

#if defined(NDEBUG)
#pragma comment(lib,"ws2_32.lib")
#pragma comment(lib,"wsock32.lib")
#pragma comment(lib,"event.lib")
#pragma comment(lib,"event_core.lib")
#pragma comment(lib,"event_extra.lib")
#pragma comment(lib,"libmysql.lib")
#pragma comment(lib,"ocilib.lib")
#endif
#if defined(_DEBUG)
#pragma comment(lib,"ws2_32.lib")
#pragma comment(lib,"wsock32.lib")
#pragma comment(lib,"event_d.lib")
#pragma comment(lib,"event_core_d.lib")
#pragma comment(lib,"event_extra_d.lib")
#pragma comment(lib,"libmysql.lib")
#pragma comment(lib,"ocilib_d.lib")
#endif

#if defined(_MSC_VER)

#define COMPILER_MSVC
#endif // MSC_VER

#ifdef COMPILER_MSVC
#define GG_LONGLONG(x) x##I64
#define GG_ULONGLONG(x) x##UI64
#else
#define GG_LONGLONG(x) x##LL
#define GG_ULONGLONG(x) x##ULL
#endif

#if !defined(ASSERT)
#include <assert.h>
# define ASSERT                         assert
#endif // ASSERT

#if !defined(TRACE) && !defined(HAS_TRACE)
# define HAS_TRACE                      1
# if !defined(DEBUG_ON)
#   define TRACE
#   define detect_memory_leaks(x)
# else
#   define TRACE                        PRINTF("\n");PRINTF
# endif // DEBUG_ON
#else
# define HAS_TRACE                      0
#endif // 

typedef unsigned int                    UINT32_T;
typedef char                            INT8_T;
typedef unsigned char                   UINT8_T;
typedef short                           INT16_T;
typedef unsigned short                  UINT16_T;
typedef int                             INT32_T;


#if defined(COMPILER_MSVC)
typedef __int64                         INT64_T;
typedef unsigned __int64                UINT64_T;
#else
typedef long long                       INT64_T;
typedef unsigned long long              UINT64_T;
#define _atoi64(val)     strtoll(val, NULL, 10)
#endif // COMPILER_MSVC

typedef float                           FLOAT_T;
typedef double                          DOUBLE_T;

typedef UINT32_T                        BOOL_T;


const UINT8_T  kuint8max  = ((UINT8_T)0xFF);
const UINT16_T kuint16max = ((UINT16_T)0xFFFF);
const UINT32_T kuint32max = ((UINT32_T)0xFFFFFFFF);
const UINT64_T kuint64max = ((UINT64_T)GG_LONGLONG(0xFFFFFFFFFFFFFFFF));
const  INT8_T  kint8min   = ((INT8_T)(-128)/*0x80*/);
const  INT8_T  kint8max   = ((INT8_T)0x7F);
const  INT16_T kint16min  = ((INT16_T)(-32768)/*0x8000*/);
const  INT16_T kint16max  = ((INT16_T)0x7FFF);
const  INT32_T kint32min  = ((INT32_T)0x80000000);
const  INT32_T kint32max  = ((INT32_T)0x7FFFFFFF);
const  INT64_T kint64min  = ((INT64_T)GG_LONGLONG(0x8000000000000000));
const  INT64_T kint64max  = ((INT64_T)GG_LONGLONG(0x7FFFFFFFFFFFFFFF));

//////////////////////////////////////////////////////////////////////////
//
#ifndef NULL
# ifdef __cplusplus
#   define NULL                         0
# else
#   define NULL                         ((void *)0)
# endif
#endif

#ifndef FALSE
# define FALSE                          0
#endif

#ifndef TRUE
# define TRUE                           1
#endif

#ifndef IN
# define IN
#endif

#ifndef OUT
# define OUT
#endif


//#ifndef __cplusplus
#ifndef max
# define max(a,b)    (((a) > (b)) ? (a) : (b))
#endif // max

#ifndef min
# define min(a,b)    (((a) < (b)) ? (a) : (b))
#endif // min
// #endif

//////////////////////////////////////////////////////////////////////////
// unused
#if !defined(UNUSED_PARAM)
#define UNUSED_PARAM(p)	                ((void)(p))	/* to avoid warnings */
#endif

#if !defined(UNUSED_LOCAL_VARIABLE)
#define UNUSED_LOCAL_VARIABLE(lv)	      UNUSED_PARAM(lv)
#endif

#ifndef TDEL
#define TDEL(a)				if(a!=NULL) { delete a; a=NULL; }
#endif

#ifndef TDELARRAY
#define TDELARRAY(a)		if(a!=NULL) { delete[] a; a=NULL; }
#endif

#ifndef CLOSEHANDLE
#define CLOSEHANDLE(a)		if(a!=NULL) { CloseHandle(a); a=NULL; }
#endif

#if !defined(HAS_STRING)
# define HAS_STRING                     1
# if !defined(UNICODE)
typedef char                            CHAR_T;
typedef unsigned char                   UCHAR_T;
#   define _STR(x)                      (x)
#   define _CHAR(x)                     (x)
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#if defined(WIN32)
#	define snprintf						_snprintf
#   define STRLWR						_strlwr_s
#   define VSNPRINTF                    vsnprintf_s
#   define SNPRINTF                     _snprintf_s
#   define STRCPY                       strcpy_s
#   define STRNCPY                      strncpy_s_
#   define STRLEN                       strlen
#   define STRNLEN                      strnlen
#   define STRCHR                       strchr_s
//#   define STRSTR                       strstr_s
//#   define STRSTR(s,l,f)                strstr(s, f)
#   define STRCAT                       strcat_s
#   define STRNCAT                      strncat_s
#   define STRCMP                       strcmp
#   define STRNCMP                      strncmp
#   define STRICMP                      stricmp
#   define STRNICMP                     strnicmp
#   define PRINTF                       printf_s
//#   define SSCANF                       sscanf_s
#   define SSCANF                       _snscanf_s
// 
#   define ATOI                         atoi
#   define ATOI64                       _atoi64
#   define ITOA                         _itoa_s
#   define I64TOA                       _ui64toa_s
#else
#   define STRLWR(s,l)					strlwr(s)
#   define VSNPRINTF                    vsnprintf
#   define SNPRINTF                     snprintf
#   define STRCPY                       strcpy
#   define STRNCPY                      strncpy
#   define STRLEN                       strlen
#   define STRNLEN                      strnlen
#   define STRCHR                       strchr
//#   define STRSTR                       strstr_s
//#   define STRSTR(s,l,f)                strstr(s, f)
#   define STRCAT                       strcat
#   define STRNCAT                      strncat
#   define STRCMP                       strcmp
#   define STRNCMP                      strncmp
#   define STRICMP                      stricmp
#   define STRNICMP                     strnicmp
#   define PRINTF                       printf
//#   define SSCANF                       sscanf_s
#   define SSCANF                       snscanf
// 
#   define ATOI                         atoi
#   define ATOI64                       _atoi64
#   define ITOA                         _itoa_s
#   define I64TOA                       _ui64toa_s

#endif
# else
# define HAS_STRING                     0
#   error "UNICODE string is not implemented!"
# endif // UNICODE
#endif // HAS_STRING

#if defined(__cplusplus)
#define DISALLOW_COPY_AND_ASSIGN(TypeName) \
	TypeName(const TypeName&);               \
	void operator=(const TypeName&)
#endif

#include "func.h"
#endif // SYS_CONFIG_H