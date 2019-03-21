#pragma once
#include "../common/common.h"

class MysqlTool {
public:
  static std::unique_ptr<MysqlTool> make() {
    return std::unique_ptr<MysqlTool>(new MysqlTool);
  }

  bool do_connect(std::string config_path) {
    int mysql_cnf = open(config_path.c_str(),O_RDONLY,0644);
    if(mysql_cnf < 0) {
      return false;
    }
    char buff[1000] = {0};
    read(mysql_cnf,buff,sizeof(buff));
    neb::CJsonObject oJson(buff);
    std::string host;
    std::string user;
    std::string passwd;
    std::string db;
    int port;
    int unix_sock;
    oJson.Get("host",host);
    oJson.Get("user",user);
    oJson.Get("passwd",passwd);
    oJson.Get("db",db);
    oJson.Get("port",port);
    oJson.Get("unix_socket",unix_sock);
    sql_sock_ = mysql_real_connect(&mysql_,host.c_str(),user.c_str(),passwd.c_str(),
                                   db.c_str(),port,NULL,unix_sock);
    if(sql_sock_ == NULL)
      return false;
    is_connected_ = true;
    return true;
  }

  ~MysqlTool() {
    if(result_ != NULL) {
      mysql_free_result(result_);
    }
    if(sql_sock_ != NULL) {
      mysql_close(sql_sock_);
    }
  }

  bool interactive(std::string code) {
    if(is_connected_ == false) {
      return false;
    }
    int ret = mysql_query(sql_sock_,code.c_str());     
    if(ret == 0) {
      if(result_ != NULL) {
        mysql_free_result(result_);
      }
      result_ = mysql_store_result(&mysql_);
      return true;
    }
    return false;
  }

  MYSQL_ROW get_slution_by_row() {
    MYSQL_ROW row;
    if((row = mysql_fetch_row(result_)) != NULL)
      return row;
    return NULL;
  }

  bool is_connected() {
    return is_connected_;
  }

private:
  MysqlTool()
    :is_connected_(false),
    sql_sock_(NULL),
    result_(NULL)
  {
    mysql_init(&mysql_);
  }

  bool is_connected_;
  MYSQL mysql_;
  MYSQL *sql_sock_;
  MYSQL_RES *result_;
};
