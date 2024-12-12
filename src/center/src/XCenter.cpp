#include "XCenter.h"

#include <format>
#include <LXMysql.h>
#include <iostream>
#include <fstream>
#include <regex>
#include <thread>

#define CENTER_CONF "ip"

constexpr auto col_id         = "id";
constexpr auto col_name       = "name";
constexpr auto col_ip         = "ip";
constexpr auto col_port       = "port";
constexpr auto col_last_heart = "last_heart";
constexpr auto col_strategy   = "strategy";
constexpr auto col_log        = "log";
constexpr auto col_time       = "log_time";
constexpr auto col_context    = "context";
constexpr auto col_user       = "user";
constexpr auto col_pass       = "pass";
constexpr auto col_device_ip  = "device_ip";
constexpr auto col_from_ip    = "from_ip";
constexpr auto table_stratery = "t_strategy";
constexpr auto table_log      = "t_log";
constexpr auto table_device   = "t_device";
constexpr auto table_audit    = "t_audit";
constexpr auto table_user     = "t_user";

#ifdef _WIN32
constexpr auto chart = "gbk";
#else
constexpr auto chart = "utf8";
#endif
/// 扩展的 trim 函数，去掉空白字符以及 '\r' 和 '\n'
std::string trim(const std::string &str)
{
    auto start = std::find_if_not(str.begin(), str.end(),
                                  [](unsigned char ch) { return std::isspace(ch) || ch == '\r' || ch == '\n'; });

    auto end = std::find_if_not(str.rbegin(), str.rend(),
                                [](unsigned char ch) { return std::isspace(ch) || ch == '\r' || ch == '\n'; })
                       .base();

    return std::string(start, end);
}


class XCenter::PImpl
{
public:
    PImpl(XCenter *owenr);
    ~PImpl() = default;

public:
    XCenter *owenr_ = nullptr;
    LXMysql *mysql_ = nullptr;
};

XCenter::PImpl::PImpl(XCenter *owenr) : owenr_(owenr)
{
}


XCenter::XCenter()
{
    impl_ = std::make_unique<PImpl>(this);
}

XCenter::~XCenter() = default;

auto XCenter::init() -> bool
{
    impl_->mysql_  = new LXMysql();
    std::string ip = "";
    /// 读取数据库ip配置
    std::ifstream fs;
    fs.open(CENTER_CONF);
    if (!fs.is_open())
    {
        std::cout << "please install center" << std::endl;
        return false;
    }
    fs >> ip;
    fs.close();
    if (ip.empty())
    {
        std::cout << "please install center" << std::endl;
        return false;
    }
    std::cout << "Init center " << ip << std::endl;

    if (!impl_->mysql_->connect(ip.c_str(), "root", "System123@", "laoxiaketang"))
    {
        std::cout << "db connect failed!" << std::endl;
        return false;
    }
    std::cout << "db connect success!" << std::endl;
    return impl_->mysql_->query(std::format("set names {}", chart).c_str());
}

