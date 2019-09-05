#ifndef __DATA_BUFFER_H__
#define __DATA_BUFFER_H__
#include <vector>
#include <algorithm>
#include <sys/uio.h>
#include <errno.h>
#include <unistd.h>
#include "log.h"

#define BUFFERSIZE 65535
class DataBuffer
{
public:
    DataBuffer()
        :mCur(0)
    {}
    ~DataBuffer() {}

    const char* GetCur() const {
        return &mCharVec[0] + mCur;
    }

    bool SetCur(const char* p) { 
        if(p == NULL) {
            PLOG(ERROR, "SetCur point is NULL");
            return false;
        }
        size_t size = p - &mCharVec[0];
        SetCur(size);
        return true;
    }

    bool SetCur(const size_t index) {
        if(index >= mCharVec.size()) {
            PLOG(ERROR, "SetCur out of range");
            return false;
        }
        mCur = index;
        return true;
    }

    const std::vector<char>& GetData() {
        return mCharVec;
    }

    size_t ReadFd(size_t fd, int *Errno) {
        char tmpbuff[BUFFERSIZE] = {0};
        bool more = true;
        while(more) {
            int num = read_from_sock(tmpbuff, BUFFERSIZE, fd, Errno);
            if(num < 0) {
                break;
            } else {
                Append(tmpbuff, num);
            }
        }
        if(*Errno != EAGAIN) {
            return -1;
        }
        return mCharVec.size(); 
    }

    int WriteFd(size_t fd, int *Errno) {
        return write_to_sock(fd, Errno);
    }


    int Append(const char* p, const int size) {
        if(p == nullptr || size < 0) {
            return -1;
        }
        for(int i = 0; i < size; i++) {
            mCharVec.push_back(*(p+i));
        }
        return size;
    }

    int Append(const DataBuffer& buff) {
        return Append(&buff.mCharVec[0], buff.mCharVec.size());
    }

    const char *findCRLF() const {
        const char CRLF[] = "\r\n";
        const char *crlf = std::search(&mCharVec[0], &mCharVec[mCharVec.size()-1] + 1, CRLF, CRLF+2);
        return crlf;
    }

    const char* findCRLF(const char *start) const {
        //assert(start <= &mCharVec[mCharVec.size()-1]);
        //assert(start >= &mCharVec[0]);
        if( start > &mCharVec[mCharVec.size()-1] || start < &mCharVec[0]) {
            PLOG(ERROR, "findCRLF start out of range"); 
            return NULL;
        }
        const char CRLF[] = "\r\n";
        const char *crlf = std::search(start, &mCharVec[mCharVec.size()-1] + 1, CRLF, CRLF+2);
        return crlf;
    }
private:

    int write_to_sock(int fd, int* Errno) {
        size_t tmp;
        size_t total = mCharVec.size();
        const char* p = &mCharVec[0];
        while(total > 0) {
            tmp = write(fd, p, total);
            if(tmp < 0) {
                if(errno == EAGAIN) {
                    usleep(500);
                    continue;
                } else {
                    *Errno = errno;
                    return -1;
                }
            }
            if(tmp == total) {
                return tmp;
            }
            total -= tmp;
            p += tmp;
        }
        return tmp;
    }

    int read_from_sock(char *buff, int size, size_t fd, int *Errno) {
        int index = 0;
        int read_size = 0;
        char *ptr = buff;
        while(size > 0) {
            read_size = read(fd, ptr + index, size);
            index += read_size;
            size -= read_size;
            if(read_size == 0) {
                *Errno = errno;
                return -1; // client close
            } else if(read_size == -1 && errno != EAGAIN) {
                *Errno = errno;
                return -1; // err
            } else {
                *Errno = errno;
                break;  // read all data
            }
        }
        return index;
    }

private:
    std::vector<char> mCharVec;
    size_t mCur;
};

#endif
