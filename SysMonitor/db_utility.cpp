/*
  db utility
*/

#include "db_utility.h"

/*****************************************************************************/
/* connect string

*/

#define DBUG_ENTER(x)
#define DBUG_ASSERT     ASSERT
#define DBUG_RETURN     return

#undef  STRSTR
#define STRSTR(s,l,f)                strstr(s, f)

#undef  STRCHR
#define STRCHR(s,l,c)   strchr(s,c)
#define STRLWR          _strlwr_s

static rc_t get_param_value(uint32_t* pos, uint32_t* len, const int8_t* str, const int8_t* param) {

  const int8_t* pdest;

  DBUG_ENTER("get_param_value");
  if (NULL == pos || NULL == len || NULL == str || NULL == param) { return RC_S_NULL_VALUE; }

  pdest = strstr((const char*)str,  (char*)param);
  if (NULL == pdest || str == pdest || ' ' != (*(pdest - 1))) { DBUG_RETURN(RC_S_NOTFOUND); }

  (*pos) = (uint32_t)(pdest - str) + 1 + STRLEN(param) + 1; /* " */

  /* len */
  pdest = str + (*pos);
  pdest = STRCHR(pdest, STRLEN(pdest), '"');
  if (NULL == pdest) { DBUG_RETURN(RC_S_NOTFOUND); }

  (*len) = (uint32_t)(pdest - str) + 1 - (*pos) - 1;  /* " */
  DBUG_RETURN(RC_S_OK);
}

