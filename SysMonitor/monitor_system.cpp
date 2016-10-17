#include "sys_config.h"
#include "monitor_system.h"
#include "func.h"
#ifdef WIN32
#include <pdh.h>
#include <pdhmsg.h>
#include <process.h>
#pragma comment(lib, "pdh.lib")
#define LOGFILENAME "performance"
//////////////////////////////////////////////////////////////////////////
/*CSysInfo*/
//////////////////////////////////////////////////////////////////////////

int
CSysInfo::write(int fd, Value& json_value)
{
	Value  temp_json_value;
	char json_data[50] = "";
	char performance_key[16] = "";
	
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
		int disk_num = 0;
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
					disk_data["disk"].append(disk_item);
					disk_num++;
				}
			}
		}
		disk_data["num"] = disk_num;
		temp_json_value.append(disk_data);
	}
	//memory
	{
		MEMORYSTATUSEX memory_status;
		memory_status.dwLength = sizeof(memory_status);
		GlobalMemoryStatusEx(&memory_status);
		_gcvt(memory_status.ullTotalPhys, 31, json_data);
		AddJsonKeyValue(json_data, temp_json_value);//MEMORY_TOTAL
		_gcvt(memory_status.ullAvailPhys, 31, json_data);
		AddJsonKeyValue(json_data, temp_json_value);//MEMORY_FREE
		_gcvt(memory_status.ullTotalVirtual, 31, json_data);
		AddJsonKeyValue(json_data, temp_json_value);//VIRTUAL_MEM_TATAL
		_gcvt(memory_status.ullAvailVirtual, 31, json_data);
		AddJsonKeyValue(json_data, temp_json_value); // VIRTUAL_MEM_FREE
	}
	
	//pdh performance counter
	{
		int performance_num = m_loadconfig->get_performance_counter_num();
		int performance_by_sec = m_loadconfig->get_performance_by_sec();
		string** performance_name = m_loadconfig->get_performance_name();
		for (int i = 0; i < performance_num; i++){
			double perfordata = WriteperformanceVaule(i, performance_by_sec, (char*)(performance_name[i])->c_str());
			_gcvt(perfordata, 31, json_data);
			AddJsonKeyValue(json_data, temp_json_value);
		}
	}
	temp_json_value.append(m_loadconfig->get_os_version());
	temp_json_value.append(m_loadconfig->get_os_name());
	json_value["system"] =temp_json_value ;
	return 0;
}

double 
CSysInfo::WriteperformanceVaule(int index, int counter_by_sec,char* str_counter_path_buffer)
{
	PDH_STATUS Status;
	HQUERY Query = NULL;
	HCOUNTER Counter;
	PDH_FMT_COUNTERVALUE DisplayValue;
	DWORD CounterType;
	CHAR CounterPathBuffer[PDH_MAX_COUNTER_PATH];
	ZeroMemory(&CounterPathBuffer, sizeof(CounterPathBuffer));
	strcpy_s(CounterPathBuffer, str_counter_path_buffer);
	WriteLog(LOGFILENAME, "Counter selected: %s", CounterPathBuffer);
	printf_s("\n\nCounter selected: %s", CounterPathBuffer);
	
	Status = PdhOpenQuery(NULL, NULL, &Query);
	if (Status != ERROR_SUCCESS)
	{
		WriteLog(LOGFILENAME, "PdhOpenQuery failed with status 0x%x.\n", Status);
		printf("\nPdhOpenQuery failed with status 0x%x.", Status);
		goto Cleanup;
	}
	Status = PdhAddCounter(Query, CounterPathBuffer, 0, &Counter);
	if (Status != ERROR_SUCCESS){
		WriteLog(LOGFILENAME, "PdhAddCounter failed with status 0x%x.\n", Status);
		printf_s("\nPdhAddCounter failed with status 0x%x.", Status);
		goto Cleanup;
	}
	Status = PdhCollectQueryData(Query);
	if (Status != ERROR_SUCCESS){
		WriteLog(LOGFILENAME, "first PdhCollectQueryData failed with 0x%x.\n", Status);
		printf("\nfirst PdhCollectQueryData failed with 0x%x.\n", Status);
		goto Cleanup;
	}
	
	int performance_num = m_loadconfig->get_performance_counter_num();
	if (counter_by_sec > 0 && (index + counter_by_sec >= performance_num)){
		int sleeptime = 1000 / counter_by_sec;
		Sleep(sleeptime);
		Status = PdhCollectQueryData(Query);
		if (Status != ERROR_SUCCESS){
			WriteLog(LOGFILENAME, "secend PdhCollectQueryData failed with status 0x%x.\n", Status);
			printf("\nsecend PdhCollectQueryData failed with status 0x%x.", Status);
		}
	}

	Status = PdhGetFormattedCounterValue(Counter,
		PDH_FMT_DOUBLE,
		&CounterType,
		&DisplayValue);
	if (Status != ERROR_SUCCESS){
		WriteLog(LOGFILENAME, "PdhGetFormattedCounterValue failed with status 0x%x.\n", Status);
		printf("\nPdhGetFormattedCounterValue failed with status 0x%x.", Status);
		goto Cleanup;
	}
	if (Query)
		PdhCloseQuery(Query);
	return DisplayValue.doubleValue;
Cleanup:
	if (Query)
		PdhCloseQuery(Query);
	return 0;
}

