#include "func.h"
#include "monitor_system.h"
#include "mysql_monitor.h"
#include "Oracle_Monitor.h"
#include "web_monitor.h"
#include "linux_monitor_system.h"
#include "mssql_monitor.h"
#ifdef WIN32
#include <pdh.h>
#include <pdhmsg.h>
#include <process.h>
#pragma comment(lib, "pdh.lib")
#include <tlhelp32.h>
#include <stdio.h>
#include <fstream>
//////////////////////////////////////////////////////////////////////////
/*CSysInfo*/
//////////////////////////////////////////////////////////////////////////
CSysInfo* CSysInfo::_instance = NULL;
int
CSysInfo::write(int fd, Value& json_value)
{
	char json_data[50] = "";
	char counter_key[16] = "";
	
	//disk
	{
		DWORD diskcount = 0;
		DWORD diskinfo = GetLogicalDrives();
		while (diskinfo){
			if (diskinfo & 1)
				diskcount++;
			diskinfo = diskinfo >> 1;
		}
		int dslength = GetLogicalDriveStrings(0, NULL);
		CHAR* DStr = new CHAR[dslength];
		memset(DStr, 0, dslength);
		GetLogicalDriveStrings(dslength, (LPTSTR)DStr);
		int dtype = 0;
		BOOL fResult = FALSE;
		unsigned __int64 i64freebytestocaller = 0;
		unsigned __int64 i64totalbytes = 0;
		unsigned __int64 i64freebytes = 0;
		Value disk_data;
		for (int i = 0; i < dslength / 4; ++i)
		{
			string strdriver = DStr + i * 4;
			dtype = GetDriveType(strdriver.c_str());
			if (dtype == DRIVE_FIXED){
				fResult = GetDiskFreeSpaceEx(strdriver.c_str(),
					(PULARGE_INTEGER)&i64freebytestocaller,
					(PULARGE_INTEGER)&i64totalbytes,
					(PULARGE_INTEGER)&i64freebytes);
				if (fResult){
					Value disk_item;
					_gcvt(i64totalbytes / 1024, 31, json_data);
					AddJsonKeyValue((char*)strdriver.substr(0, 1).c_str(), disk_item);//disk_name 
					AddJsonKeyValue(json_data, disk_item);//DISK_TOTAL
					_gcvt(i64freebytes / 1024, 31, json_data);
					AddJsonKeyValue(json_data, disk_item); // DISK_FREE
					disk_data.append(disk_item);
				}
			}
		}
		TDELARRAY(DStr);
		json_value.append(disk_data);
	}
	//memory
	{
		MEMORYSTATUSEX memory_status;
		memory_status.dwLength = sizeof(memory_status);
		GlobalMemoryStatusEx(&memory_status);
		_gcvt(memory_status.ullTotalPhys, 31, json_data);
		AddJsonKeyValue(json_data, json_value);//MEMORY_TOTAL
		_gcvt(memory_status.ullAvailPhys, 31, json_data);
		AddJsonKeyValue(json_data, json_value);//MEMORY_FREE
		_gcvt(memory_status.ullTotalVirtual, 31, json_data);
		AddJsonKeyValue(json_data, json_value);//VIRTUAL_MEM_TATAL
		_gcvt(memory_status.ullAvailVirtual, 31, json_data);
		AddJsonKeyValue(json_data, json_value); // VIRTUAL_MEM_FREE
	}
	
	//pdh performance counter
	{
		int counter_num = CLoadConfig::CreateInstance()->get_counter_num();
		char** counter_name = CLoadConfig::CreateInstance()->get_counter_name();
		WriteCounterVaule(counter_num, counter_name, &json_value);
	}
	json_value.append(CLoadConfig::CreateInstance()->get_os_version());
	json_value.append(CLoadConfig::CreateInstance()->get_os_name());
	return 0;
}

