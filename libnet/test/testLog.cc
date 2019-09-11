#include <iostream>
#include "../log.h"

int main()
{
    PLOG(INFO, "hello world %s", "test");
    return 0;
}

