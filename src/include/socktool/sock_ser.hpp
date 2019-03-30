#pragma once
#include "../common/common.h"
#include "../common/include/base_sock.hpp"
#include "../common/include/server.h"
enum LISTENSOCKSTATUS { SETIP = 1 << 0, SETPORT = 1 << 1 };
enum ERRTYPE { CREATESOCK = -1, BIND = -2, LISTEN = -3, OK = 0 };
enum SERVERCONNECTSOCKSTATUS { SETSOCK, EMPTYSOCK };
class ListenSockTool : public BaseSock {
public:
  static std::unique_ptr<ListenSockTool> make() {
    return std::unique_ptr<ListenSockTool>(new ListenSockTool);
  }

  ListenSockTool &set_ip(const std::string &ip) {
    ip_ = ip;
    status_ |= SETIP;
    return *this;
  }

  ListenSockTool &set_port(const short port) {
    port_ = port;
    status_ |= SETPORT;
    return *this;
  }

  int open_sock() {
    if (!(status_ & SETIP)) {
      ip_ = "127.0.0.1";
    }
    if (!(status_ & SETPORT)) {
      port_ = SERVERPORT;
    }
    sockfd_ = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd_ == -1) {
      return CREATESOCK;
    }
    set_nonblock(sockfd_);
    sockaddr_in host_sock;
    bzero(&host_sock, sizeof(sockaddr_in));
    host_sock.sin_family = AF_INET;
    host_sock.sin_port = htons(port_);
    host_sock.sin_addr.s_addr = inet_addr(ip_.c_str());
    int bind_status = bind(sockfd_, reinterpret_cast<sockaddr *>(&host_sock),
                           sizeof(sockaddr));
    if (bind_status == -1) {
      return BIND;
    }
    sock_is_open_ = true;
    return OK;
  }

  int start_listen() {
    if (listen(sockfd_, 500) == -1) {
      return LISTEN;
    }
    return OK;
  }

  int do_accept(std::shared_ptr<sockaddr> addr,socklen_t *len = NULL) {
    socklen_t sockaddr_size;
    socklen_t *ptr = (len == NULL)?(&sockaddr_size):len; 
    return accept(sockfd_, reinterpret_cast<sockaddr *>(addr.get()), ptr); 
  }

  ~ListenSockTool() {
    if (sock_is_open_ == true)
      close(sockfd_);
  }

protected:
  virtual int write_to_sock(std::shared_ptr<char> src, const size_t size) {
    return -1;
  };
  virtual int read_from_sock(std::shared_ptr<char> dir, size_t buffer_size) {
    return -1;
  };
  ListenSockTool() : status_(0), port_(0) {}
  int status_;
  std::string ip_;
  short port_;
};

class ServerConnetSockTool : public BaseSock {
public:
  ServerConnetSockTool() : status_(EMPTYSOCK) {}
  ServerConnetSockTool(int sock) {
    sockfd_ = sock;
    sock_is_open_ = true;
    status_ = SETSOCK;
  }
  void set_sockfd(int sock) {
    ASSERT_MSG(sock >= 0, "sock is less then 0 when set sockfd");
    sock_is_open_ = true;
    sockfd_ = sock;
    status_ = SETSOCK;
  }

  int write_to_sock(std::shared_ptr<char> src, const size_t size) {
    ASSERT_MSG(src != NULL, "src is NULL when writing");
    ASSERT_MSG(sock_is_open_ == true, "sock is not open when wirting");
    size_t tmp;
    size_t total = size;
    const char *ptr = src.get();
    while (total > 0) {
      tmp = write(sockfd_, ptr, total);
      if (tmp < 0) {
        if (tmp == 0) {
          return -2;
        }
        if (errno == EINTR)
          return -1;
        if (errno == EAGAIN) {
          usleep(500);
          continue;
        }
      }
      if (tmp == total)
        return size;
      total -= tmp;
      ptr += tmp;
    }
    return tmp;
  }

  int read_from_sock(std::shared_ptr<char> dir, size_t buffer_size) {
    char *buff = dir.get();
    ASSERT_MSG(buff == NULL, "buff is point to NULL when read");
    int index = 0;
    int read_size = 0;
    char *ptr = buff;
    while (buffer_size > 1) {
      read_size = read(sockfd_, ptr + index, buffer_size - 1);
      index += read_size;
      buffer_size -= read_size;
      if (read_size == 0) {
        return -2;
      } else if (read_size == -1 && errno != EAGAIN) {
        return -2;
      }
    }
    return index;
  }

protected:
  int status_;
};
