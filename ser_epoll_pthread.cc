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
#include <boost/shared_ptr.hpp>
#include <semaphore.h>
#include <time.h>
//#define TEST
#define EVENTSIZE 300
#define BUFSIZE 3000
static int HASHSIZE;
using namespace std;
typedef  void *(*FUNPTR)(void*);
union semun
{
    int val;
};
sem_t sem_product;
sem_t sem_capacity;
int ep_fd;
pthread_mutex_t queue_lock;
pthread_rwlock_t map_lock;
enum HTTPTYPE
{
    READ,
    WRITE,
    ERR
};

struct cache_node
{
//    string id;
    size_t size;
    long long cont;
    time_t lasttime;
    time_t per;
    char * data;
};
map<string,cache_node> cache;
//创建一个hash映射 完成文件分类

//这里用的hash函数与shell脚本中的hash函数必须保持一致
int _hash_(const string& filename)
{
    int ret = 0;
    string::const_iterator it = filename.begin();
    while(it != filename.end())
    {
        ret = ret*2 + (*it) - '0';
        it++;
    }
#ifdef TEST
    cout<<__FUNCTION__<<endl;
#endif
    return ret % HASHSIZE;
}

string _itoa_(int num)
{
    string ret;
    char tmp[2] = {0};
    if(num == 0)
    {
        ret = "0";
        return ret;
    }
    while(num)
    {
        tmp[0] = num%10 + '0';
        ret = tmp + ret;
        num /= 10;
    }
#ifdef TEST
    cout<<__FUNCTION__<<endl;
#endif
    return ret;
}

string file_hash(const string& filename)
{
    assert(!(filename.empty()));
    int num_of_dir = _hash_(filename);
    string ret = _itoa_(num_of_dir);
#ifdef TEST
    cout<<__FUNCTION__<<endl;
#endif

    ret = "./books/dir"+ret;
    ret = ret+"/";
    ret = ret + filename;
    return ret;
}

bool init_hash()
{
    FILE* cfg = fopen("./hash_config.cfg","r");
    if(cfg == NULL)
    {
        return false;
    }
    fscanf(cfg,"%d",&HASHSIZE);
    fclose(cfg);
    return true;
}

//事件队列
queue<int> fd_que;

void main_job(int fd,boost::shared_ptr<char>& buf)
{
    char *p = buf.get();
    
}


void pthread_handler(void* arg)
{
    cout<<"great success"<<endl;
    //连接事件
    //接受数据
    //写数据
#ifdef TEST
    int pthread_id = (int)(long)arg;
#endif
    while(true)
    {
        sem_wait(&sem_product);
        pthread_mutex_lock(&queue_lock);
        int fd = fd_que.front();
        fd_que.pop();
        pthread_mutex_unlock(&queue_lock);
        sem_post(&sem_capacity);
        
        //开始读取数据
#ifdef TEST
        cout<<"read begin by"<<pthread_id<<endl;
#endif
        
        boost::shared_ptr<char> buf(new char[BUFSIZE]);
        int index = 0;
        int read_size = 0;
        char *p = buf.get();
#ifdef TEST
        cout<<__LINE__<<endl;
#endif
        while( (read_size=read(fd,p+index,BUFSIZE-1)) > 0 )
        {
            index += read_size;
        }
        printf("%s\n",p);
        if(read_size == -1)
        {
            close(fd);
            continue;
        }
        //开始处理数据
        main_job(fd,buf);
        close(fd);
    }   
}
    
void Do_Connect(void* arg)
{
    int socket = (int)(long)(arg);
    int epoll_fd = ep_fd;
    struct sockaddr_in cli_addr;
    socklen_t len = sizeof(cli_addr); 
    int newsock = accept(socket,(struct sockaddr*)&cli_addr,&len);
    if(newsock < 0)
    {
        perror("accept");
        return;
    }
    int flags;
    if((flags = fcntl(newsock,F_GETFL,NULL)) == -1)
    {
        close(newsock);
        return;
    }
    if(fcntl(newsock,F_SETFL,flags|O_NONBLOCK)== -1)
    {
        close(newsock);
        return;
    }//设置非阻塞

    struct epoll_event event;
    event.data.fd = newsock;
    event.events = EPOLLIN | EPOLLONESHOT;
    int ret = epoll_ctl(epoll_fd,EPOLL_CTL_ADD,newsock,&event);
    if(ret < 0)
    {
        perror("epoll_ctl");
        return;
    }
#ifdef TEST
    printf("connetc with:%s:%d\n",inet_ntoa(cli_addr.sin_addr),ntohs(cli_addr.sin_port));
#endif
    return;
}


void pthread_prudect(void* arg)
{

#ifdef TEST
    cout<<__FUNCTION__<<endl;
#endif
    int fd = (int)reinterpret_cast<long>(arg);
    sem_wait(&sem_capacity);
    pthread_mutex_lock(&queue_lock);
#ifdef TEST
    cout<<__FUNCTION__<<endl;
#endif
    fd_que.push(fd);
    pthread_mutex_unlock(&queue_lock);
    sem_post(&sem_product);
}

void Do_read_wirte(int fd)
{
#ifdef TEST
    cout<<__FUNCTION__<<"::"<<fd<<endl;
#endif
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
//to do 注册退出信号
    
    if(argc != 2)
    {
        usage();
        return 1;
    }
    //创建信号量
    int ret = sem_init(&sem_capacity,0,500);
    if(ret == -1)
    {
        perror("init:");
        return 1;
    }

    ret = sem_init(&sem_product,0,0);
    if(ret == -1)
    {
        perror("init:");
        return 1;
    }

    init_hash();

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
    ret = bind(listen_sock,(struct sockaddr*)&host_sock,sizeof(host_sock));
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
    int cpu_num = sysconf(_SC_NPROCESSORS_ONLN)*2;
    pthread_t *pthread_id = new pthread_t[cpu_num];
    pthread_attr_t att;
    pthread_attr_init(&att);
    pthread_attr_setdetachstate(&att,PTHREAD_CREATE_DETACHED);
    int *id = new int[cpu_num];
    for(int i = 0 ;i<cpu_num;i++)
    {
        id[i] = i+1;
    }
    for(int i = 0 ;i < cpu_num;i++)
    {
        pthread_create(&pthread_id[i],&att,(FUNPTR)pthread_handler,reinterpret_cast<void*>(id[i]));
    }
    delete []id;
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
                //Do_Connect(listen_sock,ep_fd);
                //pthread_t tmp;
                //pthread_create(&tmp,&att,(FUNPTR)Do_Connect,reinterpret_cast<void*>(listen_sock));
                Do_Connect(reinterpret_cast<void*>(listen_sock));
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
    pthread_attr_destroy(&att);
    return 0;
}
