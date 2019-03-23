#pragma once
#include "../common/common.h"

enum STATUS {
  SETIP = 1<<0,
  SETPORT = 1<<1
};

enum ERRTYPE {
  CREATESOCK = -1,
  BIND = -2,
  LISTEN = -3,
  OK = 0
};

class ServerSockTool : public BaseSock {
public:
  std::unique_ptr<ServerSockTool> make() {
    return std::unique_ptr<ServerSockTool>(new ServerSockTool);
  }

  ServerSockTool& set_ip(const std::string& ip) {
    ip_ = ip;
    status_ |= SETIP;
    return *this;
  }

  ServerSockTool& set_port(const int port) {
    port_ = port;
    status_ |= SETPORT;
    return *this;
  }

  int open_sock() {
    if(!(status_ & SETIP)) {
      ip_ = "127.0.0.1";
    }
    if(!(status_ &SETPORT)) {
      port_ = SERVERPORT;
    }
    sockfd_ = socket(AF_INET,SOCK_STREAM,0);
    if(sockfd_ == -1) {
      return CREATESOCK;
    }
    sockaddr_in host_sock;
    bzero(&host_sock,sizeof(sockaddr_in));
    host_sock.sin_family = AF_INET;
    host_sock.sin_port = htons(port_);
    host_sock.sin_addr.s_addr = inet_addr(ip_.c_str());
    int bind_status = bind(sockfd_,reinterpret_cast<sockaddr*>(&host_sock),sizeof(sockaddr)); 
    if(bind_status == -1) {
      return BIND;
    }
    return OK;
  }

  int start_listen() {
    if(listen(sockfd_,500) == -1) {
      return LISTEN;
    }
    return OK;
  }

  int do_accept(std::shared_ptr<sockaddr> addr) {
    return accept(sockfd_,reinterpret_cast<sockaddr*>(addr.get()),NULL); 
  }

protected:
  ServerSockTool() : status_(0), port_(0) {}
  int status_;
  std::string ip_;
  short port_;
};
