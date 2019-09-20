#include <iostream>
#include "../net_httpserver.h"


int main()
{
    NetHTTPServer server(9876,20);
    server.run();
    return 0;
}

