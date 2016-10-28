#include "sys_config.h"
#include "load_config.h"
#include "func.h"
#include "link_manager.h"
CLoadConfig::CLoadConfig()
{
	m_monitor_config = new MonitorConfig;
	memset(m_monitor_config, 0, sizeof(MonitorConfig));
}

CLoadConfig::~CLoadConfig()
{
	TDEL(m_monitor_config);

}

void CLoadConfig::LoadConfig(CLoadConfig* this_ins)
{
	char filebuf[256] = "";
	GetCurrentPath(filebuf, "config.ini");
	MonitorConfig* config = this_ins->m_monitor_config;
	//service
	config->listen_port = GetIniKeyInt("service", "port", filebuf);
	config->log_flag = GetIniKeyInt("service", "logflag", filebuf);
	//monitortype
	{
		config->object_num = GetIniKeyInt("monitortype", "num", filebuf);
		config->object_type.resize(config->object_num);
		for (int i = 0; i < config->object_num; i++){
			char objectkey[6] = "";
			sprintf_s(objectkey, sizeof(objectkey), "%s%d", "type", i + 1);
			config->object_type[i] = GetIniKeyInt("monitortype", objectkey, filebuf);
		}
	}
	{
		for (int i = 0; i < config->object_num;i++){
			if (config->object_type[i] == CONFIG_SYSTEM){
				config->counter_num = GetIniKeyInt("system", "counter_num", filebuf);
				config->counter_by_sec = GetIniKeyInt("system", "counter_by_sec", filebuf);
				config->counter_name.resize(config->counter_num);
				for (int i = 0; i < config->counter_num; i++){
					char counter_key[20] = "";
					sprintf_s(counter_key, sizeof(counter_key), "%s%d", "counter_name", i + 1);
					config->counter_name[i] = GetIniKeyString("system", counter_key, filebuf);
				}
			}
			else if (config->object_type[i] == CONFIG_MYSQL){
				strcpy(config->mysql_connstr, GetIniKeyString("mysql", "connectonstr", filebuf));
			}
			else if (config->object_type[i] == CONFIG_WEB){
				config->web_counter_num = GetIniKeyInt("web", "counter_num", filebuf);
				config->web_counter_by_sec = GetIniKeyInt("web", "counter_by_sec", filebuf);
				config->web_counter_name.resize(config->web_counter_num);
				for (int i = 0; i < config->web_counter_num; i++){
					char counter_key[20] = "";
					sprintf_s(counter_key, sizeof(counter_key), "%s%d", "counter_name", i + 1);
					config->web_counter_name[i] = GetIniKeyString("web", counter_key, filebuf);
				}
			}
			else if (config->object_type[i] == CONFIG_PROCESS){
				config->process_num = GetIniKeyInt("process", "process_num", filebuf);
				config->process_name.resize(config->process_num);
				for (int i = 0; i < config->process_num; i++){
					char process_key[20] = "";
					sprintf_s(process_key, sizeof(process_key), "%s%d", "process_name", i + 1);
					config->process_name[i]=GetIniKeyString("process", process_key, filebuf);
				}
			}
			else if (config->object_type[i] == CONFIG_MSSQL){
				config->db_count = GetIniKeyInt("mssql", "dbcount", filebuf);
				config->db_default_sel = GetIniKeyInt("mssql", "dbsel", filebuf);
				for (int i = 0; i < config->db_count;i++){
					char db_source_key[20] = "";
					char db_key[20] = "";
					char db_user_key[20] = "";
					char db_password_key[20] = "";
					sprintf_s(db_source_key, sizeof(db_source_key), "%s%d", "data_source", i + 1);
					sprintf_s(db_key, sizeof(db_key), "%s%d", "data_base", i + 1);
					sprintf_s(db_user_key, sizeof(db_user_key), "%s%d", "user_name", i + 1);
					sprintf_s(db_password_key, sizeof(db_password_key), "%s%d", "password", i + 1);
					strcpy(config->db_config[i].data_source, GetIniKeyString("mssql", db_source_key, filebuf));
					strcpy(config->db_config[i].data_base, GetIniKeyString("mssql", db_key, filebuf));
					strcpy(config->db_config[i].user_name, GetIniKeyString("mssql", db_user_key, filebuf));
					strcpy(config->db_config[i].password, GetIniKeyString("mssql", db_password_key, filebuf));
				}
/*
				this_ins->m_plink_manage = new CLinkManager(this_ins);
				this_ins->m_plink_manage->Init();
*/
			}
		}
	}

}

