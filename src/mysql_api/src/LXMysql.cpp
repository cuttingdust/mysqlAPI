#include "LXMysql.h"

#include <mysql.h>
#include <iostream>


class LXMysql::PImpl
{
public:
    PImpl(LXMysql *owenr);
    ~PImpl() = default;

public:
    LXMysql *owenr_ = nullptr;
    MYSQL   *mysql_ = nullptr;
};

LXMysql::PImpl::PImpl(LXMysql *owenr) : owenr_(owenr)
{
}

LXMysql::LXMysql()
{
    impl_ = std::make_unique<PImpl>(this);
}

LXMysql::~LXMysql()
{
    std::cout << "LXMysql::~LXMysql()" << std::endl;
}

auto LXMysql::init() -> bool
{
    close();
    std::cout << "LXMysql::init()" << std::endl;
    /// 新创建一个MYSQL 对象
    impl_->mysql_ = mysql_init(nullptr);
    if (!impl_->mysql_)
    {
        std::cerr << "mysql_init failed!" << std::endl;
        return false;
    }
    return true;
}

auto LXMysql::close() -> void
{
    if (impl_->mysql_)
    {
        mysql_close(impl_->mysql_);
        impl_->mysql_ = nullptr;
    }
    std::cout << "LXMysql::close()" << std::endl;
}

auto LXMysql::connect(const char *host, const char *user, const char *pass, const char *db, unsigned short port,
                      unsigned long flag) -> bool
{
    if (!init())
    {
        std::cerr << "Mysql connect failed! msyql is not init!" << std::endl;
        return false;
    }
    if (!mysql_real_connect(impl_->mysql_, host, user, pass, db, port, nullptr, flag))
    {
        std::cerr << "Mysql connect failed!" << mysql_error(impl_->mysql_) << std::endl;
        return false;
    }
    std::cout << "mysql connect success!" << std::endl;
    return true;
}

auto LXMysql::query(const char *sql, unsigned long sql_len) -> bool
{
    if (!impl_->mysql_)
    {
        std::cerr << "Mysql query failed! msyql is not init!!!" << std::endl;
        return false;
    }
    if (!sql)
    {
        std::cerr << "sql is null!!!" << std::endl;
        return false;
    }
    if (sql_len <= 0)
        sql_len = static_cast<unsigned long>(strlen(sql));

    if (sql_len <= 0)
    {
        std::cerr << "Query sql is empty or wrong format!!!" << std::endl;
        return false;
    }

    int re = mysql_real_query(impl_->mysql_, sql, sql_len);
    if (re != 0)
    {
        std::cerr << "Mysql query failed!!!" << mysql_error(impl_->mysql_) << std::endl;
        return false;
    }
    return true;
}
