#include "include/common/common.h"
#include <map>
#include <sys/mman.h>
#include <pthread.h>
#include <sys/sem.h>
#include <queue>
#include <memory>
#include <semaphore.h>
#include <time.h>
//#define TEST
#define EVENTSIZE 300
#define BUFSIZE 3000
using namespace std;

//哈希表的大小 在寻找文件路径的时候使用
static int HASHSIZE;

typedef  void *(*FUNPTR)(void*);
//信号量
sem_t sem_product;
sem_t sem_capacity;
//监听套接字
int ep_fd;
//对事件队列的锁
pthread_mutex_t queue_lock;
//缓存的锁
pthread_rwlock_t map_lock;

enum HTTPTYPE
{
  READ,
  WRITE,
  ERR
};

struct http_msg
{
  string fileid;
  enum HTTPTYPE type;
  char *body;
};

//使用map进行缓存
struct cache_node
{
  //    string id;
  size_t size;
  long long cont;
  time_t lasttime;
  time_t per;
  char * data;
};

//缓存区
map<string,cache_node> cache;
typedef map<string,cache_node>::iterator mapit;

//使用优先级队列实现堆   这个堆用在对缓存区的清理时
struct cmp//实现大堆
{
  bool operator()(mapit a,mapit b)
  {
    time_t now = time(NULL);
    time_t a_l = now - a->second.lasttime;
    time_t b_l = now - b->second.lasttime;

    long long sub = a->second.cont - b->second.cont; 
    if(abs(sub) > 1000)
    {
      return (sub>0)?false:true;
    }
    else if(sub > 100)
    {
      time_t a_per = a->second.per;
      time_t b_per = b->second.per;
      return (a_per>b_per)?true:false;
    }
    return (a_l>b_l)?true:false; 
  }
};

priority_queue<mapit,vector<mapit>,cmp > heap;
//事件队列
queue<int> fd_que;
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
//通过url里的文件id获取文件的实际存储路径
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

//程序开始运行就因该调用这个函数 读取配置文件中的信息 因为hash长度是根据实际情况改变的
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

HTTPTYPE _read_head(const char *buf)
{
  if(buf[0]=='G' && buf[1] == 'E' && buf[2] == 'T' )
  {
    return READ;
  }

  if(buf[0]=='P' && buf[1] == 'U' && buf[2] == 'T' )
  {
    return WRITE;
  }

  return ERR;
}
//http的解析
void _read_http(const char* buf,struct http_msg* msg)
{
  //GET /api/v1/books/<book_id>
  //PUT
#ifdef TEST
  cout<<__FUNCTION__<<endl;
  cout<<buf<<endl;
#endif

  //提取第一行
  const char *line = buf;
  while(*line != '\r')
  {
    line++;
  }
  char firstline[100] = {0};
  memcpy(firstline,buf,sizeof(char)*(line-buf));
  HTTPTYPE headtype = _read_head(firstline);
  if(headtype == READ)
  {
    msg->type = READ;
#ifdef TEST
    cout<<__LINE__<<endl;
#endif
  }
  else if(headtype == WRITE)
  {
    msg->type = WRITE;
#ifdef TEST
    cout<<__LINE__<<endl;
#endif
  }
  else
  {
    msg->type = ERR;
#ifdef TEST
    cout<<__LINE__<<endl;
#endif
    return;
  }
#ifdef TEAT
  cout<<"type finish"<<endl;
#endif
  char *p = strstr(firstline,"/books/");
  p+=7;
  printf("%s\n",p);
  while(*p!=' '&&*p!='\r'&&*p!='\n'&&*p!='\0')
  {
    msg->fileid+=*p;
    p++;
  }
#ifdef TEST
  cout<<msg->fileid<<endl;
#endif
  if(msg->type == WRITE)
  {
    msg->body = strstr(const_cast<char*>(line),"\r\n\r\n");
    msg->body += 4;
  }
  return;
}

