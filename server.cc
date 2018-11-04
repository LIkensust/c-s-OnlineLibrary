#include "head.h"
using namespace std;

#define SERVER_IP "127.0.0.1"
#define SERVER_PORT 9090
#define BUF_SIZE 1024

struct sock_ev_write{//用户写事件完成后的销毁，在on_write()中执行
    struct event* write_ev;
    char* buffer;
};
struct sock_ev {//用于读事件终止（socket断开）后的销毁
    struct event_base* base;//因为socket断掉后，读事件的loop要终止，所以要有base指针
    struct event* read_ev;
};

/**
 * 销毁写事件用到的结构体
 */
void destroy_sock_ev_write(struct sock_ev_write* sock_ev_write_struct){
    if(NULL != sock_ev_write_struct){
//        event_del(sock_ev_write_struct->write_ev);//因为写事件没用EV_PERSIST，故不用event_del
        if(NULL != sock_ev_write_struct->write_ev){
            free(sock_ev_write_struct->write_ev);
        }
        if(NULL != sock_ev_write_struct->buffer){
            delete[]sock_ev_write_struct->buffer;
        }
        free(sock_ev_write_struct);
    }
}


/**
 * 读事件结束后，用于销毁相应的资源
 */
void destroy_sock_ev(struct sock_ev* sock_ev_struct){
    if(NULL == sock_ev_struct){
        return;
    }
    event_del(sock_ev_struct->read_ev);
    event_base_loopexit(sock_ev_struct->base, NULL);//停止loop循环
    if(NULL != sock_ev_struct->read_ev){
        free(sock_ev_struct->read_ev);
    }
    event_base_free(sock_ev_struct->base);
//    destroy_sock_ev_write(sock_ev_struct->sock_ev_write_struct);
    free(sock_ev_struct);
}
int getSocket(){
    int fd =socket( AF_INET, SOCK_STREAM, 0 );
    if(-1 == fd){
        cout<<"Error, fd is -1"<<endl;
    }
    return fd;
}

void on_write(int sock, short event, void* arg)
{
    cout<<"on_write() called, sock="<<sock<<endl;
    if(NULL == arg){
        cout<<"Error! void* arg is NULL in on_write()"<<endl;
        return;
    }
    struct sock_ev_write* sock_ev_write_struct = (struct sock_ev_write*)arg;

    char buffer[BUF_SIZE];
    sprintf(buffer, "fd=%d, received[%s]", sock, sock_ev_write_struct->buffer);
//    int write_num0 = write(sock, sock_ev_write_struct->buffer, strlen(sock_ev_write_struct->buffer));
//    int write_num = write(sock, sock_ev_write_struct->buffer, strlen(sock_ev_write_struct->buffer));
    int write_num = write(sock, buffer, strlen(buffer));
    destroy_sock_ev_write(sock_ev_write_struct);
    cout<<"on_write() finished, sock="<<sock<<endl;
}

void on_read(int sock, short event, void* arg)
{
    cout<<"on_read() called, sock="<<sock<<endl;
    if(NULL == arg){
        return;
    }
    struct sock_ev* event_struct = (struct sock_ev*) arg;//获取传进来的参数
    char* buffer = new char[BUF_SIZE];
    memset(buffer, 0, sizeof(char)*BUF_SIZE);
    //--本来应该用while一直循环，但由于用了libevent，只在可以读的时候才触发on_read(),故不必用while了
    int size = read(sock, buffer, BUF_SIZE);
    if(0 == size){//说明socket关闭
        cout<<"read size is 0 for socket:"<<sock<<endl;
        destroy_sock_ev(event_struct);
        close(sock);
        return;
    }
    struct sock_ev_write* sock_ev_write_struct = (struct sock_ev_write*)malloc(sizeof(struct sock_ev_write));
    sock_ev_write_struct->buffer = buffer;
    struct event* write_ev = (struct event*)malloc(sizeof(struct event));//发生写事件（也就是只要socket缓冲区可写）时，就将反馈数据通过socket写回客户端
    sock_ev_write_struct->write_ev = write_ev;
    event_set(write_ev, sock, EV_WRITE, on_write, sock_ev_write_struct);
    event_base_set(event_struct->base, write_ev);
    event_add(write_ev, NULL);
    cout<<"on_read() finished, sock="<<sock<<endl;
}


