#include <stdio.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <stdlib.h>
#include <string.h>
#include "head.h"
using namespace std;
int main(int argc,char *argv[])
{
    int i = atoi(argv[3]);
    while(i--)
    {
    struct sockaddr_in sock;
    sock.sin_family = AF_INET;
    sock.sin_addr.s_addr = inet_addr(argv[1]);
    int tmp_port = atoi(argv[2]);
    sock.sin_port = htons(tmp_port);
    
    int fd = socket(AF_INET,SOCK_STREAM,0);
    connect(fd,(struct sockaddr*)(&sock),sizeof(struct sockaddr_in));
    char buf[1024]={0};
    sprintf(buf,"hello world\n");
    printf("%s\n",buf);
    write(fd,buf,strlen(buf));
    //shutdown(fd,SHUT_WR);
    memset(buf,0,1024);
    int num = read(fd,buf,1024);
    cout<<num<<endl;
    printf("%s\n",buf);
    close(fd);
    }
    return 0;
}
