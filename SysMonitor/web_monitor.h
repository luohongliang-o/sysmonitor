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
	BOOL IsW3wpRun();


};
#endif