#include "include/common/include/server.h"
#include "include/common/common.h"
#include "include/socktool/sock_ser.hpp"
#define EPOLLEVENTNUM 50
using namespace std;
static string ip = SERVERIP;
static short port = SERVERPORT;
static short EPOLLWAITTIME = 500;

void all_works() {
  auto sock_tool = ListenSockTool::make();
  sock_tool->set_ip(ip);
  sock_tool->set_port(port);
  ASSERT_MSG(sock_tool->open_sock() == OK, "open_sock failed");
  ASSERT_MSG(sock_tool->start_listen() == OK, "start_listen failed");
  int listen_sock = sock_tool->get_sockfd();
#ifdef DEBUG
  cout << "[=DEBUG=][Listen at " << listen_sock << "]" << endl;
#endif
  struct epoll_event epoll_event, all_events[EPOLLEVENTNUM];
  shared_ptr<sockaddr> client_addr(new sockaddr);
  int epoll_fd = epoll_create(500);
  ASSERT_MSG(epoll_fd >= 0, "create epoll fail");
  // put listen sock into epoll
  // set the event with EPOLLIN|EPOLLET
  epoll_event.data.fd = listen_sock;
  epoll_event.events = EPOLLIN | EPOLLET;
  epoll_ctl(epoll_fd, EPOLL_CTL_ADD, listen_sock, &epoll_event);
  int num_of_fds;
  shared_ptr<sockaddr> addr(new sockaddr);
  while (true) {
    num_of_fds = epoll_wait(epoll_fd, all_events, EPOLLEVENTNUM, EPOLLWAITTIME);
    for (int i = 0; i < num_of_fds; i++) {
      if (all_events[i].data.fd == listen_sock) {
#ifdef DEBUG
        cout << "[=DEBUG=][accept request]" << endl;
#endif
        int client_fd = sock_tool->do_accept(addr);
        if (client_fd < 0) {
#ifdef DEBUG
          perror("accrpt");
#endif
        }
        set_nonblock(client_fd);
#ifdef DEBUG
        char *set =
            inet_ntoa(reinterpret_cast<sockaddr_in *>(addr.get())->sin_addr);
        cout << "[=DEBUG=][connect with:] " << set << endl;
#endif
        epoll_event.data.fd = client_fd;
        epoll_event.events = EPOLLIN | EPOLLET;
        epoll_ctl(epoll_fd, EPOLL_CTL_ADD, client_fd, &epoll_event);
      } else if (all_events[i].events & EPOLLIN) {
        int client_fd = all_events[i].data.fd;
        if (client_fd < 0) {
          continue;
        } else {
          //read data
        }
      }
    }
  }

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
