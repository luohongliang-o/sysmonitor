#ifndef LOAD_CONFIG_H
#define LOAD_CONFIG_H
class CLoadConfig
{
public:
	CLoadConfig();
	~CLoadConfig();

	struct MonitorConfig
	{
		//service
		short      listen_port;
		char       checkusername[32];
		//monitortype
		int        object_num;
		short*     object_type;
		//system
#ifdef WIN32
		int        performace_counter_num;
		string**   performace_name;
#endif
		//process
		int        process_num;
		string**   process_name;

	};

	static void LoadConfig(CLoadConfig* this_ins);

	int get_port();
	int get_object_num();
	short* get_object_type();
	char*  get_check_user_name();
#ifdef WIN32
	int get_performace_counter_num();
	string** get_performace_name();
#endif
	int get_process_num();
	string** get_process_name();
private:
	MonitorConfig*   m_monitor_config;

};

#endif