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

    //// ==========================================1 创建表 ================================
    const auto  tableName = "t_image";
    const auto  colID     = "id";
    const auto  colName   = "name";
    const auto  colPath   = "path";
    const auto  colSize   = "size";
    std::string sql       = "";
    sql += std::format("CREATE TABLE IF NOT EXISTS `{}` (" ///表名
                       "`{}` int AUTO_INCREMENT,"          /// 图片id
                       "`{}` varchar(1024),"               /// 图片名称
                       "`{}` varchar(2046),"               /// 图片路径
                       "`{}` int,"                         /// 图片大小
                       " PRIMARY KEY(`{}`));",
                       tableName, colID, colName, colPath, colSize, colID);

    /// 清空数据，并恢复自增id从1开始
    sql += std::format(" TRUNCATE {};", tableName);


    //// ==========================================2 插入数据 CLIENT_MULTI_STATEMENTS ================================
    for (int i = 0; i < 100; i++)
    {
        sql += std::format("INSERT INTO {} (`{}`, `{}`, `{}`) VALUES ('{}', '{}', '{}');", tableName, colName, colPath,
                           colSize, std::format("image_{}.jpg", i), R"(D:\images)", 10240);
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
    sql += std::format("UPDATE {} SET {} WHERE {}={};", tableName, tmp, colID, 10);

    //// ==========================================4 删除数据 ================================
    sql += std::format("DELETE FROM {} WHERE {}={}; ", tableName, colID, 3);


    sql += std::format("SELECT * FROM {};", tableName);

    /// 执行sql语句立刻返回，但语句并没有全部执行好，需要获取结果
    /// 把sql整个发送给mysql server，server一条条执行，返回结果
    int re = mysql_query(&mysql, sql.c_str());
    if (re != 0)
    {
        std::cout << "mysql_query failed!!!" << mysql_error(&mysql) << std::endl;
    }
    else
    {
        std::cout << "mysql_query success!!!" << std::endl;
    }

    /// 有多个返回结果
    do
    {
        std::cout << "[result]";
        MYSQL_RES *result = mysql_store_result(&mysql);
        if (result) /// SELECT
        {
            std::cout << "SELECT mysql_num_rows = " << mysql_num_rows(result) << std::endl;
            mysql_free_result(result);
        }
        else /// INSERT UPDATE DELETE CREATE DROP truncate
        {
            ///  SELECT 出错 有字段无结果
            if (mysql_field_count(&mysql) > 0)
            {
                std::cout << "Not retrieve result! " << mysql_error(&mysql) << std::endl;
            }
            else /// INSERT UPDATE DELETE CREATE DROP truncate
            {
                /// 等待服务器的处理结果
                std::cout << mysql_affected_rows(&mysql) << " rows affected!" << std::endl;
            }
        }
    }
    while (mysql_next_result(&mysql) == 0); /// 取下一条结果 0表示有结果

    /// delete 后会直接删除空间
    mysql_close(&mysql);

    std::cin.get();
    return 0;
}
