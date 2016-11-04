#pragma once
#include "monitor_system.h"
#ifdef WIN32
#include "link_manager.h"
class CMsSqlMonitor :public CMonitorSystem
{
public:
	CMsSqlMonitor()
	{
		m_plink_manage = new CLinkManager();
		m_plink_manage->Init();
	};
	~CMsSqlMonitor(){ TDEL(m_plink_manage); };
	static CMsSqlMonitor* get_instance()
	{
		if (!_instance) _instance = new CMsSqlMonitor;
		return _instance;
	}
	virtual int write(int fd, Value& json_value);
	virtual int get_object_type(){ return MONITORTYPE_MSSQL; }
protected:
	int get_counter_value(int data_sel, vector< int >& vt_data);
private:
	CLinkManager* m_plink_manage;
	DISALLOW_COPY_AND_ASSIGN(CMsSqlMonitor);
	static CMsSqlMonitor* _instance;
};
#endif