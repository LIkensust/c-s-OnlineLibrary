#ifndef __NET_COND_H__
#define __NET_COND_H__

#include <pthread.h>
#include <sys/time.h>
#include "./net_mutex.h"

class NetCond {
public:
  explicit NetCond(NetMutex &mutex) : mMutex(mutex) {
    pthread_cond_init(&mCond, NULL);
  }

  ~NetCond() { pthread_cond_destroy(&mCond); }

  int Wait(int64_t usec = 0) {
    if (usec == 0) {
      return pthread_cond_wait(&mCond, &mMutex.mMutex);
    }
    timespec ts = GetTimeSpec(usec);
    return pthread_cond_timedwait(&mCond, &mMutex.mMutex, &ts);
  }

  int Signal() { return pthread_cond_signal(&mCond); }

  int Broadcast() { return pthread_cond_broadcast(&mCond); }

private:
  int64_t GetTimeInMs() {
    struct timeval tval;
    gettimeofday(&tval, NULL);
    return tval.tv_sec * 1000000LL + tval.tv_usec;
  }

  timespec GetTimeSpec(int64_t userOffset) {
    timespec tspec;
    int64_t uTime = GetTimeInMs() + userOffset;
    tspec.tv_sec = uTime / 1000000;
    tspec.tv_nsec = (uTime % 1000000) * 1000;
    return tspec;
  }

private:
  pthread_cond_t mCond;
  NetMutex &mMutex;
};
#endif
