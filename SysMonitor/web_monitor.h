#pragma once
#include "monitor_system.h"
#ifdef WIN32
class CWebMonitor : public CSysInfo
{
public:
	CWebMonitor(){ ; }
	~CWebMonitor(){ ; }

	virtual int write(int fd, Value& json_value);
protected:
	bool IsW3wpRun();


};

#else
class CWebMonitor : public CMonitorSystem
{
public:
	CWebMonitor(){ ; }
	~CWebMonitor(){ ; }

	virtual int write(int fd, Value& json_value);
protected:
};
#endif