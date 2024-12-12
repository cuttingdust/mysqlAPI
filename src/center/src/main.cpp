#include "XCenter.h"

#include <iostream>

/// 使用说明
void Usage()
{
    std::cout << "========== Center Usage ==========" << std::endl;
    std::cout << "./center install 127.0.0.1" << std::endl;
}


int main(int argc, char *argv[])
{
    std::string cmd;
    if (argc > 1)
    {
        cmd = argv[1];
    }
    ///  安装系统
    if (cmd == "install")
    {
        std::cout << "begin install center" << std::endl;
        if (argc < 3)
        {
            Usage();
            return 0;
        }
        const std::string &ip = argv[2];
        XCenter::get()->install(ip);
    }

    std::cout << "Center start!\n";
    return 0;
}
