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

	virtual int write(int fd, Value& json_value);
	virtual int get_object_type(){ return MONITORTYPE_MYSQL; }
private:
	CMysqlConnection*  m_mysql_connection;

};

#endif