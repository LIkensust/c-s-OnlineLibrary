#include <iostream>
#include <unistd.h>
#include <syscall.h>
#include <sys/types.h>
#include "../threadpoll.h"
#include "../runnable.h"
using namespace std;
class TestTask: public Runnable {
public:

    ~TestTask() {}
    virtual void Run() {
        for(int i = 0 ;i < 3; i++) {
            pid_t tid = syscall(SYS_gettid);
            cout << "[tid]" << tid <<endl;
            sleep(1);
        }
    }

private:

};

int main()
{
    ThreadPoll tp;
    tp.Init(3);
    for(int i=0;i<10;i++) {
        TestTask *tt = new TestTask;
        tp.Execute(tt);
    }
    //tp.Shutdown();
    sleep(2);
    tp.ShutdownNow();
    return 0;
}

