#pragma once
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
#include <errno.h>
#include <map>
#endif
#ifdef HAS_NDB


static void* thread_func(void *pv);
#ifndef WIN32
#define PTHREAD_T pthread_t
#define PTHREAD_CREATE pthread_create
#define STAT stat
#define PTHREAD_SELF pthread_self
#define OPEN open
#define __O_RDONLY O_RDONLY
#define LSEEK  lseek
#define CLOSE  close

#define PTHREAD_JOIN pthread_join
#define SLEEP  sleep
#else
#define PTHREAD_T long long
#define PTHREAD_CREATE(a ,b, c,d) 1
#define STAT(file_name, buf)   1
#define PTHREAD_SELF() 1
#define OPEN(a,b) 1
#define __O_RDONLY
#define LSEEK
#define CLOSE

#define PTHREAD_JOIN
#define SLEEP Sleep
#endif // !WIN32
#define READ_LINE_NUM 500


enum
{
	LOG_NORMAL = 0, //Õý³£×´Ì¬
	LOG_WARNING,  // warning||alert
	LOG_ERROR     // error
};

typedef struct log_state_data{
	long file_size;
	long file_pos;
	short log_status;
	string log_err_msg;
	string file_name;
}LOG_STATE_DATA;

class CNdbMonitor : public CMonitorSystem
{
public:
	CNdbMonitor();
	~CNdbMonitor();

	virtual int write(int fd, Value& json_value);
	virtual int get_object_type(){ return MONITORTYPE_NDB; }
	static CNdbMonitor* get_instance()
	{
		if (!_instance) _instance = new CNdbMonitor;
		return _instance;
	}
	void compare_file_state();
	void scan_log(PTHREAD_T tid);
	char *strupr(char *str)
	{
		char *cp;       /* traverses string for C locale conversion */

		for (cp = str; *cp; ++cp)
		if (('a' <= *cp) && (*cp <= 'z'))
			*cp -= 'a' - 'A';

		return str;
	}

	void reset_log_status(PTHREAD_T tid=0);
public:
	CLock m_lock;
protected:
	void set_file_state(PTHREAD_T tid, LOG_STATE_DATA* plog_data);
	LOG_STATE_DATA* get_file_sate(PTHREAD_T tid);
protected:
	DISALLOW_COPY_AND_ASSIGN(CNdbMonitor);
	static CNdbMonitor* _instance;
	int m_log_file_num;
	typedef map<PTHREAD_T, LOG_STATE_DATA*> MAPLOG;
	MAPLOG m_map_log_state;



};

#endif