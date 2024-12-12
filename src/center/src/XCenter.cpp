#include "XCenter.h"

#include <format>
#include <LXMysql.h>
#include <iostream>
#include <fstream>

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
constexpr auto col_device_ip  = "device_ip";
constexpr auto col_from_ip    = "from_ip";
constexpr auto table_stratery = "t_strategy";
constexpr auto table_log      = "t_log";
constexpr auto table_device   = "t_device";
constexpr auto table_audit    = "t_audit";
constexpr auto chart          = "gbk";

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
    return impl_->mysql_->query("set names utf8");
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
    impl_->mysql_->startTransaction();
    {
        /// 创建策略表
        /// 清除原来数据，防止数据污染
        sql = std::format("DROP TABLE IF EXISTS {}", table_stratery);
        impl_->mysql_->query(sql.c_str());

        sql     = std::format("CREATE TABLE `{0}` ("
                                  "`{1}` INT AUTO_INCREMENT,"
                                  "`{2}` VARCHAR(256) CHARACTER SET '{3}' COLLATE '{3}_bin', "
                                  "`{4}` VARCHAR(4096), PRIMARY KEY(`{1}`))",
                              table_stratery, col_id, col_name, chart, col_strategy);
        bool re = impl_->mysql_->query(sql.c_str());
        if (!re)
        {
            impl_->mysql_->rollback();
            return false;
        }
        impl_->mysql_->query(std::format("set names {}", chart).c_str());
    }

    {
        /// Dec 11 17:50:19 Mac sshd-session: handabao [priv][60137]: USER_PROCESS: 60140 ttys001
        const char *login_event = "登录";
        const char *login_strategy =
                R"(([A-Za-z]{3} \d{1,2} \d{2}:\d{2}:\d{2}) ([A-Za-z]+) sshd-session: ([a-zA-Z0-9_]+) \[[a-zA-Z0-9]+\]\[[0-9]+\]: USER_PROCESS:)";

        /// Dec 12 00:25:28 Mac sshd-session: handabao [priv][61258]: DEAD_PROCESS: 61261 ttys001
        const char *exit_event = "离开";
        const char *exit_strategy =
                R"(([A-Za-z]{3} \d{1,2} \d{2}:\d{2}:\d{2}) ([A-Za-z]+) sshd-session: ([a-zA-Z0-9_]+) \[[a-zA-Z0-9]+\]\[[0-9]+\]: DEAD_PROCESS:)";

        XDATA data;
        data[col_name]     = login_event;
        data[col_strategy] = login_strategy;
        impl_->mysql_->insert(data, table_stratery);

        data[col_name]     = exit_event;
        data[col_strategy] = exit_strategy;
        impl_->mysql_->insert(data, table_stratery);
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
