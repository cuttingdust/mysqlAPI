#include "XClient.h"

#include <format>
#include <iostream>
#include <LXMysql.h>
#ifdef _WIN32
#include <conio.h>
#else
#include <termio.h>
#endif

constexpr auto table_user = "t_user"; /// 用户表
constexpr auto col_id     = "id";     /// 用户id
constexpr auto col_user   = "user";   /// 用户名
constexpr auto col_pass   = "pass";   /// 密码
constexpr auto chart      = "gbk";    /// 数据库字符集

constexpr auto g_max_login_times = 10; /// 最大登录失败次数

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
        /// //注入攻击
        /// 如果你支持运行多条sql语句，结束你的语句，在添加自己的语句（删库）
        /// 无密码直接登录
        /// 限制用户的权限，不用root用户
        /// 用预处理语句stmt
        /// 检查用户的输入
        /// select id from t_user where user='root' and pass=md5('123456')
        /// select id from t_user where user='1'or'1'='1' and pass=md5('1')or'c4ca4238a0b923820dcc509a6f75849b'=md5('1')
        /// username =  1'or'1'='1
        /// password = 1')or'c4ca4238a0b923820dcc509a6f75849b'=md5('1
        std::string password;
        std::cout << "input password:" << std::flush;
        password = inputPassword();
        // std::cout << "[" << password << "]" << std::endl;

        if (!checkInput(username) || !checkInput(password))
        {
            std::cout << "Injection attacks" << std::endl;
            continue;
        }

        const std::string &sql  = std::format("SELECT `{}` FROM `{}` WHERE `{}`='{}' AND `{}`={};", col_id, table_user,
                                              col_user, username, col_pass, std::format(R"(MD5("{}"))", password));
        auto               rows = impl_->mysql_->getResult(sql.c_str());
        if (!rows.empty())
        {
            std::cout << "login success！" << std::endl;
            is_login = true;
            break;
        }
        std::cout << "login failed！" << std::endl;

        std::cout << "[" << password << "]" << std::endl;
    }
    return is_login;
}

auto XClient::inputPassword() -> std::string
{
    /// 清空缓冲
    std::cin.ignore(4096, '\n');
    std::string password = "";
    for (;;)
    {
        /// 获取输入字符不显示
        char a = _getch();
        if (a <= 0 || a == '\n' || a == '\r')
            break;
        if (a == '\b')
        {
            std::cout << "-" << std::flush;
            if (!password.empty())
                password.pop_back();
            continue;
        }
        std::cout << "*" << std::flush;
        password += a;
    }
    std::cout << std::endl;
    return password;
}

auto XClient::main() -> void
{
    /// 用户登录
    if (!login())
        return;
}

auto XClient::checkInput(const std::string &in) -> bool
{
    ///不允许出现的字符
    std::string str = R"('"())";
    for (char a : str)
    {
        size_t found = in.find(a);
        if (found != std::string::npos) ///发现违规字符
        {
            return false;
        }
    }
    return true;
}
