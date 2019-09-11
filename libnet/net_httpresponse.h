#ifndef __NET_HTTPRESPONSE_H__
#define __NET_HTTPRESPONSE_H__

#include <unordered_map>
#include <string>
#include <unistd.h>
#include <cstdlib>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/fcntl.h>
#include <sys/mman.h>

#include "data_buffer.h"

#define CONNECT_TIMEOUT 500

struct execCond {
  int mStatusCode;
  std::string mPath;
  std::string mQuery;
  bool mKeepAlive;
  bool mIsCGI;
  int mLength;
};

class DataBuffer;

// TODO 替换响应方法以及可以设置响应方法
// 支持CGI以及可以处理更多的响应报头 -- done 2019.5.23
class gHttpResponse {
private:
  std::unordered_map<std::string, std::string> mHeaders; // 响应头
  int mStatusCode;
  std::string mPath; // 请求资源路径
  bool mKeepAlive;
  bool mCgi;
  int mConLength;
  std::string mQuery;

public:
  static const std::unordered_map<int, std::string> mStatusCodeToMessage;
  static const std::unordered_map<std::string, std::string> suffixToType;

  gHttpResponse(execCond &cond)
      : mStatusCode(cond.mStatusCode), mPath(cond.mPath),
        mKeepAlive(cond.mKeepAlive), mCgi(cond.mIsCGI),
        mConLength(cond.mLength), mQuery(cond.mQuery) {
    PLOG(DEBUG, "make the response");
  }

  DataBuffer makeResponse() {
    DataBuffer output;

    if (mStatusCode == 400) {
      doErrorResponse(output, "server can't prase the request");
      return output;
    }

    // 文件错误 404
    struct stat st;
    if (::stat(mPath.data(), &st) < 0) {
      mStatusCode = 404;
      doErrorResponse(output, "server can't find the file");
      return output;
    }

    // 权限错误 403
    if (!(S_ISREG(st.st_mode) || !(S_IRUSR & st.st_mode))) {
      mStatusCode = 403;
      doErrorResponse(output, "server can't read the file");
      return output;
    }

    if (S_ISDIR(st.st_mode)) {
      mPath += "/index.html";
      PLOG(DEBUG, "file path %s", mPath.c_str());
    } else if ((S_IXUSR & st.st_mode) || (S_IXGRP & st.st_mode) ||
               (S_IXOTH & st.st_mode)) {
      mCgi = true;
    }

    if (mCgi) {
      doCgiResponse(output);
    } else {
      doStaticResponse(output, st.st_size);
    }
    return output;
  }

  void doCgiResponse(DataBuffer &output) {
    // XXX 是post请求再断言
    if (mConLength != static_cast<int>(mQuery.size())) {
      PLOG(ERROR, "mConLength[%d] != mQuery.size[%d]", mConLength,
           static_cast<int>(mQuery.size()));
      return;
    }
    // 使用2个管道进行进程间通信, 因为获得参数中有 =
    // 直接设置为环境变量会有歧义
    int in_pipe[2];
    int out_pipe[2];

    ::pipe(in_pipe);
    ::pipe(out_pipe);

    pid_t id = ::fork();

    if (id < 0) {
      mStatusCode = 500;
      doErrorResponse(output, "The server is temporarily unavailable");
    } else if (id == 0) {
      ::close(in_pipe[1]);
      ::close(out_pipe[0]);

      ::dup2(in_pipe[0], 0);
      ::dup2(out_pipe[1], 1);

      std::string chLenEnv = "CONTENT_LENGTH=";
      chLenEnv += std::to_string(mConLength);

      ::putenv(const_cast<char *>(chLenEnv.c_str()));

      ::execl(mPath.c_str(), mPath.c_str(), NULL);
      ::exit(1);
    } else {
      ::close(in_pipe[0]);
      ::close(out_pipe[1]);

      size_t n = ::write(in_pipe[1], mQuery.c_str(), mQuery.size());
      if (n <= 0) {
        PLOG(ERROR, "write to pipe failed");
        return;
      }

      std::string sendText;
      char ch = '\0';
      while (::read(out_pipe[0], &ch, 1) > 0) {
        sendText.push_back(ch);
      }

      auto it = mStatusCodeToMessage.find(mStatusCode);
      output.Append("HTTP/1.1 " + std::to_string(mStatusCode) + " " +
                    it->second + "\r\n");
      // 报文头
      if (mKeepAlive) {
        output.Append("Connection: Keep-Alive\r\n");
        output.Append("Keep-Alive: timeout=" + std::to_string(CONNECT_TIMEOUT) +
                      "\r\n");
      } else {
        output.Append("Connection: close\r\n");
      }
      output.Append("Content-type: " + __getFileType() + ";charset=utf-8\r\n");
      output.Append("Content-length: " + std::to_string(sendText.size()) +
                    "\r\n");
      // TODO 还有别的头部信息?
      output.Append("Server: gHTTP\r\n");
      output.Append("\r\n");

      output.Append(sendText);
    }
  }

