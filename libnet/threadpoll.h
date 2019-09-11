#ifndef __THREADPOLL_H__
#define __THREADPOLL_H__

#include <list>
#include <pthread.h>
#include "thread.h"
#include "net_mutex.h"
#include "net_cond.h"
#include "runnable.h"
#include "task_queue.h"
#include "log.h"

class ThreadPoll {
public:
  enum TPState {
    RUNNING,
    STOP,
    TERMINATED
  };

public:
  ThreadPoll() : mCond(mMutex) {}
  ~ThreadPoll() {}

public:
  bool Init(int numThreads) {
    assert(numThreads > 0);
    mState = RUNNING;
    for (int i = 0; i < numThreads; ++i) {
      Thread *thread =
          new Thread(std::tr1::bind(&ThreadPoll::ThreadProc, this));
      int ret = thread->Start();
      if (0 != ret) {
        PLOG(ERROR, "thread start failed");
        delete thread;
        thread = NULL;
        return false;
      }
      mThreads.push_back(thread);
    }
    return true;
  }

  void ShutdownNow() {
    if (mState == STOP) {
      return;
    }

    mState = TERMINATED;
    WakeuoAllThread();
    WaitExit();

    mQueue.Clear();
    mTaskCount = 0;
  }

  void Shutdown() {
    if (mState >= STOP) {
      return;
    }
    mState = STOP;
    WakeuoAllThread();
    WaitExit();
    mState = TERMINATED;
  }

  bool Execute(Runnable *runnabel) {
    ScopedLock lock(mMutex);
    if (mState >= STOP || runnabel == NULL) {
      return false;
    }
    mQueue.Push(runnabel);
    ++mTaskCount;
    mCond.Signal();
    return true;
  }

public:
  size_t GetTaskCount() { return mTaskCount; }

  size_t GetFinishCount() { return mFinishCount; }

private:
  void ThreadProc() {
    mMutex.Lock();
    Runnable *runnable = NULL;
    while (mState <= STOP) {
      while (mState <= STOP && (runnable = mQueue.Front()) != NULL) {
        --mTaskCount;
        mQueue.Pop();
        mMutex.UnLock();
        runnable->Run();
        mMutex.Lock();
        mFinishCount++;
      }
      if (mState < STOP && runnable == NULL) {
        mCond.Wait();
      } else {
        break;
      }
    }
    mMutex.UnLock();
  }

  void WakeuoAllThread() {
    mMutex.Lock();
    mCond.Broadcast();
    mMutex.UnLock();
  }

  void WaitExit() {
    for (auto iter = mThreads.begin(); iter != mThreads.end(); ++iter) {
      (*iter)->Join();
      delete *iter;
    }
  }

private:
  mutable NetMutex mMutex;
  mutable NetCond mCond;
  TaskQueue mQueue;
  volatile size_t mTaskCount;
  volatile size_t mFinishCount;

private:
  std::list<Thread *> mThreads;
  volatile TPState mState;
};

#endif
