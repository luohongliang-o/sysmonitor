#pragma once
//#include "sys_config.h"
#ifdef WIN32
#include "ado2.h"
#include <afxmt.h>
#endif // WIN32

#include "load_config.h"

class CLoadConfig;
class CLinkManager
{
public:
	CLinkManager(CLoadConfig* loadconfig);
	~CLinkManager();

public:
	BOOL		Init();
	BOOL		Exit();
	LPOPLINK	GetLink(char *lpszErrMsg, int nSize, int dbsel, BOOL bPrimary = FALSE);// ȡ������(���ﴦ��)(����NULLΪʧ��,��Ӧ������Ϣ������lpszErrMsg��)
	void		FreeLink(LPOPLINK pLink);
	void		DisConnect(LPOPLINK pLink);
	void		CheckConnect(LPOPLINK pLink);

protected:
	BOOL		_Init(LPOPLINK pLinkInfo);
	BOOL		_Connect2(LPOPLINK pLink,int dbsel, BOOL bFailLog = TRUE);
	BOOL		ConnectDataBase(LPDBCONFIG lpdbConfig, CADODatabase* pAdoDb);
protected:
	short			m_nDefaultSel;				// Ĭ�����ݿ�
	short			m_nDbCount;					// ���ݿ����
	DBCONFIG		m_dbConfig[MAX_DBCOUNT];
	long				m_nFailCount;
	CCriticalSection	m_csLink;
	CLoadConfig*        m_pload_config;
	OPLINK			    m_aLinkInfo[MAX_LINK_NUM];	// ���ݿ����ӳ�
};
