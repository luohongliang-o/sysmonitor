#include "ndb_monitor.h"
#include <stdio.h>

CNdbMonitor* CNdbMonitor::_instance = NULL;


static void* thread_func(void *pv)
{	
	CNdbMonitor* ndb_obj = (CNdbMonitor*)pv;
	while (true){
		PTHREAD_T self_tid = PTHREAD_SELF();
		ndb_obj->scan_log(self_tid);
		SLEEP(3);
	}
	return 0;
}

CNdbMonitor::CNdbMonitor() :m_lock()
{	
	m_log_file_num = CLoadConfig::CreateInstance()->get_ndb_log_num();
	char** log_file_list = CLoadConfig::CreateInstance()->get_ndb_log_file_name();
	for (int i = 0; i < m_log_file_num; i++){
		struct stat statbuff;
		
		if (-1 == STAT(log_file_list[i], &statbuff)){
			fprintf(stderr, "error stat :%s\n", strerror(errno));
			exit(EXIT_FAILURE);
		}
		LOG_STATE_DATA* log_state_data = new LOG_STATE_DATA;
		log_state_data->file_size = statbuff.st_size;
		log_state_data->file_pos = 0;
		log_state_data->log_status = LOG_NORMAL;
		log_state_data->log_err_msg = "";
		log_state_data->file_name = log_file_list[i];
		PTHREAD_T tid = 0;
		unsigned threadid;
		threadid = PTHREAD_CREATE(&tid, NULL, thread_func, (void*)this);
		m_map_log_state.insert(MAPLOG::value_type(tid, log_state_data));
		if (0 != threadid){
			fprintf(stderr, "error creating thread:%s %d\n", strerror(errno), threadid);
			exit(EXIT_FAILURE);
		}
		printf("create thread %ld\n", tid);
		
	}
};

CNdbMonitor::~CNdbMonitor()
{
	MAPLOG::iterator ilog_state = m_map_log_state.begin();
	while (ilog_state != m_map_log_state.end())
	{
		PTHREAD_JOIN(ilog_state->first, NULL);
		TDEL(ilog_state->second);
		m_map_log_state.erase(ilog_state++);
	}
	m_map_log_state.clear();
}

void CNdbMonitor::set_file_state(PTHREAD_T tid, LOG_STATE_DATA* plog_data)
{
	CAutoLock auto_lock(&m_lock);
	MAPLOG::iterator ilog_state = m_map_log_state.find(tid);
	if (ilog_state != m_map_log_state.end()){
		ilog_state->second->file_size = plog_data->file_size;
		ilog_state->second->file_pos = plog_data->file_pos;
		ilog_state->second->log_err_msg = plog_data->log_err_msg;
		ilog_state->second->log_status = plog_data->log_status;
	}
}

LOG_STATE_DATA* CNdbMonitor::get_file_sate(PTHREAD_T tid)
{
	CAutoLock auto_lock(&m_lock);
	MAPLOG::iterator ilog_state = m_map_log_state.find(tid);
	return ilog_state->second;
}

void CNdbMonitor::reset_log_status(PTHREAD_T tid)
{
	CAutoLock auto_lock(&m_lock);
	if (0 == tid){
		MAPLOG::iterator ilog_state = m_map_log_state.begin();
		while (ilog_state != m_map_log_state.end())
		{	
			ilog_state->second->log_err_msg = "";
			ilog_state->second->log_status = 0;
			ilog_state++;
		}
	}
	else{
		MAPLOG::iterator ilog_state = m_map_log_state.find(tid);
		if (ilog_state != m_map_log_state.end()){
			ilog_state->second->log_err_msg = "";
			ilog_state->second->log_status = 0;
		}
	}
}
int CNdbMonitor::write(int fd, Value& json_value)
{
	CAutoLock auto_lock(&m_lock);
	compare_file_state();
	MAPLOG::iterator ilog_state = m_map_log_state.begin();
	while (ilog_state != m_map_log_state.end())
	{	
		Value log_data;
		log_data.append(ilog_state->second->log_status);
		log_data.append(ilog_state->second->log_err_msg);
		log_data.append(ilog_state->second->file_name);
		log_data.append((long long)ilog_state->first);
		json_value.append(log_data);
		ilog_state++;
	}
	return 0;
}

