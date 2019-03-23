#include "include/common/include/server.h"
#include "include/common/common.h"
#include "include/socktool/sock_ser.hpp"
using namespace std;

int main(int argc, char *argv[]) {
  if (argc <= 1) {
    usage();
    return 1;
  }
  int option;
  while ((option = getopt(argc, argv, "p:l:")) != -1) {
    if (option == 'p') {
      check_str(optarg, "[0-9]?");
    }
  }
  return 0;
}
