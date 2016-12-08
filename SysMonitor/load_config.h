#ifndef LOAD_CONFIG_H
#define LOAD_CONFIG_H

#include "sys_config.h"
#define MAX_LINK_NUM 30
#define MAX_DBCOUNT  5
#define  OBJECT_NUM 6
//  ˝æ›ø‚≈‰÷√
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
		CONFIG_LINUX_PROCESS
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
		char       mysql_connstr[256];
		//web
		int        web_counter_num;
		char**   web_counter_name;
		//mssql
		short      db_count;
		short      db_default_sel;
		DBCONFIG   db_config[MAX_DBCOUNT];

		//process
		int        process_num;
		char**   process_name;
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
 
	const char* get_mysql_connection_string();
	short    get_db_count();
	short    get_db_default_sel();
	LPDBCONFIG get_db_config();

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