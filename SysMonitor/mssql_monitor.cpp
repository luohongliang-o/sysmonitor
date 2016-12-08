#include "mssql_monitor.h"
#ifdef WIN32
CMsSqlMonitor* CMsSqlMonitor::_instance = NULL;
int CMsSqlMonitor::write(int fd, Value& json_value)
{
	int datanum = CLoadConfig::CreateInstance()->get_db_count();
	Value temp_json_value;
	for (int i = 0; i < datanum; i++){
		vector< int > vt_data1, vt_data2;
		//locks by sec
		int record_count = 0;
		get_counter_value(i, vt_data1);
		Sleep(1000);
		record_count = get_counter_value(i, vt_data2);
		for (int j = 0; j < record_count; j++){
			if (j == 0 || (j >= 4 && j <= 6) || j >= 13)
				vt_data2[j] = vt_data2[j] - vt_data1[j];
			if (j == 2 || j == 8){
				char temp_data[10] = "";
				sprintf_s(temp_data, sizeof(temp_data), "%.2f", vt_data2[j - 1] * 100.0 / vt_data2[j]);
				temp_json_value.append(temp_data);
			}
			else if (j == 0 || (j >= 3 && j <= 6) || j > 8)
				temp_json_value.append(vt_data2[j]);
		}
		json_value.append(temp_json_value);
	}
	return 0;
}

int CMsSqlMonitor::get_counter_value(int data_sel, vector< int >& vt_data)
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
	sprintf_s(select_str, sizeof(select_str), select_str_format, (CLoadConfig::CreateInstance()->get_db_config())[data_sel].data_base);
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

#endif