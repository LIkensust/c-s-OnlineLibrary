#ifndef __NET_HTTPSERVER_H__
#define __NET_HTTPSERVER_H__

#include <iostream>
#include <functional>
#include <cassert>
#include <cstring>
#include <memory>
#include <mutex>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include "net_httprequest.h"
#include "net_httpresponse.h"
#include "net_epoll.h"
#include "threadpool.h"
#include "net_timer.h"
#include "data_buffer.h"
#include "net_socktool.h"

#define EPOLL_TIMEOUTS -1 // epoll_wait的超时时间
#define CONNECT_TIMEOUTS 500 // 默认的连接超时时间单位毫秒
#define THREAD_WORKERS 4 // 线程池的大小

class NetHTTPRequest;
class NetEpoll;
class ThreadPool;
class NetTimerManager;

struct execCond;

class NetHTTPServer {
private:
	using listenRequestPtr = std::unique_ptr<NetHTTPRequest>;
	using epollPtr = std::unique_ptr<NetEpoll>;
	using threadPoolPtr = std::shared_ptr<ThreadPool>;
	using timerManagerPtr = std::unique_ptr<NetTimerManager>;

	int _port;
	int _listenFd;
	listenRequestPtr _listenRequest; // 监听套接字的httpRequest实例
	epollPtr _epoll; // epoll实例
	threadPoolPtr _threadPool; // 线程池实例
	timerManagerPtr _timerManager; // 定时管理器

public:
	NetHTTPServer(int port = 20000, int threadNum = 4) 
			: _port(port)
			, _listenFd(gUtils::createListenFd(port))
			, _listenRequest(new NetHTTPRequest(_listenFd))
			, _epoll(new NetEpoll())
			, _threadPool(new ThreadPool())
			, _timerManager(new NetTimerManager())
	{ 
        _threadPool->Init(threadNum);
        assert(_listenFd >= 0); 
        PLOG(INFO, "server init done...");
    }

	void run() {
		PLOG(INFO, "server start");
		// 注册监听套接字到epoll, 将listenRequest指针传进去
		_epoll->AddEpollFd(_listenFd, _listenRequest.get(), (EPOLLIN | EPOLLET));

		// 注册新的连接回调函数
		_epoll->setOnConnection(std::bind(&NetHTTPServer::__acceptConnection, this));

		//注册关闭连接函数
		_epoll->setOnCloseConnection(std::bind(&NetHTTPServer::__closeConnection, this, std::placeholders::_1));

		//注册请求回调函数
		_epoll->setOnRequest(std::bind(&NetHTTPServer::__doRequest, this, std::placeholders::_1));

		//注册响应回调函数
		_epoll->setOnResponse(std::bind(&NetHTTPServer::__doResponse, this, std::placeholders::_1));

		// 事件循环
		for( ; ; ) {
			int timeMs = _timerManager->getNextExpireTime();

			// wait for events
			// 这是真的精辟
			int eventsNum = _epoll->waitEvents(timeMs);

			if(eventsNum > 0) {
				// 分发事件处理函数
				_epoll->handleEvents(_listenFd, _threadPool, eventsNum);
			}
			_timerManager->handleExpireTimers();
		}
	}

	~NetHTTPServer() {
		PLOG(INFO, "server closed...");
	}
private:
	void __acceptConnection() {
		while( true ) {
			int new_sock = ::accept4(_listenFd, nullptr, nullptr, SOCK_NONBLOCK | SOCK_CLOEXEC);
			if(new_sock == -1) {
				if(errno == EAGAIN) {
					break;
				}
				break;
			}

			NetHTTPRequest *req = new NetHTTPRequest(new_sock);
			_timerManager->addTimer(req, CONNECT_TIMEOUTS, std::bind(&NetHTTPServer::__closeConnection, this, req));
			// XXX _epoll->addEpollFd(new_sock, req, (EPOLLIN | EPOLLONESHOT));
			_epoll->AddEpollFd(new_sock, req, (EPOLLIN | EPOLLONESHOT));
		}
	}

	void __closeConnection(NetHTTPRequest *req) {
		if(req->isWorking()) {
			return;
		}
		int fd = req->getSockFd();
		_timerManager->delTimer(req);
		_epoll->delEpollFd(fd, req, 0);
		// 释放这个套接字占用的资源, 在析构函数中close
		delete req;
		req = nullptr;
	}

