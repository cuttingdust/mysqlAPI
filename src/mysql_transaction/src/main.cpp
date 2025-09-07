#include <chrono>
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
    const char *passwd = "System123@";
    const char *db     = "laoxiaketang";

    /// CLIENT_MULTI_STATEMENTS ֧�ֶ���sql���
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
    const auto table_name = "t_video";
    const auto col_id     = "id";
    const auto col_name   = "name";
    const auto col_path   = "path";
    const auto col_size   = "size";
    const auto db_engine =
            "InnoDB"; /// ������ /// ���������ֻ�ڵ�һ�δ������ʱ������ Ҫ����Ҫɾ�������"ALTER TABLE table_name ENGINE = new_engine;"
    // const auto db_engine  = "MyISAM"; /// ������ ��֧������
    std::string sql = "";
    int         re  = 0;

    sql = std::format("DROP TABLE {};", table_name);
    re  = mysql_query(&mysql, sql.c_str());
    if (re != 0)
    {
        std::cout << "drop table failed!!! " << mysql_error(&mysql) << std::endl;
    }
    else
    {
        std::cout << "drop table success!" << std::endl;
    }

    sql = std::format("CREATE TABLE IF NOT EXISTS `{}` (" ///����
                      "`{}` int AUTO_INCREMENT,"          /// ͼƬid
                      "`{}` varchar(1024),"               /// ͼƬ����
                      "`{}` varchar(2046),"               /// ͼƬ·��
                      "`{}` int,"                         /// ͼƬ��С
                      " PRIMARY KEY(`{}`)) ENGINE={};;",
                      table_name, col_id, col_name, col_path, col_size, col_id, db_engine);

    re = mysql_query(&mysql, sql.c_str());
    if (re != 0)
    {
        std::cout << "create table failed!!! " << mysql_error(&mysql) << std::endl;
    }
    else
    {
        std::cout << "create table success!" << std::endl;
    }

    /// ���������
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

    //// ==========================================2 ���� ================================
    /// 1 ��ʼ����
    /// START TRANSACTION;
    sql = "START TRANSACTION";
    re  = mysql_query(&mysql, sql.c_str());
    if (re != 0)
    {
        std::cout << "mysql_query failed! " << mysql_error(&mysql) << std::endl;
    }
    else
    {
        std::cout << "start transaction success!" << std::endl;
    }

    /// 2 ����Ϊ�ֶ��ύ����
    /// set autocommit = 0
    sql = "SET AUTOCOMMIT = 0";
    re  = mysql_query(&mysql, sql.c_str());
    if (re != 0)
    {
        std::cout << "mysql_query failed! " << mysql_error(&mysql) << std::endl;
    }

    /// 3 sql���
    ///�����������ݣ��ع�
    for (int i = 0; i < 3; i++)
    {
        sql = std::format("INSERT INTO {} (`{}`) VALUES('{}');", table_name, col_name, "test three!");
        re  = mysql_query(&mysql, sql.c_str());
        if (re != 0)
        {
            std::cout << "mysql_query failed! " << mysql_error(&mysql) << std::endl;
        }
        else
        {
            std::cout << "insert success!" << std::endl;
        }
    }

    // sql = "COMMIT"; /// �־û�
    // re  = mysql_query(&mysql, sql.c_str());
    // if (re != 0)
    // {
    //     std::cout << "mysql_query failed! " << mysql_error(&mysql) << std::endl;
    // }
    // else
    // {
    //     std::cout << "commit success!" << std::endl;
    // }

    /// 4 �ع�ROLLBACK MYISAM ��֧��
    sql = "ROLLBACK";
    re  = mysql_query(&mysql, sql.c_str());
    if (re != 0)
    {
        std::cout << "mysql_query failed! " << mysql_error(&mysql) << std::endl;
    }
    else
    {
        std::cout << "rollback success!" << std::endl;
    }

    for (int i = 0; i < 1000; i++)
    {
        sql = std::format("INSERT INTO {} (`{}`) VALUES('{}');", table_name, col_name, "test insert!");
        re  = mysql_query(&mysql, sql.c_str());
        if (re != 0)
        {
            std::cout << "mysql_query failed!!! " << mysql_error(&mysql) << std::endl;
        }
        else
        {
            std::cout << i << " insert success!" << std::endl;
        }
    }

    /// 5 COMMIT
    sql = "COMMIT";
    re  = mysql_query(&mysql, sql.c_str());
    if (re != 0)
    {
        std::cout << "mysql_query failed! " << mysql_error(&mysql) << std::endl;
    }
    else
    {
        std::cout << "commit success!" << std::endl;
    }

    /// 6 �ָ��Զ��ύ set autocommit = 1
    sql = "SET AUTOCOMMIT = 1";
    re  = mysql_query(&mysql, sql.c_str());
    if (re != 0)
    {
        std::cout << "mysql_query failed! " << mysql_error(&mysql) << std::endl;
    }


    sql = std::format("SELECT COUNT(*) FROM {}", table_name);
    re  = mysql_query(&mysql, sql.c_str());
    if (re != 0)
    {
        std::cout << "mysql_query failed! " << mysql_error(&mysql) << std::endl;
    }
    MYSQL_RES *res = mysql_store_result(&mysql);
    if (res)
    {
        /// ȡ�õ�һ������
        MYSQL_ROW row = mysql_fetch_row(res);
        if (row)
        {
            std::cout << "t_video count(*) = " << row[0] << std::endl;
        }
    }

    //// ==========================================3 ����3�ֲ������ݵķ�ʽ�ĺ�ʱ ================================
    auto start = std::chrono::high_resolution_clock::now();
    for (int i = 0; i < 1000; i++)
    {
        sql = std::format(
                "INSERT INTO `{}` (`{}`,`{}`) VALUES ('{}','{}');", table_name, col_name, col_path, "single",
                "0123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789");
        re = mysql_query(&mysql, sql.c_str());
        if (re != 0)
        {
            std::cout << "mysql_query failed! " << mysql_error(&mysql) << std::endl;
        }
    }
    auto end = std::chrono::high_resolution_clock::now();
    auto dur = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
    std::cout << "1 ����������1ǧ������" << dur / 1000. << "��" << std::endl;

    /// ����������1ǧ������
    {
        auto start = std::chrono::high_resolution_clock::now();
        sql        = "";
        for (int i = 0; i < 1000; i++)
        {
            sql += std::format(
                    "INSERT INTO `{}` (`{}`,`{}`) VALUES ('{}','{}');", table_name, col_name, col_path, "single",
                    "0123456789012345678901234567890123456789012345678901234567890123456789012345678901234567"
                    "890123456789");
        }

        re = mysql_query(&mysql, sql.c_str());
        if (re != 0)
        {
            std::cout << "mysql_query failed! " << mysql_error(&mysql) << std::endl;
        }
        do
        {
            std::cout << mysql_affected_rows(&mysql) << std::flush;
        }
        while (mysql_next_result(&mysql) == 0);

        auto end = std::chrono::high_resolution_clock::now();
        /// ת��Ϊ���� 1000
        auto dur = duration_cast<std::chrono::milliseconds>(end - start);
        std::cout << "2 ����������1ǧ������" << dur.count() / 1000. << "��" << std::endl;
    }

    /// �������1ǧ������
    {
        /// 1 ��ʼ����
        /// START TRANSACTION;
        sql = "START TRANSACTION";
        re  = mysql_query(&mysql, sql.c_str());
        if (re != 0)
        {
            std::cout << "mysql_query failed! " << mysql_error(&mysql) << std::endl;
        }

        /// 2 ����Ϊ�ֶ��ύ����
        /// set autocommit = 0
        sql = "SET AUTOCOMMIT = 0";
        re  = mysql_query(&mysql, sql.c_str());
        if (re != 0)
        {
            std::cout << "mysql_query failed! " << mysql_error(&mysql) << std::endl;
        }

        auto start = std::chrono::high_resolution_clock::now();

        for (int i = 0; i < 1000; i++)
        {
            sql = std::format("INSERT INTO `{}` (`{}`,`{}`) VALUES ('{}','{}');", table_name, col_name, col_path,
                              "single",
                              "0123456789012345678901234567890123456789012345678901234567890123456789012345678901234567"
                              "890123456789");
            re  = mysql_query(&mysql, sql.c_str());
            if (re != 0)
            {
                std::cout << "mysql_query failed! " << mysql_error(&mysql) << std::endl;
            }
            else
                std::cout << mysql_affected_rows(&mysql) << std::flush;
        }


        sql = "COMMIT";
        re  = mysql_query(&mysql, sql.c_str());
        if (re != 0)
        {
            std::cout << "mysql_query failed! " << mysql_error(&mysql) << std::endl;
        }

        sql = "SET AUTOCOMMIT = 1";
        re  = mysql_query(&mysql, sql.c_str());
        if (re != 0)
        {
            std::cout << "mysql_query failed! " << mysql_error(&mysql) << std::endl;
        }

        auto end = std::chrono::high_resolution_clock::now();
        /// ת��Ϊ���� 1000
        auto dur = duration_cast<std::chrono::milliseconds>(end - start);
        std::cout << "3 �������1ǧ������" << dur.count() / 1000. << "��" << std::endl;
    }


    /// delete ���ֱ��ɾ���ռ�
    mysql_close(&mysql);

    std::cin.get();
    return 0;
}
