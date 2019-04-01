#pragma once
#include <stdio.h>
#define SERVERPORT 5434
#define SERVERIP "127.0.0.1"
void usage() {
  printf(
      "\033[47;32m|| Thank you for using this system.             ||\n\033[0m");
  printf(
      "\033[47;32m|| To start the server                          ||\n\033[0m");
  printf(
      "\033[47;32m|| You can run the system without any parameter.||\n\033[0m");
  printf(
      "\033[47;32m|| -use '-p' to set port                        ||\n\033[0m");
  printf(
      "\033[47;32m|| -use '-l' to set ip                          ||\n\033[0m");
}
