#pragma once
#include "../common/common.h"

class RedisTool {
public:
  static std::unique_ptr<RedisTool> make() {
    return std::unique_ptr<RedisTool>(new RedisTool);
  }

  bool do_connect(std::string config_path) {
    int redis_cnf = open(config_path.c_str(),O_RDONLY,0644);
    if(redis_cnf < 0) {
      return false;
    }
    char buff[1000] = {0};
    read(redis_cnf,buff,sizeof(buff));
    neb::CJsonObject oJson(buff);
    std::string ip;
    int port;
    oJson.Get("ip",ip);
    oJson.Get("port",port);
    redis_sock_ = redisConnect(ip.c_str(),port);
    if(redis_sock_->err) {
      redisFree(redis_sock_);
      return false;
    }
    is_connected_ = true;
    return true;
  }

  ~RedisTool() {
    if(reply_ != NULL) {
      freeReplyObject(reply_);
    }
    if(is_connected_ == true) {
      redisFree(redis_sock_);
    }
  }
    
  bool is_connected() {
    return is_connected_;
  }

  bool interaction(std::string code,int type) {
    if(is_connected_ == false) {
      return false;
    }
    if(reply_ != NULL) {
      freeReplyObject(reply_);
      reply_ = NULL;
    }
    reply_ = redisCommand(redis_sock_,code.c_str());
    if(static_cast<redisReply*>(reply_)->type != type) {
      freeReplyObject(reply_);
      return false;
    }
    return true;
  } 

  template<typename T>
    T& get_slution(T& t,int type) {
      switch (type) {
      case REDIS_REPLY_STRING:
        t = static_cast<redisReply*>(reply_)->str;
        break;
      }
    }

private:
  RedisTool()
    :is_connected_(false),
    redis_sock_(NULL),
    reply_(NULL)
  {}
  bool is_connected_;
  redisContext *redis_sock_;
  void* reply_;
};
