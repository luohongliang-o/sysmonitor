#ifndef LOAD_CONFIG_H
#define LOAD_CONFIG_H

#include "sys_config.h"
#define MAX_LINK_NUM 30
#define MAX_DBCOUNT  5
#define  OBJECT_NUM 7
// 数据库配置
typedef struct tagDBCONFIG
{
	char	data_source[32];
	char	data_base[32];
	char	user_name[32];
	char	password[32];
}DBCONFIG, *LPDBCONFIG;


class CLoadConfig
{
public:
	~CLoadConfig();
	static CLoadConfig* CreateInstance();
	enum 
	{
		CONFIG_SYSTEM = 1,
		CONFIG_MYSQL,
		CONFIG_MSSQL,
		CONFIG_ORACAL,
		CONFIG_WEB,
		CONFIG_PROCESS,
		CONFIG_LINUX_SYSINFO,
		CONFIG_LINUX_PROCESS,
		CONFIG_NDB
	};
	enum 
	{
		NDB_MYSQL=1, //NDB mysql节点
		NDB_DATA,    //NDB 数据节点
		NDB_CLUSTER  //NDB 管理节点
	};
	struct MonitorConfig
	{
		//service
		short      listen_port;
		int        log_flag;
		//monitortype
		//int        object_num;
		vector< short >     object_type;
		//system
		int        counter_num;
		char**     counter_name;
		//mysql
		int        mysql_type;
		int        mysql_database_num;
		char**     mysql_connstr;
		//web
		int        web_counter_num;
		char**     web_counter_name;
		//mssql
		short      db_count;
		short      db_default_sel;
		DBCONFIG   db_config[MAX_DBCOUNT];

		//process
		int        process_num;
		char**     process_name;
		// ndblog
		char*     ndb_log_file_name;
		int      ndb_type;
	};

	void LoadConfig();

	int      get_port();
	int      get_object_num();
	vector< short >   get_object_type();
	int      get_log_flag();
	int      get_counter_num();
	char**   get_counter_name();
	
	int      get_web_counter_num();
	char**   get_web_counter_name();
	void     get_sys_os_info();

	char*    get_os_name();
	char*    get_os_version();

	int      get_process_num();
	char**   get_process_name();
 
	int      get_mysql_type();
	int      get_mysql_dbcount();
	const char* get_mysql_connection_string(int con_index);

	short    get_db_count();
	short    get_db_default_sel();
	LPDBCONFIG get_db_config();

	char*    get_ndb_log_file_name();
	int      get_ndb_type();

private:
	CLoadConfig();
	DISALLOW_COPY_AND_ASSIGN(CLoadConfig);
private:
	MonitorConfig*   m_monitor_config;
	char m_os_name[100];
	char m_os_version[100];
	static CLoadConfig* _instance;

};

#endif