int CLoadConfig::get_port()
{
	if (m_monitor_config)
		return m_monitor_config->listen_port;
	return 0;
}

int CLoadConfig::get_log_flag()
{
	if (m_monitor_config)
		return m_monitor_config->log_flag;
	return 0;
}

vector< short > CLoadConfig::get_object_type()
{
	if (m_monitor_config)
		return m_monitor_config->object_type;
	return vector< short >(0);
}

int CLoadConfig::get_object_num()
{
	if (m_monitor_config)
		return m_monitor_config->object_num;
	return 0;
}

int CLoadConfig::get_counter_num()
{
	if (m_monitor_config)
		return m_monitor_config->counter_num;
	return 0;
}
int CLoadConfig::get_counter_by_sec()
{
	if (m_monitor_config)
		return m_monitor_config->counter_by_sec;
	return 0;
}

vector< string > CLoadConfig::get_counter_name()
{
	if (m_monitor_config)
		return m_monitor_config->counter_name;
	return vector< string >(NULL);
}

int CLoadConfig::get_web_counter_num()
{
	if (m_monitor_config)
		return m_monitor_config->web_counter_num;
	return 0;
}
int CLoadConfig::get_web_counter_by_sec()
{
	if (m_monitor_config)
		return m_monitor_config->web_counter_by_sec;
	return 0;
}

vector< string > CLoadConfig::get_web_counter_name()
{
	if (m_monitor_config)
		return m_monitor_config->web_counter_name;
	return vector< string >(NULL);
}


void CLoadConfig::get_sys_os_info()
{	
	FILE *ppipe = NULL;
	char* pbuffer = new char[1000];
	int nread_line = 0;
	int len = 0;
	ppipe = _popen("systeminfo /FO CSV /NH ", "rt");
	fgets(pbuffer, 1000, ppipe);
	char* tembufer = pbuffer;
	while (nread_line < 3){
		char* tempstr = strchr(tembufer, ',');
		len = tempstr - tembufer;
		char* tempvalue = new char[len + 1];
		strncpy(tempvalue, tembufer, len);
		tempvalue[len] = '\0';
		if (nread_line == 1)
			strcpy_s(m_os_name, tempvalue);
		else if (nread_line == 2)
			strcpy_s(m_os_version, tempvalue);
		TDELARRAY(tempvalue);
		tembufer = tembufer + len + 1;
		nread_line++;
	}
	TDELARRAY(pbuffer);
	if (feof(ppipe))
		_pclose(ppipe);
}


char* CLoadConfig::get_os_name()
{
	return m_os_name;
}

char* CLoadConfig::get_os_version()
{
	return m_os_version;
}
int CLoadConfig::get_process_num()
{
	if (m_monitor_config)
		return m_monitor_config->process_num;
	return 0;
}
vector< string > CLoadConfig::get_process_name()
{
	if (m_monitor_config)
		return m_monitor_config->process_name;
	return vector< string >(NULL);
}
short CLoadConfig::get_db_count()
{
	if (m_monitor_config)
		return m_monitor_config->db_count;
	return 0;
}
short CLoadConfig::get_db_default_sel()
{
	if (m_monitor_config)
		return m_monitor_config->db_default_sel;
	return 0;
}
LPDBCONFIG CLoadConfig::get_db_config()
{
	if (m_monitor_config)
		return m_monitor_config->db_config;
	return NULL;
}

// CLinkManager* CLoadConfig::get_link()
// {
// 	return m_plink_manage;
// }

const char* CLoadConfig::get_mysql_connection_string()
{
	if (m_monitor_config)
		return m_monitor_config->mysql_connstr;
	return NULL;
}