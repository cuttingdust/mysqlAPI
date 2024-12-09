#include <LXMysql.h>
#include <iostream>
#include <format>
#include <thread>
#include <chrono>

int main(int argc, char *argv[])
{
    LXMysql my;
    /// 1 mysql ��ʼ��
    my.init();
    my.setConnectTimeout(3); /// ���ӳ�ʱ��
    my.setReconnect(true);   /// �Զ�����

    /// 2 ����mysql ���û�е���init �ڲ����Զ�����
    if (!my.connect("192.168.1.89", "root", "Handabao123@", "laoxiaketang"))
    {
        std::cerr << "my.Connect failed!" << std::endl;
        return -1;
    }
    std::cout << "my.Connect success��" << std::endl;

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


    // /// �����Զ�����
    // for (;;)
    // {
    //     std::cout << my.query(sql.c_str()) << std::flush;
    // }

    /// ����һ����¼
    sql = std::format("INSERT INTO `{0}` (`{1}`,`{2}`) VALUES ('{3}',{4})", table_name, col_name, col_size, "test",
                      100);
    std::cout << my.query(sql.c_str()) << std::endl;

    XDATA kv;
    kv[col_name] = "test2";
    kv[col_size] = "200";
    my.insert(kv, table_name);

    kv[col_name] = "test3";
    kv[col_size] = "300";
    my.insert(kv, table_name);

    /// ���������ݲ���
    const std::string &fileName = "mysql.jpg";
    LXData             file1;
    file1.loadFile("mysql.jpg");
    kv[col_name] = fileName.c_str();
    kv[col_data] = file1;
    kv[col_size] = &file1.size;
    my.insertBin(kv, table_name);

    /// ��ȡ�����
    sql = std::format("SELECT * FROM `{0}`", table_name);
    std::cout << my.query(sql.c_str()) << std::endl;
    my.storeResult(); /// ���������ȫ���洢

    for (;;)
    {
        auto row = my.fetchRow();
        if (row.size() == 0)
            break;
        for (int i = 0; i < row.size(); i++)
        {
            if (row[i].data)
                std::cout << row[i].data << " ";
        }
        std::cout << std::endl;
    }
    my.freeResult();

    std::cout << my.query(sql.c_str()) << std::endl;
    my.useResult(); /// ��ʼ���ս����
    my.freeResult();


    my.close();
    std::cin.get();
    return 0;
}