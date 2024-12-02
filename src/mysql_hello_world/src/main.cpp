#include <iostream>
#include <mysql.h>

int main(int argc, char *argv[])
{
    MYSQL *conn = nullptr;
    mysql_init(conn);
    std::cout << "hello world" << std::endl;

    return 0;
}
