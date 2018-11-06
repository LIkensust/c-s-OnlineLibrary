#include "head.h"
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <cassert>
#include <string>
#include <map>
#include <sys/mman.h>
#include <pthread.h>
#include <sys/sem.h>
#include <queue>
#include <memory>
#define EVENTSIZE 300
#define BUFSIZE 3000
using namespace std;
typedef  void *(*FUNPTR)(void*);
union semun
{
    int val;
};
int sem_product;
int sem_capacity;
int ep_fd;
pthread_mutex_t queue_lock;

//事件队列
queue<int> fd_que;
int sem_P(int sem_id)
{
    struct sembuf semb;
    semb.sem_num = 0;
    semb.sem_op = -1;
    semb.sem_flg = SEM_UNDO;
    if(semop(sem_id,&semb,1) == -1)
    {
        return 0;
    }
    return 1;
}

int sem_V(int sem_id)
{
    struct sembuf semb;
    semb.sem_num = 0;
    semb.sem_op = 1;
    semb.sem_flg = SEM_UNDO;
    if(semop(sem_id,&semb,1) == -1)
    {
        return 0;
    }
    return 1;
}

void pthread_handler(void* arg)
{
    //连接事件
    //接受数据
    //写数据
    arg = NULL;
    while(true)
    {
    sem_P(sem_product);
    pthread_mutex_lock(&queue_lock);
    int fd = fd_que.front();
    fd_que.pop();
    pthread_mutex_unlock(&queue_lock);

    //开始读取数据
    shared_ptr<char> buf = make_shared<char>(BUFSIZE);
    int index = 0;
    int read_size = 0;
    char *p = buf.get();
    while( (read_size=read(fd,p+index,BUFSIZE-1)) > 0 )
    {
        index += read_size;
    }
    if(read_size == -1)
    {
        close(fd);
        continue;
    }
    //开始处理数据
    do
    {
        printf("%s\n",buf.get());
        write(fd,buf.get(),BUFSIZE);
    }while(0);
    close(fd);
    sem_V(sem_capacity);
    }   
}
    
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
    printf("connetc with:%s:%d\n",inet_ntoa(cli_addr.sin_addr),ntohs(cli_addr.sin_port));
    struct epoll_event event;
    event.data.fd = newsock;
    event.events = EPOLLIN | EPOLLONESHOT;
    int ret = epoll_ctl(epoll_fd,EPOLL_CTL_ADD,newsock,&event);
    if(ret < 0)
    {
        perror("epoll_ctl");
        return;
    }
    return;
}


void pthread_prudect(void* arg)
{
    int fd = (int)reinterpret_cast<long>(arg);
    sem_P(sem_capacity);
    fcntl(fd,F_SETFL,O_NONBLOCK);//设置非阻塞
    pthread_mutex_lock(&queue_lock);
    fd_que.push(fd);
    pthread_mutex_unlock(&queue_lock);
    sem_V(sem_product);
}

void Do_read_wirte(int fd)
{
    pthread_t p_t;
    pthread_attr_t att;
    pthread_attr_init(&att);
    pthread_attr_setdetachstate(&att,PTHREAD_CREATE_DETACHED);
    pthread_create(&p_t,&att,FUNPTR(pthread_prudect),reinterpret_cast<void*>(fd));
    pthread_attr_destroy(&att);
}

void usage()
{
   fprintf(stderr,"./ser port\n"); 
}


int main(int argc,char* argv[])
{
    if(argc != 1)
    {
        usage();
        return 1;
    }
    //创建信号量
    sem_capacity = semget(ftok(".",'a'),1,0666|IPC_CREAT);
    union semun sem_un;
    sem_un.val = 500;
    if(semctl(sem_capacity,0,SETVAL,sem_un) == -1)
    {
        perror("semctl");
        return 1;
    }

    sem_product = semget(ftok(".",'a'),1,0666|IPC_CREAT);
    sem_un.val = 0;
    if(semctl(sem_product,0,SETVAL,sem_un) == -1)
    {
        perror("semctl");
        return 1;
    }

    //创建锁
    pthread_mutex_init(&queue_lock,NULL);

    int listen_sock = socket(AF_INET,SOCK_STREAM,0);
    if(listen_sock < 0)
    {
        perror("socket:");
        return 1;
    }
    struct sockaddr_in host_sock;
    host_sock.sin_addr.s_addr = htonl(INADDR_ANY);
    host_sock.sin_family = AF_INET;
    host_sock.sin_port = htons(atoi(argv[1]));
    int ret = bind(listen_sock,(struct sockaddr*)&host_sock,sizeof(host_sock));
    if(ret < 0)
    {
        perror("bind");
        return -1;
    }
    ret = listen(listen_sock,5);
    if(ret < 0)
    {
        perror("listen");
        return -1;
    }
    
    ep_fd = epoll_create(10);   
    struct epoll_event event;
    event.data.fd = listen_sock;
    event.events = EPOLLIN;
    epoll_ctl(ep_fd,EPOLL_CTL_ADD,listen_sock,&event);
    struct epoll_event* event_buf = new epoll_event[EVENTSIZE];
 


    //先创建几个线程:
    int cpu_num = sysconf(_SC_NPROCESSORS_ONLN);
    pthread_t *pthread_id = new pthread_t[cpu_num];
    pthread_attr_t att;
    pthread_attr_init(&att);
    pthread_attr_setdetachstate(&att,PTHREAD_CREATE_DETACHED);
    for(int i = 0 ;i < cpu_num;i++)
    {
        pthread_create(&pthread_id[i],&att,(FUNPTR)pthread_handler,NULL);
    }
    pthread_attr_destroy(&att);
    int i;
    while(1)
    {
        memset(event_buf,0,sizeof(epoll_event)*EVENTSIZE);
        int size = epoll_wait(ep_fd,event_buf,sizeof(struct epoll_event)*EVENTSIZE,-1);
        if(size < 0)
        {
            perror("epoll_wait");
            return 0;
        }
        if(size == 0)
        {
            printf("epoll_wait time out\n");
            continue;
        }
        for(i = 0;i < size;i++)
        {
            if(!(event_buf[i].events & EPOLLIN))
            {
                continue;
            }
            if(event_buf[i].data.fd == listen_sock)
            {
                Do_Connect(listen_sock,ep_fd);
            }
            else
            {
                Do_read_wirte(event_buf[i].data.fd);
            }
        }
    }//while(1)
 

    delete []event_buf;
    delete []pthread_id;
    pthread_mutex_destroy(&queue_lock);
    
    return 0;
}
