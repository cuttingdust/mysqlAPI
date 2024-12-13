#include "XClient.h"
#include <iostream>

int main(int argc, char *argv[])
{
    std::string ip = "127.0.0.1";
    if (argc > 1)
    {
        ip = argv[0];
    }
    std::cout << "Client start!\n";
    if (!XClient::get()->init(ip))
    {
        std::cout << "Client init failed!\n";
        return -1;
    }
    XClient::get()->main();
    return 0;
}
