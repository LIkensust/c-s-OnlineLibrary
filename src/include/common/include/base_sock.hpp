#pragma once

#include "common.h"
class BaseSock {
public:
  BaseSock() : sockfd_(-1) {}
  virtual ~BaseSock() {}
  int get_sockfd() { return sockfd_; }

protected:
  int sockfd_;
};
