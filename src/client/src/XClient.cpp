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

constexpr auto table_user = "t_user"; /// �û���
constexpr auto col_id     = "id";     /// �û�id
constexpr auto col_user   = "user";   /// �û���
constexpr auto col_pass   = "pass";   /// ����
constexpr auto table_log  = "t_log";  /// ��־��
constexpr auto col_log    = "log";    /// ��־
constexpr auto chart      = "gbk";    /// ���ݿ��ַ���

constexpr auto g_max_login_times = 10; /// ����¼ʧ�ܴ���

#ifndef _WIN32
char _getch()
{
    termios new_tm;
    /// ԭ����ģʽ
    termios old_tm;
    int     fd = 0;
    if (tcgetattr(fd, &old_tm) < 0)
        return -1;

    /// ����Ϊԭʼģʽ��û�л���
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

/// ��չ�� trim ������ȥ���հ��ַ��Լ� '\r' �� '\n'
std::string trim(const std::string &str)
{
    auto start = std::find_if_not(str.begin(), str.end(),
                                  [](unsigned char ch) { return std::isspace(ch) || ch == '\r' || ch == '\n'; });

    auto end = std::find_if_not(str.rbegin(), str.rend(),
                                [](unsigned char ch) { return std::isspace(ch) || ch == '\r' || ch == '\n'; })
                       .base();

    return std::string(start, end);
}


class XClient::PImpl
{
public:
    PImpl(XClient *owenr);
    ~PImpl() = default;

public:
    auto c_log(const std::vector<std::string> &cmds) -> void;
    auto input() -> std::string;
    /////////////////////// ��¼///////////////////
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

auto XClient::PImpl::c_log(const std::vector<std::string> &cmds) -> void
{
    int pagecount = 10;
    int page      = 1;
    if (cmds.size() > 1)
        page = std::stoi(cmds[1]);
    if (cmds.size() > 2)
        pagecount = std::stoi(cmds[2]);

    std::cout << "In log" << std::endl;

    /// limit 0,10 ��0��ʼȡʮ��
    std::string sql = std::format("SELECT * FROM {} LIMIT {},{};", table_log, (page - 1) * pagecount, pagecount);
    // std::cout << sql << std::endl;
    auto rows = mysql_->getResult(sql.c_str());
    sql       = std::format("SELECT COLUMN_NAME FROM INFORMATION_SCHEMA.COLUMNS WHERE TABLE_NAME = '{}';", table_log);
    auto columns    = mysql_->getResult(sql.c_str());
    int  width      = 30;
    int  long_width = 100;
    /// ��ӡ��ͷ
    for (auto col : columns)
    {
        int w = !strcmp(col[0].data, col_log) ? long_width : width;
        std::cout << std::left << std::setw(w) << trim(col[0].data);
    }
    std::cout << std::endl;

    /// ����ÿһ��
    for (const auto &row : rows)
    {
        for (int i = 0; i < columns.size(); ++i)
        {
            int w = !strcmp(columns[i][0].data, col_log) ? long_width : width;
            std::cout << std::left << std::setw(w) << trim(row[i].data);
        }
        std::cout << std::endl;
    }
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
        /// �����û�����
        std::cout << "input username:" << std::flush;
        std::cin >> username;
        std::cout << "[" << username << "]" << std::endl;

        /// ������������
        /// //ע�빥��
        /// �����֧�����ж���sql��䣬���������䣬������Լ�����䣨ɾ�⣩
        /// ������ֱ�ӵ�¼
        /// �����û���Ȩ�ޣ�����root�û�
        /// ��Ԥ�������stmt
        /// ����û�������
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
            std::cout << "login success��" << std::endl;
            is_login = true;
            break;
        }
        std::cout << "login failed��" << std::endl;

        std::cout << "[" << password << "]" << std::endl;
    }
    return is_login;
}

auto XClient::PImpl::inputPassword() -> std::string
{
    /// ��ջ���
    std::cin.ignore(4096, '\n');
    std::string password = "";
    for (;;)
    {
        /// ��ȡ�����ַ�����ʾ
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
    ///��������ֵ��ַ�
    std::string str = R"('"())";
    for (char a : str)
    {
        size_t found = in.find(a);
        if (found != std::string::npos) ///����Υ���ַ�
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
    /// �û���¼
    if (!impl_->login())
        return;

    /// ��ҳ��ʾ t_log
    /// ��ȡ�û�
    /// ������
    for (;;)
    {
        std::cout << "Input:" << std::flush;
        std::string cmd = impl_->input();
        std::cout << cmd << std::endl;
        /// log 1 10 ��һҳ һҳʮ��
        /// �и�ո�
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
    }
}