//对socket wirte的一层封装 因为将socket设置成非阻塞了 保证信息全部写完
ssize_t socket_write(int sockfd, const char* buffer, size_t buflen)
{
  ssize_t tmp;
  size_t total = buflen;
  const char* p = buffer;
  while(1)
  {
    tmp = write(sockfd, p, total);
    if(tmp < 0)
    {
      // 当send收到信号时,可以继续写,但这里返回-1.
      if(errno == EINTR)
        return -1;
      // 当socket是非阻塞时,如返回此错误,表示写缓冲队列已满,
      // 在这里做延时后再重试.
      if(errno == EAGAIN)
      {
        usleep(500);
        continue;
      }
      return -1;
    }
    if((size_t)tmp == buflen)
      return buflen;
    total -= tmp;
    p += tmp;
  }
  return tmp;//返回已写字节数
}

//缓存的数据不是不变的 我们并不知道哪些热门数据 在缓存区快要满的时候
//对缓存区内的数据进行检查 找出冷数据 从缓存区内剔除   程序不断地运行
//这样就可以动态的筛选出当前的热数据  
//根据要求 共有200000个热数据  所以将缓存区大小设置成200000
//每次都剔除30%的数据 
//假设每秒有2000的访问量 其中的20%访问的是冷数据 等于每秒访问400个冷数据
//也就是大约200000/400=500s才会重新填满缓存区 等于每10分钟进行一次缓存区释放
//缓存区命中热数据的概率是80%  
void free_part_of_map()
{
  pthread_rwlock_rdlock(&map_lock);
  int size = cache.size();
  pthread_rwlock_unlock(&map_lock);
  if(size > 200000)
  {
    //释放
    pthread_rwlock_wrlock(&map_lock);
    do{
      cmp tmp;
      if(cache.size() < 200000)//防止重复释放 因为在进行下一步之前可能别的线程已经释放过一次
      {
        break;
      }
      else
      {
        mapit it= cache.begin();
        while(it != cache.end())
        {
          if(heap.size() < 60000)
            heap.push(it++);
          else
          {
            bool less = tmp(it,heap.top());
            if(less)
            {
              heap.pop();
              heap.push(it);
            }
            it++;
          }
        }
        while(heap.size() !=0 )
        {
          mapit it = heap.top();
          cache.erase(it);
          heap.pop();
        }
      }
    }while(0);
    pthread_rwlock_unlock(&map_lock);
  }
  else
  {
    return ;
  }
}

