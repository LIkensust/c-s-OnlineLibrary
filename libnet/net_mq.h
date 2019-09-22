#ifndef __NET_MQ_H__
#define __NET_MQ_H__
#include <mqueue.h>
#include "log.h"
#include <fcntl.h>           /* For O_* constants */
#include <sys/stat.h>        /* For mode constants */
#include <string>
#include <time.h>
#include <assert.h>
#include <memory>
#include <string.h> 
namespace {
    const char DEFAULTMQNAME[] = "/default_mq_name";
    static const size_t MESSACESIZE = 65536;
};

class MqData{
public:
    //[socket of query][time]
    MqData(const int socket,const char* data, const size_t size)
        :mSize(0),
         mTag(0)
    {
        //根据query的socket与当前时间戳生成tag
        mTag = getTag(time(NULL));
        mSize = size + sizeof(mTag);
        PLOG(DEBUG, "size %d, all size %d" ,(int)size, (int)mSize);
        mData = std::make_shared<char>(mSize);
        memcpy(mData.get(), &mTag, sizeof(mTag));
        memcpy(mData.get() + sizeof(mTag), data, size);
        PLOG(DEBUG, "data is %s | %s", mData.get() + sizeof(mTag), data);
    }

    MqData(){}

    ~MqData()
    {
        mData.reset();
    }

    void Init(const char* data, size_t size) {
        mSize = size;
        mTag = *((const size_t*)(data));
        mData = std::make_shared<char>(size + sizeof(mTag));
        memcpy(mData.get(), data, size);
    }

    std::shared_ptr<char> GetData() {
        return mData;
    }

    size_t GetTag() {
        return mTag;
    }

    size_t GetSize() {
        return mSize;
    }

private:
    size_t getTag(const int socket) {
        return (socket >> 3) + time(NULL);
    }

private:
    size_t mSize;
    size_t mTag;
    std::shared_ptr<char> mData;
    //char *mData;
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
        } else {
            PLOG(DEBUG, "LINK failed");
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
        } else {
            PLOG(DEBUG, "LINK failed");
        }
        return mLinked;
    }

    ~NetMq() {
        if(mLinked) {
            mq_close(mMq);
            mq_unlink(mName.c_str());
        }
    }

    int Send(MqData& data) {
        if(mLinked == false) {
            return -1;
        }
        return mq_send(mMq, data.GetData().get(), data.GetSize(), 1);
    }

    MqData Recv() {
        assert(mLinked == true);
        char buff[MESSACESIZE] = {0};
        unsigned int msg_prio;
        int size = mq_receive(mMq, buff, MESSACESIZE, &msg_prio);
        PLOG(DEBUG , "get data size %d", size);
        mData.Init(buff, size);
        return mData;
    }


private:
    std::string mName; 
    mqd_t mMq;
    bool mLinked;
    MqData mData;
};

#endif
