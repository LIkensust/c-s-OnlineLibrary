#include <iostream>
#include <string>
#include "../data_buffer.h"
using namespace std;

int main() {
  {
    cout << "[==test DataBuffer Part 1==]" << endl;
    DataBuffer bf;
    char data[] = "GET www.baidu.com 1.1\r\n"
                  "KEEPALIVE : FALSE\r\n\r\n"
                  "hello world\r\n";
    bf.Append(data, sizeof(data));
    const char *crlf = bf.findCRLF();
    while (true) {
      cout << "===============" << endl;
      crlf = bf.findCRLF(crlf + 2);
      if (crlf == nullptr) {
        break;
      }
      string tmp(crlf);
      cout << tmp;
    }
  }

  {
    cout << "[==test DataBuffer Part 2==]" << endl;
    DataBuffer bf;
    char data[] = "GET www.baidu.com 1.1\r\n"
                  "KEEPALIVE : FALSE\r\n\r\n"
                  "hello world\r\n";
    bf.Append(data, sizeof(data));
    int Error;
    bf.WriteFd(1, &Error);
  }
}
