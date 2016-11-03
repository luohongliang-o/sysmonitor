#include "monitor_system.h"
class CLinuxSysinfo :public CMonitorSystem
{
public:
	CLinuxSysinfo(){ ; };
	~CLinuxSysinfo(){ ; };

	virtual int write(int fd, Value& json_value);
	virtual int get_object_type(){ return MONITORTYPE_LINUX_SYSINFO; }
protected:

	void  get_loadavg(Value& json_value);         //cpu负载
	void  get_systemtime(Value& json_value);      //系统运行状态
	void  get_kernel_version(Value& json_value);  //系统版本
	void  get_os_name(Value& json_value);         //系统名称
	void  get_diskinfo(Value& json_value);        //磁盘信息
	void  get_meminfo(Value& json_value);         //内存与虚拟内存信息
	void  get_tcp_connections(Value& json_value);

	void  get_monitor_data_sec(Value& json_value);
	void  get_network_transfers(long& bytes);
	void  get_disk_io(int& io_num);
	void  get_cpu_time(int& all_time, int& idle_time);
};