/**
 * main执行accept()得到新socket_fd的时候，执行这个方法
 * 创建一个新线程，在新线程里反馈给client收到的信息
 */
void* process_in_new_thread_when_accepted(void* arg){
    long long_fd = (long)arg;
    int fd = (int)long_fd;
    if(fd<0){
        cout<<"process_in_new_thread_when_accepted() quit!"<<endl;
        return 0;
    }
    //-------初始化base,写事件和读事件--------
    struct event_base* base = event_base_new();
    struct event* read_ev = (struct event*)malloc(sizeof(struct event));//发生读事件后，从socket中取出数据

    //-------将base，read_ev,write_ev封装到一个event_struct对象里，便于销毁---------
    struct sock_ev* event_struct = (struct sock_ev*)malloc(sizeof(struct sock_ev));
    event_struct->base = base;
    event_struct->read_ev = read_ev;
    //-----对读事件进行相应的设置------------
    event_set(read_ev, fd, EV_READ|EV_PERSIST, on_read, event_struct);
    event_base_set(base, read_ev);
    event_add(read_ev, NULL);
    //--------开始libevent的loop循环-----------
    event_base_dispatch(base);
    cout<<"event_base_dispatch() stopped for sock("<<fd<<")"<<" in process_in_new_thread_when_accepted()"<<endl;
    return 0;
}

/**
 * 每当accept出一个新的socket_fd时，调用这个方法。
 * 创建一个新线程，在新线程里与client做交互
 */
void accept_new_thread(int sock){
    pthread_t thread;
    pthread_create(&thread,NULL,process_in_new_thread_when_accepted,(void*)sock);
    pthread_detach(thread);
}

/**
 * 每当有新连接连到server时，就通过libevent调用此函数。
 *    每个连接对应一个新线程
 */
void on_accept(int sock, short event, void* arg)
{
    struct sockaddr_in remote_addr;
    int sin_size=sizeof(struct sockaddr_in);
    int new_fd = accept(sock,  (struct sockaddr*) &remote_addr, (socklen_t*)&sin_size);
    if(new_fd < 0){
        cout<<"Accept error in on_accept()"<<endl;
        return;
    }
    cout<<"new_fd accepted is "<<new_fd<<endl;
    accept_new_thread(new_fd);
    cout<<"on_accept() finished for fd="<<new_fd<<endl;
}

void usage()
{
    fprintf(stderr,">>图书管理系统服务器端 1.0\n"
                    ">>启动时在参数中指定要绑定的端口\n"
                    ">>./ser -[port]\n"
                    ">>作者 ken\n");
}

int main(int argc,char *argv[]){
    if(argc != 2)
    {
        usage();
        _exit(1);
    }
    
    int port = atoi(argv[1]);
    
    int fd = getSocket();
    if(fd<0){
        cout<<"Error in main(), fd<0"<<endl;
    }
#ifdef TEST
    cout<<"main() fd="<<fd<<endl;
#endif
    struct sockaddr_in local_addr; 
    memset(&local_addr,0,sizeof(local_addr)); 
    local_addr.sin_family=AF_INET; 
    local_addr.sin_addr.s_addr=inet_addr(SERVER_IP);
    local_addr.sin_port=htons(port); 
    int bind_result = bind(fd, (struct sockaddr*) &local_addr, sizeof(struct sockaddr));
    if(bind_result < 0){
        perror("bind");
        return 1;
    }
#ifdef TEST
    cout<<"bind_result="<<bind_result<<endl;
#endif
    listen(fd, 10);
    //设置libevent事件，每当socket出现可读事件，就调用on_accept()
    struct event_base* base = event_base_new();
    struct event listen_ev;
    event_set(&listen_ev, fd, EV_READ|EV_PERSIST, on_accept, NULL);
    event_base_set(base, &listen_ev);
    event_add(&listen_ev, NULL);
    event_base_dispatch(base);
#ifdef TEST
    cout<<"event_base_dispatch() in main() finished"<<endl;
#endif
    event_del(&listen_ev);
    event_base_free(base);
#ifdef TEST
    cout<<"main() finished"<<endl;
#endif
}
