#include "web_monitor.h"
#ifdef WIN32
#include <tlhelp32.h>

CWebMonitor* CWebMonitor::_instance = NULL;
int CWebMonitor::write(int fd, Value& json_value)
{

	Value temp_json_value;
	if (IsW3wpRun()){
		temp_json_value.append(1);
		int counter_num = CLoadConfig::CreateInstance()->get_web_counter_num();
		vector< string > counter_name = CLoadConfig::CreateInstance()->get_web_counter_name();
		char json_data[50] = "";
		WriteCounterVaule(counter_num, &counter_name, &json_value);
	}
	else{
		temp_json_value.append(0);//应用程序池未开启
		temp_json_value.append(0);//0个连接
	}
	json_value.append(temp_json_value);

	return 0;
}

bool CWebMonitor::IsW3wpRun()
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

#else
int CWebMonitor::write(int fd, Value& json_value)
{
	return 0;
}
#endif

