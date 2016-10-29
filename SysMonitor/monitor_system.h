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

class CWebMonitor : public CSysInfo
{
public:
	CWebMonitor(){ ; }
	CWebMonitor(CLoadConfig* loadconfig) : CSysInfo(loadconfig){ ; }
	~CWebMonitor(){ ; }

	virtual int write(int fd, Value& json_value);
protected:
	BOOL IsW3wpRun();

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

// cross platform

class CLinuxSysinfo :public CMonitorSystem
{
public:
	CLinuxSysinfo(){ ; };
	CLinuxSysinfo(CLoadConfig* loadconfig) :CMonitorSystem(loadconfig){ ; };
	~CLinuxSysinfo(){ ; };

	virtual int write(int fd, Value& json_value);
protected:
	
	void  get_loadavg(Value& json_value);         //cpu负载
	void  get_systemtime(Value& json_value);      //系统运行状态
	void  get_kernel_version(Value& json_value);  //系统版本
	void  get_os_name(Value& json_value);         //系统名称
	void  get_diskinfo(Value& json_value);        //磁盘信息
	void  get_meminfo(Value& json_value);         //内存与虚拟内存信息
	void  get_tcp_connections(Value& json_value);

	void  get_monitor_data_sec(Value& json_value);
	void  get_network_transfers(long& bytes);
	void  get_disk_io(int& io_num);
	void  get_cpu_time(int& all_time,int& idle_time);
};

#include "simple_mysql.h"
class CMySqlMonitor :public CMonitorSystem
{
public:
	CMySqlMonitor(){ };
	CMySqlMonitor(CLoadConfig* loadconfig) :CMonitorSystem(loadconfig){ m_mysql_connection = new CMysqlConnection; };
	~CMySqlMonitor(){ 
		TDEL(m_mysql_connection);
	};

	virtual int write(int fd, Value& json_value);
	
private:
	CMysqlConnection*  m_mysql_connection;

};



class COracleMonitor :public CMonitorSystem
{
public:
	COracleMonitor(){ };
	COracleMonitor(CLoadConfig* loadconfig) :CMonitorSystem(loadconfig){  };
	~COracleMonitor(){};

	virtual int write(int fd, Value& json_value);

private:
	

};

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