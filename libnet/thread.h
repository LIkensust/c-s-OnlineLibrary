#ifndef __THREAD_H__
#define __THREAD_H__

#include <assert.h>
#include <pthread.h>
#include <tr1/functional>

class Thread {
public:
    typedef std::tr1::function<void()> ThreadFunc;

    Thread(const ThreadFunc &threadFunc)
        :mTid(0),
         mThreadFunc(threadFunc),
         mStarted(false)
    {}

    int Start() {
        assert(!mStarted);
        int ret = pthread_create(&mTid, NULL, Thread::Hook, this);
        mStarted = (ret == 0);
        return ret;
    }

private:
    static void* Hook(void* userdata) {
        Thread* thread = reinterpret_cast<Thread*>
    }

private:
    pthread_t mTid;
    ThreadFunc mThreadFunc;
    bool mStarted;
};


#endif
