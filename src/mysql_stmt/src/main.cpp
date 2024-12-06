#include <iostream>
#include <mysql.h>
#include <thread>
#include <format>
#include <map>
#include <sstream>
#include <fstream>

int main(int argc, char *argv[])
{
    /// 初始化mysql上下文
    MYSQL mysql;
    /// 单线程模式 mysql_init自动调用 mysql_library_init 线程不安全
    mysql_init(&mysql);

    const char *host   = "192.168.1.89";
    const char *user   = "root";
    const char *passwd = "Handabao123@";
    const char *db     = "laoxiaketang";

    /// CLIENT_MULTI_STATEMENTS 支持多条sql语句
    std::cout << "mysql connect..." << std::endl;
    // if (!mysql_real_connect(&mysql, host, user, passwd, db, 3306, nullptr, 0))
    if (!mysql_real_connect(&mysql, host, user, passwd, db, 3306, nullptr,
                            CLIENT_MULTI_STATEMENTS)) /// 一次执行多条语句
    {
        std::cerr << "Error: " << mysql_error(&mysql) << std::endl;
    }
    else
    {
        std::cout << "mysql connect success!" << std::endl;
    }

    /// 1 创建好存放二进制数据的表 t_data
    const auto table_name = "t_data";
    const auto col_id     = "id";
    const auto col_name   = "name";
    const auto col_data   = "data";
    const auto col_size   = "size";
    const auto db_engine =
            "InnoDB"; /// 行锁级 /// 引擎的设置只在第一次创建表的时候设置 要改需要删除表或者"ALTER TABLE table_name ENGINE = new_engine;"
    // const auto db_engine  = "MyISAM"; /// 表锁级 不支持事务
    std::string sql = "";
    int         re  = 0;

    sql = std::format("CREATE TABLE IF NOT EXISTS `{}` (" ///表名
                      "`{}` int AUTO_INCREMENT,"          /// 图片id
                      "`{}` varchar(1024),"               /// 图片名称
                      "`{}` blob,"                        /// 图片数据
                      "`{}` int,"                         /// 图片大小
                      " PRIMARY KEY(`{}`)) ENGINE={};;",
                      table_name, col_id, col_name, col_data, col_size, col_id, db_engine);

    re = mysql_query(&mysql, sql.c_str());
    if (re != 0)
    {
        std::cout << "create table failed!!! " << mysql_error(&mysql) << std::endl;
    }
    else
    {
        std::cout << "create table success!" << std::endl;
    }

    ///2 清空表 truncate t_data
    sql = std::format(" TRUNCATE {};", table_name);
    re  = mysql_query(&mysql, sql.c_str());
    if (re != 0)
    {
        std::cout << "truncate table failed!!! " << mysql_error(&mysql) << std::endl;
    }
    else
    {
        std::cout << "truncate table success!" << std::endl;
    }
    /// 3 初始化stmt mysql_stmt_init
    MYSQL_STMT *stmt = mysql_stmt_init(&mysql);
    if (!stmt)
    {
        std::cerr << "mysql_stmt_init failed!" << mysql_error(&mysql) << std::endl;
    }

    /// 4 预处理sql语句
    sql = std::format("INSERT INTO `{}` (`{}`,`{}`,`{}`) VALUES(?,?,?)", table_name, col_name, col_data, col_size);
    if (mysql_stmt_prepare(stmt, sql.c_str(), sql.size()))
    {
        std::cerr << "mysql_stmt_prepare failed!" << mysql_stmt_error(stmt) << std::endl;
    }

    /// 5 打开并读取文件
    // std::string filename = "test_stmt.zip";
    std::string filename = "mysql.jpg";
    ///读取二进制
    std::fstream in(filename, std::ios::in | std::ios::binary);
    if (!in.is_open())
    {
        std::cerr << "file " << filename << " open failed!" << std::endl;
    }

    /// 文件指针移动到结尾处
    in.seekg(0, std::ios::end);
    /// 文件大小和文件二进制地址
    int file_size = in.tellg();
    /// 回到开头
    in.seekg(0, std::ios::beg);

    char *data   = new char[file_size];
    int   readed = 0; /// 已经读了多少
    while (!in.eof())
    {
        in.read(data + readed, file_size - readed);
        /// 读取了多少字节
        if (in.gcount() <= 0)
            break;
        readed += in.gcount();
    }

    in.close();

    /// 6 绑定字段
    MYSQL_BIND bind[3]    = { 0 };
    bind[0].buffer_type   = MYSQL_TYPE_STRING; /// name 文件名
    bind[0].buffer        = const_cast<char *>(filename.c_str());
    bind[0].buffer_length = filename.size();

    bind[1].buffer_type   = MYSQL_TYPE_BLOB; /// data 文件二进制内容
    bind[1].buffer        = data;            ///二进制文件
    bind[1].buffer_length = file_size;

    /// 文件大小
    bind[2].buffer_type = MYSQL_TYPE_LONG;
    bind[2].buffer      = &file_size;

    if (mysql_stmt_bind_param(stmt, bind) != 0)
    {
        std::cerr << "mysql_stmt_bind_param failed! " << mysql_stmt_error(stmt) << std::endl;
    }


    /// 7 执行stmt sql
    if (mysql_stmt_execute(stmt) != 0)
    {
        std::cerr << "mysql_stmt_execute failed! " << mysql_stmt_error(stmt) << std::endl;
    }
    delete data;
    mysql_stmt_close(stmt);

    /// 8 查询二进制数据，并保存问文件
    sql = std::format("SELECT * FROM {};", table_name);
    re  = mysql_query(&mysql, sql.c_str());
    if (re != 0)
    {
        std::cerr << "mysql query failed!" << mysql_error(&mysql) << std::endl;
    }

    /// 获取结果集
    MYSQL_RES *res = mysql_store_result(&mysql);
    if (!res)
    {
        std::cerr << "mysql_store_result failed!" << mysql_error(&mysql) << std::endl;
    }
    /// 取一行数据
    MYSQL_ROW row = mysql_fetch_row(res);
    if (!row)
    {
        std::cerr << "mysql_fetch_row failed!" << mysql_error(&mysql) << std::endl;
    }
    std::cout << row[0] << " " << row[1] << " " << row[3] << std::endl;

    /// 获取每列数据的大小
    unsigned long *lens  = mysql_fetch_lengths(res);
    int            f_num = mysql_num_fields(res);
    for (int i = 0; i < f_num; i++)
    {
        std::cout << "[" << lens[i] << "]";
    }
    filename = "out_";
    filename += row[1];
    std::fstream out(filename, std::ios::out | std::ios::binary);
    if (!out.is_open())
    {
        std::cerr << "open file  " << filename << " failed!" << std::endl;
    }
    out.write(row[2], lens[2]);
    out.close();


    mysql_close(&mysql);

    std::cin.get();
    return 0;
}
