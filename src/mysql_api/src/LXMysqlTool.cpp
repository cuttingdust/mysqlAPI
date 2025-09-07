#include "LXMysqlTool.h"

#ifdef _WIN32
#include <windows.h>
#include <conio.h>
#else
#include <iconv.h>
#endif

#include <iostream>
#include <sstream>

auto LXMysqlTool::join(const std::vector<std::string> &strings, const std::string &delimiter) -> std::string
{
    std::ostringstream oss;

    if (strings.empty())
        return oss.str();

    for (size_t i = 0; i < strings.size(); ++i)
    {
        oss << strings[i];
        if (i < strings.size() - 1)
        { /// 在元素之间添加分隔符
            oss << delimiter;
        }
    }

    return oss.str(); /// 返回连接后的字符串
}

auto LXMysqlTool::getPassword(char *out, int out_size) -> int
{
#ifdef _WIN32
    for (int i = 0; i < out_size; i++)
    {
        char p = _getch();
        if (p == '\r' || p == '\n')
        {
            return i; // Return the length of the password
        }
        else if (p == '\b') // Check for Backspace
        {
            if (i > 0) // Prevent removing characters before the first one
            {
                std::cout << "\b \b"; // Erase the last '*'
                i--;                  // Decrement the index
            }
            continue; // Skip further processing for Backspace
        }

        std::cout << "*" << std::flush; // Display '*' for each character
        out[i] = p;                     // Store the character
    }
    return 0; // Password entry terminated without newline
#else
    bool is_begin = false;
    for (int i = 0; i < out_size;)
    {
        system("stty -echo"); // Turn off echo
        char p = std::cin.get();
        if (p != '\r' && p != '\n')
            is_begin = true;

        if (!is_begin)
        {
            continue;
        }

        system("stty echo"); // Turn echo back on
        if (p == '\r' || p == '\n')
            return i; // Return the length of the password

        else if (p == 127) // Check for Backspace (ASCII 127)
        {
            if (i > 0) // Prevent removing characters before the first one
            {
                std::cout << "\b \b"; // Erase the last '*'
                i--;                  // Decrement the index
            }
            continue; // Skip further processing for Backspace
        }

        std::cout << "*" << std::flush; // Display '*' for each character
        out[i] = p;                     // Store the character
        i++;
    }
    return 0; // Password entry terminated without newline
#endif
}

#ifndef _WIN32
auto LXMysqlTool::convert(char *from_cha, char *to_cha, char *in, size_t inlen, char *out, size_t outlen) -> size_t
{
    /// 转换上下文
    iconv_t cd;
    cd = iconv_open(to_cha, from_cha);
    if (cd == 0)
        return -1;
    memset(out, 0, outlen);
    char **pin  = &in;
    char **pout = &out;
    // std::cout << "in = " << in << std::endl;
    // std::cout << "inlen = " << inlen << std::endl;
    // std::cout << "outlen = " << outlen << std::endl;
    //返回转换字节数的数量，但是转GBK时经常不正确 >=0就成功
    size_t re = iconv(cd, pin, &inlen, pout, &outlen);
    iconv_close(cd);
    // std::cout << "result = " << (int)result << std::endl;
    return re;
}
#endif
