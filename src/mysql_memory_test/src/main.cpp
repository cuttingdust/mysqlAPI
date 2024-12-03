#include <iostream>
#include <mysql.h>

int main(int argc, char *argv[])
{
    /// 初始化mysql上下文
    /// MYSQL mysql;
    /// 单线程模式 mysql_init自动调用 线程不安全
    mysql_library_init(0, nullptr, nullptr);


    // for (;;)
    // {
    //     /// 自己申请的自己释放
    //     MYSQL *conn = new MYSQL;
    //     mysql_init(conn);
    //     mysql_close(conn);
    //     delete conn;
    // }

    // for (;;)
    // {
    //     /// 栈空间自己释放
    //     MYSQL conn;
    //     mysql_init(&conn);
    //     mysql_close(&conn);
    // }

    for (;;)
    {
        /// 内部申请 内部自己释放
        MYSQL *conn = mysql_init(nullptr);
        mysql_close(conn);
    }


    mysql_library_end();
    return 0;
}
