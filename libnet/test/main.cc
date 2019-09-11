#include <iostream>
#include "../net_httpserver.h"


int main()
{
    NetHTTPServer server(9876,5);
    server.run();
    return 0;
}

