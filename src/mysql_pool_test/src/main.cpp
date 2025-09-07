#include "LXMysqlPool.h"

#include <LXMysql.h>
#include <iostream>
#include <format>
#include <chrono>
#include <regex>
#include <thread>
#include <ctime>

constexpr auto table_chat = "chat";
constexpr auto col_name   = "name";
constexpr auto col_age    = "age";
constexpr auto col_sex    = "sex";

int main(int argc, char *argv[])
{
    XDATA data;
    data[col_name] = "zhang san";
    data[col_age]  = "20";
    data[col_sex]  = "male";

    {
        std::cout << "-------------------" << std::endl;
        clock_t begin = clock();

        LXMysql con;
        con.init();
        if (con.connect("localhost", "root", "System123@", "user"))
        {
            con.insert(data, table_chat);
        }

        std::thread th1(
                [&]
                {
                    for (int i = 0; i < 2500; ++i)
                    {
                        LXMysql con;
                        con.init();
                        if (con.connect("localhost", "root", "System123@", "user"))
                        {
                            con.insert(data, table_chat);
                            con.close();
                        }
                        con.close();
                    }
                });

        std::thread th2(
                [&]
                {
                    for (int i = 0; i < 2500; ++i)
                    {
                        LXMysql con;
                        con.init();
                        if (con.connect("localhost", "root", "System123@", "user"))
                        {
                            con.insert(data, table_chat);
                            con.close();
                        }
                        con.close();
                    }
                });

        std::thread th3(
                [&]
                {
                    for (int i = 0; i < 2500; ++i)
                    {
                        LXMysql con;
                        con.init();
                        if (con.connect("localhost", "root", "System123@", "user"))
                        {
                            con.insert(data, table_chat);
                            con.close();
                        }
                        con.close();
                    }
                });

        std::thread th4(
                [&]
                {
                    for (int i = 0; i < 2500; ++i)
                    {
                        LXMysql con;
                        con.init();
                        if (con.connect("localhost", "root", "System123@", "user"))
                        {
                            con.insert(data, table_chat);
                            con.close();
                        }
                        con.close();
                    }
                });

        th1.join();
        th2.join();
        th3.join();
        th4.join();

        con.close();

        clock_t end = clock();
        std::cout << (end - begin) << "ms" << std::endl;
    }


    {
        std::cout << "-------------------" << std::endl;
        clock_t begin = clock();

        std::thread th1(
                [&]
                {
                    for (int i = 0; i < 2500; ++i)
                    {
                        auto con = MySqlPool->getConnection();
                        con->insert(data, table_chat);
                    }
                });

        std::thread th2(
                [&]
                {
                    for (int i = 0; i < 2500; ++i)
                    {
                        auto con = MySqlPool->getConnection();
                        con->insert(data, table_chat);
                    }
                });

        std::thread th3(
                [&]
                {
                    for (int i = 0; i < 2500; ++i)
                    {
                        auto con = MySqlPool->getConnection();
                        con->insert(data, table_chat);
                    }
                });

        std::thread th4(
                [&]
                {
                    for (int i = 0; i < 2500; ++i)
                    {
                        auto con = MySqlPool->getConnection();
                        con->insert(data, table_chat);
                    }
                });

        th1.join();
        th2.join();
        th3.join();
        th4.join();


        clock_t end = clock();
        std::cout << (end - begin) << "ms" << std::endl;
    }

    {
        clock_t begin = clock();
        for (int i = 0; i < 10000; ++i)
        {
            LXMysql con;
            con.init();
            if (con.connect("localhost", "root", "System123@", "user"))
            {
                con.insert(data, table_chat);
            }
            con.close();
        }
        clock_t end = clock();
        std::cout << (end - begin) << "ms" << std::endl;
    }

    {
        std::cout << "-------------------" << std::endl;
        clock_t begin = clock();
        for (int i = 0; i < 10000; ++i)
        {
            auto con = MySqlPool->getConnection();
            con->insert(data, table_chat);
        }
        clock_t end = clock();
        std::cout << (end - begin) << "ms" << std::endl;
    }

    return 0;
}
