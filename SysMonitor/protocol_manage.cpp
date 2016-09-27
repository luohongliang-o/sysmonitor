#include "protocol_manage.h"
#include "func.h"


CProtocolManage::CProtocolManage(CLoadConfig* loadconfg)
{
	m_bCheck = false;
	m_load_config = loadconfg;
	int object_num = m_load_config->get_object_num();
	short* object_type = m_load_config->get_object_type();
}

CProtocolManage::~CProtocolManage()
{
}

int 
CProtocolManage::read(int fd, char *buf)
{
	string strJsonData;
	FastWriter temp_inswrite;
	Value temp_instance_value;
	Reader temp_instance_read;
	
	if (!temp_instance_read.parse(buf, temp_instance_value)){
		string erromsg = temp_instance_read.getFormattedErrorMessages();
		temp_instance_value["erromsg"] = erromsg.c_str();
		goto checkerr;
	}
	//checkuser
	if (!m_bCheck)
	{
		string user_name;
		Value json_value;
		user_name = temp_instance_value["username"].asString();
		if (user_name == m_load_config->get_check_user_name()){
			m_bCheck = true;
		}
		json_value["bcheck"] = m_bCheck;
		json_value["erromsg"] = "checkuser success.";
		strJsonData = temp_inswrite.write(json_value);
		memcpy(buf, strJsonData.c_str(), strJsonData.length() + 1);
		return strJsonData.length() + 1;
	}
	if (!m_bCheck){
		temp_instance_value["erromsg"] = "checkuser unsuccessfull.";
		goto checkerr;
	}
	if (m_bCheck){//chcek success and read data to client

		return get_last_buf(buf);
	}
	return 0;
checkerr:
	temp_instance_value["bcheck"] = m_bCheck;
	strJsonData = temp_inswrite.write(temp_instance_value);
	memcpy(buf, strJsonData.c_str(), strJsonData.length() + 1);
	return strJsonData.length()+1;
	
}

int CProtocolManage::get_last_buf(char* buf)
{
	rset_list_buf(5);
	list< string >::iterator ifirst_buf = m_list_buf.begin();
	if (ifirst_buf == m_list_buf.end()){
		buf = "";
		return 0;
	}
	string first_buf = *ifirst_buf;
	memcpy(buf, first_buf.c_str(), first_buf.length() + 1);
	m_list_buf.pop_front();
	return first_buf.length() + 1;
}

void CProtocolManage::rset_list_buf(int listsize)
{
	int pop_len = m_list_buf.size();
	if (pop_len >= listsize){
		for (int i = 0; i < pop_len - 2; i++){
			m_list_buf.pop_front();
		}
	}
}

int CProtocolManage::write(int fd)
{
	rset_list_buf(10);
	string strJsonData;
	FastWriter temp_inswrite;
	int object_num = m_load_config->get_object_num();
	short* object_type = m_load_config->get_object_type();
	Value last_json_value;
	last_json_value["typenum"] = object_num;
	char* buf = new char[1024 * 4];
	for (int i = 0; i < object_num; i++){
		Value json_value;
		CBuildMonitor build_monitor;
		build_monitor.ConcreteMonotor(object_type[i], m_load_config);
		CMonitorSystem* monitorsys = build_monitor.get_monitor_obj();
		monitorsys->write(fd, buf);
		json_value["type"] = object_type[i];
		json_value["data"] = buf;
		last_json_value["data"].append(json_value);
	}
	if (buf){
		delete buf;
		buf = NULL;
	}

	strJsonData = temp_inswrite.write(last_json_value);
	m_list_buf.push_back(strJsonData);
	return strJsonData.length() + 1;
}
