#pragma once
#include "monitor_system.h"
#ifdef HAS_ORACLE
class COracleMonitor :public CMonitorSystem
{
public:
	COracleMonitor(){ };
	~COracleMonitor(){};

	virtual int write(int fd, Value& json_value);

private:


};
#endif