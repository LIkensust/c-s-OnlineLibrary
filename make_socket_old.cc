#include "head.h"

static const int LISTEN_SIZE = 5;
static const int EPOLL_SIZE = 5;
static const int MAX_EVENT_SIZE = 2500;

using namespace std;

void Do_Connect(int socket,int epoll_fd)
{
    struct sockaddr_in cli_addr;
    socklen_t len = sizeof(cli_addr);
    int newsock = accept(socket,(struct sockaddr*)&cli_addr,&len);
    if(newsock < 0)
    {
        perror("accept");
        return;
    }
    struct epoll_event event;
    event.data.fd = newsock;
    event.events = EPOLLIN;
    int ret = epoll_ctl(epoll_fd,EPOLL_CTL_ADD,newsock,&event);
    if(ret < 0)
    {
        perror("epoll_ctl");
        return;
    }
    return;
}

void Do_main_job(int fd,int epoll_fd)
{
    fd = 0;
    epoll_fd = 0;
    //to do
    //创建一个线程 或者从线程池当中找出一个线程
    //处理http报文
    //从内存中或者硬盘中找到对应的数据 
    //更新状态统计信息
    //构建http报文并返回
    //关闭连接
    //
}

int main()
{
    int i = 0;
    int listen_sock = 0;
    listen_sock = socket(AF_INET,SOCK_STREAM,0);
    if(listen_sock < 0)
    {
        perror("socket");
        _exit(1);   
    }
    
    struct sockaddr_in ser_sock;
    memset(&ser_sock,0,sizeof(ser_sock));
    ser_sock.sin_family = AF_INET;
    ser_sock.sin_addr.s_addr = htonl(INADDR_ANY);
    ser_sock.sin_port = htons(8080);

    if(bind(listen_sock,(struct sockaddr*)&ser_sock,sizeof(ser_sock)) < 0)
    {
        perror("bind");
        _exit(1);
    }

    if(listen(listen_sock,LISTEN_SIZE) < 0)
    {
        perror("listen");
        _exit(1);
    }
        
    int ep_fd = epoll_create(EPOLL_SIZE);
    if(ep_fd < 0)
    {
        perror("epoll_create");
        _exit(1);
    }
     
    struct epoll_event event;
    event.data.fd = listen_sock;
    event.events = EPOLLIN;
    epoll_ctl(ep_fd,EPOLL_CTL_ADD,listen_sock,&event);

    struct epoll_event event_buf[MAX_EVENT_SIZE];

    while(1)
    {
        memset(event_buf,0,sizeof(struct epoll_event)*MAX_EVENT_SIZE);
        int size = epoll_wait(ep_fd,event_buf,MAX_EVENT_SIZE,-1);
        if(size < 0)
        {
            perror("epoll_wait");
            _exit(1);
        }
        if(size == 0)
        {
            continue;
        }
        for(i = 0;i < size;i++)
        {
            if( !(event_buf[i].events & EPOLLIN) )
            {
                continue;
            }
            if( event_buf[i].data.fd == listen_sock )
            {
                Do_Connect(listen_sock,ep_fd);
            }
            else
            {
                Do_main_job(event_buf[i].data.fd,ep_fd);
            }
        }
    }
    return 0;
}
