#ifndef __DATA_BUFFER_H__
#define __DATA_BUFFER_H__
#include <vector>

class data_buffer
{
public:
    data_buffer() {}
    ~data_buffer() {}

    int Append(const char* p, const int size) {
        if(p == nullptr || size < 0) {
            return -1;
        }
        for(int i = 0; i < size; i++) {
            mCharVec.push_back(*(p+i));
        }
        return size;
    }
    
    const std::vector<char>& GetData() {
        return mCharVec;
    }
private:
    std::vector<char> mCharVec;
};

#endif
