#include "include/common/include/server.h"
#include "include/common/common.h"
#include "include/socktool/sock_ser.hpp"
#include "include/thread_pool/thread_pool.h"
#define EPOLLEVENTNUM 50
using namespace std;
static string ip = SERVERIP;
static short port = SERVERPORT;
static short EPOLLWAITTIME = 500;

struct DataBlob {
  int client_fd;
  int epoll_fd;
};

void recv_and_work(void *datablob) {
  if (datablob == NULL) {
    return;
  }
  int cli_fd = ((DataBlob *)datablob)->client_fd;
  int epoll_fd = ((DataBlob *)datablob)->epoll_fd;
  delete (DataBlob *)datablob;
  auto client_tool = ServerConnetSockTool();
  client_tool.set_sockfd(cli_fd);
  shared_ptr<char> buff(new char[1024]);
  client_tool.read_from_sock(buff, 1024);
#ifdef DEBUG
  cout << "[=DEBUG=][get message :] " << buff.get() << endl;
#endif
  shared_ptr<char> buff_echo(new char[1024]);
  sprintf(buff_echo.get(), "I am server.I get :%s", buff.get());
  client_tool.write_to_sock(buff_echo, strlen(buff_echo.get()));
  client_tool.set_no_close();
  epoll_event event;
  event.data.fd = cli_fd;
  event.events = EPOLLIN | EPOLLET;
  epoll_ctl(epoll_fd, EPOLL_CTL_MOD, cli_fd, &event);
}

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
  epoll_event one_event, all_events[EPOLLEVENTNUM];
  shared_ptr<sockaddr> client_addr(new sockaddr);
  int epoll_fd = epoll_create(500);
  ASSERT_MSG(epoll_fd >= 0, "create epoll fail");
  // put listen sock into epoll
  // set the event with EPOLLIN|EPOLLET
  one_event.data.fd = listen_sock;
  one_event.events = EPOLLIN | EPOLLET;
  epoll_ctl(epoll_fd, EPOLL_CTL_ADD, listen_sock, &one_event);
  int num_of_fds;
  shared_ptr<sockaddr> addr(new sockaddr);
  auto thread_pool = threadpool::ThreadPool::make(10);
  thread_pool.start();
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
        one_event.data.fd = client_fd;
        one_event.events = EPOLLIN | EPOLLET;
        epoll_ctl(epoll_fd, EPOLL_CTL_ADD, client_fd, &one_event);
      } else if (all_events[i].events & EPOLLIN) {
        int client_fd = all_events[i].data.fd;
        if (client_fd < 0) {
          continue;
        } else {
          DataBlob *p = new DataBlob;
          p->client_fd = client_fd;
          p->epoll_fd = epoll_fd;
          thread_pool.add_task(bind(recv_and_work, (void *)p));
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
      usage();
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
