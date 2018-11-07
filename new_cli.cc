#include <stdio.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <stdlib.h>
#include <string.h>
#include "head.h"
#include <string>
using namespace std;

char testbuf[]="{\"message\":{\"name\":\"UNIX环境高级编程\",\"id\":\"9\",\"basic_info\":\
[\"author:ichard Stevens / Stephen A.Rago\",\"publisher:人民邮电出版社\",\"\
isbn:9\"],\"detil\":{\"Introduction\":\"现在是修改过的内容\"}}}";

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
        char buf[3000]={0};
        string fileid;
        cout<<"input the bookid"<<endl;
        cin>>fileid;
        cout<<"##==1.获取内容  2.修改内容==##"<<endl;
        int choose;
        cin>>choose;
        if(choose == 1)
        {
            fileid = "GET /api/v1/books/"+fileid;
            fileid = fileid + "\r\n\r\n";
            write(fd,fileid.c_str(),fileid.size());
            cout<<fileid<<endl;
            //shutdown(fd,SHUT_WR);
            memset(buf,0,3000);
            int num = read(fd,buf,3000);
            cout<<num<<endl;
            printf("%s\n",buf);
            close(fd);

        }
        else
        {
            fileid = "PUT /api/v1/books/"+fileid+"\r\n\r\n";
            fileid += testbuf;
            write(fd,fileid.c_str(),fileid.size());
            cout<<fileid<<endl;
            //shutdown(fd,SHUT_WR);
            memset(buf,0,3000);
            int num = read(fd,buf,3000);
            cout<<num<<endl;
            printf("%s\n",buf);
            close(fd);
        }
    }
    return 0;
}
