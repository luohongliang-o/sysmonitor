#ifndef MONITOR_SYSTEM_H
#define MONITOR_SYSTEM_H

#include "load_config.h"
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
	enum
	{
		MONITORTYPE_SYSTEM_INFO = 1,
		MONITORTYPE_TCP,
		MONITORTYPE_MYSQL,
		MONITORTYPE_MSSQL,
		MONITORTYPE_ORACAL,
		MONITORTYPE_WEB,
		MONITORTYPE_PROCESS
	};



	void ConcreteMonotor(int type, CLoadConfig* loadconfig);
	~CBuildMonitor();
	CMonitorSystem* get_monitor_obj();
private:
	CMonitorSystem* m_system_monitor;
	
};

#ifdef WIN32
class CSysInfo :public CMonitorSystem
{
public:
	CSysInfo(){ ; };
	CSysInfo(CLoadConfig* loadconfig) :CMonitorSystem(loadconfig){ ; };

	~CSysInfo(){ ; };

	virtual int write(int fd, char *buf);
protected:
	
	double WritePerformaceVaule(char* str_counter_path_buffer);
private:

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