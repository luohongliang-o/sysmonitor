#include "sys_config.h"
#include "monitor_system.h"
#include "func.h"
#ifdef WIN32
#include <pdh.h>
#include <pdhmsg.h>
#include <process.h>
#pragma comment(lib, "pdh.lib")

//////////////////////////////////////////////////////////////////////////
/*CSysInfo*/
//////////////////////////////////////////////////////////////////////////

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
		int counter_num = m_loadconfig->get_counter_num();
		int counter_by_sec = m_loadconfig->get_counter_by_sec();
		vector< string > counter_name = m_loadconfig->get_counter_name();
		for (int i = 0; i < counter_num; i++){
			double perfordata = WriteCounterVaule(i, counter_by_sec, (char*)counter_name[i].c_str());
			_gcvt(perfordata, 31, json_data);
			AddJsonKeyValue(json_data, json_value);
		}
	}
	json_value.append(m_loadconfig->get_os_version());
	json_value.append(m_loadconfig->get_os_name());
	return 0;
}

double 
CSysInfo::WriteCounterVaule(int index, int counter_by_sec, char* str_counter_path_buffer)
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
	//printf_s("\n\nCounter selected: %s", CounterPathBuffer);
	
	Status = PdhOpenQuery(NULL, NULL, &Query);
	if (Status != ERROR_SUCCESS)
	{
		WriteLog(LOGFILENAME, "PdhOpenQuery failed with status 0x%x.\n", Status);
		//printf("\nPdhOpenQuery failed with status 0x%x.", Status);
		goto Cleanup;
	}
	Status = PdhAddCounter(Query, CounterPathBuffer, 0, &Counter);
	if (Status != ERROR_SUCCESS){
		WriteLog(LOGFILENAME, "PdhAddCounter failed with status 0x%x.\n", Status);
		//printf("\nPdhAddCounter failed with status 0x%x.", Status);
		goto Cleanup;
	}
	Status = PdhCollectQueryData(Query);
	if (Status != ERROR_SUCCESS){
		WriteLog(LOGFILENAME, "first PdhCollectQueryData failed with 0x%x.\n", Status);
		//printf("\nfirst PdhCollectQueryData failed with 0x%x.\n", Status);
		goto Cleanup;
	}
	
	int counter_num = m_loadconfig->get_counter_num();
	if (counter_by_sec > 0 && (index + counter_by_sec >= counter_num)){
		int sleeptime = 1000 / counter_by_sec;
		Sleep(sleeptime);
		Status = PdhCollectQueryData(Query);
		if (Status != ERROR_SUCCESS){
			WriteLog(LOGFILENAME, "secend PdhCollectQueryData failed with status 0x%x.\n", Status);
			//printf("\nsecend PdhCollectQueryData failed with status 0x%x.", Status);
		}
	}

	Status = PdhGetFormattedCounterValue(Counter,
		PDH_FMT_DOUBLE,
		&CounterType,
		&DisplayValue);
	if (Status != ERROR_SUCCESS){
		WriteLog(LOGFILENAME, "PdhGetFormattedCounterValue failed with status 0x%x.\n", Status);
		//printf("\nPdhGetFormattedCounterValue failed with status 0x%x.", Status);
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
		WriteLog(LOGFILENAME, " Init process snapshot list failed.\n");
		//printf(" Init process snapshot list failed.\n");
		return 0;
	}
	string jsonstr;
	char json_data[51] = "";
	int process_num = m_loadconfig->get_process_num();
	vector< string > process_name = m_loadconfig->get_process_name();
	for ( int i= 0; i < process_num; i++){
		int tcpnum = 0;
		int process_status = 0;
		map< string, vector< int > >::iterator map_itorator_process_name = \
			m_map_process_name_pid.find(process_name[i]);
		if (map_itorator_process_name != m_map_process_name_pid.end()) 
			process_status = 1;                     // 查看进程状态
		if (process_status){
			vector<int> v_pidlist = m_map_process_name_pid[process_name[i].c_str()];
			//printf("\ncurrent process name is %s", process_name[i].c_str());
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
				//printf("\ncurrent process id is %d", v_pidlist[j]);
				if (feof(ppipe))
					_pclose(ppipe);
			}
		}
		Value process_data;
		process_data.append(process_name[i].c_str());
		process_data.append(tcpnum);
		process_data.append(process_status);
		json_value.append(process_data);
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
	do{
		hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, pe32.th32ProcessID);
		if (hProcess == NULL)
			printError(TEXT("OpenProcess"));
		else{
			vector< string > process_name = m_loadconfig->get_process_name();
			int procee_num = m_loadconfig->get_process_num();
			for (int i = 0; i < procee_num;i++){
				if (!process_name[i].compare(pe32.szExeFile))
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

	// Display the message
	//printf(TEXT("\n  WARNING: %s failed with error %d (%s)"), msg, eNum, sysMsg);
}


//////////////////////////////////////////////////////////////////////////
/*CWebMonitor*/
//////////////////////////////////////////////////////////////////////////

int CWebMonitor::write(int fd, Value& json_value)
{	
	Value temp_json_value;
	if (IsW3wpRun()){
		temp_json_value.append(1);
		
		int counter_num = m_loadconfig->get_web_counter_num();
		int counter_by_sec = m_loadconfig->get_web_counter_by_sec();
		vector< string > counter_name = m_loadconfig->get_web_counter_name();
		char json_data[50] = "";
		for (int i = 0; i < counter_num; i++){
			double perfordata = WriteCounterVaule(i, counter_by_sec, (char*)counter_name[i].c_str());
			_gcvt(perfordata, 31, json_data);
			AddJsonKeyValue(json_data, temp_json_value);
		}
	}
	else{
		temp_json_value.append(0);//应用程序池未开启
		temp_json_value.append(0);//0个连接
	}
	json_value.append(temp_json_value);
	
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
	if (!Process32First(hProcessSnap, &pe32)){
		CloseHandle(hProcessSnap);          // clean the snapshot object
		return(FALSE);
	}
	do{
		hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, pe32.th32ProcessID);
		if (hProcess == NULL)
			;//return FALSE;
		else if (!strcmp(pe32.szExeFile, "w3wp.exe"))
			bret = TRUE;
		CloseHandle(hProcess);
		if (bret) 
			break;
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
	int datanum = m_loadconfig->get_db_count();
	Value temp_json_value;
	for (int i = 0; i < datanum;i++){
		vector< int > vt_data1, vt_data2;
		//locks by sec
		int record_count = 0;
		get_counter_value(i,vt_data1);
		Sleep(1000);
		record_count = get_counter_value(i,vt_data2);
		for (int j = 0; j < record_count;j++){
			if (j == 0 || (j >= 4 && j <= 6) || j >= 13)
				vt_data2[j] = vt_data2[j] - vt_data1[j];
			if (j == 2 || j == 8){
				char temp_data[10] = "";
				sprintf_s(temp_data, sizeof(temp_data), "%.2f", vt_data2[j - 1] * 100.0 / vt_data2[j]);
				temp_json_value.append(temp_data);
			}
			else if (j == 0 || (j >= 3 && j <= 6)|| j > 8)
				temp_json_value.append(vt_data2[j]);
		}
		json_value.append(temp_json_value);
	}
	
	return 0;
}

int CMsSqlMonitor::get_counter_value(int data_sel,vector< int >& vt_data)
{
	const char select_str_format[1024] =
"select cntr_value from sys.sysperfinfo \
where counter_name in ('Full Scans/sec', 'Average Latch Wait Time (ms)', 'User Connections', 'Processes blocked') or \
(counter_name in ('buffer cache hit ratio', 'buffer cache hit ratio base',\
'Lazy Writes/sec', 'Page reads/sec', 'Page writes/sec', 'Database pages') and object_name like '%%buffer manager%%') or \
(counter_name in ('Cache Hit Ratio', 'Cache Hit Ratio base') and instance_name = '%s') or \
(counter_name in ('Number of Deadlocks/sec', 'Average Wait Time (ms)', 'Lock Requests/sec') and instance_name = '_Total') \
order by object_name, counter_name";

	char dberror[256];
	LPOPLINK plink = m_plink_manage->GetLink(dberror, 256, data_sel);
	long record_count = 0;
	CADODatabase* p_ado_db = plink->ado_db;
	CADORecordset ado_recordset(p_ado_db);
	char select_str[1024];
	sprintf_s(select_str, sizeof(select_str), select_str_format, (m_loadconfig->get_db_config())[data_sel].data_base);
	try{
		do {
			if (!ado_recordset.Open(select_str, CADORecordset::openStoredProc)) break;
			if (!ado_recordset.IsOpen()) break;
			record_count = ado_recordset.GetRecordCount();
			vt_data.resize(record_count);
			for (long i = 0; i < record_count; i++, ado_recordset.MoveNext()){
				long field_count = ado_recordset.GetFieldCount();
				long cntr_value = 0;
				ado_recordset.GetFieldValue(0, cntr_value);
				vt_data[i] = cntr_value;
			}
		} while (FALSE);

	}
	catch (...){
	}
	m_plink_manage->FreeLink(plink);
	return record_count;
}
#endif // WIN32
//////////////////////////////////////////////////////////////////////////
/*CMySqlMonitor */
//////////////////////////////////////////////////////////////////////////

int
CMySqlMonitor::write(int fd, Value& json_value)
{	
	uint64_t row_count = 0;
	uint32_t field_count = 0;
	m_mysql_connection->connect(m_loadconfig->get_mysql_connection_string());
	m_mysql_connection->execute("show status;", TRUE);
	CMysqlRecordSet* record_set = m_mysql_connection->get_record_set();
	
	record_set->get_field_count(&field_count);
	record_set->get_row_count(&row_count);
	for (int i = 0; i < row_count;i++){
		CMysqlRecord* record = record_set->get_record();
		uchar_t* temp_value[2] = { 0 };
		for (int j = 0; j < field_count;j++){
			rc_t rt = record->get_data(j, &temp_value[j]);
			int a = 0;
		}
		record_set->next();
	}
	return 0;
}
//////////////////////////////////////////////////////////////////////////
/*
COracleMonitor
*/
//////////////////////////////////////////////////////////////////////////
#include "ocilib.hpp"
using namespace ocilib;
int COracleMonitor::write(int fd, Value& json_value)
{
	int ncount = 0;
	try
	{
		Environment::Initialize();
		ocilib::Connection con("xe", "lhl", "123456");
		Statement st(con);
		st.Execute("select * from person_info");
		Resultset rs = st.GetResultset();
		while (rs.Next())
		{

		}
		ncount = rs.GetCount();

	}
	catch (std::exception &ex)
	{
		const char *erro = ex.what() ;
		int a = 0;
	}
	Environment::Cleanup();
	return 0;
}
//////////////////////////////////////////////////////////////////////////
/* CBuilderMonitor */
//////////////////////////////////////////////////////////////////////////
void CBuildMonitor::ConcreteMonitor(int type, CLoadConfig* loadconfig)
{
	
#ifdef WIN32
	if (is_object_exist(type, loadconfig) && MONITORTYPE_SYSTEM_INFO == type)
		m_system_monitor = new CSysInfo(loadconfig);
	if (is_object_exist(type, loadconfig) && MONITORTYPE_PROCESS == type)
		m_system_monitor = new CProcessMonitor(loadconfig);
	if (is_object_exist(type, loadconfig) && MONITORTYPE_MSSQL == type)
		m_system_monitor = new CMsSqlMonitor(loadconfig);
#endif // WEIN32
	if (is_object_exist(type, loadconfig) && MONITORTYPE_MYSQL == type)
		m_system_monitor = new CMySqlMonitor(loadconfig);
	if (is_object_exist(type, loadconfig) && MONITORTYPE_LINUX_SYSINFO == type)
		m_system_monitor = new CLinuxSysinfo(loadconfig);
	if (is_object_exist(type, loadconfig) && MONITORTYPE_WEB == type)
		m_system_monitor = new CWebMonitor(loadconfig);
	if (is_object_exist(type, loadconfig) && MONITORTYPE_ORACAL == type)
		m_system_monitor = new COracleMonitor(loadconfig);
}

CBuildMonitor::~CBuildMonitor()
{
	TDEL(m_system_monitor);
}

CMonitorSystem* CBuildMonitor::get_monitor_obj()
{
	return m_system_monitor;
}

BOOL CBuildMonitor::is_object_exist(int type, CLoadConfig* loadconfig)
{
	int object_num = loadconfig->get_object_num();
	vector< short > object_type = loadconfig->get_object_type();
	for (int i = 0; i < object_num;i++){
		if (object_type[i] == type)
			return TRUE;
	}
	return FALSE;

}
