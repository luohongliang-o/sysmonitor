#include "mysql_monitor.h"

#ifdef HAS_MYSQL
#if !defined(WIN32)
#include <map>
#include <stdio.h>
#endif

CMysqlMonitor* CMysqlMonitor::_instance = NULL;

void CMysqlMonitor::get_master_slave_data(CMysqlConnection* pconn, Value& json_data)
{
	UINT64_T row_count = 0;
	UINT32_T field_count = 0;
	CMysqlRecordSet* record_set;
	pconn->execute("show slave status;", TRUE);
	record_set = pconn->get_record_set();
	if (record_set){
		record_set->get_row_count(&row_count);
		record_set->get_field_count(&field_count);
		for (int i = 0; i < row_count; i++){
			Value item;
			CMysqlRecord* record = record_set->get_record();
			UCHAR_T *temp_value = NULL;
			if (RC_S_OK == record->get_data(10, &temp_value))
				item.append((char*)temp_value);
			else item.append("");
			if (RC_S_OK == record->get_data(11, &temp_value))
				item.append((char*)temp_value);
			else item.append("");
			if (RC_S_OK == record->get_data(32, &temp_value))
				item.append((char*)temp_value);
			else item.append("");
			json_data.append(item);
			record_set->next();
		}
	}
}
#ifndef WIN32
void CMysqlMonitor::get_ndb_show_state(char* line_str, Value& json_data)
{
	char tempvalue[7][40];
	sscanf(line_str, "%s %s %s %s %s %s %s", &tempvalue[0], &tempvalue[1], &tempvalue[2], &tempvalue[3], &tempvalue[4], &tempvalue[5], &tempvalue[6]);
	json_data.append(tempvalue[0] + 3);
	char tempstr[128] = "";
	if (strstr(tempvalue[1], "@")) {
		json_data.append(tempvalue[1] + 1);	
		sprintf_s(tempstr, 128, "%s %s", tempvalue[2], tempvalue[3]);
		json_data.append(tempstr);
	}
	if (strstr(tempvalue[1], "(not")){
		strncpy(tempstr, tempvalue[6], (size_t)strlen(tempvalue[6]) - 1);
		json_data.append(tempstr);
		json_data.append("not connected");
	}

}
#endif

