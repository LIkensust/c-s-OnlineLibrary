#ifndef __NET_MQ_H__
#define __NET_MQ_H__
#include <mqueue.h>
#include "log.h"
#include <fcntl.h>           /* For O_* constants */
#include <sys/stat.h>        /* For mode constants */
#include <string>

namespace {
    const char DEFAULTMQNAME[] = "/default_mq_name";
};

class NetMq {
public:
    NetMq()
        :mName(DEFAULTMQNAME),
         mLinked(false)
    {}

    NetMq(const std::string name)
        :mName(name),
         mLinked(false)
    {}

    bool Create() {
        if(mLinked) {
            return mLinked;
        }
        mMq = mq_open(mName.c_str(), O_RDWR | O_CREAT | O_EXCL,
                      0777, NULL);
        if(mMq != -1) {
            mLinked = true;
        }
        return mLinked;
    }

    bool Link() {
        if(mLinked) {
            return mLinked;
        }
        mMq = mq_open(mName.c_str(), O_RDWR, 0777, NULL);
        if(mMq != -1) {
            mLinked = true;
        }
        return mLinked;
    }

    ~NetMq() {
        if(mLinked) {
            mq_close(mMq);
            mq_unlink(mName.c_str());
        }
    }

private:
    std::string mName; 
    mqd_t mMq;
    bool mLinked;
};

#endif
