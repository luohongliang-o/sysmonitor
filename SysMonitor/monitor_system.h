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
	virtual int write(int fd, Value& json_value) = 0;

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

	CLoadConfig* m_loadconfig;
};

#ifdef WIN32

class CSysInfo :public CMonitorSystem
{
public:
	CSysInfo(){ ; };
	CSysInfo(CLoadConfig* loadconfig) :CMonitorSystem(loadconfig)//,Thread(loadconfig)
	{ ; };

	~CSysInfo(){ ; };

	virtual int write(int fd, Value& json_value);
	
protected:
	
	void WriteCounterVaule(int counter_num, vector<string>* list_counter, Value* json_value);
};

#include <tlhelp32.h>
class CProcessMonitor : public CMonitorSystem
{
public:
	CProcessMonitor(){ ; }
	CProcessMonitor(CLoadConfig* loadconfig) :CMonitorSystem(loadconfig){ ; }
	
	~CProcessMonitor(){ ; }
	virtual int write(int fd, Value& json_value);
protected:
	BOOL GetProcessList();
	void printError(TCHAR* msg);

	map< string, vector< int > > m_map_process_name_pid;
};


#include "link_manager.h"

class CMsSqlMonitor :public CMonitorSystem
{
public:
	CMsSqlMonitor(){};
	CMsSqlMonitor(CLoadConfig* loadconfig) :CMonitorSystem(loadconfig)
	{
		m_plink_manage = new CLinkManager(loadconfig);
		m_plink_manage->Init();
	};
	~CMsSqlMonitor(){ TDEL(m_plink_manage); };

	virtual int write(int fd, Value& json_value);
protected:
	int get_counter_value(int data_sel,vector< int >& vt_data);
private:
	CLinkManager* m_plink_manage;
};
#endif // WIN32


class CBuildMonitor
{
public:
	CBuildMonitor()
	{
		m_system_monitor = NULL;
	};
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
	};

	void ConcreteMonitor(int type, CLoadConfig* loadconfig);
	~CBuildMonitor();
	CMonitorSystem* get_monitor_obj();
protected:
	BOOL is_object_exist(int type, CLoadConfig* loadconfig);
private:
	CMonitorSystem* m_system_monitor;

};
#endif