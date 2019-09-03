#ifndef __LOG_H__
#define __LOG_H__

#include <iostream>
#include <string>
#include <sys/time.h>
#include <unistd.h>
#include <stdarg.h>
#include <cinttypes>

#define PLOG(t, m) Log::DoLog(t, m) 

enum LOGTYPE {
    DEBUG   = 1,
    INFO    = 2,
    WARNING = 3,
    ERROR   = 4
};


class Log 
{
public:
    Log()
         :mTypeSetted(false)
    {}
    static void DoLog(const LOGTYPE type, std::string mess) {
        if(type < mLogLevel) {
            return;
        }
        std::cout << "[" << getTimeStap()  << "]";
        std::cout << "[" << getLevel(type) << "]";
        std::cout << "[" << mess << "]"    << std::endl;
    } 

    bool SetLevel(const LOGTYPE& type) {
        if(mTypeSetted == true) {
            return false;
        }
        mLogLevel = type;
        mTypeSetted = true;
        return true;
    }

private:
    static std::string getLevel(const LOGTYPE& type) {
        switch(type) {
            case DEBUG:
                return "DEBUG";
            case INFO:
                return "INFO";
            case WARNING:
                return "WARNING";
            case ERROR :
                return "ERROR";
            default:
                return "";
        }
    }

    static uint64_t getTimeStap() {
        struct timeval time;
        gettimeofday(&time, NULL);
        return time.tv_sec;
    }

private:
    static LOGTYPE mLogLevel;
    bool mTypeSetted;
};

LOGTYPE Log::mLogLevel = INFO;

#endif
