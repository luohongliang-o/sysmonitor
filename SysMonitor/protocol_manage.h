#ifndef PROTOCOL_MANAGE_H
#define PROTOCOL_MANAGE_H
#include "monitor_system.h"

class CProtocolManage
{
public:
	CProtocolManage();
	~CProtocolManage();

	int read(int fd, char *buf);	
	int write(int fd = 0);
protected:
	int get_last_buf(char* buf);
	void rset_list_buf(int listsize);

private:	

	bool  m_bCheck;
	list< string > m_list_buf;
	
	char m_os_name[100];
	char m_os_version[100];
	int  m_log_flag;
	CBuildMonitor* m_build_monitor;
};

#endif