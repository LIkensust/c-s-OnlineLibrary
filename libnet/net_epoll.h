#ifndef __NET_EPOLL_H__
#define __NET_EPOLL_H__

#include <memory>
#include <functional>
#include <sys/epoll.h>
#include "log.h"
#include "data_buffer.h"
#include "net_mutex.h"
#include "net_cond.h"
#include "threadpoll.h"
#include "runnable.h"

#define MAXEVENTS 4096

class EpollTask : public Runnable {
  typedef std::function<void(NetHTTPRequest *)> NetCallBackFunction;
};

class NetEpoll {
public:
  NetEpoll() : mEpollFd(epoll_create1(EPOLL_CLOEXEC)), mEventVec(MAXEVENTS) {
    assert(mEpollFd >= 0);
    PLOG(INFO, "epoll init");
  }

private:
  typedef std::vector<struct epoll_event> EventVec;

  int mEpollFd;
  EventVec mEventVec;
};

#endif
