#ifndef MONITOR_SYSTEM_H
#define MONITOR_SYSTEM_H
#include "../jsoncpp/include/json.h"
using namespace Json;

#include "load_config.h"
#ifdef WIN32
#include <process.h>
#else
#include <pthread.h>
#endif // WIN32


#if !defined(HAS_MYSQL)
#define HAS_MYSQL
#endif

#if !defined(HAS_NDB)
#define HAS_NDB
#endif

#if !defined(HAS_ORACLE)
//#define HAS_ORACLE
#endif
enum
{
	MONITORTYPE_SYSTEM_INFO = 1,
	MONITORTYPE_MYSQL,
	MONITORTYPE_MSSQL,
	MONITORTYPE_ORACAL,
	MONITORTYPE_WEB,
	MONITORTYPE_PROCESS,
	MONITORTYPE_LINUX_SYSINFO,
	MONITORTYPE_LINUX_PROCESS,
	MONITORTYPE_NDB
};
class CMonitorSystem
{
public:
	CMonitorSystem(){};
	virtual ~CMonitorSystem(){};
	virtual int write(int fd, Value& json_value) = 0;
	virtual int get_object_type() = 0; 

	void AddJsonKeyValue(char* str_data, Value& json_value)
	{
		string data = str_data;
		if (data.rfind('.') == data.length() - 1){
			data.replace(data.length() - 1, data.length(), "");
		}
		int point_pos = data.find('.');
		if (point_pos > 0 && (data.length() -1 - point_pos >2)){
			data = data.substr(0, point_pos + 3);
		}
		json_value.append(data.c_str());
	}
private:
	DISALLOW_COPY_AND_ASSIGN(CMonitorSystem);
};

#ifdef WIN32

class CSysInfo :public CMonitorSystem
{
public:
	CSysInfo(){ ; };
	~CSysInfo()
	{
		;
	};

	virtual int write(int fd, Value& json_value);
	virtual int get_object_type(){ return MONITORTYPE_SYSTEM_INFO; }
	static CSysInfo* get_instance()
	{
		if (!_instance) _instance = new CSysInfo;
		return _instance;
	}
protected:
	void WriteCounterVaule(int counter_num, char** list_counter, Value* json_value);
private:
	DISALLOW_COPY_AND_ASSIGN(CSysInfo);
	static CSysInfo* _instance;
};


class CProcessMonitor : public CMonitorSystem
{
public:
	CProcessMonitor(){ ; }
	~CProcessMonitor()
	{ 
		m_map_process_name_pid.clear(); 
	}
	virtual int write(int fd, Value& json_value);
	virtual int get_object_type(){ return MONITORTYPE_PROCESS; }
	static CProcessMonitor* get_instance()
	{
		if (!_instance) _instance = new CProcessMonitor;
		return _instance;
	}
protected:
	BOOL GetProcessList();
	void printError(TCHAR* msg);

	map< string, vector< int > > m_map_process_name_pid;
private:
	DISALLOW_COPY_AND_ASSIGN(CProcessMonitor);
	static CProcessMonitor* _instance;
};



#endif // WIN32


class CBuildMonitor
{
public:
	CBuildMonitor()
	{
		m_vector_system_monitor = vector<CMonitorSystem*>(NULL);
		int object_num = CLoadConfig::CreateInstance()->get_object_num();
		m_vector_system_monitor.resize(OBJECT_NUM);
	};

	void ConcreteMonitor(int object_index,int type);
	~CBuildMonitor();
	CMonitorSystem* get_monitor_obj(int object_index);

private:
	vector<CMonitorSystem*> m_vector_system_monitor;

};

class CLock
{
public:
	CLock()	{ 
#ifdef WIN32
		InitializeCriticalSection(&m_cs); 
#else
		pthread_mutex_init(&m_cs, NULL);
#endif
	}
	~CLock() { 
#ifdef DEBUG
		DeleteCriticalSection(&m_cs);
#else
		pthread_mutex_destroy(&m_cs);
#endif // DEBUG
	};
	void Lock() { 
#ifdef WIN32
		EnterCriticalSection(&m_cs); 
#else
		pthread_mutex_lock(&m_cs);
#endif
	};
	void Unlock() { 
#ifdef WIN32
		LeaveCriticalSection(&m_cs); 
#else
		pthread_mutex_unlock(&m_cs);
#endif;
	};
private:
#ifndef WIN32
	pthread_mutex_t m_cs;
#else
	CRITICAL_SECTION m_cs;
#endif
};

//////////////////////////////////////////////////////////////////////////
// ×Ô¶¯Ëø
class CAutoLock
{
public:
	CAutoLock(CLock* pLock){ m_pLock = pLock; pLock->Lock(); }
	~CAutoLock() { m_pLock->Unlock(); }
protected:
	CLock* m_pLock;
};

#endif