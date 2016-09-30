#ifndef MONITOR_SYSTEM_H
#define MONITOR_SYSTEM_H

#include "load_config.h"


class CLock
{
public:
	CLock()	{ InitializeCriticalSection(&m_cs); }
	~CLock() { DeleteCriticalSection(&m_cs); };
	VOID Lock() { EnterCriticalSection(&m_cs); };
	VOID Unlock() { LeaveCriticalSection(&m_cs); };
private:
	CRITICAL_SECTION m_cs;
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

class CMonitorSystem
{
public:
	CMonitorSystem(){ ; };
	CMonitorSystem(CLoadConfig * loadconfig)
	{ 
		m_loadconfig = loadconfig; 
	};
	virtual ~CMonitorSystem()
	{
	};
	virtual int write(int fd,char* buf) = 0;

	void AddJsonKeyValue(char* str_key, char* str_data, Value& json_value)
	{
		string data = str_data;
		string key = str_key;
		if (data.rfind('.') == data.length() - 1){
			data.replace(data.length() - 1, data.length(), "");
		}
		json_value[key.c_str()] = data.c_str();
	}

	CLoadConfig* m_loadconfig;
};


#ifdef WIN32

class CSysInfo;
class CMySqlMonitor;

#endif // WIN32

class CBuildMonitor
{
public:
	CBuildMonitor(){};
	enum
	{
		MONITORTYPE_SYSTEM_INFO = 1,
		MONITORTYPE_TCP,
		MONITORTYPE_MYSQL,
		MONITORTYPE_MSSQL,
		MONITORTYPE_ORACAL,
		MONITORTYPE_WEB,
		MONITORTYPE_PROCESS,
		MONITORTYPE_LINUX_SYSINFO,
	};

	void ConcreteMonotor(int type, CLoadConfig* loadconfig);
	~CBuildMonitor();
	CMonitorSystem* get_monitor_obj();
private:
	CMonitorSystem* m_system_monitor;
	
};

#ifdef WIN32

class Thread
{
public:
	
	static UINT WINAPI ThreadProc(LPVOID pParam);
	~Thread();
protected:
	Thread();
	Thread(CLoadConfig* loadconfig);
	virtual int ThreadKernalFunc(WPARAM wparam = 0, LPARAM lparam = 0) { return 0; };
	CLock m_lock;
private:
	vector<HANDLE> m_vthreadid;
	string** m_performace_name;
	int      m_ncur_performace_index;
	
};


class CSysInfo :public CMonitorSystem//,public Thread
{
public:
	CSysInfo(){ ; };
	CSysInfo(CLoadConfig* loadconfig) :CMonitorSystem(loadconfig)//,Thread(loadconfig)
	{ ; };

	~CSysInfo(){ ; };

	virtual int write(int fd, char *buf);
	
protected:
	//virtual int ThreadKernalFunc(WPARAM wparam = 0, LPARAM lparam = 0);
	double WritePerformaceVaule(char* str_counter_path_buffer);
	//Value m_jsonvalue_performace;
};

#include <tlhelp32.h>
class CProcessMonitor : public CMonitorSystem
{
public:
	CProcessMonitor(){ ; }
	CProcessMonitor(CLoadConfig* loadconfig) :CMonitorSystem(loadconfig){ ; }
	
	~CProcessMonitor(){ ; }
	virtual int write(int fd, char* buf);
protected:
	BOOL GetProcessList();
	void printError(TCHAR* msg);
private:
	map< string, vector< int > > m_map_process_name_pid;
};
#endif // WIN32


// cross platform

class CLinuxSysinfo :public CMonitorSystem
{
public:
	CLinuxSysinfo(){ ; };
	CLinuxSysinfo(CLoadConfig* loadconfig) :CMonitorSystem(loadconfig){ ; };
	~CLinuxSysinfo(){ ; };

	virtual int write(int fd, char *buf);
protected:
	void  get_meminfo(Value& json_value);
};

class CMySqlMonitor :public CMonitorSystem
{
public:
	CMySqlMonitor(){ ; };
	CMySqlMonitor(CLoadConfig* loadconfig) :CMonitorSystem(loadconfig){ ; };
	~CMySqlMonitor(){ ; };

	virtual int write(int fd, char *buf);
	
private:

};
#endif