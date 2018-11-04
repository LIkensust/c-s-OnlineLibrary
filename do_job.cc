#include <iostream>
#include "head.h"
#include <pthread.h>
#include <cassert>
#include <string>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
using namespace std;
int HASHSIZE = 0;

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

string _itoa_(int num)
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
    //需要投影到HASHSIZE个文件夹中 每个文件夹里边有2000个文件
    //
    //需不需要维护hash表？
    //目前看不需要维护hash表
    assert(!(filename.empty()));
    int num_of_dir = _hash_(filename);
    string ret = _itoa_(num_of_dir);
    ret = "./books/dir"+ret;
    ret = ret+"/";
    ret = ret + filename;
    return ret;
}

bool init_hash()
{
    FILE* cfg = fopen("./hash_config.cfg","r");
    if(cfg == NULL)
    {
        return false;
    }
    fscanf(cfg,"%d",&HASHSIZE);
    fclose(cfg);
    return true;
}

int test()
{
    init_hash();
    string ret;
    FILE* booklist = fopen("./booklist","r");
    char buf[20] = {0};
    while(!feof(booklist))
    {
        memset(buf,0,20);
        fscanf(booklist,"%s\n",buf);
        string name = buf;
        ret = file_hash(name);
        int fd = open(ret.c_str(),O_RDONLY,0644);
        if(fd <= 0)
        {
            printf("open fild\n");
        }
        else
        {
            printf("fd:%d,path:%s\n",fd,ret.c_str());
        }

    }
    return 0;
}

int main()
{
    test();
    return 0;
}
