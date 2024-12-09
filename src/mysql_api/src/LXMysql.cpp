#include "LXMysql.h"

#include <format>
#include <mysql.h>
#include <iostream>
#include <sstream>
#include <fstream>

//////////////////////////////////工具函数///////////////////////////////////
auto join(const std::vector<std::string> &strings, const std::string &delimiter) -> std::string
{
    std::ostringstream oss;

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
/////////////////////////////////////////////////////////////////////////////

class LXMysql::PImpl
{
public:
    PImpl(LXMysql *owenr);
    ~PImpl() = default;

public:
    LXMysql   *owenr_  = nullptr;
    MYSQL     *mysql_  = nullptr;
    MYSQL_RES *result_ = nullptr;
};

LXMysql::PImpl::PImpl(LXMysql *owenr) : owenr_(owenr)
{
}

LXData::LXData()
{
}

LXData::LXData(const char *data)
{
    if (!data)
        return;
    this->data = data;
    this->size = static_cast<unsigned long>(strlen(data));
    this->type = LX_DATA_TYPE::LXD_TYPE_STRING;
}

LXData::LXData(const char *data, int size, const LX_DATA_TYPE &type)
{
    if (!data || size <= 0)
        return;
    this->data = data;
    this->size = size;
    this->type = type;
}

LXData::LXData(const int *d)
{
    if (!d)
        return;
    this->type = LX_DATA_TYPE::LXD_TYPE_LONG;
    this->data = reinterpret_cast<const char *>(d);
    this->size = sizeof(int);
}

auto LXData::loadFile(const char *fileName) -> bool
{
    if (!fileName)
        return false;
    std::fstream in(fileName, std::ios::in | std::ios::binary);
    if (!in.is_open())
    {
        std::cerr << "LoadFile " << fileName << " failed!" << std::endl;
        return false;
    }
    /// 文件大小
    in.seekg(0, std::ios::end);
    size = in.tellg();
    in.seekg(0, std::ios::beg);
    if (size <= 0)
    {
        return false;
    }
    data       = new char[size];
    int readed = 0;
    while (!in.eof())
    {
        in.read(const_cast<char *>(data) + readed, size - readed);
        if (in.gcount() > 0)
            readed += in.gcount();
        else
            break;
    }
    in.close();
    this->type = LXD_TYPE_BLOB;
    return true;
}

auto LXData::saveFile(const char *fileName) -> bool
{
    if (!data || size <= 0)
        return false;

    if (!fileName)
        return false;
    std::fstream out(fileName, std::ios::out | std::ios::binary);
    if (!out.is_open())
    {
        std::cerr << "SaveFile " << fileName << " failed!" << std::endl;
        return false;
    }
    out.write(data, size);
    out.close();
    return true;
}

auto LXData::drop() -> void
{
    delete data;
    data = nullptr;
}

LXMysql::LXMysql()
{
    std::cout << "LXMysql::LXMysql()" << std::endl;
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
    if (impl_->mysql_ == nullptr && !init())
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

auto LXMysql::option(const LX_OPT &opt, const void *arg) -> bool
{
    if (!impl_->mysql_)
    {
        std::cerr << "Mysql option failed! msyql is not init!!!" << std::endl;
        return false;
    }
    int re = mysql_options(impl_->mysql_, static_cast<mysql_option>(opt), arg);
    if (re != 0)
    {
        std::cerr << "mysql_options failed!" << mysql_error(impl_->mysql_) << std::endl;
        return false;
    }
    return true;
}

auto LXMysql::setConnectTimeout(unsigned int sec) -> bool
{
    if (!impl_->mysql_)
    {
        std::cerr << "Mysql setConnectTimeout failed! msyql is not init!!!" << std::endl;
        return false;
    }
    if (sec <= 0)
    {
        std::cerr << "timeout is wrong!!!" << std::endl;
        return false;
    }
    return option(LX_OPT_CONNECT_TIMEOUT, &sec);
}

auto LXMysql::setReconnect(bool bRe) -> bool
{
    if (!impl_->mysql_)
    {
        std::cerr << "Mysql setReconnect failed! msyql is not init!!!" << std::endl;
        return false;
    }
    return option(LX_OPT_RECONNECT, &bRe);
}

auto LXMysql::ping() -> bool
{
    if (!impl_->mysql_)
    {
        std::cerr << "Mysql ping failed! msyql is not init!!!" << std::endl;
        return false;
    }
    int re = mysql_ping(impl_->mysql_);
    if (re == 0)
    {
        std::cout << "mysql ping success!" << std::endl;
        return true;
    }
    else
    {
        std::cerr << "mysql ping failed! " << mysql_error(impl_->mysql_) << std::endl;
        return false;
    }
}

auto LXMysql::storeResult() -> bool
{
    if (!impl_->mysql_)
    {
        std::cerr << "Mysql storeResult failed! msyql is not init!!!" << std::endl;
        return false;
    }
    freeResult();
    impl_->result_ = mysql_store_result(impl_->mysql_);
    if (!impl_->result_)
    {
        std::cerr << "mysql_store_result failed!" << mysql_error(impl_->mysql_) << std::endl;
        return false;
    }
    return true;
}

auto LXMysql::useResult() -> bool
{
    if (!impl_->mysql_)
    {
        std::cerr << "Mysql useResult failed! msyql is not init!!!" << std::endl;
        return false;
    }
    freeResult();
    impl_->result_ = mysql_use_result(impl_->mysql_);
    if (!impl_->result_)
    {
        std::cerr << "mysql_use_result failed!" << mysql_error(impl_->mysql_) << std::endl;
        return false;
    }
    return true;
}

auto LXMysql::freeResult() -> void
{
    if (impl_->result_)
    {
        mysql_free_result(impl_->result_);
        impl_->result_ = nullptr;
    }
}

auto LXMysql::fetchRow() -> std::vector<LXData>
{
    std::vector<LXData> row;
    if (!impl_->result_)
    {
        // std::cerr << "Mysql fetchRow failed! result is null!!!" << std::endl;
        return row;
    }
    MYSQL_ROW r = mysql_fetch_row(impl_->result_);
    if (!r)
    {
        // std::cerr << "Mysql fetchRow failed! row is null!!!" << std::endl;
        return row;
    }
    unsigned long *lengths = mysql_fetch_lengths(impl_->result_);
    if (!lengths)
    {
        std::cerr << "Mysql fetchRow failed! lengths is null!!!" << std::endl;
        return row;
    }
    MYSQL_FIELD *fields = mysql_fetch_fields(impl_->result_);
    if (!fields)
    {
        std::cerr << "Mysql fetchRow failed! fields is null!!!" << std::endl;
        return row;
    }

    for (int i = 0; i < mysql_num_fields(impl_->result_); ++i)
    {
        int  iSize = lengths[i];
        auto type  = static_cast<LXData::LX_DATA_TYPE>(fields[i].type);
        row.emplace_back(r[i], iSize, type);
    }
    return row;
}

auto LXMysql::getInsertSql(const XDATA &kv, const std::string &table_name) -> std::string
{
    std::string sql;
    if (kv.empty() || table_name.empty())
    {
        return sql;
    }

    std::vector<std::string> keys;
    std::vector<std::string> values;
    for (const auto &[key, data] : kv)
    {
        auto tmp = "`" + key + "`";
        keys.emplace_back(tmp);
        tmp = std::string("'") + data.data + std::string("'");
        values.emplace_back(tmp);
    }

    const std::string &key_str = join(keys, ",");
    const std::string &val_str = join(values, ",");
    sql                        = std::format("INSERT INTO `{0}` ({1}) VALUES ({2});", table_name, key_str, val_str);

    return sql;
}

auto LXMysql::insert(const XDATA &kv, const std::string &table_name) -> bool
{
    if (!impl_->mysql_)
    {
        std::cerr << "Mysql insert failed! msyql is not init!!!" << std::endl;
        return false;
    }

    const std::string &sql = getInsertSql(kv, table_name);
    if (sql.empty())
    {
        std::cerr << "Mysql insert failed! sql is empty!!!" << std::endl;
        return false;
    }
    if (!query(sql.c_str()))
        return false;
    int num = mysql_affected_rows(impl_->mysql_);
    if (num <= 0)
        return false;
    return true;
}

auto LXMysql::insertBin(const XDATA &kv, const std::string &table_name) -> bool
{
    if (!impl_->mysql_)
    {
        std::cerr << "Mysql insertBin failed! msyql is not init!!!" << std::endl;
        return false;
    }

    if (kv.empty() || table_name.empty())
    {
        std::cerr << "Mysql insertBin failed! kv or table_name is empty!!!" << std::endl;
        return false;
    }

    std::vector<std::string> keys;
    std::vector<std::string> values;
    MYSQL_BIND               bind[256] = { 0 };
    int                      i         = 0;
    for (const auto &[key, data] : kv)
    {
        keys.emplace_back("`" + key + "`");
        values.emplace_back("?");
        bind[i].buffer        = const_cast<char *>(data.data);
        bind[i].buffer_length = data.size;
        bind[i].buffer_type   = static_cast<enum_field_types>(data.type);
        ++i;
    }

    const std::string &key_str = join(keys, ",");
    const std::string &val_str = join(values, ",");
    const std::string &sql     = std::format("INSERT INTO `{0}` ({1}) VALUES ({2});", table_name, key_str, val_str);

    /// 预处理SQL语句
    MYSQL_STMT *stmt = mysql_stmt_init(impl_->mysql_);
    if (!stmt)
    {
        std::cerr << "Mysql insertBin failed! mysql_stmt_init failed!!!" << std::endl;
        return false;
    }

    if (mysql_stmt_prepare(stmt, sql.c_str(), static_cast<unsigned long>(sql.size())) != 0)
    {
        mysql_stmt_close(stmt);
        std::cerr << "Mysql insertBin failed! mysql_stmt_prepare failed!!!" << std::endl;
        return false;
    }

    if (mysql_stmt_bind_param(stmt, bind) != 0)
    {
        mysql_stmt_close(stmt);
        std::cerr << "Mysql insertBin failed! mysql_stmt_bind_param failed!!!" << std::endl;
        return false;
    }

    if (mysql_stmt_execute(stmt) != 0)
    {
        mysql_stmt_close(stmt);
        std::cerr << "Mysql insertBin failed! mysql_stmt_execute failed!!!" << std::endl;
        return false;
    }

    mysql_stmt_close(stmt);
    return true;
}

auto LXMysql::getUpdateSql(const XDATA &kv, const std::string &table_name, std::string where) -> std::string
{
    std::string sql;
    if (kv.empty() || table_name.empty())
    {
        return sql;
    }

    std::vector<std::string> sets;
    for (const auto &[key, data] : kv)
    {
        auto tmp = "`" + key + "`";
        tmp += "=";
        tmp += std::string("'") + data.data + std::string("'");
        sets.emplace_back(tmp);
    }

    const std::string &set_str = join(sets, ",");
    sql                        = std::format("UPDATE `{0}` SET {1} WHERE {2};", table_name, set_str, where);

    return sql;
}

auto LXMysql::update(const XDATA &kv, const std::string &table_name, const std::string &where) -> int
{
    if (!impl_->mysql_)
    {
        std::cerr << "Mysql update failed! msyql is not init!!!" << std::endl;
        return -1;
    }

    const std::string &sql = getUpdateSql(kv, table_name, where);
    if (sql.empty())
    {
        std::cerr << "Mysql update failed! sql is empty!!!" << std::endl;
        return -1;
    }
    if (!query(sql.c_str()))
        return -1;
    return mysql_affected_rows(impl_->mysql_);
}

auto LXMysql::updateBin(const XDATA &kv, const std::string &table_name, const std::string &where) -> int
{
    if (!impl_->mysql_)
    {
        std::cerr << "Mysql updateBin failed! msyql is not init!!!" << std::endl;
        return -1;
    }

    if (kv.empty() || table_name.empty())
    {
        std::cerr << "Mysql updateBin failed! kv or table_name is empty!!!" << std::endl;
        return -1;
    }

    std::vector<std::string> sets;
    MYSQL_BIND               bind[256] = { 0 };
    int                      i         = 0;
    for (const auto &[key, data] : kv)
    {
        sets.emplace_back("`" + key + "`=?");
        bind[i].buffer        = const_cast<char *>(data.data);
        bind[i].buffer_length = data.size;
        bind[i].buffer_type   = static_cast<enum_field_types>(data.type);
        ++i;
    }

    const std::string &set_str = join(sets, ",");
    const std::string &sql     = std::format("UPDATE `{0}` SET {1} WHERE {2};", table_name, set_str, where);

    /// 预处理SQL语句
    MYSQL_STMT *stmt = mysql_stmt_init(impl_->mysql_);
    if (!stmt)
    {
        std::cerr << "Mysql updateBin failed! mysql_stmt_init failed!!!" << std::endl;
        return -1;
    }

    if (mysql_stmt_prepare(stmt, sql.c_str(), static_cast<unsigned long>(sql.size())) != 0)
    {
        mysql_stmt_close(stmt);
        std::cerr << "Mysql updateBin failed! mysql_stmt_prepare failed!!!" << std::endl;
        return -1;
    }

    if (mysql_stmt_bind_param(stmt, bind) != 0)
    {
        mysql_stmt_close(stmt);
        std::cerr << "Mysql updateBin failed! mysql_stmt_bind_param failed!!!" << std::endl;
        return -1;
    }

    if (mysql_stmt_execute(stmt) != 0)
    {
        mysql_stmt_close(stmt);
        std::cerr << "Mysql updateBin failed! mysql_stmt_execute failed!!!" << std::endl;
        return -1;
    }

    mysql_stmt_close(stmt);
    return mysql_stmt_affected_rows(stmt);
}
