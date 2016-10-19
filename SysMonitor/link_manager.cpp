#include "link_manager.h"
#include "sys_config.h"

CLinkManager::CLinkManager(CLoadConfig* loadconfig)
{
	m_pload_config = loadconfig;
	memset(m_aLinkInfo, 0, sizeof(OPLINK)*MAX_LINK_NUM);
	m_nFailCount = 0;
	//::CoInitialize(NULL);
}

CLinkManager::~CLinkManager()
{
	//::CoUninitialize();
	Exit();
}

BOOL CLinkManager::_Init(LPOPLINK pLinkInfo)
{
	if (!pLinkInfo)
		return FALSE;
	pLinkInfo->cur_sel = m_nDefaultSel;
	TDEL(pLinkInfo->ado_db);
	pLinkInfo->is_connected = FALSE;
	pLinkInfo->is_busy = FALSE;
	pLinkInfo->ado_db= new CADODatabase();
	if (!pLinkInfo->ado_db)
		return FALSE;
	return TRUE;
}

// 初始化
BOOL CLinkManager::Init()
{
	m_nDbCount = m_pload_config->get_db_count();
	m_nDefaultSel = m_pload_config->get_db_default_sel();
	if (m_nDefaultSel < 0 || m_nDefaultSel >= m_nDbCount) m_nDefaultSel = 0;
	memcpy(m_dbConfig, m_pload_config->get_db_config(),sizeof(DBCONFIG)*m_nDbCount);

	for (DWORD dwLinkIndex = 0; dwLinkIndex < MAX_LINK_NUM; dwLinkIndex++){
		_Init(&m_aLinkInfo[dwLinkIndex]);
	}
	return TRUE;
}

// 退出
BOOL CLinkManager::Exit()
{
	for (DWORD dwLinkIndex = 0; dwLinkIndex < MAX_LINK_NUM; dwLinkIndex++)
	{
		TDEL(m_aLinkInfo[dwLinkIndex].ado_db);
	}
	return TRUE;
}

LPOPLINK CLinkManager::GetLink(char *lpszErrMsg, int nSize, int dbsel,BOOL bPrimary)
{
	if (dbsel >= MAX_DBCOUNT){
		sprintf_s(lpszErrMsg, nSize, "连接的第[%d]个数据库已超出当前数据库个数范围.", dbsel);
		return NULL;
	}
	if (bPrimary) dbsel = m_nDefaultSel;
	DWORD	dwLinkIndex = 0;
	DWORD	dwFirstFreeIndex = MAX_LINK_NUM;
	LPOPLINK pLinkInfo = NULL;
	m_csLink.Lock();
	for (dwLinkIndex = 0; dwLinkIndex < MAX_LINK_NUM; dwLinkIndex++){
		if (!m_aLinkInfo[dwLinkIndex].is_busy){
			if (dwFirstFreeIndex >= MAX_LINK_NUM) dwFirstFreeIndex = dwLinkIndex;
			if (!bPrimary)	break;
			if (m_aLinkInfo[dwLinkIndex].cur_sel == m_nDefaultSel) break;
		}
	}
	if (dwLinkIndex >= MAX_LINK_NUM){
		if (dwFirstFreeIndex >= MAX_LINK_NUM){
			if (lpszErrMsg)
				strncpy(lpszErrMsg, "后台繁忙,请稍后再试", nSize);
			m_csLink.Unlock();
			return NULL;
		}else
			pLinkInfo = &m_aLinkInfo[dwFirstFreeIndex];
		
	}
	else
		pLinkInfo = &m_aLinkInfo[dwLinkIndex];
	pLinkInfo->is_busy = TRUE;
	m_csLink.Unlock();
	//连接默认数据库
	if (bPrimary){
		if (pLinkInfo->cur_sel != m_nDefaultSel){
			pLinkInfo->ado_db->Close();
			pLinkInfo->is_connected = FALSE;
		}
		if (!pLinkInfo->is_connected){
			if (ConnectDataBase(&m_dbConfig[m_nDefaultSel], pLinkInfo->ado_db)){
				pLinkInfo->is_connected = TRUE;
				pLinkInfo->cur_sel = m_nDefaultSel;
				if (m_nFailCount > 0){
					char szError[256] = { 0 };
					sprintf_s(szError, 256, "连接[%s]-%s 数据库成功", m_dbConfig[m_nDefaultSel].data_source, m_dbConfig[m_nDefaultSel].data_base);
					InterlockedExchange(&m_nFailCount, 0);
				}
			}
		}
	}
	if (pLinkInfo->is_connected){
		pLinkInfo->busy_time = (long)time(NULL);
		return pLinkInfo;
	}

	if (bPrimary){
		char szError[256] = { 0 };
		sprintf_s(szError, 256, "连接[%s]-%s 数据库失败", m_dbConfig[m_nDefaultSel].data_source, m_dbConfig[m_nDefaultSel].data_base);
		if (lpszErrMsg)
			strncpy(lpszErrMsg, "连接数据库失败!", nSize);
		InterlockedIncrement(&m_nFailCount);
		// 连接未成功
		m_csLink.Lock();
		pLinkInfo->is_busy = FALSE;
		m_csLink.Unlock();
		return NULL;
	}

	if (_Connect2(pLinkInfo, dbsel)){
		if (m_nFailCount > 0){
			long nSel = pLinkInfo->cur_sel;
			char szError[256] = { 0 };
			sprintf_s(szError, 256, "连接[%s]-%s 数据库成功", m_dbConfig[nSel].data_source, m_dbConfig[nSel].data_base);
			InterlockedExchange(&m_nFailCount, 0);
		}
		return pLinkInfo;
	}

	InterlockedIncrement(&m_nFailCount);

	// 连接都未成功
	m_csLink.Lock();
	pLinkInfo->is_busy = FALSE;
	m_csLink.Unlock();

	if (lpszErrMsg)
		strncpy(lpszErrMsg, "连接数据库失败!", nSize);

	return NULL;
}


