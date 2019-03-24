#include "include/common/include/server.h"
#include "include/common/common.h"
#include "include/socktool/sock_ser.hpp"
using namespace std;
static string ip = SERVERIP;
static short port = SERVERPORT;

void all_works() {
  auto sock_tool = ListenSockTool::make();
  sock_tool->set_ip(ip);
  sock_tool->set_port(port);
  ASSERT_MSG(sock_tool->open_sock() == OK, "open_sock failed");
  ASSERT_MSG(sock_tool->start_listen() == OK, "start_listen failed");
  int listen_sock = sock_tool->get_sockfd();
  set_nonblock(listen_sock);
  struct epoll_event ev, events[50];
  shared_ptr<sockaddr> client_addr(new sockaddr);
  int epoll_fd = epoll_create(500);
  ASSERT_MSG(epoll_fd >= 0, "create epoll fail");

  // release listen_sock
  sock_tool.reset();
}

int main(int argc, char *argv[]) {
  int option;
  bool customize = false;
  while ((option = getopt(argc, argv, "p:l:")) != -1) {
    switch (option) {
    case 'p': {
      auto regex_tool = RegexTool::make();
      regex_tool->set_regex("[0-9]*");
      vector<pair<int, int>> tmp = regex_tool->check_str(optarg);
      if (tmp.empty() || tmp[0].first != 0 ||
          (tmp[0].second != (int)strlen(optarg))) {
        ASSERT_MSG(false, "port must be num and not null");
      }
      port = atoi(optarg);
      customize = true;
      break;
    }
    case 'l': {
      auto regex_tool = RegexTool::make();
      regex_tool->set_regex(IPREGEX);
      vector<pair<int, int>> tmp = regex_tool->check_str(optarg);
      if (tmp.empty() || tmp[0].first != 0 ||
          (tmp[0].second != (int)strlen(optarg))) {
        ASSERT_MSG(false, "ip must be 0.0.0.0 ~ 255.255.255.255");
      }
      ip = optarg;
      customize = true;
      break;
    }
    case '?': {
      cout << "[Using an unknow option]" << endl;
      return -1;
      break;
    }
    }
  }
  if (customize == true) {
    cout << "[Using customized setting]" << endl;
  } else {
    cout << "[Using default setting]" << endl;
  }
  cout << "[ip:] " << ip << endl;
  cout << "[port:] " << port << endl;
  all_works(); // start
  return 0;
}
