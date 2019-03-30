#pragma once
#include "common.h"
class BaseSock {
public:
  BaseSock() : sockfd_(-1), sock_is_open_(false) {}
  virtual ~BaseSock() {
    if (sock_is_open_) {
      close(sockfd_);
    }
  }
  int get_sockfd() { return sockfd_; }
  virtual int write_to_sock(std::shared_ptr<char> src, const size_t size) = 0;
  virtual int read_from_sock(std::shared_ptr<char> dir, size_t buffer_size) = 0;
//  void set_nonblock() {
//    ASSERT_MSG(sock_is_open_ == true, "sock is not open when set nonblock");
//    int opt = fcntl(sockfd_, F_GETFL);
//    ASSERT_MSG(opt >= 0, "set nonblock fail");
//    opt |= O_NONBLOCK;
//    ASSERT_MSG(fcntl(sockfd_, F_SETFL, opt) >= 0, "set nonblock fail");
//  }
//  void set_block() {
//    ASSERT_MSG(sock_is_open_ == true, "sock is not open when set block");
//    int opt = fcntl(sockfd_, F_GETFL);
//    ASSERT_MSG(opt >= 0, "set nonblock fail");
//    opt &= ~O_NONBLOCK;
//    ASSERT_MSG(fcntl(sockfd_, F_SETFL, opt) >= 0, "set block fail");
//  }

protected:
  int sockfd_;
  bool sock_is_open_;
};
