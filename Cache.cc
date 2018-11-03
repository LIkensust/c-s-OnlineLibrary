#include <iostream>
#include <string>
#include <map>
//维护一个缓冲区 
//功能：打开的文件进行内存映射之后 将指向映射区的指针存放
//读文件先到缓冲区查找 找不到就读到缓存区
//如果缓冲区文件过多 就进行一次释放
//如果被释放的是脏内存 就先写到文件中 再释放
//如果被释放的内存未被修改 直接释放
//
//修改文件的逻辑是：由于映射方式的限制 文件不可以超过原本长度 这样会
//造成很大的限制 所以先上写锁 直接释放 然后开一片地址存放
//新的内容 写完之后直接将文件内容写入原文件

//有可能一的线程查找到文件在缓冲中 但是在上读取的时候缓冲中的内容被
//释放了
//
//所以按照经典的生产者消费者模型：
using namespace std;

struct Bascinf
{
    const void * const _ptr;
    const size_t _size;

    Bascinf()
        :_ptr(NULL),
        _size(0)
    {}
};

struct Cacheinf
{
    long long _visited;
    size_t _size;
    std::string _id;
    bool _changed;
    void* _ptr;

    Cacheinf()
        :_visited(0),
        _size(0),
        _changed(false),
        _ptr(NULL)
    {}
};


typedef map<string,Cacheinf> Map;

Bascinf getfile(Map& cachemap,string id)
{
    Bascinf ret;
    if(cachemap.find(id) == cachemap.end())
    {
        //do open
        return ret;
    }
    return ret;
}



