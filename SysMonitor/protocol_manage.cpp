#include "protocol_manage.h"
#include "func.h"
#include "load_config.h"
#define  CHECK_WORD "sysmonitor"
#define  CHECK_ERROR "check error."
#define  OBJECT_NUM 6
CProtocolManage::CProtocolManage()
{
	m_bCheck = false;
	m_list_buf = list< string >(NULL);
	m_log_flag = CLoadConfig::CreateInstance()->get_log_flag();
	vector< short > object_type = CLoadConfig::CreateInstance()->get_object_type();
	m_build_monitor = new CBuildMonitor;
	for (int i = 0; i < OBJECT_NUM;i++){
		m_build_monitor->ConcreteMonitor(i,object_type[i]);
	}

}

CProtocolManage::~CProtocolManage()
{
	TDEL(m_build_monitor);
	m_list_buf.clear();
}

int 
CProtocolManage::read(int fd, char *buf)
{
	//checkuser
	string check_word = buf;
	if (check_word == CHECK_WORD){
		if (!m_bCheck){
			m_bCheck = true;
		}
		if (m_bCheck)
			return get_last_buf(buf);
	}
	else{
		memcpy(buf, CHECK_ERROR, strlen(CHECK_ERROR)+1);
		return strlen(CHECK_ERROR)+1;
	}
	return 0;
}

int CProtocolManage::get_last_buf(char* buf)
{
	//rset_list_buf(5);
	if (m_list_buf.size() == 0) {
		*buf = NULL;
		return 0;
	}
	list< string >::iterator ifirst_buf = m_list_buf.begin();
	if (ifirst_buf == m_list_buf.end()){
		*buf = NULL;
		return 0;
	}
	string first_buf = *ifirst_buf;
	memcpy(buf, first_buf.c_str(), first_buf.length()+1 );
	m_list_buf.pop_front();
	return first_buf.length()+1;
}

void CProtocolManage::rset_list_buf(int limit_size)
{
	int list_size = m_list_buf.size();
	int remainder_size = 5;
	int pop_len = list_size - remainder_size;
	if (list_size >= limit_size && list_size > remainder_size){
		for (int i = 0; i < pop_len; i++){
			m_list_buf.pop_front();
		}
	}
}

int CProtocolManage::write(int fd)
{
	rset_list_buf(10);
	string strJsonData;
	FastWriter temp_inswrite;
	
	Value last_json_value;
	vector< short > object_type = CLoadConfig::CreateInstance()->get_object_type();
	for (int i = 0; i < OBJECT_NUM; i++){
		Value json_value;
		CMonitorSystem* monitorsys = m_build_monitor->get_monitor_obj(i);
		if (monitorsys){
			monitorsys->write(fd, json_value);
			if (json_value.isNull()){
				Reader temp_read;
				Value temp_value;
				temp_read.parse("[]", temp_value);
				last_json_value.append(temp_value);
			}
			else
				last_json_value.append(json_value);	
		}
		else{
			Reader temp_read;
			Value temp_value;
			temp_read.parse("[]", temp_value);
			last_json_value.append(temp_value);
		}
	}
	strJsonData = temp_inswrite.write(last_json_value);
	strJsonData.replace(strJsonData.rfind("\n"), strJsonData.rfind("\n"), "");
	m_list_buf.push_back(strJsonData);
	WriteLog(m_log_flag, LOGFILENAME, "write data---%s", strJsonData.c_str());
	
	return strJsonData.length()+1;
}