rc_t db_parse_cs(db_conn_str_t* db_cs, const int8_t* str) {

  uint32_t value_pos;
  uint32_t value_len;

  char_t lowercase_cs_buf[MAX_DB_CS_LEN] = {0};
  const char_t* strLCS = lowercase_cs_buf;

  DBUG_ENTER("db_parse_cs");
  DBUG_ASSERT(db_cs);
  DBUG_ASSERT(str);

  DBUG_ASSERT(DB_CS_LAST + 1 == sizeof(db_cs_param) / sizeof(const char_t*));

  if (MAX_DB_CS_LEN < STRLEN(str)) { DBUG_RETURN(RC_E_NOMEM); }

  /* init */
  db_cs->cs_name        = db_cs_default_value;
  db_cs->db_driver      = db_cs_default_value;
  db_cs->db_source      = db_cs_default_value;
  db_cs->db_name        = db_cs_default_value;
  db_cs->db_user        = db_cs_default_value;
  db_cs->db_pwd         = db_cs_default_value;
  db_cs->db_charset     = db_cs_default_charset;
  db_cs->db_host        = db_cs_default_value;

  db_cs->db_port        = db_default_value;
  db_cs->db_flag        = db_flag_default_value;
  db_cs->db_socket      = db_cs_default_value;

  db_cs->db_slave_id    = db_default_value;
  db_cs->db_keep_alive  = db_default_value;
  db_cs->db_max_conn    = db_default_value;
  db_cs->db_base_conn   = db_default_value;

  STRCPY(db_cs->cs_buf, sizeof(db_cs->cs_buf), str);
  //STRCPY(db_cs->cs_org, sizeof(db_cs->cs_buf), str);

  STRCPY(lowercase_cs_buf, sizeof(db_cs->cs_buf), db_cs->cs_buf);
  STRLWR(lowercase_cs_buf, sizeof(lowercase_cs_buf));

  /* name */
  if (RC_S_OK == get_param_value(&value_pos, &value_len, strLCS, db_cs_param[DB_CS_NAME])
    && value_len
    ) {

      db_cs->cs_name  = db_cs->cs_buf + value_pos;
      db_cs->cs_buf[value_pos + value_len] = 0x00;
  }

  /* driver */
  if (RC_S_OK == get_param_value(&value_pos, &value_len, strLCS, db_cs_param[DB_CS_DRIVER])
    && value_len
  ) {

    db_cs->db_driver  = db_cs->cs_buf + value_pos;
    db_cs->cs_buf[value_pos + value_len] = 0x00;
  }

  /* source */
  if (RC_S_OK == get_param_value(&value_pos, &value_len, strLCS, db_cs_param[DB_CS_SOURCE])
    && value_len
  ) {

    const int8_t* pdest;

    db_cs->db_source = db_cs->cs_buf + value_pos;
    db_cs->cs_buf[value_pos + value_len] = 0x00;

    /* get host & port */
    pdest = STRCHR(db_cs->db_source, STRLEN(db_cs->db_source), ',');
    if (pdest) {

      db_cs->db_host = db_cs->db_source;
      db_cs->cs_buf[value_pos + pdest - db_cs->db_source] = 0x00;
      db_cs->db_port = ATOI(db_cs->db_source + (pdest - db_cs->db_source + 1));
    }
  }

  /* name */
  if (RC_S_OK == get_param_value(&value_pos, &value_len, strLCS, db_cs_param[DB_CS_DBNAME])
    && value_len
  ) {

    db_cs->db_name = db_cs->cs_buf + value_pos;
    db_cs->cs_buf[value_pos + value_len] = 0x00;
  }

  /* user */
  if (RC_S_OK == get_param_value(&value_pos, &value_len, strLCS, db_cs_param[DB_CS_DBUSER])
    && value_len
  ) {

    db_cs->db_user = db_cs->cs_buf + value_pos;
    db_cs->cs_buf[value_pos + value_len] = 0x00;
  }

  /* pwd */
  if (RC_S_OK == get_param_value(&value_pos, &value_len, strLCS, db_cs_param[DB_CS_DBPWD])
    && value_len
  ) {

    db_cs->db_pwd = db_cs->cs_buf + value_pos;
    db_cs->cs_buf[value_pos + value_len] = 0x00;
  }

  /* charset */
  if (RC_S_OK == get_param_value(&value_pos, &value_len, strLCS, db_cs_param[DB_CS_DBCHARSET])
    && value_len
  ) {

    db_cs->db_charset = db_cs->cs_buf + value_pos;
    db_cs->cs_buf[value_pos + value_len] = 0x00;
  }

  /* flag */
  if (RC_S_OK == get_param_value(&value_pos, &value_len, strLCS, db_cs_param[DB_CS_DBFLAG])
    && value_len
  ) {

    db_cs->cs_buf[value_pos + value_len] = 0x00;
    db_cs->db_flag = ATOI(db_cs->cs_buf + value_pos);
  }

  /* socket */
  if (RC_S_OK == get_param_value(&value_pos, &value_len, strLCS, db_cs_param[DB_CS_DBSOCKET])
    && value_len
  ) {

    db_cs->db_socket = db_cs->cs_buf + value_pos;
    db_cs->cs_buf[value_pos + value_len] = 0x00;
  }

  /* slave id */
  if (RC_S_OK == get_param_value(&value_pos, &value_len, strLCS, db_cs_param[DB_CS_SLAVEID])
    && value_len
  ) {
    
    db_cs->cs_buf[value_pos + value_len] = 0x00;
    db_cs->db_slave_id = ATOI(db_cs->cs_buf + value_pos);
  }

  /* keep alive */
  if (RC_S_OK == get_param_value(&value_pos, &value_len, strLCS, db_cs_param[DB_CS_KEEPALIVE])
    && value_len
  ) {

    db_cs->cs_buf[value_pos + value_len] = 0x00;
    db_cs->db_keep_alive = ATOI(db_cs->cs_buf + value_pos);
  }

  /* max conn */
  if (RC_S_OK == get_param_value(&value_pos, &value_len, strLCS, db_cs_param[DB_CS_MAXCONN])
    && value_len
  ) {

    db_cs->cs_buf[value_pos + value_len] = 0x00;
    db_cs->db_max_conn = ATOI(db_cs->cs_buf + value_pos);
  }

  /* base conn */
  if (RC_S_OK == get_param_value(&value_pos, &value_len, strLCS, db_cs_param[DB_CS_BASECONN])
    && value_len
  ) {

    db_cs->cs_buf[value_pos + value_len] = 0x00;
    db_cs->db_base_conn = ATOI(db_cs->cs_buf + value_pos);
  }

  /* Compress */
  if (RC_S_OK == get_param_value(&value_pos, &value_len, strLCS, db_cs_param[DB_CS_COMPRESS])
    && value_len
    ) {

      db_cs->cs_buf[value_pos + value_len] = 0x00;
      if (0 == STRCMP(db_cs_value_enable, db_cs->cs_buf + value_pos)) {

        // mysql CLIENT_COMPRESS
#define CLIENT_COMPRESS       32
        db_cs->db_flag |= CLIENT_COMPRESS;
      }
      //db_cs->db_base_conn = ATOI(db_cs->cs_buf + value_pos);
  }

  DBUG_RETURN(RC_S_OK);
}
