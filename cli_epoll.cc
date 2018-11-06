#include <stdio.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <iostream>
#include <string>

typedef  void *(*FUNPTR) (void *);
using namespace std;
struct sockaddr_in sockaddr;

void pthread_job(void* a)
{
    printf("get in \n");
    int sock_fd = socket(AF_INET,SOCK_STREAM,0);
    if(sock_fd < 0)
    {
        perror("socket");
        pthread_exit(NULL);
    }
    connect(sock_fd,(struct sockaddr*)&sockaddr,sizeof(sockaddr));  
    printf("connect success\n \n");
    char buf[2500] = {0};
    string filename = "/api/v1/books/"; 
    string bookid ;
    cin>>bookid;
    filename += bookid;
    string http;
    http = "GET ";
    http += filename;
    http += "\r\n";
    write(sock_fd,http.c_str(),http.size());
    
    memset(buf,0,sizeof(buf));
    int size = read(sock_fd,buf,1024);
    if(size <= 0)
    {
        printf("read\n");
        close(sock_fd);
        pthread_exit(NULL);
    }
    printf("service say:%s\n",buf);
    
    close(sock_fd);
    pthread_exit(NULL);
}

void usage()
{
    fprintf(stdout,"./cli"
            "[ip]"
            "port");
    _exit(0);
}

int main(int argc,char *argv[])
{
    cout<<"hello"<<endl;
    if(argc < 4)
    {
        usage();
    }

    sockaddr.sin_family = AF_INET;
    sockaddr.sin_addr.s_addr = inet_addr(argv[1]);
    int tmp_port = atoi(argv[2]);
    sockaddr.sin_port = htons(tmp_port);

    pthread_attr_t attr;
    pthread_attr_init(&attr);
    pthread_attr_setdetachstate(&attr,PTHREAD_CREATE_DETACHED);

    pthread_t tid;

    int times = atoi(argv[3]);
    int totle = times;   
    while(times--)
    {
        pthread_create(&tid,NULL,(FUNPTR)pthread_job,(void*)(totle - times));
    }
    //int sock = creat_sock(argv[1],argv[2]); 
    pthread_join(tid,NULL);
    return 0;
}

