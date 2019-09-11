#ifndef __NET_NetHTTPRequest_H__
#define __NET_NetHTTPRequest_H__

#include <unordered_map>
#include <sstream>
#include <algorithm>
#include <assert.h>
#include <stdlib.h>
#include "log.h"
#include "data_buffer.h"

#define WEB_ROOT "../wwwroot"

class NetTimer;

class NetHTTPRequest {
    friend class NetHTTPRequestTester;
public:
	enum NetHTTPRequestParseState {
		ExpectRequestLine,
		ExpectHeaders,
		ExpectBody,
		GotAll
	};

	enum gHttpMethod {
		INVALID, GET, POST, HEAD, PUT, DELETE
	};

	enum gHttpVersion {
		UnKnown, HTTP1_0, HTTP1_1
	};

private:
	int mFd;
	DataBuffer mInBuff; // 读缓冲区
	DataBuffer mOutBuff; // 写缓冲区
	bool mWorking; // 正在工作不能被超时事件断开连接
	bool mCgi;

	// 定时器
	NetTimer *mTimer;

	// 报文解析
	NetHTTPRequestParseState mState; // 解析状态
	gHttpMethod mMethod; 
	gHttpVersion mVersion;
	std::string mPath; // url路径
	std::string mQuery; // url 参数
	std::unordered_map<std::string, std::string> mHeaders;
	// std::string _content; // post正文, 暂时不用

public:
	NetHTTPRequest(int fd) 
			: mFd(fd)
			, mWorking(false)
			, mCgi(false)
			, mTimer(nullptr)
			, mState(ExpectRequestLine)
			, mMethod(INVALID)
			, mVersion(UnKnown)
	{
        assert(fd >= 0); 
        PLOG(DEBUG, "create the request"); 
    }

	~NetHTTPRequest() {
		::close(mFd);
	}

	int getSockFd() {
		return mFd;
	}

	int readBuff(int *savedErrno) {
		return mInBuff.ReadFd(mFd, savedErrno);
	}

	int writeBuff(int *savedErrno) {
		return mOutBuff.WriteFd(mFd, savedErrno);
	}

	void appendOutBuffer(const DataBuffer& buf) {
		mOutBuff.Append(buf);
	}

	//int writeAbleBytes() {
	//	return mOutBuff.readableBytes();
	//}


	// 定时器相关
	void setTimer(NetTimer *timer) {
		mTimer = timer;
	}

	NetTimer* getTimer() {
		return mTimer;
	}

	void setWorking() {
		mWorking = true;
	}

	void setNoWorking() {
		mWorking = false;
	}

	bool isWorking() const {
		return mWorking;
	}

	// cgi
	bool isCGi() {
		return mCgi;
	}

	int getCgiLength() {
		int len;
		if(__getMethod() == POST) {
			std::stringstream ss;
			ss << getHeader("Content-Length");
		 	ss >> len;
		} else {
			len = mQuery.size();
		}
		return len;
	}

	bool parseRequest() {
		bool ok = true;
		bool hasMore = true;

		while(hasMore) {
			if(mState == ExpectRequestLine) {
				// 处理请求行
				const char *crlf = mInBuff.findCRLF();
				if(crlf) {
					ok = __parseRequestLine(mInBuff.GetCur(), crlf);
					if(ok) {
						mInBuff.SetCur(crlf + 2);
						mState = ExpectHeaders;
					} else {
						hasMore = false;
					}
				} else {
					hasMore = false;
				}
			} else if (mState == ExpectHeaders) {
				// 处理报头
				const char *crlf = mInBuff.findCRLF();
				if(crlf) {
                    PLOG(DEBUG, "%s", crlf);
					const char *colon = std::find(mInBuff.GetCur(), crlf, ':');
					if(colon != crlf) {
                        PLOG(DEBUG, "find a head line");
						__addHeader(mInBuff.GetCur(), colon, crlf);
					} else { // 报头处理完毕
						// XXX 处理post模块
                        PLOG(DEBUG, "can't find head line");
						if(__getMethod() == POST) {
							mState = ExpectBody;
							mCgi = true;
						} else {
							mState = GotAll;
							hasMore = false;
						}
					}
					mInBuff.SetCur(crlf + 2);
				} else {
					hasMore = false;
				}
			} else if (mState == ExpectBody) {
				// 处理报文体
				int len = atoi(getHeader("Content-Length").c_str());
				__setQuery(mInBuff.GetCur(), len);
				mInBuff.SetCur(len);
				mState = GotAll;
				hasMore = false;
			}
		}
		return ok;
	}

