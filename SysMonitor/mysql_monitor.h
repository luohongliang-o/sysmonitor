#pragma once
#include "monitor_system.h"
#ifdef HAS_MYSQL
#include "simple_mysql.h"

class CMysqlMonitor :public CMonitorSystem
{
public:
	enum 
	{
		MYSQL_GENERAL,
		MYSQL_MASTER_SLAVE,
		MYSQL_NDB
	};
	CMysqlMonitor()
	{
		m_mysql_default_proc = "call proc_general_monitor()";
		int mysql_connection_num = CLoadConfig::CreateInstance()->get_mysql_dbcount();
		for (int i = 0; i < mysql_connection_num; i++){
			CMysqlConnection* mysql_connection = new CMysqlConnection;
			mysql_connection->connect(CLoadConfig::CreateInstance()->get_mysql_connection_string(i));
			UINT32_T error_num = 0;;
			const CHAR_T* error_msg = mysql_connection->get_last_error(&error_num);
			if (error_num)
				printf("error number:%d  error msg:%s\n", error_num, error_msg);
			else
				m_mysql_list_connection.push_back(mysql_connection);
		}
	};
	~CMysqlMonitor(){
		int mysql_connection_num = CLoadConfig::CreateInstance()->get_mysql_dbcount();
		for (int i = 0; i < mysql_connection_num; i++)
			TDEL(m_mysql_list_connection[i]);
	};
	static CMysqlMonitor* get_instance()
	{
		if (!_instance) _instance = new CMysqlMonitor;
		return _instance;
	}

	virtual int write(int fd, Value& json_value);
	virtual int get_object_type(){ return MONITORTYPE_MYSQL; }
protected:
	void get_master_slave_data(CMysqlConnection* pconn, Value& json_data);
#ifndef WIN32
	void get_ndb_show_state(char* line_str,Value& json_data);
#endif
private:
	vector<CMysqlConnection*> m_mysql_list_connection;
	DISALLOW_COPY_AND_ASSIGN(CMysqlMonitor);
	static CMysqlMonitor* _instance;
	const char* m_mysql_default_proc;
};

#endif