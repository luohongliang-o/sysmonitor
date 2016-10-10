#include "sys_config.h"
#include "load_config.h"


CLoadConfig::CLoadConfig()
{
	m_monitor_config = new MonitorConfig;
	memset(m_monitor_config, 0, sizeof(MonitorConfig));
}


CLoadConfig::~CLoadConfig()
{
	
	TDELARRAY(m_monitor_config->object_type);

#ifdef WIN32
	if (NULL != m_monitor_config->performace_name){
		for (int i = 0; i < m_monitor_config->performace_counter_num;i++)
			TDEL(m_monitor_config->performace_name[i]);;
		TDELARRAY(m_monitor_config->performace_name);
	}
#endif
	if (NULL != m_monitor_config->process_name){
		for (int i = 0; i < m_monitor_config->process_num; i++)
			TDEL(m_monitor_config->process_name[i]);
		TDELARRAY(m_monitor_config->process_name);
	}
	TDEL(m_monitor_config);
}

void CLoadConfig::LoadConfig(CLoadConfig* this_ins)
{
	char filebuf[256] = "";
	GetCurrentPath(filebuf, "config.ini");
	MonitorConfig* config = this_ins->m_monitor_config;
	//service
	{
		config->listen_port = GetIniKeyInt("service", "port", filebuf);
		strcpy(config->checkusername, GetIniKeyString("service", "checkusername", filebuf));
	}
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
	//system
#ifdef WIN32
	{
		config->performace_counter_num = GetIniKeyInt("system", "performacecounternum", filebuf);
		config->performace_name = new string*[config->performace_counter_num];
		for (int i = 0; i < config->performace_counter_num; i++){
			char performace_key[16] = "";
			sprintf_s(performace_key, sizeof(performace_key), "%s%d", "performacename", i + 1);
			config->performace_name[i] = new string(GetIniKeyString("system", performace_key, filebuf));
		}
	}
#endif
	//process
	{
		config->process_num = GetIniKeyInt("process", "processnum", filebuf);
		config->process_name = new string*[config->process_num];
		for (int i = 0; i < config->process_num; i++){
			char process_key[15] = "";
			sprintf_s(process_key, sizeof(process_key), "%s%d", "processname", i + 1);
			config->process_name[i] = new string(GetIniKeyString("process", process_key, filebuf));

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
#ifdef WIN32
int CLoadConfig::get_performace_counter_num()
{
	if (m_monitor_config)
		return m_monitor_config->performace_counter_num;
	return 0;
}

string** CLoadConfig::get_performace_name()
{
	if (m_monitor_config)
		return m_monitor_config->performace_name;
	return NULL;
}
#endif
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