//////////////////////////////////////////////////////////////////////////
/*CProcessMonitor*/
//////////////////////////////////////////////////////////////////////////
#include <stdio.h>
#include <fstream>

int
CProcessMonitor::write(int fd, Value& json_value)
{
	if (!GetProcessList()){
		printf(" Init process snapshot list failed.\n");
		return 0;
	}
	string jsonstr;
	char json_data[51] = "";
	int process_num = m_loadconfig->get_process_num();
	string** process_name = m_loadconfig->get_process_name();
	for ( int i= 0; i < process_num; i++){
		int tcpnum = 0;
		int process_status = 0;
		map< string, vector< int > >::iterator map_itorator_process_name = \
			m_map_process_name_pid.find(*process_name[i]);
		if (map_itorator_process_name != m_map_process_name_pid.end()) 
			process_status = 1;                     // �鿴����״̬
		if (process_status){
			vector<int> v_pidlist = m_map_process_name_pid[process_name[i]->c_str()];
			printf("\ncurrent process name is %s", process_name[i]->c_str());
			for (int j = 0; j < v_pidlist.size(); j++){
				char pbuffer[1000];
				FILE *ppipe = NULL;
				int nread_line = 0;
				char tcp[10], local_address[50], remote_address[50], state[100];
				int pid = 0;
				ppipe = _popen("netstat -npoa TCP ", "rt");
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
				printf("\ncurrent process id is %d", v_pidlist[j]);
				if (feof(ppipe))
					_pclose(ppipe);
			}
		}
		Value process_data;
		process_data.append(process_name[i]->c_str());
		process_data.append(tcpnum);
		process_data.append(process_status);
		json_value["process"].append(process_data);
	}	
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
	if (hProcessSnap == INVALID_HANDLE_VALUE)
	{
		printError(TEXT("CreateToolhelp32Snapshot (of processes)"));
		return(FALSE);
	}

	// Set the size of the structure before using it.
	pe32.dwSize = sizeof(PROCESSENTRY32);

	// Retrieve information about the first process,
	// and exit if unsuccessful
	if (!Process32First(hProcessSnap, &pe32))
	{
		printError(TEXT("Process32First")); // show cause of failure
		CloseHandle(hProcessSnap);          // clean the snapshot object
		return(FALSE);
	}
	do
	{
		hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, pe32.th32ProcessID);
		if (hProcess == NULL)
			printError(TEXT("OpenProcess"));
		else{
			string** process_name = m_loadconfig->get_process_name();
			int procee_num = m_loadconfig->get_process_num();
			for (int i = 0; i < procee_num;i++)
			{
				if (!process_name[i]->compare(pe32.szExeFile))
					m_map_process_name_pid[pe32.szExeFile].push_back(pe32.th32ProcessID);
			}
			
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

	// Display the message
	printf(TEXT("\n  WARNING: %s failed with error %d (%s)"), msg, eNum, sysMsg);
}


//////////////////////////////////////////////////////////////////////////
/*CWebMonitor*/
//////////////////////////////////////////////////////////////////////////

int CWebMonitor::write(int fd, Value& json_value)
{	
	Value temp_json_value;
	if (IsW3wpRun()){
		temp_json_value.append(1);
		
		int performance_num = m_loadconfig->get_web_performance_counter_num();
		int performance_by_sec = m_loadconfig->get_web_performance_by_sec();
		string** performance_name = m_loadconfig->get_web_performance_name();
		char json_data[50] = "";
		for (int i = 0; i < performance_num; i++){
			double perfordata = WriteperformanceVaule(i, performance_by_sec, (char*)(performance_name[i])->c_str());
			_gcvt(perfordata, 31, json_data);
			AddJsonKeyValue(json_data, temp_json_value);
		}
		
	}
	else{
		temp_json_value.append(0);//Ӧ�ó����δ����
	}
	json_value["web"].append(temp_json_value);
	
	return 0;
}


BOOL CWebMonitor::IsW3wpRun()
{
	BOOL bret = FALSE;
	HANDLE hProcessSnap;
	HANDLE hProcess;
	PROCESSENTRY32 pe32;

	// Take a snapshot of all processes in the system.
	hProcessSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
	if (hProcessSnap == INVALID_HANDLE_VALUE)
		return(FALSE);

	// Set the size of the structure before using it.
	pe32.dwSize = sizeof(PROCESSENTRY32);

	// Retrieve information about the first process,
	// and exit if unsuccessful
	if (!Process32First(hProcessSnap, &pe32))
	{
		CloseHandle(hProcessSnap);          // clean the snapshot object
		return(FALSE);
	}
	do
	{
		hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, pe32.th32ProcessID);
		if (hProcess == NULL)
			;//return FALSE;
		else if (!strcmp(pe32.szExeFile, "w3wp.exe")){
			bret = TRUE;
			break;
		}
	} while (Process32Next(hProcessSnap, &pe32));

	CloseHandle(hProcessSnap);
	return bret;
}
//////////////////////////////////////////////////////////////////////////
/*
CMsSqlMonitor
*/
//////////////////////////////////////////////////////////////////////////

