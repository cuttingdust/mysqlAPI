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
    const char *passwd = "Handabao123@";
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

    /// 1 �����ô�Ŷ��������ݵı� t_data
    const auto table_name = "t_data";
    const auto col_id     = "id";
    const auto col_name   = "name";
    const auto col_data   = "data";
    const auto col_size   = "size";
    const auto db_engine =
            "InnoDB"; /// ������ /// ���������ֻ�ڵ�һ�δ������ʱ������ Ҫ����Ҫɾ�������"ALTER TABLE table_name ENGINE = new_engine;"
    // const auto db_engine  = "MyISAM"; /// ������ ��֧������
    std::string sql = "";
    int         re  = 0;

    sql = std::format("CREATE TABLE IF NOT EXISTS `{}` (" ///����
                      "`{}` int AUTO_INCREMENT,"          /// ͼƬid
                      "`{}` varchar(1024),"               /// ͼƬ����
                      "`{}` blob,"                        /// ͼƬ����
                      "`{}` int,"                         /// ͼƬ��С
                      " PRIMARY KEY(`{}`)) ENGINE={};;",
                      table_name, col_id, col_name, col_data, col_size, col_id, db_engine);

    re = mysql_query(&mysql, sql.c_str());
    if (re != 0)
    {
        std::cout << "create table failed!!! " << mysql_error(&mysql) << std::endl;
    }
    else
    {
        std::cout << "create table success!" << std::endl;
    }

    ///2 ��ձ� truncate t_data
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
    /// 3 ��ʼ��stmt mysql_stmt_init
    MYSQL_STMT *stmt = mysql_stmt_init(&mysql);
    if (!stmt)
    {
        std::cerr << "mysql_stmt_init failed!" << mysql_error(&mysql) << std::endl;
    }

    /// 4 Ԥ����sql���
    sql = std::format("INSERT INTO `{}` (`{}`,`{}`,`{}`) VALUES(?,?,?)", table_name, col_name, col_data, col_size);
    if (mysql_stmt_prepare(stmt, sql.c_str(), sql.size()))
    {
        std::cerr << "mysql_stmt_prepare failed!" << mysql_stmt_error(stmt) << std::endl;
    }

    /// 5 �򿪲���ȡ�ļ�
    // std::string filename = "test_stmt.zip";
    std::string filename = "mysql.jpg";
    ///��ȡ������
    std::fstream in(filename, std::ios::in | std::ios::binary);
    if (!in.is_open())
    {
        std::cerr << "file " << filename << " open failed!" << std::endl;
    }

    /// �ļ�ָ���ƶ�����β��
    in.seekg(0, std::ios::end);
    /// �ļ���С���ļ������Ƶ�ַ
    int file_size = in.tellg();
    /// �ص���ͷ
    in.seekg(0, std::ios::beg);

    char *data   = new char[file_size];
    int   readed = 0; /// �Ѿ����˶���
    while (!in.eof())
    {
        in.read(data + readed, file_size - readed);
        /// ��ȡ�˶����ֽ�
        if (in.gcount() <= 0)
            break;
        readed += in.gcount();
    }

    in.close();

    /// 6 ���ֶ�
    MYSQL_BIND bind[3]    = { 0 };
    bind[0].buffer_type   = MYSQL_TYPE_STRING; /// name �ļ���
    bind[0].buffer        = const_cast<char *>(filename.c_str());
    bind[0].buffer_length = filename.size();

    bind[1].buffer_type   = MYSQL_TYPE_BLOB; /// data �ļ�����������
    bind[1].buffer        = data;            ///�������ļ�
    bind[1].buffer_length = file_size;

    /// �ļ���С
    bind[2].buffer_type = MYSQL_TYPE_LONG;
    bind[2].buffer      = &file_size;

    if (mysql_stmt_bind_param(stmt, bind) != 0)
    {
        std::cerr << "mysql_stmt_bind_param failed! " << mysql_stmt_error(stmt) << std::endl;
    }


    /// 7 ִ��stmt sql
    if (mysql_stmt_execute(stmt) != 0)
    {
        std::cerr << "mysql_stmt_execute failed! " << mysql_stmt_error(stmt) << std::endl;
    }
    delete data;
    mysql_stmt_close(stmt);

    /// 8 ��ѯ���������ݣ����������ļ�
    sql = std::format("SELECT * FROM {};", table_name);
    re  = mysql_query(&mysql, sql.c_str());
    if (re != 0)
    {
        std::cerr << "mysql query failed!" << mysql_error(&mysql) << std::endl;
    }

    /// ��ȡ�����
    MYSQL_RES *res = mysql_store_result(&mysql);
    if (!res)
    {
        std::cerr << "mysql_store_result failed!" << mysql_error(&mysql) << std::endl;
    }
    /// ȡһ������
    MYSQL_ROW row = mysql_fetch_row(res);
    if (!row)
    {
        std::cerr << "mysql_fetch_row failed!" << mysql_error(&mysql) << std::endl;
    }
    std::cout << row[0] << " " << row[1] << " " << row[3] << std::endl;

    /// ��ȡÿ�����ݵĴ�С
    unsigned long *lens  = mysql_fetch_lengths(res);
    int            f_num = mysql_num_fields(res);
    for (int i = 0; i < f_num; i++)
    {
        std::cout << "[" << lens[i] << "]";
    }
    filename = "out_";
    filename += row[1];
    std::fstream out(filename, std::ios::out | std::ios::binary);
    if (!out.is_open())
    {
        std::cerr << "open file  " << filename << " failed!" << std::endl;
    }
    out.write(row[2], lens[2]);
    out.close();


    mysql_close(&mysql);

    std::cin.get();
    return 0;
}
