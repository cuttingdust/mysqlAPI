#include <iostream>
#include <mysql.h>
#include <thread>

int main(int argc, char *argv[])
{
    /// 初始化mysql上下文
    MYSQL mysql;
    /// 单线程模式 mysql_init自动调用 mysql_library_init 线程不安全
    mysql_init(&mysql);

    const char *host   = "192.168.1.89";
    const char *user   = "root";
    const char *passwd = "Handabao123@";
    const char *db     = "mysql";

    int to = 3;                                                     /// 超时时间
    int re = mysql_options(&mysql, MYSQL_OPT_CONNECT_TIMEOUT, &to); /// 设置超时时间
    if (re != 0)
    {
        std::cout << "mysql_options failed!" << mysql_error(&mysql) << std::endl;
    }
    int recon = 1;
    re        = mysql_options(&mysql, MYSQL_OPT_RECONNECT, &recon); /// 设置自动重连
    if (re != 0)
    {
        std::cout << "mysql_options failed!" << mysql_error(&mysql) << std::endl;
    }


    std::cout << "mysql connect..." << std::endl;
    if (!mysql_real_connect(&mysql, host, user, passwd, db, 3306, nullptr, 0))
    {
        std::cerr << "Error: " << mysql_error(&mysql) << std::endl;
    }
    else
    {
        std::cout << "mysql connect success!" << std::endl;
    }

    /// user select * from user
    /// 1 执行SQL语句
    const char *sql = "select * from user";
    /// mysql_real_query  sql语句中可以包含二进制数据
    /// mysql_query sql语句中只能是字符串
    /// 0返回表示成功
    // re = mysql_real_query(&mysql, sql, strlen(sql));
    /// Commands out of sync; you can't run this command now
    /// 执行sql语句后，必须获取结果集并且清理
    re = mysql_query(&mysql, sql);
    if (re != 0)
    {
        std::cout << "mysql_real_query faied! :" << sql << " " << mysql_error(&mysql) << std::endl;
    }
    else
    {
        std::cout << "mysql_real_query success! :" << sql << std::endl;
    }

    /// 2 获取结果集
    /// mysql_use_result 不实际读取数据
    /// MYSQL_RES* result = mysql_use_result(&mysql);
    /// mysql_store_result 读取所有数据，注意缓存大小 MYSQL_OPT_MAX_ALLOWED_PACKET 默认 64M
    MYSQL_RES *result = mysql_store_result(&mysql);
    if (!result)
    {
        std::cout << "mysql_store_result failed! " << mysql_error(&mysql) << std::endl;
    }
    else
    {
        std::cout << "mysql_store_result success!" << std::endl;
    }

    /// 获取表字段
    MYSQL_FIELD *field = nullptr;
    while (field = mysql_fetch_field(result))
    {
        std::cout << "key:" << field->name << std::endl;
    }

    /// 获取表字段数量
    const int f_num = mysql_num_fields(result);

    /// 3 遍历结果集
    MYSQL_ROW row;
    while ((row = mysql_fetch_row(result)))
    {
        unsigned long *lens = mysql_fetch_lengths(result);

        // std::cout << lens[0] << "[" << row[0] << "," << row[1] << "]" << std::endl;
        for (int i = 0; i < f_num; i++)
        {
            std::cout << mysql_fetch_field_direct(result, i)->name << ":";
            if (row[i])
                std::cout << row[i];
            else
                std::cout << "NULL";
            std::cout << ",";
        }
        std::cout << "\n==========================================" << std::endl;
    }
    /// 清理结果集
    mysql_free_result(result);


    mysql_close(&mysql);

    std::cin.get();
    return 0;
}