//主要的业务逻辑在这个函数内
void main_job(int fd,shared_ptr<char>& buf)
{
  char *p = buf.get();
  struct http_msg msg;
  _read_http(p,&msg);
#ifdef TEST
  cout<<"http 解析完毕"<<endl;
#endif
  if(msg.type == READ)
  {
    memset(p,0,BUFSIZE);
    string filename = msg.fileid;
    pthread_rwlock_rdlock(&map_lock);
    map<string,cache_node>::iterator it = cache.find(filename);
    bool flag = (it == cache.end());
    pthread_rwlock_unlock(&map_lock);
    free_part_of_map();//检测当前缓存是否过多 过多就释放
    if(flag == true)
    {
      //不在缓存区内
#ifdef TEST
      cout<<"不在缓存区"<<endl;
#endif  
      string ret = file_hash(filename);
      int filesize = -1;
      struct stat statbuf;
      if(stat(ret.c_str(),&statbuf) < 0)
      {
        //404
        sprintf(p,"HTTP/1.1 404 NOTFOUND\r\n\r\n");
        write(fd,p,strlen(p)+1);
      }
      else
      {
        filesize = statbuf.st_size;
        int file_fd = open(ret.c_str(),O_RDONLY,0666);
#ifdef TEST
        cout<<"file_fd:"<<file_fd<<endl;
#endif
        void *fp = mmap(NULL,(size_t)filesize,PROT_READ,MAP_SHARED,file_fd,(off_t)0);
        close(file_fd);
#ifdef TEST
        cout<<"fp:"<<fp<<endl;
#endif
        struct cache_node tmp;
        tmp.cont = 1;
        tmp.data = (char *)fp;
        tmp.size = filesize;
        tmp.lasttime = time(NULL);
        tmp.per = 1;
        //上锁
        pthread_rwlock_wrlock(&map_lock);
        cache[filename] = tmp;
        it = cache.find(filename);
        pthread_rwlock_unlock(&map_lock);
        //解锁
      }
    }
    else
    {
      pthread_rwlock_wrlock(&map_lock);
      it->second.cont++;
      time_t tmp = it->second.lasttime;
      it->second.lasttime = time(NULL);
      it->second.per = (3*it->second.per + 7*(it->second.lasttime - tmp))/10;  
      pthread_rwlock_unlock(&map_lock);
    }
    //在缓存区内
    pthread_rwlock_rdlock(&map_lock);
    const struct cache_node& node = it->second;
    sprintf(p,"HTTP/1.1 200 OK\r\nConnection_Type:application/json\r\n\r\n");
    int len = strlen(p);
    memcpy(p+strlen(p),node.data,node.size);
    len+=node.size;
    int writesize = socket_write(fd,p,len);
    pthread_rwlock_unlock(&map_lock);
#ifdef TEST
    cout<<"   write:"<<writesize<<endl;
#endif
  }
  else if(msg.type == WRITE)
  {
    string filename = msg.fileid;
    pthread_rwlock_rdlock(&map_lock);
    map<string,cache_node>::iterator it = cache.find(filename);
    bool flag = (it == cache.end());
    pthread_rwlock_unlock(&map_lock);
    if(flag == true)
    {
      //不在缓存区
      string ret = file_hash(filename);
      struct stat statbuf;
      if(stat(ret.c_str(),&statbuf) < 0)
      {
        //404
        sprintf(p,"HTTP/1.1 404 NOTFOUND\r\n\r\n");
        write(fd,p,strlen(p)+1);
      }
      else
      {
        FILE *fout = fopen(ret.c_str(),"w");
        if(fout == NULL)
        {
          char tmpbuf[]="HTTP/1.1 503 Server Unavailable\r\n\r\n";
          socket_write(fd,tmpbuf,strlen(tmpbuf));
          return; 
        }
        char *data = msg.body;
        while(*data != '\0')
        {
          fprintf(fout,"%c",*data++);                  
        }
        fclose(fout);
        char tmpbuf[]="HTTP/1.1 200 OK\r\n\r\n";
        socket_write(fd,tmpbuf,strlen(tmpbuf));
        return; 
      }    
    }
    else
    {
      //在缓存区内
      pthread_rwlock_wrlock(&map_lock);
      cache_node &tmp = cache[filename];
      munmap((void*)tmp.data,tmp.size);
      string ret = file_hash(filename);
      struct stat statbuf;
      if(stat(ret.c_str(),&statbuf) < 0)
      {
        //404
        sprintf(p,"HTTP/1.1 404 NOTFOUND\r\n\r\n");
        write(fd,p,strlen(p)+1);
      }
      else
      {
        FILE *fout = fopen(ret.c_str(),"w");
        if(fout == NULL)
        {
          char tmpbuf[]="HTTP/1.1 503 Server Unavailable\r\n\r\n";
          socket_write(fd,tmpbuf,strlen(tmpbuf));
          return; 
        }
        char *data = msg.body;
        while(*data != '\0')
        {
          fprintf(fout,"%c",*data++);                  
        }
        fclose(fout);
        char tmpbuf[]="HTTP/1.1 200 OK\r\n\r\n";
        socket_write(fd,tmpbuf,strlen(tmpbuf));
        struct stat stbuf;
        stat(ret.c_str(),&stbuf);
        int tmpfd = open(ret.c_str(),O_RDONLY,0644);
        tmp.data = (char*)mmap(NULL,stbuf.st_size,PROT_READ,MAP_SHARED,tmpfd,(off_t)0);
#ifdef TEST
        cout<<"new mmap:";
        printf("%s\n",tmp.data);
#endif
        tmp.size = stbuf.st_size;
        close(tmpfd);
      }       
      pthread_rwlock_unlock(&map_lock);
#ifdef TEST
      cout<<"替换完成"<<endl;
#endif
    }

  }
  else
  {
    char tmpbuf[]="HTTP/1.1 400 Bad Request\r\n\r\n";
    socket_write(fd,tmpbuf,strlen(tmpbuf));
    return; 
  }
  return ;
}

//线程池内的线程执行的函数  使用生产者消费者模型
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

    shared_ptr<char> buf(new char[BUFSIZE]);
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
    if(read_size == 0)
    {
      close(fd);
      continue;
    }
    if(read_size == -1 && errno != EAGAIN)
    {
      close(fd);
      continue;
    }
    printf("%s\n",p);
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
