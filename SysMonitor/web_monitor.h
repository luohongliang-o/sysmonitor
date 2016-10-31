#pragma once
#include "mysql_monitor.h"
class CWebMonitor : public CSysInfo
{
public:
	CWebMonitor(){ ; }
	CWebMonitor(CLoadConfig* loadconfig) : CSysInfo(loadconfig){ ; }
	~CWebMonitor(){ ; }

	virtual int write(int fd, Value& json_value);
protected:
	BOOL IsW3wpRun();

};