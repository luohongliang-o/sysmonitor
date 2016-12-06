#include "mysql_monitor.h"
#ifdef HAS_MYSQL
CMysqlMonitor* CMysqlMonitor::_instance = NULL;
int
CMysqlMonitor::write(int fd, Value& json_value)
{
	UINT64_T row_count = 0;
	UINT32_T field_count = 0;
	m_mysql_connection->connect(CLoadConfig::CreateInstance()->get_mysql_connection_string());
	m_mysql_connection->execute("show status;", TRUE);
	CMysqlRecordSet* record_set = m_mysql_connection->get_record_set();

	record_set->get_field_count(&field_count);
	record_set->get_row_count(&row_count);
	for (int i = 0; i < row_count; i++){
		CMysqlRecord* record = record_set->get_record();
		UCHAR_T* temp_value[2] = { 0 };
		for (int j = 0; j < field_count; j++){
			CHAR_T *filed_name = NULL;
			record_set->get_field_name(&filed_name, j);
			rc_t rt = record->get_data(j, &temp_value[j]);
		
		}
		record_set->next();
	}
	return 0;
}
#endif