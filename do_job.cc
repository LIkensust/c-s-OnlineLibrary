#include <iostream>
#include "head.h"
#include <pthread.h>
#include <cassert>
#include <string>

#define HASHSIZE 1000
using namespace std;
//创建一个hash映射 完成文件分类

//这里用的hash函数与shell脚本中的hash函数必须保持一致
int _hash_(const string& filename)
{
    int ret = 0;
    string::const_iterator it = filename.begin();
    while(it != filename.end())
    {
        ret = ret*2 + (*it) - '0';
        it++;
    }
    return ret % HASHSIZE;
}

string itoa(int num)
{
    string ret;
    char tmp[2] = {0};
    if(num == 0)
    {
        ret = "0";
        return ret;
    }
    while(num)
    {
        tmp[0] = num%10 + '0';
        ret = tmp + ret;
        num /= 10;
    }
    return ret;
}

string file_hash(const string& filename)
{
    //文件名是一个1-19位长度的 不重复的数字
    //共计200 0000个文件
    //需要投影到HASHSIZE个文件夹中 每个文件夹里边有2000个文件
    //
    //需不需要维护hash表？
    //目前看不需要维护hash表
    assert(!(filename.empty()));
    int num_of_dir = _hash_(filename);
    string ret = itoa(num_of_dir);
    return ret;
}


int main()
{
    std::cout << "Hello world" << std::endl;
    return 0;
}

