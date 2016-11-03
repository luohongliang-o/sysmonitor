#ifndef SYS_MONITOR_H
#define SYS_MONITOR_H

# if defined(__cplusplus)
#   define EXTERN_C                     extern "C"
# else
#   define EXTERN_C
# endif // __cplusplus


#if defined(WIN32)
#	if defined(MYDLLAPI_EXPORT)
#		define MYDLLAPI __declspec(dllexport)
#	else
#		define MYDLLAPI __declspec(dllimport)
#   endif

#else
#define MYDLLAPI
#endif
//extern "C" __declspec(dllexport) void start();
EXTERN_C MYDLLAPI void start();
//EXTERN_C MYDLLAPI void __stdcall stop();
#endif