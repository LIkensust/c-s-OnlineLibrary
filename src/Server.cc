#include "include/common/include/server.h"
#include "include/common/common.h"
#include "include/socktool/sock_ser.hpp"
#define IPREGEX                                                                \
  "^([0-9]|[1-9][0-9]|1[0-9]{1,2}|2[0-4][0-9]|25[0-5]).([0-9]|[1-9][0-9]|1[0-" \
  "9]{1,2}|2[0-4][0-9]|25[0-5]).([0-9]|[1-9][0-9]|1[0-9]{1,2}|2[0-4][0-9]|25[" \
  "0-5]).([0-9]|[1-9][0-9]|1[0-9]{1,2}|2[0-4][0-9]|25[0-5])$"
using namespace std;
static string ip;
static short port;

int main(int argc, char *argv[]) {
  if (argc <= 1) {
    usage();
    return 1;
  }
  int option;
  while ((option = getopt(argc, argv, "p:l:")) != -1) {
    if (option == 'p') {
      auto regex_tool = RegexTool::make();
      regex_tool->set_regex("[0-9]*");
      vector<pair<int, int>> tmp = regex_tool->check_str(optarg);
      if (tmp.empty() || tmp[0].first != 0 ||
          (tmp[0].second != (int)strlen(optarg))) {
        ASSERT_MSG(false, "port must be num and not null");
      }
      port = atoi(optarg);
    }
    if (option == 'l') {
      auto regex_tool = RegexTool::make();
      regex_tool->set_regex(IPREGEX);
      vector<pair<int, int>> tmp = regex_tool->check_str(optarg);
      if (tmp.empty() || tmp[0].first != 0 ||
          (tmp[0].second != (int)strlen(optarg))) {
        ASSERT_MSG(false, "ip must be 0.0.0.0 ~ 255.255.255.255");
      }
      ip = optarg;
    }
  }
  cout << "[Using customized:]" << endl;
  cout << "[ip:] " << ip << endl;
  cout << "[port:] " << port << endl;
  return 0;
}
