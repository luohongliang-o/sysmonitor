#pragma once
#include "monitor_system.h"
#ifdef HAS_ORACLE
class COracleMonitor :public CMonitorSystem
{
public:
	COracleMonitor(){ };
	~COracleMonitor(){};

	virtual int write(int fd, Value& json_value);
	static COracleMonitor* get_instance()
	{
		if (!_instance) _instance = new COracleMonitor;
		return _instance;
	}
	virtual int get_object_type(){ return MONITORTYPE_ORACAL; }
private:
	DISALLOW_COPY_AND_ASSIGN(COracleMonitor);
	static COracleMonitor* _instance;

};
#endif