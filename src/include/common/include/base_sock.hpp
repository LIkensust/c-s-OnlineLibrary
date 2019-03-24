#pragma once
#include "common.h"
enum SOCKSTATUS { NONBLOCK, BLOCK };

class BaseSock {
public:
  BaseSock() : sockfd_(-1), sock_status_(BLOCK), sock_is_open_(false) {}
  virtual ~BaseSock() {
    if (sock_is_open_) {
      close(sockfd_);
    }
  }
  int get_sockfd() { return sockfd_; }
  virtual int write_to_sock(std::shared_ptr<char> src, const size_t size) = 0;
  virtual int read_from_sock(std::shared_ptr<char> dir, size_t buffer_size) = 0;
  void set_nonblock() {
    ASSERT_MSG(sock_is_open_ == true, "sock is not open when set nonblock");
    int options = fcntl(sockfd_, F_GETFL);
    ASSERT_MSG(options >= 0, "set nonblock fail");
    options |= O_NONBLOCK;
    ASSERT_MSG(fcntl(sockfd_, F_SETFL, options) >= 0, "set nonblock fail");
    sock_status_ = NONBLOCK;
  }
  void set_block() {
    ASSERT_MSG(sock_is_open_ == true, "sock is not open when set block");
    int options = fcntl(sockfd_, F_GETFL);
    ASSERT_MSG(options >= 0, "set nonblock fail");
    options &= ~O_NONBLOCK;
    ASSERT_MSG(fcntl(sockfd_, F_SETFL, options) >= 0, "set block fail");
    sock_status_ = BLOCK;
  }

protected:
  int sockfd_;
  int sock_status_;
  bool sock_is_open_;
};
