#ifndef __NET_EPOLL_H__
#define __NET_EPOLL_H__

#include <memory>
#include <functional>
#include <sys/epoll.h>
#include "log.h"
#include "data_buffer.h"
#include "net_mutex.h"
#include "net_cond.h"
#include "threadpool.h"
#include "runnable.h"

#include "net_httprequest.h"

#define MAXEVENTS 4096

class NetEpoll {

public:
    typedef std::function<void()> NetConnectionCallBackFunction;
    typedef std::function<void(NetHTTPRequest*)> NetCallBackFunction;
private:
    typedef  std::vector<struct epoll_event> eventList;

    int mEpollFd;
    eventList mEvents;
    NetConnectionCallBackFunction mOnConnection;
    NetCallBackFunction mOnCloseConnection;
    NetCallBackFunction mOnRequest;
    NetCallBackFunction mOnResponse;


public:
    NetEpoll() : mEpollFd(epoll_create1(EPOLL_CLOEXEC)), mEvents(MAXEVENTS) {
        assert(mEpollFd >= 0);
        PLOG(INFO, "epoll init");
    }

    int AddEpollFd(int fd, NetHTTPRequest *req, int event) {
        struct epoll_event ev;
        ev.data.ptr = static_cast<void*>(req);
        ev.events = event;
        return epoll_ctl(mEpollFd, EPOLL_CTL_ADD, fd, &ev);
    }

	int ModEpollFd(int fd, NetHTTPRequest *req, int event) {
		struct epoll_event ev;
		ev.data.ptr = static_cast<void*>(req);
		//ev.data.ptr = (void*)req;
		ev.events = event;
		return ::epoll_ctl(mEpollFd, EPOLL_CTL_MOD, fd, &ev);
	}

	int delEpollFd(int fd, NetHTTPRequest *req, int event) {
		struct epoll_event ev;
		ev.data.ptr = static_cast<void*>(req);
		//ev.data.ptr = (void*)req;
		ev.events = event;
		return ::epoll_ctl(mEpollFd, EPOLL_CTL_DEL, fd, &ev);
	}

	int waitEvents(int timeoutMs) {
		int eventsNum = ::epoll_wait(mEpollFd, &*mEvents.begin(), static_cast<int>(mEvents.size()), timeoutMs);
		if(eventsNum == 0) {
			PLOG(INFO, "epoll wait time out");
		} else if (eventsNum < 0) {
			PLOG(WARNING, "epoll wait return -1");
		}

		return eventsNum;
	}

	void handleEvents(int listen_fd, std::shared_ptr<ThreadPool>& thread_pool, int eventsNum) {
		assert(eventsNum > 0);
		PLOG(INFO, "begin handle events");
		for(int i = 0; i < eventsNum; ++i) {
			// 直接强转?
			NetHTTPRequest *req = static_cast<NetHTTPRequest*>(mEvents[i].data.ptr);
			// NetHTTPRequest *req = (NetHTTPRequest*)(mEvents[i].data.ptr);
			int work_fd = req->getSockFd();

			if(work_fd == listen_fd) {
				mOnConnection(); // 回调函数
			} else {
				if( (mEvents[i].events & EPOLLERR) ||
					(mEvents[i].events & EPOLLHUP) ||
					((!mEvents[i].events) & EPOLLIN) ) {
					req->setNoWorking();
					mOnCloseConnection(req);
				} else if (mEvents[i].events & EPOLLIN) {
					req->setWorking();
					thread_pool->Execute(new EpollTask(mOnRequest, req));
				} else if (mEvents[i].events & EPOLLOUT) {
					req->setWorking();
					thread_pool->Execute(new EpollTask(mOnRequest, req));
				} else {
					std::cout << __FILE__ << " : " << __LINE__ << "epoll event err" << std::endl;
				}
			}
		}
		PLOG(INFO, "events handle done");
	}

	void setOnConnection(const NetConnectionCallBackFunction& cb) {
		mOnConnection = cb;
	}

	void setOnCloseConnection(const NetCallBackFunction& cb) {
	    mOnCloseConnection = cb;
	}

	void setOnRequest(const NetCallBackFunction& cb) {
		mOnRequest = cb;
	}

	void setOnResponse(const NetCallBackFunction& cb) {
		mOnRequest = cb;
	}

	~NetEpoll() {
		::close(mEpollFd);
		PLOG(INFO, "close epollfd...");
	}


private:
    class EpollTask : public Runnable {
    public:
        EpollTask(NetEpoll::NetCallBackFunction f,
                  NetHTTPRequest* req)
            :mF(f),
            mReq(req)
        {
        }
        virtual ~EpollTask() {
            Destroy();
        }
        virtual void Destroy() {
            delete this;
        }
        virtual void Run() {
            mF(mReq);
        }
    private:
        NetEpoll::NetCallBackFunction mF;
        NetHTTPRequest *mReq;
    };
};


#endif
