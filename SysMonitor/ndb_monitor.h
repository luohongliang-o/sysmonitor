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
#endif
#ifdef HAS_NDB
#ifndef WIN32
	static void* thread_func(void *pv);
#endif // !WIN32
#define READ_LINE_NUM 500
enum
{
	LOG_NORMAL = 0, //Õý³£×´Ì¬
	LOG_WARNING,  // warning||alert
	LOG_ERROR     // error
};

class CNdbMonitor : public CMonitorSystem
{
public:

	
	CNdbMonitor();
	~CNdbMonitor(){ 
#ifndef WIN32
		pthread_join(m_work_thread,NULL);
#endif // !WIN32
	};

	virtual int write(int fd, Value& json_value);
	virtual int get_object_type(){ return MONITORTYPE_NDB; }
	static CNdbMonitor* get_instance()
	{
		if (!_instance) _instance = new CNdbMonitor;
		return _instance;
	}

	void set_file_pos(long pos)
	{
		CAutoLock auto_lock(&m_lock);
		m_file_pos = pos;
	}
	void reset_log_status()
	{
		CAutoLock auto_lock(&m_lock);
		m_cur_err_msg = "";
		m_cur_log_status = LOG_NORMAL;
	}
	
	void set_log_msg(const char* msg)
	{
		CAutoLock auto_lock(&m_lock);
		m_cur_err_msg = msg;
	}
	void set_cur_log_status(int status)
	{ 
		CAutoLock auto_lock(&m_lock);
		m_cur_log_status = status; 
	}

	void compare_file_state();
	
	CLock m_lock;

	char *strupr(char *str)
	{
		char *cp;       /* traverses string for C locale conversion */

		for (cp = str; *cp; ++cp)
		if (('a' <= *cp) && (*cp <= 'z'))
			*cp -= 'a' - 'A';

		return str;
	}
	void scan_log();
protected:
	DISALLOW_COPY_AND_ASSIGN(CNdbMonitor);
	static CNdbMonitor* _instance;
	int m_cur_log_type;
	string m_cur_file_name;
	string m_cur_err_msg;
	int m_cur_log_status;
	long m_file_size;
	long m_file_pos;
	
#ifndef  WIN32
	pthread_t m_work_thread;
#endif // ! WIN32


};

#endif