#pragma once
#include "monitor_system.h"
#ifdef WIN32
class CWebMonitor : public CSysInfo
{
public:
	CWebMonitor(){ ; }
	~CWebMonitor(){ ; }
	static CWebMonitor* get_instance()
	{
		if (!_instance) _instance = new CWebMonitor;
		return _instance;
	}
	virtual int write(int fd, Value& json_value);
	virtual int get_object_type(){ return MONITORTYPE_WEB; }
protected:
	bool IsW3wpRun();
private:
	DISALLOW_COPY_AND_ASSIGN(CWebMonitor);
	static CWebMonitor* _instance;

};

#else
class CWebMonitor : public CMonitorSystem
{
public:
	static CWebMonitor* get_instance()
	{
		if (!_instance) _instance = new CWebMonitor;
		return _instance;
	}

	CWebMonitor(){ ; }
	~CWebMonitor(){ ; }

	virtual int write(int fd, Value& json_value);
	virtual int get_object_type(){ return MONITORTYPE_WEB; }
private:
	DISALLOW_COPY_AND_ASSIGN(CWebMonitor);
	static CWebMonitor* _instance;

};
#endif