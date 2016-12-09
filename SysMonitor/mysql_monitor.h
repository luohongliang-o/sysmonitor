#pragma once
#include "monitor_system.h"
#ifdef HAS_MYSQL
#include "simple_mysql.h"

class CMysqlMonitor :public CMonitorSystem
{
public:
	CMysqlMonitor()
	{
		m_mysql_default_proc = "call proc_general_monitor()";
		m_mysql_connection = new CMysqlConnection;
		m_mysql_connection->connect(CLoadConfig::CreateInstance()->get_mysql_connection_string());
	};
	~CMysqlMonitor(){
		TDEL(m_mysql_connection);
	};
	static CMysqlMonitor* get_instance()
	{
		if (!_instance) _instance = new CMysqlMonitor;
		return _instance;
	}

	virtual int write(int fd, Value& json_value);
	virtual int get_object_type(){ return MONITORTYPE_MYSQL; }
private:
	CMysqlConnection*  m_mysql_connection;
	DISALLOW_COPY_AND_ASSIGN(CMysqlMonitor);
	static CMysqlMonitor* _instance;
	const char* m_mysql_default_proc;

};

#endif