#include <iostream>
#include <mysql.h>
#include <thread>

int main(int argc, char *argv[])
{
    /// ��ʼ��mysql������
    MYSQL mysql;
    /// ���߳�ģʽ mysql_init�Զ����� mysql_library_init �̲߳���ȫ
    mysql_init(&mysql);

    const char *host   = "192.168.1.89";
    const char *user   = "root";
    const char *passwd = "Handabao123@";
    const char *db     = "mysql";

    int to = 3;                                                     /// ��ʱʱ��
    int re = mysql_options(&mysql, MYSQL_OPT_CONNECT_TIMEOUT, &to); /// ���ó�ʱʱ��
    if (re != 0)
    {
        std::cout << "mysql_options failed!" << mysql_error(&mysql) << std::endl;
    }
    int recon = 1;
    re        = mysql_options(&mysql, MYSQL_OPT_RECONNECT, &recon); /// �����Զ�����
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
    /// 1 ִ��SQL���
    const char *sql = "select * from user";
    /// mysql_real_query  sql����п��԰�������������
    /// mysql_query sql�����ֻ�����ַ���
    /// 0���ر�ʾ�ɹ�
    // re = mysql_real_query(&mysql, sql, strlen(sql));
    /// Commands out of sync; you can't run this command now
    /// ִ��sql���󣬱����ȡ�������������
    re = mysql_query(&mysql, sql);
    if (re != 0)
    {
        std::cout << "mysql_real_query faied! :" << sql << " " << mysql_error(&mysql) << std::endl;
    }
    else
    {
        std::cout << "mysql_real_query success! :" << sql << std::endl;
    }

    /// 2 ��ȡ�����
    /// mysql_use_result ��ʵ�ʶ�ȡ����
    /// MYSQL_RES* result = mysql_use_result(&mysql);
    /// mysql_store_result ��ȡ�������ݣ�ע�⻺���С MYSQL_OPT_MAX_ALLOWED_PACKET Ĭ�� 64M
    MYSQL_RES *result = mysql_store_result(&mysql);
    if (!result)
    {
        std::cout << "mysql_store_result failed! " << mysql_error(&mysql) << std::endl;
    }
    else
    {
        std::cout << "mysql_store_result success!" << std::endl;
    }
    /// 3 ���������
    MYSQL_ROW row;
    while ((row = mysql_fetch_row(result)))
    {
        unsigned long *lens = mysql_fetch_lengths(result);

        std::cout << lens[0] << "[" << row[0] << "," << row[1] << "]" << std::endl;
    }

    mysql_close(&mysql);

    std::cin.get();
    return 0;
}
