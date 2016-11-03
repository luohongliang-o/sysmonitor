#ifndef MONITOR_SYSTEM_H
#define MONITOR_SYSTEM_H
#include "load_config.h"
#include "json.h"
using namespace Json;

#if !defined(HAS_MYSQL)
#define HAS_MYSQL
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
};
class CMonitorSystem
{
public:
	CMonitorSystem(){ ; };
	virtual ~CMonitorSystem()
	{
	};
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
};

#ifdef WIN32

class CSysInfo :public CMonitorSystem
{
public:
	CSysInfo(){ ; };
	~CSysInfo(){ ; };

	virtual int write(int fd, Value& json_value);
	virtual int get_object_type(){ return MONITORTYPE_SYSTEM_INFO; }
protected:
	
	void WriteCounterVaule(int counter_num, vector<string>* list_counter, Value* json_value);
};


class CProcessMonitor : public CMonitorSystem
{
public:
	CProcessMonitor(){ ; }
	~CProcessMonitor(){ m_map_process_name_pid.clear(); }
	virtual int write(int fd, Value& json_value);
	virtual int get_object_type(){ return MONITORTYPE_PROCESS; }
protected:
	BOOL GetProcessList();
	void printError(TCHAR* msg);

	map< string, vector< int > > m_map_process_name_pid;
};



#endif // WIN32


class CBuildMonitor
{
public:
	CBuildMonitor()
	{
		m_system_monitor = NULL;
	};

	void ConcreteMonitor(int type);
	~CBuildMonitor();
	CMonitorSystem* get_monitor_obj();

private:
	CMonitorSystem* m_system_monitor;

};
#endif