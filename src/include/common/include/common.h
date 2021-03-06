#pragma once
// C++ head
#include <functional>
#include <iostream>
#include <memory>
#include <mutex>
#include <queue>
#include <string>
#include <thread>
#include <utility>
#include <vector>
// C head
#include <arpa/inet.h>
#include <assert.h>
#include <fcntl.h>
#include <mysql/mysql.h>
#include <netinet/in.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/epoll.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
// thirdpart
#include "../../CJsonObject/CJsonObject.hpp"

#define ASSERT_MSG(op, msg)                                                    \
  do {                                                                         \
    if (!(op)) {                                                               \
      std::cout << "In file:" << __FILE__ << std::endl;                        \
      std::cout << "Line :" << __LINE__ << std::endl;                          \
      std::cout << msg << std::endl;                                           \
      _exit(1);                                                                \
    }                                                                          \
  } while (0)

#define IPREGEX                                                                \
  "^([0-9]|[1-9][0-9]|1[0-9]{1,2}|2[0-4][0-9]|25[0-5]).([0-9]|[1-9][0-9]|1[0-" \
  "9]{1,2}|2[0-4][0-9]|25[0-5]).([0-9]|[1-9][0-9]|1[0-9]{1,2}|2[0-4][0-9]|25[" \
  "0-5]).([0-9]|[1-9][0-9]|1[0-9]{1,2}|2[0-4][0-9]|25[0-5])$"

void set_nonblock(int fd) {
  int opt = fcntl(fd, F_GETFL);
  ASSERT_MSG(opt >= -1, "set nonblock fail:option is less than 0");
  opt |= O_NONBLOCK;
  ASSERT_MSG(fcntl(fd, F_SETFL, opt) >= 0, "set nonblock fail");
}
