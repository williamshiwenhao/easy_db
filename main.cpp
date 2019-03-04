#include <iostream>
#include <string>

#include "server.h"

using std::string;
int main()
{
    Server server;
    string dir = "data";
    server.InitServer(dir);
    server.Work();
    return 0;
}
