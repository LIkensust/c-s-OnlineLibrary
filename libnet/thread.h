#ifndef __THREAD_H__
#define __THREAD_H__

#include <assert.h>
#include <pthread.h>
#include <tr1/functional>

class Thread {
public:
  typedef std::tr1::function<void()> ThreadFunc;

  Thread(const ThreadFunc &threadFunc)
      : mTid(0), mThreadFunc(threadFunc), mStarted(false) {}

  int Start() {
    assert(!mStarted);
    int ret = pthread_create(&mTid, NULL, Thread::Hook, this);
    mStarted = (ret == 0);
    return ret;
  }

  int Join() {
    assert(mStarted);
    return pthread_join(mTid, NULL);
  }

  int Detach() {
    assert(mStarted);
    return pthread_detach(mTid);
  }

  int SetPriority(int priority, int policy = SCHED_OTHER) {
    struct sched_param schedparam;
    schedparam.sched_priority = priority;
    return pthread_setschedparam(mTid, policy, &schedparam);
  }

private:
  static void *Hook(void *userdata) {
    Thread *thread = reinterpret_cast<Thread *>(userdata);
    assert(thread != NULL);
    if (thread->mThreadFunc) {
      thread->mThreadFunc();
    }
    return NULL;
  }

private:
  pthread_t mTid;
  ThreadFunc mThreadFunc;
  bool mStarted;
};

#endif
