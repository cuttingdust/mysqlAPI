#include <iostream>
#include <mysql.h>
#include <thread>
#include <format>
#include <map>
#include <sstream>

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

    std::cout << "mysql connect..." << std::endl;
    if (!mysql_real_connect(&mysql, host, user, passwd, db, 3306, nullptr, 0))
    {
        std::cerr << "Error: " << mysql_error(&mysql) << std::endl;
    }
    else
    {
        std::cout << "mysql connect success!" << std::endl;
    }

    //// ==========================================1 创建表 ================================
    const auto tableName = "t_image";
    const auto colID     = "id";
    const auto colName   = "name";
    const auto colPath   = "path";
    const auto colSize   = "size";
    auto       sql       = std::format("CREATE TABLE IF NOT EXISTS `{}` (" ///表名
                                       "`{}` int AUTO_INCREMENT,"          /// 图片id
                                       "`{}` varchar(1024),"               /// 图片名称
                                       "`{}` varchar(2046),"               /// 图片路径
                                       "`{}` int,"                         /// 图片大小
                                       " PRIMARY KEY(`{}`));",
                                       tableName, colID, colName, colPath, colSize, colID);
    // std::cout << sql << std::endl;
    int re = mysql_query(&mysql, sql.c_str());
    if (re != 0)
    {
        std::cout << "create table " << tableName << "failed!!!: " << std::endl;
    }
    else
    {
        std::cout << "create table " << tableName << " success!" << std::endl;
    }

    /// 清空数据，并恢复自增id从1开始
    sql = std::format(" TRUNCATE {};", tableName);
    re  = mysql_query(&mysql, sql.c_str());
    if (re != 0)
    {
        std::cout << "mysql_query failed!" << mysql_error(&mysql) << std::endl;
    }


    //// ==========================================2 插入数据 CLIENT_MULTI_STATEMENTS ================================
    for (int i = 0; i < 1000; i++)
    {
        sql = std::format("INSERT INTO {} (`{}`, `{}`, `{}`) VALUES ('{}', '{}', '{}');", tableName, colName, colPath,
                          colSize, std::format("image_{}.jpg", i), R"(D:\images)", 10240);

        // std::cout << sql << std::endl;
        re = mysql_query(&mysql, sql.c_str());
        if (re == 0)
        {
            int count = mysql_affected_rows(&mysql);
            std::cout << "mysql_affected_rows " << count << " id =" << mysql_insert_id(&mysql) << std::endl;
        }
        else
        {
            std::cout << "insert failed!!!:" << mysql_error(&mysql) << std::endl;
        }
    }


    //// ========================================== 3 修改数据 ================================
    /// 根据map自动生成 update sql语句
    std::map<std::string, std::string> kv;
    kv.emplace(colName, "image_update001.png");
    kv.emplace(colSize, "5000");
    std::string tmp = "";
    for (auto ptr = kv.begin(); ptr != kv.end(); ptr++)
    {
        tmp += "`";
        tmp += ptr->first;
        tmp += "`='";
        tmp += ptr->second;
        tmp += "',";
    }
    tmp += " id=id ";
    sql = std::format("UPDATE {} SET {} WHERE {}={};", tableName, tmp, colID, 2);

    std::cout << sql << std::endl;
    re = mysql_query(&mysql, sql.c_str());
    if (re == 0)
    {
        int count = mysql_affected_rows(&mysql);
        std::cout << "update mysql_affected_rows " << count << std::endl;
    }
    else
    {
        std::cout << "update failed!!!:" << mysql_error(&mysql) << std::endl;
    }


    //// ==========================================4 删除数据 ================================
    sql = std::format("DELETE FROM {} WHERE {}={}; ", tableName, colID, 3);
    re  = mysql_query(&mysql, sql.c_str());
    if (re == 0)
    {
        int count = mysql_affected_rows(&mysql);
        std::cout << "delete mysql_affected_rows " << count << std::endl;
    }
    else
    {
        std::cout << "delete failed!!!:" << mysql_error(&mysql) << std::endl;
    }

    /// delete 不会实际删除空间，只做了标识
    // sql = std::format("DELETE FROM {}; ", tableName);
    // re  = mysql_query(&mysql, sql.c_str());
    // if (re == 0)
    // {
    //     int count = mysql_affected_rows(&mysql);
    //     std::cout << "delete mysql_affected_rows " << count << std::endl;
    // }
    // else
    // {
    //     std::cout << "delete failed!" << mysql_error(&mysql) << std::endl;
    // }

    /// delete 后会直接删除空间
    mysql_close(&mysql);

    std::cin.get();
    return 0;
}
