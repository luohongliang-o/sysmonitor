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

	virtual int write(int fd, Value& json_value);
protected:
	int get_counter_value(int data_sel, vector< int >& vt_data);
private:
	CLinkManager* m_plink_manage;
};
#endif