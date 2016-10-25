#ifndef MONITOR_SYSTEM_H
#define MONITOR_SYSTEM_H

#include "load_config.h"

#ifdef WIN32
class CSysInfo;
class CMsSqlMonitor;
class CProcessMonitor;

#endif // WIN32
class CMySqlMonitor;
class CLinuxSysinfo;
class CWebMonitor;
class COracleMonitor;

class CMonitorSystem
{
public:
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

	static CMonitorSystem* CreateInstance(int type);
	void set_loadconfig(CLoadConfig * loadconfig)
	{
		m_loadconfig = loadconfig;
	};
	
	virtual ~CMonitorSystem(){ ; };
	virtual int write(int fd, Value& json_value){ return 0; };
	virtual int get_object_type(){ return 0; };

	void AddJsonKeyValue(char* str_data, Value& json_value)
	{
		string data = str_data;
		if (data.rfind('.') == data.length() - 1){
			data.replace(data.length() - 1, data.length(), "");
		}
		json_value.append(data.c_str());
	}

	CLoadConfig* m_loadconfig;
//protected:
	CMonitorSystem(){ ; };
private:
	static CMonitorSystem* _instance;
};

#ifdef WIN32

class CSysInfo :public CMonitorSystem
{
public:
	CSysInfo(){ ; };
	~CSysInfo(){ ; };
	virtual int write(int fd, Value& json_value);
	virtual int get_object_type(){ return MONITORTYPE_SYSTEM_INFO; };
protected:
	double WriteCounterVaule(int index, int counter_by_sec, char* str_counter_path_buffer);
	
};

#include <tlhelp32.h>
class CProcessMonitor : public CMonitorSystem
{
public:
	CProcessMonitor(){ ; };
	~CProcessMonitor(){ ; };

	virtual int write(int fd, Value& json_value);
	virtual int get_object_type(){ return MONITORTYPE_PROCESS; };

protected:
	BOOL GetProcessList();
	void printError(TCHAR* msg);
	map< string, vector< int > > m_map_process_name_pid;

};

class CWebMonitor : public CSysInfo
{
public:
	CWebMonitor(){ ; };
	~CWebMonitor(){ ; }

	virtual int write(int fd, Value& json_value);
	virtual int get_object_type(){ return MONITORTYPE_WEB; };
protected:
	BOOL IsW3wpRun();
};

#include "link_manager.h"

class CMsSqlMonitor :public CMonitorSystem
{
public:
	CMsSqlMonitor(){ ; };
	~CMsSqlMonitor(){ ; };

	virtual int write(int fd, Value& json_value);
	virtual int get_object_type(){ return MONITORTYPE_MSSQL; };
protected:
	int get_counter_value(int data_sel,vector< int >& vt_data);
};
#endif // WIN32

// cross platform

class CLinuxSysinfo :public CMonitorSystem
{
public:
	CLinuxSysinfo(){ ; };
	~CLinuxSysinfo(){ ; };
	virtual int write(int fd, Value& json_value);
	virtual int get_object_type(){ return MONITORTYPE_LINUX_SYSINFO; };

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
	CMySqlMonitor(){ m_mysql_connection = new CMysqlConnection; };
	~CMySqlMonitor()
	{ 
		TDEL(m_mysql_connection);
	};
	
	virtual int write(int fd, Value& json_value);
	virtual int get_object_type(){ return MONITORTYPE_MYSQL; };
private:
	CMysqlConnection*  m_mysql_connection;

};


class COracleMonitor :public CMonitorSystem
{
public:
	COracleMonitor(){ };
	~COracleMonitor(){};

	virtual int write(int fd, Value& json_value);
	virtual int get_object_type(){ return MONITORTYPE_ORACAL; };
};

class CBuildMonitor
{
public:
	CBuildMonitor()
	{
		m_vector_monitor = vector< CMonitorSystem* >(NULL);
	};
	void ConcreteMonitor(CLoadConfig* loadconfig);
	~CBuildMonitor();

	int write_all(int fd,Value& json_value);
private:
	vector< CMonitorSystem* > m_vector_monitor;
};

#endif

