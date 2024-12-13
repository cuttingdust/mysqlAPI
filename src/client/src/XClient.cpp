#include "XClient.h"

#include <format>
#include <iostream>
#include <LXMysql.h>
#ifdef _WIN32
#include <conio.h>
#else
#include <termio.h>
#endif

constexpr auto chart             = "gbk"; /// 数据库字符集
constexpr auto g_max_login_times = 10;    /// 最大登录失败次数

#ifndef _WIN32
char _getch()
{
    termios new_tm;
    /// 原来的模式
    termios old_tm;
    int     fd = 0;
    if (tcgetattr(fd, &old_tm) < 0)
        return -1;

    /// 更改为原始模式，没有回显
    cfmakeraw(&new_tm);
    if (tcsetattr(fd, TCSANOW, &new_tm) < 0)
    {
        return -1;
    }
    char c = getchar();
    if (tcsetattr(fd, TCSANOW, &old_tm) < 0)
    {
        return -1;
    }
    return c;
}

#endif

class XClient::PImpl
{
public:
    PImpl(XClient *owenr);
    ~PImpl() = default;

public:
    XClient *owenr_ = nullptr;
    LXMysql *mysql_ = nullptr;
};

XClient::PImpl::PImpl(XClient *owenr) : owenr_(owenr)
{
}

XClient::XClient()
{
    impl_ = std::make_unique<PImpl>(this);
}

XClient::~XClient() = default;

auto XClient::init(const std::string &ip) -> bool
{
    std::cout << "XClient::Init " << ip << std::endl;
    impl_->mysql_ = new LXMysql();
    if (!impl_->mysql_->connect(ip.c_str(), "root", "System123@", "laoxiaketang"))
    {
        std::cerr << "db connect faield!" << std::endl;
        return false;
    }
    std::cout << "db connect success!" << std::endl;
    return impl_->mysql_->query(std::format("set names {};", chart).c_str());
}

auto XClient::login() -> bool
{
    bool is_login = false;

    for (int i = 0; i < g_max_login_times; i++)
    {
        std::string username = "";
        /// 接收用户输入
        std::cout << "input username:" << std::flush;
        std::cin >> username;
        std::cout << "[" << username << "]" << std::endl;

        /// 接收密码输入
        std::string password;
        std::cout << "input password:" << std::flush;
        password = inputPassword();
        std::cout << "[" << password << "]" << std::endl;
    }
    return is_login;
}

auto XClient::inputPassword() -> std::string
{
    //清空缓冲
    std::cin.ignore(4096, '\n');
    std::string password = "";
    for (;;)
    {
        //获取输入字符不显示
        char a = _getch();
        if (a <= 0 || a == '\n' || a == '\r')
            break;
        std::cout << "*" << std::flush;
        password += a;
    }
    return password;
}

auto XClient::main() -> void
{
    /// 用户登录
    if (!login())
        return;
}