auto XCenter::install(const std::string &ip) -> bool
{
    /// 1 生成配置文件 数据库的IP
    std::ofstream of;
    of.open(CENTER_CONF);
    if (!of.is_open())
    {
        std::cout << "open conf " << CENTER_CONF << " failed!" << std::endl;
        return false;
    }
    of << ip;
    of.close();

    /// 初始化表和数据
    if (!init())
        return false;

    std::cout << "XCenter::Install() " << ip << std::endl;

    std::string sql = "";
    bool        re  = false;
    impl_->mysql_->startTransaction();
    {
        /// 创建策略表
        /// 清除原来数据，防止数据污染
        sql = std::format("DROP TABLE IF EXISTS {}", table_stratery);
        impl_->mysql_->query(sql.c_str());

        sql = std::format("CREATE TABLE `{0}` ("
                          "`{1}` INT AUTO_INCREMENT,"
                          "`{2}` VARCHAR(256) CHARACTER SET '{3}' COLLATE '{3}_bin', "
                          "`{4}` VARCHAR(4096), PRIMARY KEY(`{1}`))",
                          table_stratery, col_id, col_name, chart, col_strategy);
        re  = impl_->mysql_->query(sql.c_str());
        if (!re)
        {
            impl_->mysql_->rollback();
            return false;
        }
    }

    {
        /// Dec 11 17:50:19 Mac sshd-session: handabao [priv][60137]: USER_PROCESS: 60140 ttys001
        const char *login_event = "登录";
        const char *login_strategy =
                R"(([A-Za-z]{3} \\d{1,2} \\d{2}:\\d{2}:\\d{2}) ([A-Za-z]+) sshd-session: ([a-zA-Z0-9_]+) \\[[a-zA-Z0-9]+\\]\\[[0-9]+\\]: USER_PROCESS: [0-9]+ [a-zA-Z0-9_]+)";

        /// Dec 12 00:25:28 Mac sshd-session: handabao [priv][61258]: DEAD_PROCESS: 61261 ttys001
        const char *exit_event = "离开";
        const char *exit_strategy =
                R"(([A-Za-z]{3} \\d{1,2} \\d{2}:\\d{2}:\\d{2}) ([A-Za-z]+) sshd-session: ([a-zA-Z0-9_]+) \\[[a-zA-Z0-9]+\\]\\[[0-9]+\\]: DEAD_PROCESS: [0-9]+ [a-zA-Z0-9_]+)";

        XDATA data;
        data[col_name]     = login_event;
        data[col_strategy] = login_strategy;
        impl_->mysql_->insert(data, table_stratery);

        data[col_name]     = exit_event;
        data[col_strategy] = exit_strategy;
        impl_->mysql_->insert(data, table_stratery);
    }

    /// 创建用户表，初始化管理员用户 root 123456 md5
    /// t_user id user pass
    sql = std::format("DROP TABLE IF EXISTS {}", table_user);
    impl_->mysql_->query(sql.c_str());
    sql = std::format("CREATE TABLE `{0}` ("
                      "`{1}` INT AUTO_INCREMENT,"
                      "`{2}` VARCHAR(256) CHARACTER SET '{3}' COLLATE '{3}_bin',"
                      "`{4}` VARCHAR(1024),PRIMARY KEY(`{1}`))",
                      table_user, col_id, col_user, chart, col_pass);
    re  = impl_->mysql_->query(sql.c_str());
    if (!re)
    {
        impl_->mysql_->rollback();
        return false;
    }

    {
        XDATA data;
        data[col_user] = "root";
        data[col_pass] = "@md5('System123@')";
        impl_->mysql_->insert(data, table_user);
    }

    /// 创建日志表 t_log
    sql = std::format("DROP TABLE IF EXISTS {}", table_log);
    impl_->mysql_->query(sql.c_str());
    sql = std::format("CREATE TABLE `{0}` ("
                      "`{1}` INT AUTO_INCREMENT,"
                      "`{2}` VARCHAR(16),"
                      "`{3}` VARCHAR(2048),"
                      "`{4}` datetime, PRIMARY KEY(`{1}`));",
                      table_log, col_id, col_ip, col_log, col_time);
    impl_->mysql_->query(sql.c_str());

    /// 创建设备表Agent t_device
    sql = std::format("DROP TABLE IF EXISTS {};", table_device);
    impl_->mysql_->query(sql.c_str());
    sql = std::format("CREATE TABLE `{0}` ("
                      "`{1}` INT AUTO_INCREMENT,"
                      "`{2}` VARCHAR(16),"
                      "`{3}` VARCHAR(2048),"
                      "`{4}` datetime, PRIMARY KEY(`{1}`));",
                      table_device, col_id, col_ip, col_name, col_last_heart);
    impl_->mysql_->query(sql.c_str());


    ///创建审计结果 t_audit
    sql = std::format("DROP TABLE IF EXISTS {};", table_audit);
    impl_->mysql_->query(sql.c_str());

    sql = std::format("CREATE TABLE IF NOT EXISTS `{0}` ("
                      "`{1}` INT AUTO_INCREMENT,"
                      "`{2}` VARCHAR(256),"
                      "`{3}` VARCHAR(2048),"
                      "`{4}` VARCHAR(256),"
                      "`{5}` VARCHAR(16),"
                      "`{6}` VARCHAR(16),"
                      "`{7}` INT,"
                      "`{8}` datetime,PRIMARY KEY(`{1}`));",
                      table_audit, col_id, col_name, col_context, col_user, col_device_ip, col_from_ip, col_port,
                      col_last_heart);
    impl_->mysql_->query(sql.c_str());


    impl_->mysql_->commit();
    impl_->mysql_->stopTransaction();


    return true;
}

