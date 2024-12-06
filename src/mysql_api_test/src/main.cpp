#include <LXMysql.h>
#include <iostream>
#include <format>

int main(int argc, char *argv[])
{
    LXMysql my;
    /// 1 mysql 初始化
    std::cout << "my.Init() = " << my.init() << std::endl;

    /// 2 连接mysql 如果没有调用init 内部会自动调用
    if (my.connect("localhost", "root", "Handabao123@", "laoxiaketang"))
    {
        std::cout << "my.Connect success！" << std::endl;
    }

    /// 3 执行sql语句创建表
    std::string       sql        = "";
    const std::string table_name = "t_video2";
    const std::string col_id     = "id";
    const std::string col_name   = "name";
    const std::string col_data   = "data";
    const std::string col_size   = "size";

    sql = std::format("CREATE TABLE IF NOT EXISTS `{0}` ("
                      "`{1}` INT AUTO_INCREMENT, "
                      "`{2}` VARCHAR(1024),"
                      "`{3}` BLOB,"
                      "`{4}` INT,PRIMARY KEY(`{1}`))",
                      table_name, col_id, col_name, col_data, col_size);

    std::cout << my.query(sql.c_str()) << std::endl;

    my.close();
    std::cin.get();
    return 0;
}
