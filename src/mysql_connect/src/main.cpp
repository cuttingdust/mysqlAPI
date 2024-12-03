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


    for (int i = 0; i < 1000; ++i)
    {
        int re1 = mysql_ping(&mysql);
        if (re1 == 0)
        {
            std::cout << host << ":mysql ping success!" << std::endl;
        }
        else
        {
            std::cout << host << ":mysql ping failed! " << mysql_error(&mysql) << std::endl;
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    }

    mysql_close(&mysql);

    std::cin.get();
    return 0;
}
