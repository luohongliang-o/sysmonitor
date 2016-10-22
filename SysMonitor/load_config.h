#ifndef LOAD_CONFIG_H
#define LOAD_CONFIG_H
#ifdef WIN32
#include "ado2.h"
#endif // WIN32
#include "sys_config.h"
#ifdef WIN32

#define MAX_LINK_NUM 30
#define MAX_DBCOUNT  5
typedef struct tagOPLINK
{
	BOOL	is_busy;						// 是否使用中
	short	cur_sel;						// 选择哪个数据库
	BOOL	is_connected;					// 是否连接
	long	busy_time;					// 开始忙时间点
	CADODatabase* ado_db;
} OPLINK, *LPOPLINK;

#endif
// 数据库配置
typedef struct tagDBCONFIG
{
	CHAR	data_source[32];
	CHAR	data_base[32];
	CHAR	user_name[32];
	CHAR	password[32];
}DBCONFIG, *LPDBCONFIG;

class CLinkManager;
class CLoadConfig
{
public:
	CLoadConfig();
	CLoadConfig(const CLoadConfig& other);
	~CLoadConfig();

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
		char       checkusername[32];
		//monitortype
		int        object_num;
		vector< short >     object_type;
		//system
		int        counter_num;
		vector< string >   counter_name;
		int        counter_by_sec;
		//mysql
		char       mysql_connstr[256];
		//web
		int        web_counter_num;
		vector< string >   web_counter_name;
		int        web_counter_by_sec;
		//mssql
		short      db_count;
		short      db_default_sel;
		DBCONFIG   db_config[MAX_DBCOUNT];

		//process
		int        process_num;
		vector< string >   process_name;
	};

	static void LoadConfig(CLoadConfig* this_ins);

	int      get_port();
	int      get_object_num();
	vector< short >   get_object_type();
	char*    get_check_user_name();
	int      get_counter_by_sec();
	int      get_counter_num();
	vector< string > get_counter_name();
	
	int      get_web_counter_by_sec();
	int      get_web_counter_num();
	vector< string > get_web_counter_name();

	void     get_sys_os_info();

	char*    get_os_name();
	char*    get_os_version();

	int      get_process_num();
	vector< string > get_process_name();

	const char* get_mysql_connection_string();
	short    get_db_count();
	short    get_db_default_sel();
	LPDBCONFIG get_db_config();
	CLinkManager* get_link();
private:
	MonitorConfig*   m_monitor_config;
	char m_os_name[100];
	char m_os_version[100];
	CLinkManager *m_plink_manage;
};

#endif