#pragma once
#include "db_utility.h"
#ifdef WIN32
# include <winsock2.h>
#endif //

#include "mysql.h"


class CCMysqlRecordSet;
//
class CMysqlRecord {
public:
	void Release();

public:
	rc_t get_data(UINT32_T idx, UINT8_T** val, UINT32_T* len);

	rc_t get_data(UINT32_T idx, UCHAR_T** val);
	rc_t get_data(UINT32_T idx, UINT16_T* val);
	rc_t get_data(UINT32_T idx, UINT32_T* val);
	rc_t get_data(UINT32_T idx, UINT64_T* val);

	rc_t get_data(UINT32_T idx, FLOAT_T* val);
	rc_t get_data(UINT32_T idx, DOUBLE_T* val);

public:
	CMysqlRecord();
	~CMysqlRecord();

	void set_mysql_row(MYSQL_ROW row, UINT32_T filed_count, const unsigned long* lengths);

private:
	MYSQL_ROW m_row;
	UINT32_T m_filed_count;
	const unsigned long* m_lengths;

	DISALLOW_COPY_AND_ASSIGN(CMysqlRecord);
}; // CMysqlRecord

//////////////////////////////////////////////////////////////////////////
// RecordSet
class CMysqlRecordSet
{
public:
	void Release();

public:
	rc_t fetch();
	rc_t close();

	rc_t first();
	rc_t last();

	rc_t next();
	rc_t prev();

public:
	rc_t get_field_count(UINT32_T* count);
	rc_t get_field_name(CHAR_T** name, UINT32_T idx);
	rc_t get_field_idx(UINT32_T* idx, const CHAR_T* name);

	rc_t get_row_count(UINT32_T* count);
	rc_t get_row_count(UINT64_T* count);

public:
	CMysqlRecord* get_record();

public:
	CMysqlRecordSet();
	~CMysqlRecordSet();

	void set_mysql_res(MYSQL_RES* res);

private:
	MYSQL_RES*  m_res;
	MYSQL_FIELD *m_fields;
	MYSQL_ROW   m_row;
	UINT64_T    m_curr_row_no;

	// OPT
	unsigned long*   m_lengths;
	UINT64_T    m_row_count;
	UINT32_T    m_filed_count;

	CMysqlRecord m_rec;
	

	DISALLOW_COPY_AND_ASSIGN(CMysqlRecordSet);
}; // CMysqlRecordSet

class CMysqlConnection 
{
public:
	void Release();

public:
	/* connect string */
	rc_t connect(const CHAR_T*);
	rc_t connect(const void*) { return RC_E_UNSUPPORTED; }
	rc_t connect(const db_conn_str_t*);

	rc_t disconnect();
	rc_t reconnect();
	rc_t check_connect(BOOL_T bKeepAlive, BOOL_T bFix);

public:
	rc_t execute(const CHAR_T*, BOOL_T);
	rc_t commit();
	rc_t rollback();

public:
	const CHAR_T* get_last_error(UINT32_T*);

public:
	CMysqlRecordSet* get_record_set();

public:
	static CMysqlConnection* CreateInstance(const CHAR_T* strParam = "");
	static const CHAR_T* m_gcStrName;

	static rc_t db_lib_init();
	static void db_lib_deinit();
	static rc_t db_thd_init();
	static void db_thd_deinit();

public:
	CMysqlConnection();
	~CMysqlConnection();

private:
	rc_t clear_all_result_set();
	void reset_error();
	BOOL_T didConnected();

private:
	rc_t      m_nStatus;

	MYSQL     m_mysql;
	MYSQL_RES* m_myres;

	CMysqlRecordSet  m_RecordSet;

	const CHAR_T*         m_strDB;
	const db_conn_str_t*  m_dbCon;

	DISALLOW_COPY_AND_ASSIGN(CMysqlConnection);
}; // CMysqlConnection