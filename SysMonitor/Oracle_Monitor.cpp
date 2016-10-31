#include "Oracle_Monitor.h"
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
		const char *erro = ex.what();
		int a = 0;
	}
	Environment::Cleanup();
	return 0;
}
