#include "sys_config.h"
#include "load_config.h"
#include "func.h"
#include "link_manager.h"
CLoadConfig::CLoadConfig()
{
	m_monitor_config = new MonitorConfig;
	memset(m_monitor_config, 0, sizeof(MonitorConfig));
	m_plink_manage = NULL;
}


CLoadConfig::~CLoadConfig()
{
	TDELARRAY(m_monitor_config->object_type);
	if (NULL != m_monitor_config->performance_name){
		for (int i = 0; i < m_monitor_config->performance_counter_num;i++)
			TDEL(m_monitor_config->performance_name[i]);;
		TDELARRAY(m_monitor_config->performance_name);
	}
	if (NULL != m_monitor_config->web_performance_name){
		for (int i = 0; i < m_monitor_config->web_performance_counter_num; i++)
			TDEL(m_monitor_config->web_performance_name[i]);;
		TDELARRAY(m_monitor_config->web_performance_name);
	}
	if (NULL != m_monitor_config->process_name){
		for (int i = 0; i < m_monitor_config->process_num; i++)
			TDEL(m_monitor_config->process_name[i]);
		TDELARRAY(m_monitor_config->process_name);
	}
	TDEL(m_monitor_config);
	TDEL(m_plink_manage);
}

void CLoadConfig::LoadConfig(CLoadConfig* this_ins)
{
	char filebuf[256] = "";
	GetCurrentPath(filebuf, "config.ini");
	MonitorConfig* config = this_ins->m_monitor_config;
	//service
	config->listen_port = GetIniKeyInt("service", "port", filebuf);
	strcpy(config->checkusername, GetIniKeyString("service", "checkusername", filebuf));
	//monitortype
	{
		config->object_num = GetIniKeyInt("monitortype", "num", filebuf);
		config->object_type = new short[config->object_num];
		for (int i = 0; i < config->object_num; i++){
			char objectkey[6] = "";
			sprintf_s(objectkey, sizeof(objectkey), "%s%d", "type", i + 1);
			config->object_type[i] = GetIniKeyInt("monitortype", objectkey, filebuf);
		}
	}
	{
		for (int i = 0; i < config->object_num;i++){
			if (config->object_type[i] == CONFIG_SYSTEM){
				config->performance_counter_num = GetIniKeyInt("system", "performance_counter_num", filebuf);
				config->performance_by_sec = GetIniKeyInt("system", "performance_by_sec", filebuf);
				config->performance_name = new string*[config->performance_counter_num];
				for (int i = 0; i < config->performance_counter_num; i++){
					char performance_key[20] = "";
					sprintf_s(performance_key, sizeof(performance_key), "%s%d", "performance_name", i + 1);
					config->performance_name[i] = new string(GetIniKeyString("system", performance_key, filebuf));
				}
			}
			else if (config->object_type[i] == CONFIG_WEB){
				config->web_performance_counter_num = GetIniKeyInt("web", "performance_counter_num", filebuf);
				config->web_performance_by_sec = GetIniKeyInt("web", "performance_by_sec", filebuf);
				config->web_performance_name = new string*[config->web_performance_counter_num];
				for (int i = 0; i < config->web_performance_counter_num; i++){
					char performance_key[20] = "";
					sprintf_s(performance_key, sizeof(performance_key), "%s%d", "performance_name", i + 1);
					config->web_performance_name[i] = new string(GetIniKeyString("web", performance_key, filebuf));
				}
			}
			else if (config->object_type[i] == CONFIG_PROCESS){
				config->process_num = GetIniKeyInt("process", "process_num", filebuf);
				config->process_name = new string*[config->process_num];
				for (int i = 0; i < config->process_num; i++){
					char process_key[20] = "";
					sprintf_s(process_key, sizeof(process_key), "%s%d", "process_name", i + 1);
					config->process_name[i] = new string(GetIniKeyString("process", process_key, filebuf));
				}
			}
			else if (config->object_type[i] == CONFIG_MSSQL){
				this_ins->m_plink_manage = new CLinkManager(this_ins);
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

char* CLoadConfig::get_check_user_name()
{
	if (m_monitor_config)
		return m_monitor_config->checkusername;
	return NULL;
}

short* CLoadConfig::get_object_type()
{
	if (m_monitor_config)
		return m_monitor_config->object_type;
	return NULL;
}

int CLoadConfig::get_object_num()
{
	if (m_monitor_config)
		return m_monitor_config->object_num;
	return 0;
}

int CLoadConfig::get_performance_counter_num()
{
	if (m_monitor_config)
		return m_monitor_config->performance_counter_num;
	return 0;
}
int CLoadConfig::get_performance_by_sec()
{
	if (m_monitor_config)
		return m_monitor_config->performance_by_sec;
	return 0;
}

string** CLoadConfig::get_performance_name()
{
	if (m_monitor_config)
		return m_monitor_config->performance_name;
	return NULL;
}

int CLoadConfig::get_web_performance_counter_num()
{
	if (m_monitor_config)
		return m_monitor_config->web_performance_counter_num;
	return 0;
}
int CLoadConfig::get_web_performance_by_sec()
{
	if (m_monitor_config)
		return m_monitor_config->web_performance_by_sec;
	return 0;
}

string** CLoadConfig::get_web_performance_name()
{
	if (m_monitor_config)
		return m_monitor_config->web_performance_name;
	return NULL;
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
string** CLoadConfig::get_process_name()
{
	if (m_monitor_config)
		return m_monitor_config->process_name;
	return NULL;
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

CLinkManager* CLoadConfig::get_link()
{
	return m_plink_manage;
}