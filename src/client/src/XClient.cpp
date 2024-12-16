#include "XClient.h"

#include <algorithm>
#include <LXMysql.h>

#include <format>
#include <iomanip>
#include <iostream>
#include <sstream>

#ifdef _WIN32
#include <conio.h>
#else
#include <termios.h>
#endif

constexpr auto table_user  = "t_user";  /// 用户表
constexpr auto col_id      = "id";      /// 用户id
constexpr auto col_user    = "user";    /// 用户名
constexpr auto col_pass    = "pass";    /// 用户密码
constexpr auto table_log   = "t_log";   /// 日志表
constexpr auto col_log     = "log";     /// 日志
constexpr auto table_audit = "t_audit"; /// 审计表
constexpr auto chart       = "gbk";     /// 字符

constexpr auto g_max_login_times = 10; /// 用户输入的最大次数

#ifndef _WIN32
char _getch()
{
    termios new_tm;
    /// 原来的模式
    termios old_tm;
    int     fd = 0;
    if (tcgetattr(fd, &old_tm) < 0)
        return -1;

    /// 更改为原始模式,没有回显
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

std::string trim(const std::string &str)
{
    auto start = std::find_if_not(str.begin(), str.end(),
                                  [](unsigned char ch) { return std::isspace(ch) || ch == '\r' || ch == '\n'; });

    auto end = std::find_if_not(str.rbegin(), str.rend(),
                                [](unsigned char ch) { return std::isspace(ch) || ch == '\r' || ch == '\n'; })
                       .base();

    return std::string(start, end);
}

std::vector<int> calculate_column_widths(const std::vector<std::string> &columns, const XROWS &rows)
{
    std::vector<int> widths(columns.size());

    /// 计算列名的最大宽度
    for (size_t i = 0; i < columns.size(); ++i)
    {
        widths[i] = columns[i].size();
    }

    /// 计算每一行中对应列的数据长度并更新宽度
    for (const auto &row : rows)
    {
        for (size_t i = 0; i < columns.size(); ++i)
        {
            if (row[i].data)
            {
                widths[i] = std::max(widths[i], static_cast<int>(strlen(row[i].data)));
            }
        }
    }

    ///  为每列增加一点额外的空间
    for (auto &width : widths)
    {
        width += 2; /// 增加空白
    }

    return widths;
}

std::string center(const std::string &str, int width)
{
    int space = width - str.size();
    if (space <= 0)
        return str; // 如果字符串超出宽度，直接返回字符串
    int pad_left  = space / 2;
    int pad_right = space - pad_left;

    return std::string(pad_left, ' ') + str + std::string(pad_right, ' ');
}

void print_table(const XCOLUMNS &columns, const XROWS &rows)
{
    constexpr auto r_separator = '-';
    constexpr auto c_separator = "|";

    /// 计算列宽
    auto column_widths = calculate_column_widths(columns, rows);

    /// 打印分隔线
    auto printSplit = [&]() -> void
    {
        std::cout << r_separator;
        for (const auto &width : column_widths)
        {
            std::cout << std::setfill(r_separator) << std::setw(width) << "" << std::setfill(' ') << r_separator;
        }
        std::cout << std::endl;
    };

    /// 打印表头
    auto printTableHead = [&]() -> void
    {
        printSplit();
        std::cout << c_separator;
        for (size_t i = 0; i < columns.size(); ++i)
        {
            std::cout << std::left << std::setw(column_widths[i]) << trim(columns[i]) << c_separator;
        }
        std::cout << std::endl;
        printSplit();
    };

    /// 打印每行数据
    auto printData = [&]() -> void
    {
        for (const auto &row : rows)
        {
            for (size_t i = 0; i < columns.size(); ++i)
            {
                if (row[i].data)
                    std::cout << c_separator << std::left << std::setw(column_widths[i]) << trim(row[i].data);
                else
                    std::cout << c_separator << std::left << std::setw(column_widths[i]) << "NULL";
            }
            std::cout << c_separator;
            std::cout << std::endl;
        }
    };
    printTableHead();
    printData();
    printSplit();
}


class XClient::PImpl
{
public:
    PImpl(XClient *owenr);
    ~PImpl() = default;

public:
    auto c_audit(const std::vector<std::string> &cmds) -> void;
    ////////////////////////////////////////////////////////
    auto c_log(const std::vector<std::string> &cmds) -> void;
    auto input() -> std::string;
    /////////////////////// 登录 ///////////////////
    auto checkInput(const std::string &in) -> bool;
    auto inputPassword() -> std::string;
    auto login() -> bool;

public:
    XClient *owenr_ = nullptr;
    LXMysql *mysql_ = nullptr;
};

XClient::PImpl::PImpl(XClient *owenr) : owenr_(owenr)
{
}

auto XClient::PImpl::c_audit(const std::vector<std::string> &cmds) -> void
{
    const auto &table_name = table_audit;
    std::cout << "In " << table_name << ":" << std::endl;
    int         total   = mysql_->getCount(table_name);
    const auto &columns = mysql_->getColumns(table_name);
    const auto &rows    = mysql_->getRows(table_name, "*", { 0, 10 });
    if (rows.empty() || columns.empty())
    {
        std::cout << "No data" << std::endl;
        return;
    }
    print_table(columns, rows);
    std::cout << "Total = " << total << std::endl;
}

auto XClient::PImpl::c_log(const std::vector<std::string> &cmds) -> void
{
    int pagecount = 10;
    int page      = 1;
    if (cmds.size() > 1)
        page = atoi(cmds[1].c_str());
    if (cmds.size() > 2)
        pagecount = atoi(cmds[2].c_str());

    if (page <= 0 || pagecount <= 0)
    {
        std::cout << "page or pagecount error" << std::endl;
        return;
    }
    const auto &table_name = table_log;
    int         start      = (page - 1) * pagecount;
    int         end        = pagecount;
    std::cout << "In " << table_name << ":" << std::endl;

    const auto &columns = mysql_->getColumns(table_name);
    const auto &rows    = mysql_->getRows(table_name, "*", { start, end });
    if (rows.empty() || columns.empty())
    {
        std::cout << "No data" << std::endl;
        return;
    }
    print_table(columns, rows);
    std::cout << std::format("Page = {} PageCount = {}", page, pagecount) << std::endl;
}

auto XClient::PImpl::input() -> std::string
{
    std::string input = "";
    for (;;)
    {
        char a = getchar();
        if (a <= 0 || a == '\n' || a == '\r')
            break;
        input += a;
    }
    return input;
}

auto XClient::PImpl::login() -> bool
{
    bool is_login = false;

    for (int i = 0; i < g_max_login_times; i++)
    {
        std::string username = "";
        /// 接受用户输入
        std::cout << "input username:" << std::flush;
        std::cin >> username;
        std::cout << "[" << username << "]" << std::endl;

        /// 注入攻击
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
        auto               rows = mysql_->getResult(sql.c_str());
        if (!rows.empty())
        {
            std::cout << "login success!" << std::endl;
            is_login = true;
            break;
        }
        std::cout << "login failed!!!" << std::endl;

        std::cout << "[" << password << "]" << std::endl;
    }
    return is_login;
}

auto XClient::PImpl::inputPassword() -> std::string
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
        if (a == '\b' || a == '\x7F')
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

auto XClient::PImpl::checkInput(const std::string &in) -> bool
{
    /// 不允许出现的字符
    std::string str = R"('"())";
    for (char a : str)
    {
        size_t found = in.find(a);
        if (found != std::string::npos) /// 发现违规字符
        {
            return false;
        }
    }
    return true;
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

auto XClient::main() -> void
{
    /// 用户登录
    // if (!impl_->login())
    //     return;

    /// 分页显示 t_log
    /// 获取用户的输入

    for (;;)
    {
        std::cout << "Input:" << std::flush;
        std::string cmd = impl_->input();
        /// log 1 10 第一页 一页十行
        /// 切割空格
        std::vector<std::string> cmds;
        char                    *p = strtok(const_cast<char *>(cmd.c_str()), " ");
        while (p)
        {
            cmds.push_back(p);
            p = strtok(0, " ");
        }
        std::string type = cmd;
        if (cmds.size() > 0)
            type = cmds[0];

        if (type == "log")
        {
            impl_->c_log(cmds);
        }
        else if (type == "audit")
        {
            impl_->c_audit(cmds);
        }
        else if (type == "exit")
        {
            break;
        }
    }
}
