#ifndef __NET_SOCKTOOL_H__
#define __NET_SOCKTOOL_H__
#include <iostream>
#include <string.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <unistd.h>
#include <assert.h>

#define LISTENQ 1024

class gUtils {
public:
  static int createListenFd(int port = 20000) {
    port = ((port <= 1024) || (port >= 65535)) ? 20000 : port;
    int listen_sock = ::socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK, 0);
    if (listen_sock < 0) {
      std::cout << __FILE__ << " : "
                << "__LINE__"
                << "err" << std::endl;
      return -1;
    }

    int opt = 1;
    ::setsockopt(listen_sock, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    struct sockaddr_in local;
    ::bzero((char *)&local, sizeof(local));
    local.sin_family = AF_INET;
    local.sin_port = ::htons(port);
    local.sin_addr.s_addr = ::htonl(INADDR_ANY);
    socklen_t len = sizeof(local);

    if (::bind(listen_sock, (struct sockaddr *)&local, len) < 0) {
      std::cout << __FILE__ << " : " << __LINE__ << "err" << std::endl;
      return -1;
    }

    if (::listen(listen_sock, LISTENQ) < 0) {
      std::cout << __FILE__ << " : " << __LINE__ << "err" << std::endl;
      return -1;
    }

    return listen_sock;
  }

  static int setNonBlocking(int fd) {
    int flag = ::fcntl(fd, F_GETFL, 0);
    if (flag == -1) {
      std::cout << __FILE__ << " : " << __LINE__ << "err" << std::endl;
      return -1;
    }
    flag |= O_NONBLOCK;
    if (::fcntl(fd, F_SETFL, flag) == -1) {
      std::cout << __FILE__ << " : " << __LINE__ << "err" << std::endl;
      return -1;
    }

    return 0;
  }

  static std::string urlDecode(std::string &str) {
    std::string strtmp = "";
    size_t length = str.length();
    size_t i = 0;
    for (; i < length; i++) {
      if (str[i] == '+')
        strtmp += ' ';
      else if (str[i] == '%') {
        assert(i + 2 < length);
        unsigned char high = FromHex((unsigned char)str[++i]);
        unsigned char low = FromHex((unsigned char)str[++i]);
        strtmp += high * 16 + low;
      } else {
        strtmp += str[i];
      }
    }
    return strtmp;
  }

private:
  static unsigned char FromHex(unsigned char x) {
    unsigned char y = 0;
    if (x >= 'A' && x <= 'Z') {
      y = x - 'A' + 10;
    } else if (x >= 'a' && x <= 'z') {
      y = x - 'a' + 10;
    } else if (x >= '0' && x <= '9') {
      y = x - '0';
    } else
      assert(0);

    return y;
  }
};

#endif
