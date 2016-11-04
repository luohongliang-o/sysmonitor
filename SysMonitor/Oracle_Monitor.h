#pragma once
#include "monitor_system.h"
#ifdef HAS_ORACLE

#include "ocilib.h"

class COracleMonitor :public CMonitorSystem
{
public:
	COracleMonitor();
	
	~COracleMonitor();
	static COracleMonitor* get_instance()
	{
		if (!_instance) _instance = new COracleMonitor;
		return _instance;
	}
	virtual int write(int fd, Value& json_value);
	virtual int get_object_type(){ return MONITORTYPE_ORACAL; }

	
private:
	DISALLOW_COPY_AND_ASSIGN(COracleMonitor);
	static COracleMonitor* _instance;
	OCI_Connection *m_connection;
	OCI_Statement  *m_statement;
	OCI_Resultset  *m_result_set;
};
#endif