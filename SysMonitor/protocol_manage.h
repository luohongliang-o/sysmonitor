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
	
private:	

	bool  m_bCheck;
	CLoadConfig* m_load_config;
};

#endif