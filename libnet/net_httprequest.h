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
	int _fd;
	DataBuffer _inBuff; // 读缓冲区
	DataBuffer _outBuff; // 写缓冲区
	bool _working; // 正在工作不能被超时事件断开连接
	bool _cgi;

	// 定时器
	NetTimer *_timer;

	// 报文解析
	NetHTTPRequestParseState _state; // 解析状态
	gHttpMethod _method; 
	gHttpVersion _version;
	std::string _path; // url路径
	std::string _query; // url 参数
	std::unordered_map<std::string, std::string> _headers;
	// std::string _content; // post正文, 暂时不用

public:
	NetHTTPRequest(int fd) 
			: _fd(fd)
			, _working(false)
			, _cgi(false)
			, _timer(nullptr)
			, _state(ExpectRequestLine)
			, _method(INVALID)
			, _version(UnKnown)
	{
        assert(fd >= 0); 
        PLOG(INFO, "create the request"); 
    }

	~NetHTTPRequest() {
		::close(_fd);
	}

	int getSockFd() {
		return _fd;
	}

	int readBuff(int *savedErrno) {
		return _inBuff.readFd(_fd, savedErrno);
	}

	int writeBuff(int *savedErrno) {
		return _outBuff.writeFd(_fd, savedErrno);
	}

	void appendOutBuffer(const gBuffer& buf) {
		_outBuff.appendData(buf);
	}

	int writeAbleBytes() {
		return _outBuff.readableBytes();
	}


	// 定时器相关
	void setTimer(NetTimer *timer) {
		_timer = timer;
	}

	NetTimer* getTimer() {
		return _timer;
	}

	void setWorking() {
		_working = true;
	}

	void setNoWorking() {
		_working = false;
	}

	bool isWorking() const {
		return _working;
	}

	// cgi
	bool isCGi() {
		return _cgi;
	}

	int getCgiLength() {
		int len;
		if(__getMethod() == POST) {
			std::stringstream ss;
			ss << getHeader("Content-Length");
		 	ss >> len;
		} else {
			len = _query.size();
		}
		return len;
	}

	bool parseRequest() {
		bool ok = true;
		bool hasMore = true;

		while(hasMore) {
			if(_state == ExpectRequestLine) {
				// 处理请求行
				const char *crlf = _inBuff.findCRLF();
				if(crlf) {
					ok = __parseRequestLine(_inBuff.readPeek(), crlf);
					if(ok) {
						_inBuff.retrieveUntil(crlf + 2);
						_state = ExpectHeaders;
					} else {
						hasMore = false;
					}
				} else {
					hasMore = false;
				}
			} else if (_state == ExpectHeaders) {
				// 处理报头
				const char *crlf = _inBuff.findCRLF();
				if(crlf) {
					const char *colon = std::find(_inBuff.readPeek(), crlf, ':');
					if(colon != crlf) {
						__addHeader(_inBuff.readPeek(), colon, crlf);
					} else { // 报头处理完毕
						// XXX 处理post模块
						if(__getMethod() == POST) {
							_state = ExpectBody;
							_cgi = true;
						} else {
							_state = GotAll;
							hasMore = false;
						}
					}
					_inBuff.retrieveUntil(crlf + 2);
				} else {
					hasMore = false;
				}
			} else if (_state == ExpectBody) {
				// 处理报文体
				int len = atoi(getHeader("Content-Length").c_str());
				__setQuery(_inBuff.readPeek(), len);
				_inBuff.retrieve(len);
				_state = GotAll;
				hasMore = false;
			}
		}
		return ok;
	}

	bool parseFinish() {
		return _state == GotAll;
	}

	void resetParse() {
		_state = ExpectRequestLine;
		_method = INVALID;
		_version = UnKnown;
		_path = "";
		_query = "";
		_cgi = false;
		_headers.clear();
	}

	std::string getPath() const {
		return _path;
	}

	std::string getQuery() const {
		return _query;
	}

	std::string getHeader(const std::string& key) const {
		std::string res;
		if(_headers.find(key) != _headers.end()) {
			res = _headers.find(key)->second;
		}
		return res;
	}

	std::string getMethod() const {
		switch(_method) {
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
		return con == "Keep-Alive" || (_version == HTTP1_1 && con != "close");
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
			_method = GET;
		else if (meth == "POST") 
			_method = POST;
		else if (meth == "HEAD")
			_method = HEAD;
		else if (meth == "PUT")
			_method = PUT;
		else if (meth == "DELETE") 
			_method = DELETE;
		else 
			_method = INVALID;

		return _method != INVALID;
	}

	// 设置url路径
	void __setPath(const char *begin, const char *end) {
		std::string subPath;
		subPath.assign(begin, end);
		if(subPath == "/") {
			subPath = "/index.html";
		}
		_path = WEB_ROOT + subPath;
	}

	// 设置url参数
	void __setQuery(const char *begin, const char *end) {
		_query.assign(begin, end);
		_cgi = true;
	}

	// 设置post参数
	void __setQuery(const char *begin, size_t len) {
		_query.assign(begin, len);
	}
	// 设置版本
	void __setVersion(gHttpVersion ver) {
		_version = ver;
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

		_headers[key] = val;
	}

	std::string __getContenLength() {
		return _headers["Content-Length"];
	}


	gHttpMethod __getMethod() {
		return _method;
	}

	void __printHead() {
		std::cout << "start print head" << std::endl;
		for(auto& pair : _headers) {
			std::cout << pair.first << "---->" << pair.second << std::endl;
		}
		std::cout << "end of print head" << std::endl;
	}

};

#endif
