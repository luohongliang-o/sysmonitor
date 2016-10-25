#ifndef PROTOCOL_MANAGE_H
#define PROTOCOL_MANAGE_H
#include "sys_config.h"
#include "monitor_system.h"
#include "load_config.h"

class CProtocolManage
{
public:
	CProtocolManage(CLoadConfig* loadconfg);
	~CProtocolManage();

	int read(int fd, char *buf);	
	int write(int fd);
protected:
	int get_last_buf(char* buf);
	void rset_list_buf(int listsize);

private:	

	bool  m_bCheck;
	CLoadConfig* m_load_config;
	list< string > m_list_buf;
	
	char m_os_name[100];
	char m_os_version[100];

	CBuildMonitor* m_build_monitor;
};

#endif