void CNdbMonitor::compare_file_state()
{	
	MAPLOG::iterator ilog_state = m_map_log_state.begin();
	while (ilog_state != m_map_log_state.end())
	{
		LOG_STATE_DATA* plog_state_data = ilog_state->second;
		if (plog_state_data)
		{
			int hFile = OPEN(plog_state_data->file_name.c_str(), __O_RDONLY);
			int read_time = 10;
			while (-1 == hFile && read_time > 0)
			{
				hFile = OPEN(plog_state_data->file_name.c_str(), __O_RDONLY);
				if (-1 == hFile){
					fprintf(stderr, "compare fopen error with msg is: %s file:%s\n", strerror(errno), plog_state_data->file_name.c_str());
				}
				read_time--;
			}
			long cur_file_size = 0;
			if (-1 != hFile){
				long cur_offset = LSEEK(hFile, 0, SEEK_CUR);
				cur_file_size = LSEEK(hFile, 0, SEEK_END);
				LSEEK(hFile, cur_offset, SEEK_SET);
				WriteLog(1, LOGFILENAME, "cur_file_size:%ld file_size:%ld file_pos:%ld", cur_file_size, plog_state_data->file_size, cur_offset);
				CLOSE(hFile);
				if (plog_state_data->file_size > cur_file_size) {
					plog_state_data->file_pos = 0;
					plog_state_data->file_size = cur_file_size;
					set_file_state(ilog_state->first, plog_state_data);
				}
			}
		}
		ilog_state++;
	}
}

void CNdbMonitor::scan_log(PTHREAD_T tid)
{	
	char buf[1000];
	LOG_STATE_DATA* plog_state_data = get_file_sate(tid);
	if (plog_state_data){
		FILE *pFile = fopen(plog_state_data->file_name.c_str(), "rb");
		if (!pFile){
			fprintf(stderr, "sacan fopen error with msg is: %s file:%s\n", strerror(errno), plog_state_data->file_name.c_str());
		}
		int line_num = 0;
		if (pFile){
			fseek(pFile, plog_state_data->file_pos, SEEK_SET);
		}
		printf("file_name:%s file_pos:%ld tid:%ld\n", plog_state_data->file_name.c_str(), plog_state_data->file_pos, tid);
		while (pFile && line_num < READ_LINE_NUM) {
			if (!fgets(buf, sizeof(buf), pFile)){
				fprintf(stderr, "error fget :%s file:%s\n", strerror(errno), plog_state_data->file_name.c_str());
				printf("%s\n", buf);
				break;
			}
			//printf("2-file_pos:%ld\n",ftell(pFile));
			char date_str[500], time_str[500], other_str[500], tag_str[500];
			sscanf(buf, "%s %s %s %s", date_str, time_str, other_str, tag_str);
			//printf("%s %s %s %s\n",date_str, time_str, other_str, tag_str);
			bool b_update_errmsg = false;
			char* up_tag_str = strupr(tag_str);
			int log_sataus = 0;
			if ((strstr(up_tag_str, "WARNING") != NULL || strstr(up_tag_str, "ALERT") != NULL) && plog_state_data->log_status != LOG_ERROR){
				plog_state_data->log_status = LOG_WARNING;
				b_update_errmsg = true;
			}
			if (strstr(up_tag_str, "ERROR") != NULL){
				b_update_errmsg = true;
				plog_state_data->log_status = LOG_ERROR;
			}
			if (b_update_errmsg){
				string err_msg, buf_more_str;
				buf_more_str = buf + strlen(date_str) + 1 + strlen(time_str) + 1 + strlen(other_str) + 1 + strlen(tag_str) + 1;
				size_t found = buf_more_str.rfind("\n");
				if (found != string::npos){
					buf_more_str.replace(found, 2, "");
				}
				err_msg.append(date_str);
				err_msg.append(" ");
				err_msg.append(time_str);
				err_msg.append(":");
				err_msg.append(buf_more_str);
				plog_state_data->log_err_msg = buf_more_str;
				//WriteLog(1, LOGFILENAME, "buf[0]:%s buf[1]:%s buf[2]:%s buf[3]:%s %s" , date_str, time_str, other_str, tag_str,err_msg.c_str());
			}
			line_num++;
		}

		if (pFile){
			plog_state_data->file_pos = ftell(pFile);
			set_file_state(tid, plog_state_data);
			fclose(pFile);
		}
			
	}
		
	
	
}