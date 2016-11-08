/*
  db utility
*/

#ifndef DB_UTILITY_
#define DB_UTILITY_

#include "sys_config.h"
#include "error.h"

/*****************************************************************************/
/* connect string

  <Server Name="sql_17_mobile", Driver="sqloledb" Source="192.168.0.17,1433" DBName="mobile" DBUsr="mobileusr" DBPwd="123456" Timeout="10s" BaseConn="20" MaxConn="1000" />
*/
#define MAX_DB_CS_LEN     256

static const char_t* db_cs_default_value    = _STR("");
static const char_t* db_cs_default_charset  = _STR("utf8");
static const char_t* db_cs_value_enable     = _STR("yes");

static const uint32_t db_default_value      = 0;
static const uint32_t db_flag_default_value = 0x00030000; // CLIENT_MULTI_RESULTS | CLIENT_MULTI_STATEMENTS
                                            // CLIENT_COMPRESS

typedef struct st_db_conn_str {

  const char_t* cs_name;

  const char_t* db_driver;
  const char_t* db_source;  /* host */

  const char_t* db_name;
  const char_t* db_user;
  const char_t* db_pwd;

  const char_t* db_charset;

  const char_t* db_host;
  uint32_t      db_port;    /* port */
  uint32_t      db_flag;

  const char_t* db_socket;  /* mysql */

  uint32_t      db_slave_id;

  uint32_t      db_keep_alive; /* second */
  uint32_t      db_max_conn;
  uint32_t      db_base_conn;

  char_t        cs_buf[MAX_DB_CS_LEN];
  //char_t        cs_org[MAX_DB_CS_LEN];

} db_conn_str_t;

typedef enum DB_CONN_STR_E_ {

  DB_CS_NAME        = 0
  , DB_CS_DRIVER
  , DB_CS_SOURCE
  , DB_CS_DBNAME
  , DB_CS_DBUSER
  , DB_CS_DBPWD
  , DB_CS_DBCHARSET
  , DB_CS_DBFLAG
  , DB_CS_DBSOCKET
  , DB_CS_SLAVEID
  , DB_CS_KEEPALIVE
  , DB_CS_MAXCONN
  , DB_CS_BASECONN
  , DB_CS_COMPRESS
  , DB_CS_LAST

} DB_CONN_STR_E;

static const char_t* db_cs_param[] = {

  _STR("name")
  , _STR("driver")
  , _STR("source")
  , _STR("dbname")
  , _STR("dbusr")
  , _STR("dbpwd")
  , _STR("dbcharset")
  , _STR("dbflag")
  , _STR("dbsocket")
  , _STR("slaveid")
  , _STR("keepalive")
  , _STR("maxconn")
  , _STR("baseconn")
  , _STR("compress")
  , NULL
};

static const char_t* db_cs_driver[] = {

  _STR("MYSQLCI")    /* mysql c api */
  , _STR("sqloledb")
  , NULL
};

typedef enum DB_TYPE_E_ {

  DB_MYSQL        = 0
  , DB_SQLSERVER
  , DB_ORACLE
  , DBT_UNKNOWN // ocbc.h

} DB_TYPE_E;

rc_t db_parse_cs(db_conn_str_t* db_cs, const char_t* str);

/*****************************************************************************/

#endif  // DB_UTILITY_
