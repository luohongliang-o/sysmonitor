#pragma once
#include "monitor_system.h"
#ifdef HAS_MYSQL
#include "simple_mysql.h"

class CMysqlMonitor :public CMonitorSystem
{
public:
	CMysqlMonitor()
	{
		m_mysql_connection = new CMysqlConnection;
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
};

#endif