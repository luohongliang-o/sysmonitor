#pragma once
#include "monitor_system.h"
#include "simple_mysql.h"

class CMysqlMonitor :public CMonitorSystem
{
public:
	CMysqlMonitor(){ };
	CMysqlMonitor(CLoadConfig* loadconfig) :CMonitorSystem(loadconfig){ m_mysql_connection = new CMysqlConnection; };
	~CMysqlMonitor(){
		TDEL(m_mysql_connection);
	};

	virtual int write(int fd, Value& json_value);

private:
	CMysqlConnection*  m_mysql_connection;

};