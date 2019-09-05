#include <iostream>
#include <string>
#include "../net_httprequest.h"
using namespace std;

class NetHTTPRequestTester {
public:
    void test(const char* data, int size){
        NetHTTPRequest nhr(1);
        nhr.mInBuff.Append(data, size);
        nhr.parseRequest();
        //cout<<nhr.getPath()<<endl;
        //cout<<nhr.getQuery()<<endl;
        //cout<<nhr.getHeader("Referer")<<endl;
        //cout<<nhr.getMethod()<<endl;
        PLOG(INFO,nhr.getPath().c_str());
        PLOG(INFO,nhr.getQuery().c_str());
        PLOG(INFO,nhr.getHeader("Accept").c_str());
        PLOG(INFO,nhr.getMethod().c_str());
        for(auto it = nhr.mHeaders.begin(); it != nhr.mHeaders.end(); ++it) {
            cout<<it->first<<endl;
        }
    }
private:
};

int main()
{
    char data[] = "GET /search?hl=zh-CN&source=hp&q=domety&aq=f&oq= HTTP/1.1\r\n"
        "Accept: image/gif, image/x-xbitmap, image/jpeg, image/pjpeg, application/vnd.ms-excel, application/vnd.ms-powerpoint, application/msword,"
        "application/x-silverlight, application/x-shockwave-flash, */*\r\n"
        "Referer: <a href=\"http:\/\/www.google.cn\/\">http://www.google.cn/</a>\r\n"
        "Accept-Language: zh-cn\r\n"
        "Accept-Encoding: gzip, deflate\r\n"
        "User-Agent: Mozilla/4.0 (compatible; MSIE 6.0; Windows NT 5.1; SV1; .NET CLR 2.0.50727; TheWorld)\r\n"
        "Host: <a href=\"http:\/\/www.google.cn\">www.google.cn</a>\r\n"
        "Connection: Keep-Alive\r\n"
        "Cookie: PREF=ID=80a06da87be9ae3c:U=f7167333e2c3b714:NW=1:TM=1261551909:LM=1261551917:S=ybYcq2wpfefs4V9g;\r\n\r\n"
        "NID=31=ojj8d-IygaEtSxLgaJmqSjVhCspkviJrB6omjamNrSm8lZhKy_yMfO2M4QMRKcH1g0iQv9u-2hfBW7bUFwVh7pGaRUb0RnHcJU37y-FxlRugatx63JLv7CWMD6UB_O_r";
    //cout << string(data) <<endl;
    NetHTTPRequestTester tester;
    tester.test(data, sizeof(data));
    return 0;
}

