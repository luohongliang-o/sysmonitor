#include "mysql_monitor.h"

int
CMysqlMonitor::write(int fd, Value& json_value)
{
	uint64_t row_count = 0;
	uint32_t field_count = 0;
	m_mysql_connection->connect(m_loadconfig->get_mysql_connection_string());
	m_mysql_connection->execute("show status;", TRUE);
	CMysqlRecordSet* record_set = m_mysql_connection->get_record_set();

	record_set->get_field_count(&field_count);
	record_set->get_row_count(&row_count);
	for (int i = 0; i < row_count; i++){
		CMysqlRecord* record = record_set->get_record();
		uchar_t* temp_value[2] = { 0 };
		for (int j = 0; j < field_count; j++){
			rc_t rt = record->get_data(j, &temp_value[j]);
			int a = 0;
		}
		record_set->next();
	}
	return 0;
}