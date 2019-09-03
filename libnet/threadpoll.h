#ifndef __THREADPOLL_H__
#define __THREADPOLL_H__

#include <list>
#include <pthread.h>
#include "net_mutex.h"
#include "net_cond.h"
#include "runnable.h"
#include "task_queue.h"
#include "log.h"


class ThreadPoll
{
public:
    enum TPState
    {
        RUNNING,
        STOP,
        TERMINATED
    };

public:
    ThreadPoll()
        :mCond(mMutex)
    {}
    ~ThreadPoll() {}

private:
    mutable NetMutex  mMutex;
    mutable NetCond   mCond;
    TaskQueue         mQueue;
    volatile size_t   mTaskCount;
    volatile size_t   mFinishCount;

private:
    std::list<Thread*> mThreads;
    volatile TPState   mState;
};

#endif