void CMysqlMonitor::InitJsonValue(Value& json_value)
{
	Reader temp_read;
	Value master_slave_json_data;
#ifndef WIN32
	Value ndb_memoryusage_json_data, ndb_state_data, mgm_state_data, api_state_data;
#endif;


}
int CMysqlMonitor::write(int fd, Value& json_value)
{	
	UINT64_T row_count = 0;
	UINT32_T field_count = 0;
	CMysqlRecordSet* record_set = NULL;
	int mysql_type = CLoadConfig::CreateInstance()->get_mysql_type();
	Value master_slave_json_data;
	Value ndb_memoryusage_json_data, ndb_state_data, mgm_state_data, api_state_data;
	Reader temp_read;
	//FastWriter tepm_write;
	temp_read.parse("[]", master_slave_json_data);
	temp_read.parse("[]", ndb_state_data);
	temp_read.parse("[]", mgm_state_data);
	temp_read.parse("[]", api_state_data);
	temp_read.parse("[]", ndb_memoryusage_json_data);
	if (m_mysql_list_connection.size() == 0) {
		json_value.append(master_slave_json_data);
		json_value.append(ndb_state_data);
		json_value.append(mgm_state_data);
		json_value.append(api_state_data);
		json_value.append(ndb_memoryusage_json_data);
		for (int i = 0; i < 22;i++){
			json_value.append("");
		}
		return 0;
	}
	CMysqlConnection* mysql_connection = m_mysql_list_connection[0];
	
	if (mysql_type == MYSQL_MASTER_SLAVE){
		int dbcount = CLoadConfig::CreateInstance()->get_mysql_dbcount();
		for (int i = 0; i < dbcount; i++){
			mysql_connection = m_mysql_list_connection[i];
			get_master_slave_data(mysql_connection, master_slave_json_data);
		}
	}
	json_value.append(master_slave_json_data);
	
#ifndef WIN32
	if(mysql_type == MYSQL_NDB){
		char line_buf[256];
		int ndb_node_count = 0,api_node_count=0,mgm_node_count;
		int ndb_node_line_end = -1,api_node_line_end =-1,mgm_node_line_end= -1;
		int index = 0;
		std::map<int,char*> map_id_ip;
		FILE *pFile = popen("ndb_mgm -e 'show'", "r");
		while (fgets(line_buf, 256, pFile)){
			int id;
			if (index <= ndb_node_line_end){
				char tempIP[40];
				char* ip = new char[40];
				sscanf(line_buf, "id=%d %s", &id, tempIP);
				strcpy(ip,tempIP+1);
				map_id_ip[id] = ip;
			}
			if(index<=ndb_node_line_end){
				Value item;
				get_ndb_show_state(line_buf,item);
				ndb_state_data.append(item);
			}
			if(index <= mgm_node_line_end){
				Value item;
				get_ndb_show_state(line_buf,item);
				mgm_state_data.append(item);
			}
			if(index <= api_node_line_end){
				Value item;
				get_ndb_show_state(line_buf,item);
				api_state_data.append(item);
			}
			if (strstr(line_buf, "[ndbd(NDB)]")){
				char tempvalue[40];
				sscanf(line_buf, "%s %d", tempvalue, &ndb_node_count);
				ndb_node_line_end = ndb_node_count + index;
			}
			if (strstr(line_buf, "[ndb_mgmd(MGM)]")){
				char tempvalue[40];
				sscanf(line_buf, "%s %d", tempvalue, &mgm_node_count);
				mgm_node_line_end = mgm_node_count + index;
			}
			if (strstr(line_buf, "[mysqld(API)]")){
				char tempvalue[40];
				sscanf(line_buf, "%s %d", tempvalue, &api_node_count);
				api_node_line_end = api_node_count + index;
			}
			index++;
		}
		
		json_value.append(ndb_state_data);
		json_value.append(mgm_state_data);
		json_value.append(api_state_data);
		pclose(pFile);
		pFile = popen("ndb_mgm -e 'all report memoryusage'", "r");
		index=0;
		
		std::map<int,std::vector<int> > map_id_data;
		while (fgets(line_buf, 256, pFile)){
			char tempvalue[4][40];
			int id, usage_rate;
			if (strstr(line_buf, "Node")){
				sscanf(line_buf, "%s %d: %s %s %s %d", &tempvalue[0], &id, &tempvalue[1], &tempvalue[2], &tempvalue[3], &usage_rate);
				map_id_data[id].push_back(usage_rate);
			}
			index++;
		}
		std::map<int,std::vector<int> >::iterator it_map = map_id_data.begin();
		while (it_map != map_id_data.end()){
			Value item;
			char tempvalue[40];
			sprintf_s(tempvalue,40,"%d",it_map->first);
			item.append(tempvalue);
			item.append(map_id_ip[it_map->first]);
			sprintf_s(tempvalue,40,"%d",it_map->second.at(0));
			item.append(tempvalue);
			sprintf_s(tempvalue,40,"%d",it_map->second.at(1));
			item.append(tempvalue);
			it_map++;
			ndb_memoryusage_json_data.append(item);
		}
		pclose(pFile);
		json_value.append(ndb_memoryusage_json_data);
		std::map<int,char*>::iterator it_map_id = map_id_ip.begin();
		while (it_map_id!=map_id_ip.end()){
			TDELARRAY(it_map_id->second);
			it_map_id++;
		}

	}
#else
	json_value.append(ndb_state_data);
	json_value.append(mgm_state_data);
	json_value.append(api_state_data);
	json_value.append(ndb_memoryusage_json_data);
#endif

	mysql_connection->execute(m_mysql_default_proc, TRUE);
	record_set = mysql_connection->get_record_set();
	if (record_set){
		CMysqlRecord* record = record_set->get_record();
		UCHAR_T* temp_value = NULL;
		rc_t rt = record->get_data(0, &temp_value);
		if (RC_S_OK == rt) json_value.append((char*)temp_value);
	}
	else json_value.append("");

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
	mysql_connection->execute(str_mysql_status, TRUE);
	record_set = mysql_connection->get_record_set();
	if (record_set){
		record_set->get_row_count(&row_count);
		for (int i = 0; i < row_count; i++){
			CMysqlRecord* record = record_set->get_record();
			UCHAR_T* temp_value = NULL;
			rc_t rt = record->get_data(1, &temp_value);
			if (RC_S_OK == rt) json_value.append((char*)temp_value);
			else json_value.append("");
			record_set->next();
		}
	}
	else{
		for (int i = 0; i < 21;i++)
			json_value.append("");
	}

	return 0;
}
#endif