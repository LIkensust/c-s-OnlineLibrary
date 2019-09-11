#ifndef __NET_MUTEX_H__
#define __NET_MUTEX_H__
#include <mutex>

class NetMutex {
public:
  NetMutex() { pthread_mutex_init(&mMutex, NULL); }

  ~NetMutex() { pthread_mutex_destroy(&mMutex); }

  int Lock() { return pthread_mutex_lock(&mMutex); }

  int TryLock() { return pthread_mutex_trylock(&mMutex); }

  int UnLock() { return pthread_mutex_unlock(&mMutex); }

private:
  friend class NetCond;
  pthread_mutex_t mMutex;

private:
  NetMutex(const NetMutex &) = delete;
  NetMutex &operator=(const NetMutex &) = delete;
};

class ScopedLock {
public:
  explicit ScopedLock(NetMutex &mutex) : mMutex(mutex) { mMutex.Lock(); }

  ~ScopedLock() { mMutex.UnLock(); }

private:
  NetMutex &mMutex;

private:
  ScopedLock(const ScopedLock &) = delete;
  ScopedLock &operator=(const ScopedLock &) = delete;
};

#endif
