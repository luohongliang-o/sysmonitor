#include "mysql_monitor.h"

#ifdef HAS_MYSQL
CMysqlMonitor* CMysqlMonitor::_instance = NULL;
int
CMysqlMonitor::write(int fd, Value& json_value)
{
	UINT64_T row_count = 0;
	UINT32_T field_count = 0;
	const char* str_mysql_status = "show status where variable_name in\
(\
'Questions'\
, 'Com_select'\
, 'Com_insert'\
, 'Com_update'\
, 'Com_delete'\
, 'Uptime'\
, 'Slow_queries'\
, 'Threads_connected'\
, 'Threads_running'\
, 'Aborted_connects'\
, 'Innodb_buffer_pool_pages_total'\
, 'Innodb_buffer_pool_pages_free'\
, 'Innodb_buffer_pool_read_requests'\
, 'Innodb_buffer_pool_reads'\
, 'Bytes_received'\
, 'Bytes_sent'\
, 'Connections'\
, 'Qcache_free_memory'\
, 'Qcache_hits'\
, 'Queries'\
, 'Table_locks_waited'\
);";
	m_mysql_connection->execute(m_mysql_default_proc, TRUE);
	CMysqlRecordSet* record_set = m_mysql_connection->get_record_set();
	if (record_set){
		CMysqlRecord* record = record_set->get_record();
		UCHAR_T* temp_value = NULL;
		rc_t rt = record->get_data(0, &temp_value);
		if (RC_S_OK == rt) json_value.append((char*)temp_value);
	}
	else json_value.append("");

	m_mysql_connection->execute(str_mysql_status, TRUE);
	record_set = m_mysql_connection->get_record_set();
	record_set->get_field_count(&field_count);
	record_set->get_row_count(&row_count);
	for (int i = 0; i < row_count; i++){
		CMysqlRecord* record = record_set->get_record();
		UCHAR_T* temp_value = NULL;
		rc_t rt = record->get_data(1, &temp_value);
		if (RC_S_OK == rt) json_value.append((char*)temp_value);
		else json_value.append("");		
		record_set->next();
	}
	return 0;
}
#endif