	// 处理请求报文
	void __doRequest(NetHTTPRequest *req) {
		_timerManager->delTimer(req); // 正在处理的不设置超时
		assert(req != nullptr);
		int work_fd = req->getSockFd();

		int readErrno;
		int readSize = req->readBuff(&readErrno);

		// 客户端断开
		if(readSize == 0) {
			req->setNoWorking();
			__closeConnection(req);
			return;
		}

		// 非EAGAIN错误, 断开连接
		if(readSize < 0 && (readErrno != EAGAIN)) {
			req->setNoWorking();
			__closeConnection(req);
			return;
		}

		// EAGAIN错误释放线程使用权, 并监听下次可读事件
		if(readSize < 0 && (readErrno == EAGAIN)) {
			_epoll->ModEpollFd(work_fd, req, (EPOLLIN | EPOLLONESHOT));
			req->setNoWorking();
			_timerManager->addTimer(req, CONNECT_TIMEOUTS, std::bind(&NetHTTPServer::__closeConnection, this, req));
			return;
		}

		// 报文解析出错断开连接, 发送 400 报文
		if(!req->parseRequest()) {
			// 发送400报文
			execCond cond;
			cond.mStatusCode = 400;
			cond.mPath = "";
			cond.mKeepAlive = false;
			cond.mIsCGI = false;
			cond.mQuery = "";
			cond.mLength = 0;
			gHttpResponse rsp(cond);
			req->appendOutBuffer(rsp.makeResponse());

			// XXX 注意此处可能有bug
			int writeErrno;
			req->writeBuff(&writeErrno);
			req->setNoWorking();
			__closeConnection(req);
			return;
		}

		// 解析完成 发送200
		if(req->parseFinish()) {
			execCond cond;
			cond.mStatusCode = 200;
			cond.mPath = req->getPath();
			cond.mKeepAlive = req->keepAlive();
			cond.mIsCGI = req->isCGi();
			cond.mLength = req->getCgiLength();
			cond.mQuery = req->getQuery();
			gHttpResponse rsp(cond);
			req->appendOutBuffer(rsp.makeResponse());
			_epoll->ModEpollFd(work_fd, req, (EPOLLIN | EPOLLOUT | EPOLLONESHOT));
		}
	}

	void __doResponse(NetHTTPRequest *req) {
		_timerManager->delTimer(req);
		assert(req != nullptr);
		int work_fd = req->getSockFd();

		int toWrite = req->writeAbleBytes();

		if(toWrite == 0) {
			_epoll->ModEpollFd(work_fd, req, (EPOLLIN | EPOLLONESHOT));
			req->setNoWorking();
			_timerManager->addTimer(req, CONNECT_TIMEOUTS, std::bind(&NetHTTPServer::__closeConnection, this, req));
			return;
		}

		int writeErrno;
		int ret = req->writeBuff(&writeErrno);

		if(ret < 0 && writeErrno == EAGAIN) {
			_epoll->ModEpollFd(work_fd, req, (EPOLLIN | EPOLLOUT | EPOLLONESHOT));
			return;
		}

		if(ret < 0 && (writeErrno != EAGAIN)) {
			req->setNoWorking();
			__closeConnection(req);
			return;
		}

		if(ret == toWrite) {
			if(req->keepAlive()) {
				req->resetParse();
				_epoll->ModEpollFd(work_fd, req, (EPOLLIN | EPOLLONESHOT ));
				req->setNoWorking();
				_timerManager->addTimer(req, CONNECT_TIMEOUTS, std::bind(&NetHTTPServer::__closeConnection, this, req));
			} else {
				req->setNoWorking();
				__closeConnection(req);
			}
			return;
		}

		// 可能还有事件发生, 超时时间过再断开连接 防止TCP多次建立连接
		_epoll->ModEpollFd(work_fd, req, (EPOLLIN | EPOLLOUT | EPOLLONESHOT ));
		req->setNoWorking();
		_timerManager->addTimer(req, CONNECT_TIMEOUTS, std::bind(&NetHTTPServer::__closeConnection, this, req));
		return;
	}

}; // end of NetHTTPServer

#endif
