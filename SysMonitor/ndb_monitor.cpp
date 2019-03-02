#include "ndb_monitor.h"
#include <stdio.h>

CNdbMonitor* CNdbMonitor::_instance = NULL;

#ifndef WIN32
static void* thread_func(void *pv)
{	
	CNdbMonitor* ndb_obj = (CNdbMonitor*)pv;
	ndb_obj->scan_log();
}
#endif

CNdbMonitor::CNdbMonitor() :m_lock()
#ifndef WIN32
, m_work_thread(0)
#endif
{	
	m_file_pos = 0;
	m_cur_log_status = LOG_NORMAL;
	m_cur_log_type = CLoadConfig::CreateInstance()->get_ndb_type();
	m_cur_file_name =CLoadConfig::CreateInstance()->get_ndb_log_file_name();
	m_cur_err_msg="";
	
#ifndef WIN32
	struct stat statbuff;
	if (-1 == stat(m_cur_file_name.c_str(), &statbuff)){
		fprintf(stderr, "error stat :%s\n", strerror(errno));
		exit(EXIT_FAILURE);
	}
	m_file_size = statbuff.st_size;

	if (0 == m_work_thread){
		printf("create thread\n");
		unsigned threadid;
		threadid = pthread_create(&m_work_thread, NULL, thread_func, (void*)this);
		if (0 != threadid){
			fprintf(stderr, "error creating thread:%s %d\n", strerror(errno), threadid);
			exit(EXIT_FAILURE);
		}
	}
#endif
};

int CNdbMonitor::write(int fd, Value& json_value)
{
#ifndef WIN32
	
	compare_file_state();
 	json_value.append(m_cur_log_status);
 	json_value.append(m_cur_err_msg.c_str());
	json_value.append(m_cur_file_name.c_str());
	
#endif // 
	return 0;
}

void CNdbMonitor::compare_file_state()
{
#ifndef WIN32
	int hFile = open(m_cur_file_name.c_str(), O_RDONLY);
	int read_time = 10;
	while (-1==hFile && read_time>0)
	{
		hFile = open(m_cur_file_name.c_str(), O_RDONLY);
		if(-1==hFile){
			fprintf(stderr, "compare fopen error with msg is: %s file:%s\n", strerror(errno), m_cur_file_name.c_str());
		}
		read_time--;
	}
	long cur_file_size=0;
	if(-1!=hFile){
		long cur_offset = lseek(hFile,0,SEEK_CUR);
		cur_file_size = lseek(hFile,0,SEEK_END);
		lseek(hFile,cur_offset,SEEK_SET);
		WriteLog(1, LOGFILENAME, "cur_file_size:%ld file_size:%ld file_pos:%ld" ,cur_file_size,m_file_size,cur_offset);
		close(hFile);
		if (m_file_size > cur_file_size) {
			set_file_pos(0);
			m_file_size = cur_file_size;
		}
	}
#endif // !WIN32
	
}

void CNdbMonitor::scan_log()
{	
#ifndef WIN32
	char buf[1000];
	while (true)
	{	
		FILE *pFile = fopen(m_cur_file_name.c_str(), "rb");
		if (!pFile){
			fprintf(stderr, "sacan fopen error with msg is: %s file:%s\n", strerror(errno), m_cur_file_name.c_str());
		}
		int line_num = 0;
		if(pFile){
			fseek(pFile,m_file_pos,SEEK_SET);
		}
		printf("m_file_pos:%ld\n",m_file_pos);
		while (pFile && line_num < READ_LINE_NUM) {
			if (!fgets(buf, sizeof(buf), pFile)){
				fprintf(stderr, "error fget :%s file:%s\n", strerror(errno),m_cur_file_name.c_str());
				printf("%s\n", buf);
				break;
			}
			//printf("2-file_pos:%ld\n",ftell(pFile));
			char date_str[500], time_str[500], other_str[500], tag_str[500];
			sscanf(buf, "%s %s %s %s", date_str, time_str, other_str, tag_str);
			//printf("%s %s %s %s\n",date_str, time_str, other_str, tag_str);
			bool b_update_errmsg = false;
			char* up_tag_str = strupr(tag_str);
			if ((strstr(up_tag_str, "WARNING") != NULL || strstr(up_tag_str, "ALERT") != NULL) && m_cur_log_status != LOG_ERROR){
				set_cur_log_status(LOG_WARNING);
				b_update_errmsg = true;
			}
			if (strstr(up_tag_str, "ERROR") != NULL){
				b_update_errmsg = true;
				set_cur_log_status(LOG_ERROR);
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
				set_log_msg(err_msg.c_str());
				//WriteLog(1, LOGFILENAME, "buf[0]:%s buf[1]:%s buf[2]:%s buf[3]:%s %s" , date_str, time_str, other_str, tag_str,err_msg.c_str());
			}
			line_num++;
		}
		
		if (pFile){
			set_file_pos(ftell(pFile));
			fclose(pFile);
		}
		sleep(1);
	}
	
#endif // !WIN32
	
}
