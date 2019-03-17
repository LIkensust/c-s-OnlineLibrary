#include "include/common/common.h"
using namespace std;
int main ()
{
    int fd = open ("text.json", O_CREAT | O_TRUNC | O_RDWR, 0644);
    if(fd <= 0)
    {
        perror("open");
        _exit(1);
    }
    printf("%d\n",fd);
    neb::CJsonObject oJson ("");
    oJson.AddEmptySubObject ("message");
    oJson["message"].Add ("name", "UNIX环境高级编程");
    oJson["message"].Add ("id", "9787115147318");
    oJson["message"].AddEmptySubArray ("basic_info");
    oJson["message"]["basic_info"].Add ("ichard Stevens / Stephen A.Rago");
    oJson["message"]["basic_info"].Add ("人民邮电出版社");
    oJson["message"].AddEmptySubObject("detil");
    oJson["message"]["detil"].Add("Introduction","本书是被誉为UNIX编程“圣经”的Advanced Programming in the UNIX Environment一书的更新版。在本书第1版出版后的十几年中，UNIX行业已经有了巨大的变化，特别是影响UNIX编程接口的有关标准变化很大。本书在保持了前一版风格的基础上，根据最新的标准对内容进行了修订和增补，反映了最新的技术发展。书中除了介绍UNIX文件和目录、标准I/O库、系统数据文件和信息、进程环境、进程控制、进程关系、信号、线程、线程控制、守护进程、各种I/O、进程间通信、网络IPC、伪终端等方面的内容，还在此基础上介绍了多个应用示例，包括如何创建数据库函数库以及如何与网络打印机通信等。此外，还在附录中给出了函数原型和部分习题的答案。");
    string json;
    json = oJson.ToString();
    int ret = write(fd,json.c_str(),json.size());
    if(ret <= 0)
    {
        perror("write");
    }
    close(fd);
    return 0;
}
