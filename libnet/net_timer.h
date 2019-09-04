#ifndef __NET_TIMER_H__
#define __NET_TIMER_H__

#include <iostream>
#include <vector>
#include <queue>
#include <chrono>
#include <cassert>
#include <mutex>
#include <functional>
#include "net_httprequest.h"
#include "log.h"

using TimeoutCallBack = std::function<void()>;
using Clock = std::chrono::high_resolution_clock;
using MS = std::chrono::milliseconds;
using Timestamp = Clock::time_point; 

class NetHTTPRequest;

class NetTimer {
private:
	Timestamp _expireTime; 
	TimeoutCallBack _callBack;
	bool _delete;

public:
	NetTimer(const Timestamp& when, const TimeoutCallBack& tcb) 
		 : _expireTime(when)
		 , _callBack(tcb)
		 , _delete(false)
	{}

	void setDel() {
		_delete = true;
	}

	bool isDeleted() {
		return _delete;
	}

	Timestamp getExpireTime() const { 
		return _expireTime;
	}

	void runCallBackFunc() {
		_callBack();
	}

	~NetTimer() {}
}; 

struct cmp {
	bool operator()(NetTimer *a, NetTimer *b) {
		assert(a != nullptr && b != nullptr);
		return (a->getExpireTime()) > (b->getExpireTime());
	}
};

class NetTimerManager {
private:
	using TimerQueue = std::priority_queue<NetTimer*, std::vector<NetTimer*>, cmp>;
	TimerQueue _timeQueue;
	Timestamp _now;
	std::mutex _lock;

public:
	NetTimerManager() : _now(Clock::now()) { 
        PLOG(INFO, "timermanager init done...");
    }

	void updateTime() {
		_now = Clock::now();
	}

	void addTimer(NetHTTPRequest *req, const int& timeout, const TimeoutCallBack& tcb) {
		std::unique_lock<std::mutex> lock(_lock);
		assert(req != nullptr);

		updateTime();
		NetTimer *timer = new NetTimer(_now + MS(timeout), tcb);
		_timeQueue.push(timer);

		if(req->getTimer() != nullptr) {
			delTimer(req);
		}
		req->setTimer(timer);
	}

	void delTimer(NetHTTPRequest *req) {
		assert(req != nullptr);
		NetTimer *timer = req->getTimer();
		if(timer == nullptr) {
			return;
		}
		if(timer->isDeleted()) {
			return;
		}
		timer->setDel();
		req->setTimer(nullptr);
	}

	void handleExpireTimers() {
		std::unique_lock<std::mutex> lock(_lock);
		updateTime();
		while(!_timeQueue.empty()) {
			NetTimer *timer = _timeQueue.top();
			assert(timer != nullptr);

			// 定时器被删除
			if(timer->isDeleted()) {
				_timeQueue.pop();
				delete timer; // 添加时使用 new 产生对象
				continue;
			}

			// 优先级队列头没有超时
			if(std::chrono::duration_cast<MS>(timer->getExpireTime() - _now).count() > 0) {
				return;
			}

			// 超时事件发生
			timer->runCallBackFunc();
			_timeQueue.pop();
			delete timer;
		}
	}

	// 返回超时时间(优先级队列中最早的超时时间和当前时间差)
	int getNextExpireTime() {
		std::unique_lock<std::mutex> lock(_lock);
		updateTime();
		int res = -1;
		while(!_timeQueue.empty()) {
			NetTimer *timer = _timeQueue.top();
			if(timer->isDeleted()) {
				_timeQueue.pop();
				delete timer;
				continue;
			}
			res = std::chrono::duration_cast<MS>(timer->getExpireTime() - _now).count();
			res = (res < 0) ? 0 : res;
			break;
		}

		return res;
	}


	~NetTimerManager() {}
}; // end of NetTimerManager


#endif
