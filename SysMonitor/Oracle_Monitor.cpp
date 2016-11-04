#include "Oracle_Monitor.h"
#if defined(HAS_ORACLE)

COracleMonitor* COracleMonitor::_instance = NULL;

COracleMonitor::COracleMonitor()
{
	OCI_Initialize(NULL, "", OCI_ENV_DEFAULT | OCI_ENV_THREADED);
	OCI_EnableWarnings(TRUE);
	m_connection = OCI_ConnectionCreate("xe", "lhl", "123456", OCI_SESSION_DEFAULT);
}

COracleMonitor::~COracleMonitor()
{
	OCI_ConnectionFree(m_connection);
	OCI_Cleanup();
}

int COracleMonitor::write(int fd, Value& json_value)
{
	int ncount = 0;
	//DWORD tick1 = GetTickCount();
	try
	{
		
		//ocilib::Connection con("xe", "lhl", "123456");
		m_statement = OCI_StatementCreate(m_connection);
		OCI_ExecuteStmt(m_statement, "select * from person_info");
		m_result_set = OCI_GetResultset(m_statement);
		while (OCI_FetchNext(m_result_set))
		{

		}
		ncount = OCI_GetColumnCount(m_result_set);

	}
	catch (std::exception &ex)
	{
		const char *erro = ex.what();
		int a = 0;
	}
	//DWORD tick2 = GetTickCount();
	//printf("oracle wirte time:%d\n", tick2 - tick1);
	return 0;
}

#endif