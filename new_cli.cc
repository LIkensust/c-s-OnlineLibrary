#include "head.h"
#include <string>
#include "./CJsonObject/CJsonObject.hpp"
#include <string>
#include <cstdio>
#include <cstdlib>
#define DATASIZE 5000
using namespace std;

char testbuf[]="{\"message\":{\"name\":\"UNIX环境高级编程\",\"id\":\"9\",\"basic_info\":\
                [\"author:ichard Stevens / Stephen A.Rago\",\"publisher:人民邮电出版社\",\"\
                isbn:9\"],\"detil\":{\"Introduction\":\"现在是修改过的内容\"}}}";

void showbook(const char *p)
{
    string str = p;
    neb::CJsonObject oJson(str.c_str());
    string name;
    string isbn;
    string author;
    string publisher;
    string Introduction;
    oJson["message"]["name"].Get("name",name);
    oJson["message"]["id"].Get("isbn",isbn);
    author = oJson["message"]["basic_info"][0].ToString();
    publisher = oJson["message"]["basic_info"][1].ToString();
    oJson["detil"].Get("Introduction",Introduction);

    printf("《%s》\n",name.c_str());
    printf("ISBN号:%s\n",isbn.c_str());
    printf("作者:%s\n",author.c_str());
    printf("出版社%s\n",publisher.c_str());
    printf("描述%s\n",Introduction.c_str());
}

void usage()
{
    fprintf(stderr,"图书信息管理系统 1.0\n"
            "支持查询图书信息 提交对某个图书信息的修改\n"
            "./cli [服务器ip] [服务端口号]\n");
    return;
}

void menu()
{
    printf("================================\n");
    printf("===      图书信息管理系统    ===\n");
    printf("===          1.查询图书      ===\n");
    printf("===          2.修改图书      ===\n");
    printf("================================\n");
}

string put_msg()
{
    string ret;
    neb::CJsonObject oJson ("");
    return ret;
}


enum INPUTTYPE
{
    OK,
    TOOLONG,
    NOTNUM,
    EMPTY
};

INPUTTYPE checkinput(string input)
{
    INPUTTYPE type = OK;
    string::iterator it = input.begin();
    if(input.empty())
    {
        type = EMPTY;
        return type;
    }
    if(input.size() > 20)
    {
        type = TOOLONG;
        return type;
    }
    while(it != input.end())
    {
        if( !(*it<='9' && *it >= '0') )
        {
            type = NOTNUM;
            return type;
        }
    }
    return type;
}

void do_read(int fd)
{
    system("clear");
    printf("============进入书籍查询系统=========\n");
    char buf[DATASIZE]={0};
    string fileid;
    printf(">请输入要查询的书的id(id见文件booklist)\n");
    INPUTTYPE type = checkinput(fileid);
    while(type != OK)
    {
        switch(type)
        {
        case NOTNUM:
            printf("请输入数字选项\n");
            break;
        case EMPTY:
            printf("请输入选项\n");
            break;
        case TOOLONG:
            printf("输入太长\n");
            break;
        default:break;
        }
        menu();
        printf("请再次输入:\n");
        cin>>fileid;
        type = checkinput(fileid);
    }

    fileid = "GET /api/v1/books/"+fileid;
    fileid = fileid + "\r\n\r\n";
    write(fd,fileid.c_str(),fileid.size());
    checkinput(fileid);
    //shutdown(fd,SHUT_WR);
    memset(buf,0,DATASIZE);
    int num = read(fd,buf,DATASIZE);
    if(num < 0)
    {
        printf("服务器无响应\n");
        close(fd);
        return;
    }
    //对响应进行解析
    cout<<num<<endl;
    printf("%s\n",buf);
    close(fd);
}

void do_write(int fd)
{
    system("clear");
    printf("============进入书籍修改系统=========\n");
    char buf[DATASIZE]={0};
    string fileid;
    printf(">请输入要修改的书的id(id见文件booklist)\n");
    INPUTTYPE type = checkinput(fileid);
    while(type != OK)
    {
        switch(type)
        {
        case NOTNUM:
            printf("请输入数字选项\n");
            break;
        case EMPTY:
            printf("请输入选项\n");
            break;
        case TOOLONG:
            printf("输入太长\n");
            break;
        default:break;
        }
        menu();
        printf("请再次输入:\n");
        cin>>fileid;
        type = checkinput(fileid);
    }
   
    
    
    fileid = "PUT /api/v1/books/"+fileid+"\r\n\r\n";
    fileid += testbuf;
    write(fd,fileid.c_str(),fileid.size());
    memset(buf,0,DATASIZE);
    int num = read(fd,buf,DATASIZE);
    cout<<num<<endl;
    printf("%s\n",buf);
    close(fd);
}

int main(int argc,char *argv[])
{
    showbook(testbuf);
    if(argc != 3)
    {
        usage();
        return 0;
    }
    while(1)
    {
        struct sockaddr_in sock;
        sock.sin_family = AF_INET;
        sock.sin_addr.s_addr = inet_addr(argv[1]);
        int tmp_port = atoi(argv[2]);
        sock.sin_port = htons(tmp_port);

        int fd = socket(AF_INET,SOCK_STREAM,0);
        int con_success = connect(fd,(struct sockaddr*)(&sock),sizeof(struct sockaddr_in));
        if(con_success < 0)
        {
            printf("连接服务器失败 程序退出\n");
            return 1;
        }

        string input;
        int choose;
        menu();
        cin>>input;

        //对输入合法性的检测
        INPUTTYPE type = checkinput(input);
        while(type != OK)
        {
            switch(type)
            {
            case NOTNUM:
                printf("请输入数字选项\n");
                break;
            case EMPTY:
                printf("请输入选项\n");
                break;
            case TOOLONG:
                printf("输入太长\n");
                break;
            default:break;
            }
            menu();
            printf("请再次输入:\n");
            cin>>input;
            type = checkinput(input);
        }
        choose = atoi(input.c_str());
        if(choose == 1)
        {
            do_read(fd);
        }
        else if(choose == 2)
        {
            do_write(fd);
        }
        else if(choose == 0)
        {
            close(fd);
            return 1;
        }
        else
        {
            printf("请输入正确选项\n");
            close(fd);
        }
    system("clear");
    }
    return 0;
}
