#include <iostream>
#include <mysql.h>
#include <thread>
#include <format>
#include <map>
#include <sstream>
#include <fstream>

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

    const auto  proc_name = "p_test";
    const auto  col_in    = "p_in";
    const auto  col_out   = "p_out";
    const auto  col_inout = "p_inout";
    std::string sql       = "";
    int         re        = 0;

    /// ɾ���洢����
    sql = std::format("DROP PROCEDURE IF EXISTS `{}`;", proc_name);
    re  = mysql_query(&mysql, sql.c_str());
    if (re != 0)
    {
        std::cout << "delete PROCEDURE failed!!!: " << mysql_error(&mysql) << std::endl;
    }

    /// 1 �����洢����
    sql = std::format("CREATE PROCEDURE `{0}` (IN {1} INT,OUT {2} INT,INOUT {3} INT)"
                      " BEGIN"
                      " SELECT {1},{2},{3};"
                      " SET {1}=100, {2}=200, {3}=300;"
                      " SELECT {1},{2},{3};"
                      " END",
                      proc_name, col_in, col_out, col_inout);
    re  = mysql_query(&mysql, sql.c_str());
    if (re != 0)
    {
        std::cout << "create PROCEDURE failed!!!: " << mysql_error(&mysql) << std::endl;
    }
    ///2 �������������
    std::cout << "IN in=1 out=2 inout=3" << std::endl;
    sql = "SET @A=1;SET @B=2;SET @C=3;";
    re  = mysql_query(&mysql, sql.c_str());
    if (re != 0)
    {
        std::cout << "set failed!!! : " << mysql_error(&mysql) << std::endl;
    }

    do
    {
        std::cout << "SET affect " << mysql_affected_rows(&mysql) << std::endl;
    }
    while (mysql_next_result(&mysql) == 0); /// 0 ���н���� -1 û�н�� >1����

    /// 3 ���ô洢���� call
    sql = std::format("CALL {}(@A,@B,@C)", proc_name);
    re  = mysql_query(&mysql, sql.c_str());
    if (re != 0)
    {
        std::cout << mysql_error(&mysql) << std::endl;
    }
    std::cout << "In Proc:";
    do
    {
        MYSQL_RES *res = mysql_store_result(&mysql);
        if (!res)
            continue;

        /// �ֶ�����
        int f_count = mysql_num_fields(res);

        /// ��ӡ�����
        for (;;)
        {
            /// ��ȡһ�м�¼
            MYSQL_ROW row = mysql_fetch_row(res);
            if (!row)
                break;
            for (int i = 0; i < f_count; i++)
            {
                if (row[i])
                {
                    std::cout << row[i] << " ";
                }
                else
                    std::cout << "NULL"
                              << " ";
            }
            std::cout << std::endl;
        }
        mysql_free_result(res);
    }
    while (mysql_next_result(&mysql) == 0); /// 0 ���н���� -1 û�н�� >1����

    /// 4 ��ȡ�洢���̵Ľ��
    sql = "SELECT @A,@B,@C";
    re  = mysql_query(&mysql, sql.c_str());
    if (re != 0)
    {
        std::cout << mysql_error(&mysql) << std::endl;
    }
    MYSQL_RES *res = mysql_store_result(&mysql);
    std::cout << "out: ";
    MYSQL_ROW row = mysql_fetch_row(res);
    std::cout << " in=" << row[0];
    std::cout << " out=" << row[1];
    std::cout << " inout=" << row[2];
    mysql_free_result(res);
    std::cout << std::endl;

    mysql_close(&mysql);

    std::cin.get();
    return 0;
}
