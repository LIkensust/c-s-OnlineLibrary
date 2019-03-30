#include "../src/include/common/common.h"
#include <iostream>

int main() {
  int fd = open("text.txt", O_CREAT | O_RDWR, 0644);
  int sock_fd = socket(AF_INET, SOCK_STREAM, 0);
  set_nonblock(fd);
  set_nonblock(sock_fd);
  std::cout << "Hello world" << std::endl;
  return 0;
}