auto XCenter::addDevice(const std::string &ip, const std::string &name) -> bool
{
    if (!impl_->mysql_)
    {
        std::cerr << "XCenter::addDevice failed! mysql is null!" << std::endl;
        return false;
    }

    if (ip.empty() || name.empty())
    {
        std::cerr << "XCenter::addDevice failed! ip or name is empty!" << std::endl;
        return false;
    }

    XDATA data;
    data[col_ip]   = ip.c_str();
    data[col_name] = name.c_str();
    return impl_->mysql_->insert(data, table_device);
}

auto XCenter::main() -> void
{
    if (!impl_->mysql_)
    {
        std::cerr << "XCenter::main failed! mysql is null!" << std::endl;
        return;
    }

    /// 只审计运行之后的事件
    /// 找到最后一个事件，取到id号
    int  lastid = 0;
    auto rows   = impl_->mysql_->getResult(std::format("SELECT MAX({}) FROM {};", col_id, table_log).c_str());
    if (rows[0][0].data)
    {
        lastid = atoi(rows[0][0].data);
    }
    std::cout << "last id is " << lastid << std::endl;

    /// 取到审计策略
    rows.clear();
    impl_->mysql_->query(std::format("set names {}", chart).c_str());
    rows = impl_->mysql_->getResult(std::format("SELECT * FROM {}", table_stratery).c_str());
    /// 正则表达式map key 审计事件名称
    std::map<std::string, std::regex> strategies;
    for (auto row : rows)
    {
        if (row[1].data && row[2].data) /// 初始化正则
        {
            std::string name     = row[1].data;
            std::string strategy = row[2].data;
            // std::cout << "name:" << name << " ,strategies:" << strategy << std::endl;

            strategies[name] = std::regex(strategy);
        }
    }


    for (;;) /// 循环审计
    {
        /// 获取agent存储最新数据
        std::string sql  = std::format("SELECT * FROM {} WHERE {}>{}", table_log, col_id, lastid);
        auto        rows = impl_->mysql_->getResult(sql.c_str());
        if (rows.empty())
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }

        /// 遍历日志列表
        for (auto row : rows)
        {
            lastid = atoi(row[0].data);
            if (!row[2].data)
                continue;
            std::cout << row[2].data << std::endl;
            for (const auto &[name, strategy] : strategies)
            {
                /// 正则结果
                std::smatch match;
                std::string data = row[2].data;
                data             = trim(data);
                /// 匹配正则，返回结果到match
                bool ret = std::regex_match(data, match, strategy);
                if (!ret || match.empty())
                {
                    continue;
                }
                std::cout << name << std::endl;

                // XDATA d;
                // /// 审计成功的，事件名称
                // d[col_name]    = name.c_str();
                // d[col_context] = data.c_str();
                // if (row[1].data)
                //     d[col_device_ip] = row[1].data;
                // /// 匹配结果， 下标0 是整个字符串 1是第一个匹配结果
                // std::string user    = match[1];
                // std::string from_ip = match[2];
                // std::string port    = match[3];
                // d["user"]           = user.c_str();
                // d["from_ip"]        = from_ip.c_str();
                // d["port"]           = port.c_str();
                // my->Insert(d, "t_audit");
            }
        }
    }
}
