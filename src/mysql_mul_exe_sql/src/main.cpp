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
    // if (!mysql_real_connect(&mysql, host, user, passwd, db, 3306, nullptr, 0))
    if (!mysql_real_connect(&mysql, host, user, passwd, db, 3306, nullptr,
                            CLIENT_MULTI_STATEMENTS)) /// һ��ִ�ж������
    {
        std::cerr << "Error: " << mysql_error(&mysql) << std::endl;
    }
    else
    {
        std::cout << "mysql connect success!" << std::endl;
    }

    //// ==========================================1 ������ ================================
    const auto  tableName = "t_image";
    const auto  colID     = "id";
    const auto  colName   = "name";
    const auto  colPath   = "path";
    const auto  colSize   = "size";
    std::string sql       = "";
    sql += std::format("CREATE TABLE IF NOT EXISTS `{}` (" ///����
                       "`{}` int AUTO_INCREMENT,"          /// ͼƬid
                       "`{}` varchar(1024),"               /// ͼƬ����
                       "`{}` varchar(2046),"               /// ͼƬ·��
                       "`{}` int,"                         /// ͼƬ��С
                       " PRIMARY KEY(`{}`));",
                       tableName, colID, colName, colPath, colSize, colID);

    /// ������ݣ����ָ�����id��1��ʼ
    sql += std::format(" TRUNCATE {};", tableName);


    //// ==========================================2 �������� CLIENT_MULTI_STATEMENTS ================================
    for (int i = 0; i < 100; i++)
    {
        sql += std::format("INSERT INTO {} (`{}`, `{}`, `{}`) VALUES ('{}', '{}', '{}');", tableName, colName, colPath,
                           colSize, std::format("image_{}.jpg", i), R"(D:\images)", 10240);
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
    sql += std::format("UPDATE {} SET {} WHERE {}={};", tableName, tmp, colID, 10);

    //// ==========================================4 ɾ������ ================================
    sql += std::format("DELETE FROM {} WHERE {}={}; ", tableName, colID, 3);


    sql += std::format("SELECT * FROM {};", tableName);

    /// ִ��sql������̷��أ�����䲢û��ȫ��ִ�кã���Ҫ��ȡ���
    /// ��sql�������͸�mysql server��serverһ����ִ�У����ؽ��
    int re = mysql_query(&mysql, sql.c_str());
    if (re != 0)
    {
        std::cout << "mysql_query failed!!!" << mysql_error(&mysql) << std::endl;
    }
    else
    {
        std::cout << "mysql_query success!!!" << std::endl;
    }

    /// �ж�����ؽ��
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
            ///  SELECT ���� ���ֶ��޽��
            if (mysql_field_count(&mysql) > 0)
            {
                std::cout << "Not retrieve result! " << mysql_error(&mysql) << std::endl;
            }
            else /// INSERT UPDATE DELETE CREATE DROP truncate
            {
                /// �ȴ��������Ĵ�����
                std::cout << mysql_affected_rows(&mysql) << " rows affected!" << std::endl;
            }
        }
    }
    while (mysql_next_result(&mysql) == 0); /// ȡ��һ����� 0��ʾ�н��

    /// delete ���ֱ��ɾ���ռ�
    mysql_close(&mysql);

    std::cin.get();
    return 0;
}