int CMsSqlMonitor::write(int fd, Value& json_value)
{
	char dberror[256];
	CLinkManager* plink_manage = m_loadconfig->get_link();
	int datanum = m_loadconfig->get_db_count();
	for (int i = 0; i < datanum;i++){
		LPOPLINK plink = plink_manage->GetLink(dberror, 256, i);
	}
	

	return 0;
}
#endif // WIN32



//////////////////////////////////////////////////////////////////////////
/*CMySqlMonitor */
//////////////////////////////////////////////////////////////////////////

int
CMySqlMonitor::write(int fd, Value& json_value)
{
	return 0;
}

//////////////////////////////////////////////////////////////////////////
/* CBuilderMonitor */
//////////////////////////////////////////////////////////////////////////
void CBuildMonitor::ConcreteMonotor(int type, CLoadConfig* loadconfig)
{
#ifdef WIN32
	if (MONITORTYPE_SYSTEM_INFO == type)
		m_system_monitor = new CSysInfo(loadconfig);
	if (MONITORTYPE_PROCESS == type)
		m_system_monitor = new CProcessMonitor(loadconfig);
#endif // WEIN32
	if (MONITORTYPE_MYSQL == type)
		m_system_monitor = new CMySqlMonitor(loadconfig);
	if (MONITORTYPE_LINUX_SYSINFO == type)
		m_system_monitor = new CLinuxSysinfo(loadconfig);
	if (MONITORTYPE_WEB == type)
		m_system_monitor = new CWebMonitor(loadconfig);
}

CBuildMonitor::~CBuildMonitor()
{
	TDEL(m_system_monitor);
}

CMonitorSystem* CBuildMonitor::get_monitor_obj()
{
	return m_system_monitor;
}
