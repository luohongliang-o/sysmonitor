#pragma once
#include "monitor_system.h"
class COracleMonitor :public CMonitorSystem
{
public:
	COracleMonitor(){ };
	COracleMonitor(CLoadConfig* loadconfig) :CMonitorSystem(loadconfig){  };
	~COracleMonitor(){};

	virtual int write(int fd, Value& json_value);

private:


};
