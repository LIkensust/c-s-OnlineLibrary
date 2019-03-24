#ifndef __HEAD_H__
#define __HEAD_H__
// C++ head
#include <iostream>
#include <memory>
#include <string>
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

#endif //__HEAD_H__
