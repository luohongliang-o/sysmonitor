#include "protocol_manage.h"
#include "func.h"


CProtocolManage::CProtocolManage(CLoadConfig* loadconfg)
{
	m_bCheck = false;
	m_load_config = loadconfg;
	int object_num = m_load_config->get_object_num();
	short* object_type = m_load_config->get_object_type();
	m_bget_systeminfo = false;
	
	get_global_info();
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
		if (user_name == m_load_config->get_check_user_name())
			m_bCheck = true;
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
		if (monitorsys){
			monitorsys->write(fd, json_value);
			//兼容处理linux 系统版本信息
			json_value["type"] = object_type[i];
			if (object_type[i] == build_monitor.MONITORTYPE_LINUX_SYSINFO){
				Value temp_json_value;
				temp_json_value[OS_NAME] = json_value[OS_NAME];
				temp_json_value[OS_VERSION] = json_value[OS_VERSION];
				last_json_value["global"].append(temp_json_value);
				json_value.removeMember(OS_NAME);
				json_value.removeMember(OS_VERSION);
			}
			last_json_value["data"].append(json_value);
		}else{
			json_value["type"] = object_type[i];
			last_json_value["data"].append(json_value);
		}
	}
	TDELARRAY(buf);
	get_global_info();
	if (is_init_global_info()){
		Value json_value;
		json_value[OS_NAME] = m_os_name;
		json_value[OS_VERSION] = m_os_version;
		last_json_value["global"].append(json_value);
	}
	
	strJsonData = temp_inswrite.write(last_json_value);
	m_list_buf.push_back(strJsonData);
	return strJsonData.length() + 1;
}

void CProtocolManage::get_global_info()
{
#ifdef WIN32
	if (!m_bget_systeminfo){
		FILE *ppipe = NULL;
		char* pbuffer = new char[1000];
		int nread_line = 0;
		int len = 0;
		ppipe = _popen("systeminfo /FO CSV /NH ", "rt");
		fgets(pbuffer, 1000, ppipe);
		char* tembufer = pbuffer;
		while (nread_line < 3){
			char* tempstr = strchr(tembufer, ',');
			len = tempstr - tembufer;
			char* tempvalue = new char[len + 1];
			strncpy(tempvalue, tembufer, len);
			tempvalue[len] = '\0';
			if (nread_line == 1)
				strcpy_s(m_os_name, tempvalue);
			else if (nread_line == 2)
				strcpy_s(m_os_version, tempvalue);
			TDELARRAY(tempvalue);
			tembufer = tembufer + len + 1;
			nread_line++;
		}
		TDELARRAY(pbuffer);
		if (feof(ppipe))
			_pclose(ppipe);
		m_bget_systeminfo = true;
	}
#endif // WIN32
}
bool CProtocolManage::is_init_global_info()
{
	return m_bget_systeminfo;
}