  void doStaticResponse(DataBuffer &output, long fileSize) {
    if (fileSize < 0) {
      PLOG(ERROR, "fileSize [%ld]", fileSize);
      return;
    }

    auto it = mStatusCodeToMessage.find(mStatusCode);
    if (it == mStatusCodeToMessage.end()) {
      mStatusCode = 400;
      doErrorResponse(output, "UnKnow status code");
      return;
    }

    // 响应行
    output.Append("HTTP/1.1 " + std::to_string(mStatusCode) + " " + it->second +
                  "\r\n");
    // 报文头
    if (mKeepAlive) {
      output.Append("Connection: Keep-Alive\r\n");
      output.Append("Keep-Alive: timeout=" + std::to_string(CONNECT_TIMEOUT) +
                    "\r\n");
    } else {
      output.Append("Connection: close\r\n");
    }
    output.Append("Content-type: " + __getFileType() + ";charset=utf-8\r\n");
    output.Append("Content-length: " + std::to_string(fileSize) + "\r\n");
    // TODO 还有别的头部信息?
    output.Append("Server: gHTTP\r\n");
    output.Append("\r\n");

    //报文体
    int reqFile_fd = ::open(mPath.data(), O_RDONLY, 0);
    // 存储映射IO
    void *mmapRet =
        ::mmap(NULL, fileSize, PROT_READ, MAP_PRIVATE, reqFile_fd, 0);
    ::close(reqFile_fd);
    if (mmapRet == (void *)-1) {
      munmap(mmapRet, fileSize);
      output.Retrieve();
      mStatusCode = 404;
      doErrorResponse(output, "server can't find the file");
      return;
    }

    char *srcAddr = static_cast<char *>(mmapRet);
    output.Append(srcAddr, fileSize);
    munmap(srcAddr, fileSize);
  }

  void doErrorResponse(DataBuffer &output, std::string message) {
    std::string body;

    auto it = mStatusCodeToMessage.find(mStatusCode);
    if (it == mStatusCodeToMessage.end()) {
      return;
    }

    body += "<html><title>server Error</title>";
    body += "<body bgcolor=\"ffffff\">";
    body += std::to_string(mStatusCode) + " : " + it->second + "\n";
    body += "<p>" + message + "</p>";
    body += "<hr><em>web server</em></body></html>";

    // 响应行
    output.Append("HTTP/1.1 " + std::to_string(mStatusCode) + " " + it->second +
                  "\r\n");

    output.Append("Server: gHTTP\r\n");
    output.Append("Content-Type: text/html; charset=utf-8\r\n");
    output.Append("Connection: close\r\n");
    output.Append("Content-Length: " + std::to_string(body.size()) +
                  "\r\n\r\n");

    output.Append(body);
  }

  ~gHttpResponse() {}

private:
  std::string __getFileType() {
    // 找不到文件后缀, 默认纯文本
    if (mPath.find_last_of('.') == std::string::npos) {
      if (mCgi) {
        return "text/html";
      }
      return "text/plain";
    }

    auto it = suffixToType.find(mPath.substr(mPath.find_last_of('.')));
    if (it == suffixToType.end()) {
      if (mCgi) {
        return "text/html";
      }
      return "text/plain";
    }

    return it->second;
  }

}; // end of gHttpResponse

const std::unordered_map<int, std::string> gHttpResponse::mStatusCodeToMessage{
  { 200, "OK" },
  { 400, "Bad Request" },
  { 404, "Not Found" },
  { 500, "Internal Server Error" }
};

const std::unordered_map<std::string, std::string> gHttpResponse::suffixToType{
  { ".html", "text/html" },
  { ".xml", "text/xml" },
  { ".xhtml", "application/xhtml+xml" },
  { ".txt", "text/plain" },
  { ".rtf", "application/rtf" },
  { ".pdf", "application/pdf" },
  { ".word", "application/nsword" },
  { ".png", "image/png" },
  { ".gif", "image/gif" },
  { ".jpg", "image/jpeg" },
  { ".jpeg", "image/jpeg" },
  { ".au", "audio/basic" },
  { ".mpeg", "video/mpeg" },
  { ".mpg", "video/mpeg" },
  { ".avi", "video/x-msvideo" },
  { ".gz", "application/x-gzip" },
  { ".tar", "application/x-tar" },
  { ".css", "text/css" }
};

#endif
