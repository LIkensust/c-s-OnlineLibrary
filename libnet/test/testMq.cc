#include <iostream>
#include <unistd.h>
#include <sys/wait.h>
#include <stdio.h>
#include "../net_mq.h"
using namespace std;
int main()
{
    srand((unsigned int)time(NULL));
    int id = 0;
    id = fork();
    if(id > 0) {
        NetMq q;
        q.Create();
        for(int i = 0;i<10;i++) {
            sleep(rand()%2);
            char buff[] = "hello world";
            MqData data(1,buff,sizeof(buff));
            q.Send(data);
        }
        wait(NULL);
    } else {
        sleep(1);
        NetMq q;
        q.Link();
        for(int i=0;i<10;i++) {
            sleep(rand()%2);
            MqData ret = q.Recv();
            auto data = ret.GetData();
            printf("message : [%s]\n", data.get() + sizeof(size_t));
        }
    }
    return 0;
}

