#include <iostream>
#include "../log.h"

int main()
{
    Log l;
    l.SetLevel(DEBUG);
    //Log::DoLog(INFO, "hello world");
    PLOG(INFO, "hello world");
    return 0;
}

