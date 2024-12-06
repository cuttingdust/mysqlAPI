#include <LXMysql.h>
#include <iostream>
#include <format>

int main(int argc, char *argv[])
{
    LXMysql my;
    /// 1 mysql ��ʼ��
    std::cout << "my.Init() = " << my.init() << std::endl;

    /// 2 ����mysql ���û�е���init �ڲ����Զ�����
    if (my.connect("localhost", "root", "Handabao123@", "laoxiaketang"))
    {
        std::cout << "my.Connect success��" << std::endl;
    }

    /// 3 ִ��sql��䴴����
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
