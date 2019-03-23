#pragma once

#include "common.h"
class BaseSock {
public:
  BaseSock() : sockfd_(-1) {}
  virtual ~BaseSock() {}

protected:
  int sockfd_;
};
