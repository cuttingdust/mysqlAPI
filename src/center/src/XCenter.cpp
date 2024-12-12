#include "XCenter.h"

#include <format>
#include <LXMysql.h>
#include <iostream>
#include <fstream>

#define CENTER_CONF "ip"

constexpr auto tabele_name  = "t_strategy";
constexpr auto col_id       = "id";
constexpr auto col_name     = "name";
constexpr auto col_strategy = "strategy";
constexpr auto chart        = "gbk";

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

    impl_->mysql_->startTransaction();
    {
        /// 创建策略表
        /// 清除原来数据，防止数据污染
        std::string sql = std::format("DROP TABLE IF EXISTS {}", tabele_name);
        impl_->mysql_->query(sql.c_str());

        sql     = std::format("CREATE TABLE `{0}` ("
                                  "`{1}` INT AUTO_INCREMENT,"
                                  "`{2}` VARCHAR(256) CHARACTER SET '{3}' COLLATE '{3}_bin', "
                                  "`{4}` VARCHAR(4096), PRIMARY KEY(`{1}`))",
                              tabele_name, col_id, col_name, chart, col_strategy);
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
        impl_->mysql_->insert(data, tabele_name);

        data[col_name]     = exit_event;
        data[col_strategy] = exit_strategy;
        impl_->mysql_->insert(data, tabele_name);
    }

    impl_->mysql_->commit();
    impl_->mysql_->stopTransaction();


    return true;
}