void
CSysInfo::WriteCounterVaule(int counter_num,char** list_counter, Value* json_value)
{
	char** temp_list_counter = list_counter;
	vector<PDH_HCOUNTER*> vt_hcounter;
	char json_data[50] = "";
	PDH_FMT_COUNTERVALUE DisplayValue;
	HQUERY Query = NULL;
	PDH_STATUS Status;
	
	Status = PdhOpenQuery(NULL, NULL, &Query);
	if (Status != ERROR_SUCCESS) goto Cleanup;
	
	for (int i = 0; i < counter_num; i++){
		PDH_HCOUNTER *pdh_hcounter = (HCOUNTER*)GlobalAlloc(GPTR, sizeof(HCOUNTER));
		Status = PdhAddCounter(Query, temp_list_counter[i], 0, pdh_hcounter);
		if (Status != ERROR_SUCCESS) goto Cleanup;
		vt_hcounter.push_back(pdh_hcounter);
	}
	
	Status = PdhCollectQueryData(Query);
	if (Status != ERROR_SUCCESS) goto Cleanup;
	Sleep(1000);
	PdhCollectQueryData(Query);
	if (Status != ERROR_SUCCESS) goto Cleanup;
	for (int j = 0; j < counter_num; j++){
		Status = PdhGetFormattedCounterValue(*vt_hcounter[j],
			PDH_FMT_DOUBLE,
			(LPDWORD)NULL,
			&DisplayValue);
		if (Status != ERROR_SUCCESS) goto Cleanup;
		_gcvt(DisplayValue.doubleValue, 31, json_data);
		AddJsonKeyValue(json_data, *json_value);
	}
	
Cleanup:
	for (unsigned int x = 0; x < vt_hcounter.size(); x++){
		PdhRemoveCounter(*vt_hcounter[x]);
		GlobalFree(vt_hcounter[x]);
	}
	if (Query)
		PdhCloseQuery(Query);
}
//////////////////////////////////////////////////////////////////////////
/*CProcessMonitor*/
//////////////////////////////////////////////////////////////////////////
CProcessMonitor* CProcessMonitor::_instance = NULL;
int
CProcessMonitor::write(int fd, Value& json_value)
{
	if (!GetProcessList()) return 0;
	char pbuffer[1000];
	FILE *ppipe = NULL;
	int nread_line = 0;
	char tcp[10], local_address[50], remote_address[50], state[100];
	int pid = 0;
	ppipe = _popen("netstat -npoa TCP ", "rt");
	string jsonstr;
	char json_data[51] = "";
	int process_num = CLoadConfig::CreateInstance()->get_process_num();
	char** process_name = CLoadConfig::CreateInstance()->get_process_name();
	for ( int i= 0; i < process_num; i++){
		int tcpnum = 0;
		int process_status = 0;
		map< string, vector< int > >::iterator map_itorator_process_name = \
			m_map_process_name_pid.find(process_name[i]);
		if (map_itorator_process_name != m_map_process_name_pid.end()) 
			process_status = 1;                     // 查看进程状态
		if (process_status){
			vector<int> v_pidlist = m_map_process_name_pid[process_name[i]];
			for (unsigned int j = 0; j < v_pidlist.size(); j++){
				while (fgets(pbuffer, 1000, ppipe)){
					if (nread_line>3){
						sscanf(pbuffer, "%s %s %s %s %d\n", 
							tcp, local_address, remote_address, state, &pid);
						if (pid == v_pidlist[j] && !strcmp(state, "ESTABLISHED")){
							tcpnum++;
						}
					}
					nread_line++;
				}
			}
		}
		Value process_data;
		process_data.append(process_name[i]);
		process_data.append(tcpnum);
		process_data.append(process_status);
		json_value.append(process_data);
	}	
	if (feof(ppipe))
		_pclose(ppipe);
	return 0;
}

BOOL
CProcessMonitor::GetProcessList()
{
	HANDLE hProcessSnap;
	HANDLE hProcess;
	PROCESSENTRY32 pe32;

	// Take a snapshot of all processes in the system.
	hProcessSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
	if (hProcessSnap == INVALID_HANDLE_VALUE){
		printError(TEXT("CreateToolhelp32Snapshot (of processes)"));
		return(FALSE);
	}

	// Set the size of the structure before using it.
	pe32.dwSize = sizeof(PROCESSENTRY32);

	if (!Process32First(hProcessSnap, &pe32)){
		printError(TEXT("Process32First")); // show cause of failure
		CloseHandle(hProcessSnap);          // clean the snapshot object
		return(FALSE);
	}
	do{
		hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, pe32.th32ProcessID);
		if (hProcess == NULL)
			printError(TEXT("OpenProcess"));
		else{
			char** process_name = CLoadConfig::CreateInstance()->get_process_name();
			int procee_num = CLoadConfig::CreateInstance()->get_process_num();
			for (int i = 0; i < procee_num;i++){
				if (!strcmp(process_name[i],pe32.szExeFile))
					m_map_process_name_pid[pe32.szExeFile].push_back(pe32.th32ProcessID);
			}
			CloseHandle(hProcess);
		}
	} while (Process32Next(hProcessSnap, &pe32));

	CloseHandle(hProcessSnap);
	return(TRUE);
}

void 
CProcessMonitor::printError(TCHAR* msg)
{
	DWORD eNum;
	TCHAR sysMsg[256];
	TCHAR* p;

	eNum = GetLastError();
	FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
		NULL, eNum,
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), // Default language
		sysMsg, 256, NULL);

	// Trim the end of the line and terminate it with a null
	p = sysMsg;
	while ((*p > 31) || (*p == 9))
		++p;
	do { *p-- = 0; } while ((p >= sysMsg) &&
		((*p == '.') || (*p < 33)));
}


#endif // WIN32

//////////////////////////////////////////////////////////////////////////
/* CBuilderMonitor */
//////////////////////////////////////////////////////////////////////////
void CBuildMonitor::ConcreteMonitor(int object_index,int type)
{
#ifdef WIN32
	if (MONITORTYPE_SYSTEM_INFO == type)
		m_vector_system_monitor[object_index]= CSysInfo::get_instance();
	else if (MONITORTYPE_PROCESS == type)
		m_vector_system_monitor[object_index] = CProcessMonitor::get_instance();
	else if (MONITORTYPE_MSSQL == type)
		m_vector_system_monitor[object_index] = CMsSqlMonitor::get_instance();
#endif // WEIN32
#if defined(HAS_MYSQL)
	else if (MONITORTYPE_MYSQL == type)
		m_vector_system_monitor[object_index] = CMysqlMonitor::get_instance();
#endif
	else if (MONITORTYPE_LINUX_SYSINFO == type)
		m_vector_system_monitor[object_index] = CLinuxSysinfo::get_instance();
	else if (MONITORTYPE_WEB == type)
		m_vector_system_monitor[object_index] = CWebMonitor::get_instance();
#if defined(HAS_ORACLE)
	else if (MONITORTYPE_ORACAL == type)
		m_vector_system_monitor[object_index] = COracleMonitor::get_instance();
#endif
}

CBuildMonitor::~CBuildMonitor()
{
	for (unsigned int i = 0; i < m_vector_system_monitor.size();i++){
		TDEL(m_vector_system_monitor[i]);
	}
	m_vector_system_monitor.clear();
}

CMonitorSystem* CBuildMonitor::get_monitor_obj(int object_index)
{
	return m_vector_system_monitor[object_index];
}
