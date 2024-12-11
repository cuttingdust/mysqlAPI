#include "XAgent.h"

#include <iostream>

int main(int argc, char *argv[])
{
    if (argc < 2)
    {
        std::cout << "please set ip" << std::endl;
        std::cout << "./agent 127.0.0.1" << std::endl;
        return 0;
    }

    /// 1 初始化agent 连接数据库
    const std::string ip = argv[1];
    if (!XAgent::get()->init(ip))
    {
        std::cout << "Agent init failed!" << std::endl;
        return -1;
    }
    std::cout << "Agent start!" << std::endl;

    return 0;
}
