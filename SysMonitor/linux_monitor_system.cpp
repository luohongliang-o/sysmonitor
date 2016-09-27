#ifndef WIN32
#include "monitor_system.h"
int 
CLinuxSysinfo::write(int fd, char *buf)
{

	return 0;
}

#endif