#include "include/common/common.h"
#include "include/database/mysql_operation.h"
#include "include/database/redis_operation.h"
#include <string>
//#define TEST
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
  name = oJson["message"]["name"].ToString();
  isbn = oJson["message"]["id"].ToString();
  author = oJson["message"]["basic_info"][0].ToString();
  publisher = oJson["message"]["basic_info"][1].ToString();
  Introduction = oJson["message"]["detil"]["Introduction"].ToString();

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
  printf("===          0.退出          ===\n");
  printf("================================\n");
}

string put_msg()
{
  string ret;
  string name;
  string author;
  string publisher;
  string Introduction;
  string isbn;
  printf("请输入书名\n");
  cin>>name;
  printf("请输入作者\n");
  cin>>author;
  printf("请输入出版社\n");
  cin>>publisher;
  printf("请输入描述\n");
  cin>>Introduction;
  printf("请输入ISBN号\n");
  cin>>isbn;
  neb::CJsonObject oJson ("");
  oJson.AddEmptySubObject ("message");
  oJson["message"].Add ("name", name.c_str());
  oJson["message"].Add ("id", isbn.c_str());
  oJson["message"].AddEmptySubArray ("basic_info");
  oJson["message"]["basic_info"].Add (author.c_str());
  oJson["message"]["basic_info"].Add (publisher.c_str());
  oJson["message"].AddEmptySubObject("detil");
  oJson["message"]["detil"].Add("Introduction",Introduction.c_str());


  ret = oJson.ToString();
  return ret;
}

enum HTTPTYPE
{
  SUCCESS,
  ERR
};

struct HTTPMSG
{
  HTTPTYPE type;
  int code;
  char *body;
};

int read_http_code(const char* buf)
{
  int ret = -1; 
  char num [5] ={0};
  memcpy(num,buf+9,3);
  ret =  atoi(num);
  return ret;
}

int read_http(const char *buf,HTTPMSG& msg)
{
  //HTTP/1.1 404 NOTFOUND\r\n\r\n
  //
  //HTTP/1.1 200 OK\r\n
  //Connection_Type:application/json\r\n\r\n
  //data;
  //
  //HTTP/1.1 503 Server Unavailable
  //
  //HTTP/1.1 200 OK\r\n\r\n"
  //
  //
  //HTTP/1.1 503 Server Unavailable
  //
  msg.type = ERR;
  if(buf == NULL)
    return 0;
  const char *p = buf;
  while(*p != '\r')
  {
    p++;
  }
  char firstline[100] = {0}; 
  memcpy(firstline,buf,p-buf);
  int code = read_http_code(firstline);
  if(code <= 0)
  {
    msg.type = ERR;
    return -1;
  }
  if(code == 200)
  {
    msg.type = SUCCESS;
  }
  msg.body = (strstr(const_cast<char*>(p),"\r\n\r\n") + 4);
  return 1;
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
    it++;
  }
#ifdef TEST
  cout<<__FUNCTION__<<endl;
#endif
  return type;
}

void do_read(int fd)
{
  system("clear");
  printf("============进入书籍查询系统=========\n");
  char buf[DATASIZE]={0};
  string fileid;
  printf(">请输入要查询的书的id(id见文件booklist)\n");
  cin>>fileid;
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
    printf("请再次输入:\n");
    cin>>fileid;
    type = checkinput(fileid);
  }

  fileid = "GET /api/v1/books/"+fileid;
  fileid = fileid + "\r\n\r\n";
  write(fd,fileid.c_str(),fileid.size());
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
  struct HTTPMSG msg;
  int stat = read_http(buf,msg);
  if(stat == -1)
  {
    printf("未知错误\n");
    sleep(1);
    close(fd);
  }
  if(msg.type == ERR)
  {
    if(msg.code == 404)
    {
      printf("404 没有这本书\n");
    }
    if(msg.code == 503)
    {
      printf("503 服务器错误\n");
    }
    sleep(1000);
    close(fd);
    return ;
  }
  showbook(msg.body);
  printf("输入任意键继续\n");
  fflush(stdin);
  getchar();
  getchar();
  close(fd);
}

void do_write(int fd)
{
  system("clear");
  printf("============进入书籍修改系统=========\n");
  char buf[DATASIZE]={0};
  string fileid;
  printf(">请输入要修改的书的id(id见文件booklist)\n");
  cin>>fileid;
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


  string newdata = put_msg(); 
  printf("========新输入的信息===========\n");
  showbook(newdata.c_str());
  printf("========确认修改吗?(y/n)=======\n");
  string yesorno;
  cin>>yesorno;
  while(yesorno!="y" && yesorno!="n")
  {
    cin>>yesorno;
  }
  if(yesorno == "n")
  {
    printf("放弃修改\n");
    printf("输入任意键继续\n");
    fflush(stdin);
    getchar();
    getchar();
    close(fd);
  }
  fileid = "PUT /api/v1/books/"+fileid+"\r\n\r\n";
  fileid += newdata.c_str();
  write(fd,fileid.c_str(),fileid.size());
  memset(buf,0,DATASIZE);
  int num = read(fd,buf,DATASIZE);
  if(num <= 0)
  {
    printf("服务器无响应\n");
    close(fd);
    return ;
  }
  //对响应进行解析
  struct HTTPMSG msg;
  int stat = read_http(buf,msg);
  if(stat == -1)
  {
    printf("未知错误\n");
    sleep(1000);
    close(fd);
  }
  if(msg.type == ERR)
  {
    if(msg.code == 404)
    {
      printf("404 没有这本书\n");
    }
    if(msg.code == 503)
    {
      printf("503 服务器错误\n");
    }
    sleep(1);
    close(fd);
    return ;
  }

  printf("输入任意键继续\n");
  fflush(stdin);
  getchar();
  close(fd);

}

int main(int argc,char *argv[])
{
 auto mysqltool = MysqlTool::make(); 
 mysqltool->do_connect("./config/mysql_cnf.js");
 if(mysqltool->is_connected()) {
   mysqltool->interactive("select * from book_inf");
   MYSQL_ROW row ;
   while((row = mysqltool->get_slution_by_row()) !=NULL) {
     cout<<row[0]<<endl;
     cout<<row[1]<<endl;
     cout<<row[2]<<endl;
   }
 } else {
   cout<<"connect fail"<<endl;
   return 0;
 }
  auto redistool = RedisTool::make(); 
  if(redistool->do_connect("./config/redis_cnf.js")) {
    cout<<"connect success"<<endl;
  } else {
    cout<<"can't connect"<<endl;
  }
  redistool->interaction("get test",REDIS_REPLY_STRING);
  string ret;
  redistool->get_slution(ret,REDIS_REPLY_STRING);
  cout<<ret<<endl;
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
    printf("请输入选项\n");
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
#ifdef TEST
    cout<<"input choose success"<<endl;
#endif
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