	bool parseFinish() {
		return mState == GotAll;
	}

	void resetParse() {
		mState = ExpectRequestLine;
		mMethod = INVALID;
		mVersion = UnKnown;
		mPath = "";
		mQuery = "";
		mCgi = false;
		mHeaders.clear();
	}

	std::string getPath() const {
		return mPath;
	}

	std::string getQuery() const {
		return mQuery;
	}

	std::string getHeader(const std::string& key) const {
		std::string res;
		if(mHeaders.find(key) != mHeaders.end()) {
			res = mHeaders.find(key)->second;
		}
		return res;
	}

	std::string getMethod() const {
		switch(mMethod) {
			case GET     : return "GET";
			case POST    : return "POST";
			case HEAD    : return "HEAD";
			case PUT     : return "PUT";
			case DELETE  : return "DELETE";
			case INVALID : return "";
		}
	}

	bool keepAlive() const {
		std::string con = getHeader("Connection");
		return con == "Keep-Alive" || (mVersion == HTTP1_1 && con != "close");
	}

private:
	// 解析请求行
	bool __parseRequestLine(const char *begin, const char *end) {
		bool succeed = false;
		const char *start = begin;
		const char *space = std::find(start, end, ' ');
		if(space != end && __setMethod(start, space)) {
			start = space + 1;
			space = std::find(start, end, ' ');
			if(space != end) {
				const char *query = std::find(start, space, '?'); 
				if(query != space) {
					__setPath(start, query);
					__setQuery(query + 1, space);
				} else {
					__setPath(start, space);
				}
				start = space + 1;
				succeed = end - start == 8 && std::equal(start, end - 1, "HTTP/1.");
				if(succeed) {
					if(*(end - 1) == '1') {
						__setVersion(HTTP1_1);
					} else if (*(end - 1) == '0') {
						__setVersion(HTTP1_0);
					} else {
						succeed = false;
					}
				}
			}
		}

		return succeed;
	}

	// 设置HTTP方法
	bool __setMethod(const char *begin, const char *end) {
		std::string meth(begin, end);
		if(meth == "GET") 
			mMethod = GET;
		else if (meth == "POST") 
			mMethod = POST;
		else if (meth == "HEAD")
			mMethod = HEAD;
		else if (meth == "PUT")
			mMethod = PUT;
		else if (meth == "DELETE") 
			mMethod = DELETE;
		else 
			mMethod = INVALID;

		return mMethod != INVALID;
	}

	// 设置url路径
	void __setPath(const char *begin, const char *end) {
		std::string subPath;
		subPath.assign(begin, end);
		if(subPath == "/") {
			subPath = "/index.html";
		}
		mPath = WEB_ROOT + subPath;
	}

	// 设置url参数
	void __setQuery(const char *begin, const char *end) {
		mQuery.assign(begin, end);
		mCgi = true;
	}

	// 设置post参数
	void __setQuery(const char *begin, size_t len) {
		mQuery.assign(begin, len);
	}
	// 设置版本
	void __setVersion(gHttpVersion ver) {
		mVersion = ver;
	}


	// 增加报头
	void __addHeader(const char *start, const char *colon, const char *end) {
		std::string key(start, colon);
		++colon;
		while(colon < end && *colon == ' ')
			++colon;

		std::string val(colon, end);
		while(!val.empty() && val[val.size() - 1] == ' ')
			val.resize(val.size() - 1);

		mHeaders[key] = val;
	}

	std::string __getContenLength() {
		return mHeaders["Content-Length"];
	}


	gHttpMethod __getMethod() {
		return mMethod;
	}

	void __printHead() {
		std::cout << "start print head" << std::endl;
		for(auto& pair : mHeaders) {
			std::cout << pair.first << "---->" << pair.second << std::endl;
		}
		std::cout << "end of print head" << std::endl;
	}

};

#endif