// 释放连接
void CLinkManager::FreeLink(LPOPLINK pLink)
{
	if (!pLink) return;

	m_csLink.Lock();
	pLink->busy_time = 0;
	pLink->is_busy = FALSE;
	m_csLink.Unlock();
}

// 断开连接
void CLinkManager::DisConnect(LPOPLINK pLink)
{
	if (!pLink) return;
	pLink->ado_db->Close();
	pLink->is_connected = FALSE;

	m_csLink.Lock();
	pLink->busy_time = 0;
	pLink->is_busy = FALSE;
	m_csLink.Unlock();
}

void CLinkManager::CheckConnect(LPOPLINK pLink)
{
	if (!pLink)
		return;
	if (!pLink->ado_db){
		pLink->is_connected = FALSE;
		return;
	}
	pLink->ado_db->Close();
	pLink->is_connected = FALSE;
}

BOOL CLinkManager::_Connect2(LPOPLINK pLink, int dbsel,BOOL bFailLog)
{
	if (!pLink)
		return FALSE;
	pLink->ado_db->Close();
	if (ConnectDataBase(&m_dbConfig[dbsel], pLink->ado_db)){
		pLink->is_connected = TRUE;
		pLink->cur_sel = dbsel;
		pLink->busy_time = (long)time(NULL);
		return TRUE;
	}else{
		if (bFailLog){
			char szError[256] = { 0 };
			sprintf_s(szError, 256, "连接[%s]-%s 数据库失败", m_dbConfig[dbsel].data_source, m_dbConfig[dbsel].data_base);
		}
	}
	return FALSE;
}

BOOL CLinkManager::ConnectDataBase(LPDBCONFIG lpdbConfig, CADODatabase* pAdoDb)
{
	if (pAdoDb == NULL) return FALSE;
	pAdoDb->Close();
	CString	strConnect;
	strConnect.Format("Provider=sqloledb;Data Source=%s;Network Library=DBMSSOCN;Initial Catalog=%s;", \
		lpdbConfig->data_source, lpdbConfig->data_base);
	pAdoDb->SetConnectionTimeout();
	pAdoDb->Open(strConnect, lpdbConfig->user_name, lpdbConfig->password);
	return pAdoDb->IsOpen();
}

