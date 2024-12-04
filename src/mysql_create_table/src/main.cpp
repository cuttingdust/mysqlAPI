#include <iostream>
#include <mysql.h>
#include <thread>
#include <format>
#include <map>
#include <sstream>

int main(int argc, char *argv[])
{
    /// ��ʼ��mysql������
    MYSQL mysql;
    /// ���߳�ģʽ mysql_init�Զ����� mysql_library_init �̲߳���ȫ
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

    //// ==========================================1 ������ ================================
    const auto tableName = "t_image";
    const auto colID     = "id";
    const auto colName   = "name";
    const auto colPath   = "path";
    const auto colSize   = "size";
    auto       sql       = std::format("CREATE TABLE IF NOT EXISTS `{}` (" ///����
                                       "`{}` int AUTO_INCREMENT,"          /// ͼƬid
                                       "`{}` varchar(1024),"               /// ͼƬ����
                                       "`{}` varchar(2046),"               /// ͼƬ·��
                                       "`{}` int,"                         /// ͼƬ��С
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

    /// ������ݣ����ָ�����id��1��ʼ
    sql = std::format(" TRUNCATE {};", tableName);
    re  = mysql_query(&mysql, sql.c_str());
    if (re != 0)
    {
        std::cout << "mysql_query failed!" << mysql_error(&mysql) << std::endl;
    }


    //// ==========================================2 �������� CLIENT_MULTI_STATEMENTS ================================
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


    //// ========================================== 3 �޸����� ================================
    /// ����map�Զ����� update sql���
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


    //// ==========================================4 ɾ������ ================================
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

    /// delete ����ʵ��ɾ���ռ䣬ֻ���˱�ʶ
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

    /// delete ���ֱ��ɾ���ռ�
    mysql_close(&mysql);

    std::cin.get();
    return 0;
}
