#include <iostream>
#include <mysql.h>

int main(int argc, char *argv[])
{
    /// ��ʼ��mysql������
    /// MYSQL mysql;
    /// ���߳�ģʽ mysql_init�Զ����� �̲߳���ȫ
    mysql_library_init(0, nullptr, nullptr);


    // for (;;)
    // {
    //     /// �Լ�������Լ��ͷ�
    //     MYSQL *conn = new MYSQL;
    //     mysql_init(conn);
    //     mysql_close(conn);
    //     delete conn;
    // }

    // for (;;)
    // {
    //     /// ջ�ռ��Լ��ͷ�
    //     MYSQL conn;
    //     mysql_init(&conn);
    //     mysql_close(&conn);
    // }

    for (;;)
    {
        /// �ڲ����� �ڲ��Լ��ͷ�
        MYSQL *conn = mysql_init(nullptr);
        mysql_close(conn);
    }


    mysql_library_end();
    return 0;
}
