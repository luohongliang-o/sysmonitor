#pragma once
#include "load_config.h"
#ifdef WIN32
#include "ado2.h"
#include <afxmt.h>


typedef struct tagOPLINK
{
	BOOL	is_busy;						// 是否使用中
	short	cur_sel;						// 选择哪个数据库
	BOOL	is_connected;					// 是否连接
	long	busy_time;					// 开始忙时间点
	CADODatabase* ado_db;
} OPLINK, *LPOPLINK;

class CLinkManager
{
public:
	CLinkManager();
	~CLinkManager();

public:
	BOOL		Init();
	BOOL		Exit();
	LPOPLINK	GetLink(char *lpszErrMsg, int nSize, int dbsel, BOOL bPrimary = FALSE);// 取得连接(事物处理)(返回NULL为失败,相应错误信息保存在lpszErrMsg中)
	void		FreeLink(LPOPLINK pLink);
	void		DisConnect(LPOPLINK pLink);
	void		CheckConnect(LPOPLINK pLink);

protected:
	BOOL		_Init(LPOPLINK pLinkInfo);
	BOOL		_Connect2(LPOPLINK pLink,int dbsel, BOOL bFailLog = TRUE);
	BOOL		ConnectDataBase(LPDBCONFIG lpdbConfig, CADODatabase* pAdoDb);
protected:
	short			m_nDefaultSel;				// 默认数据库
	short			m_nDbCount;					// 数据库个数
	DBCONFIG		m_dbConfig[MAX_DBCOUNT];
	long				m_nFailCount;
	CCriticalSection	m_csLink;
	OPLINK			    m_aLinkInfo[MAX_LINK_NUM];	// 数据库连接池
};
#endif // WIN32