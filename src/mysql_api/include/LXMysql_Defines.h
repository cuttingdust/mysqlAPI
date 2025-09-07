#ifndef LXMYSQL_DEFINES_H
#define LXMYSQL_DEFINES_H

struct MysqlConInfo
{
    char host[128]    = { 0 };
    char user[128]    = { 0 };
    char pass[128]    = { 0 };
    char db_name[128] = { 0 };
    int  port         = 3306;
};

/// #���ݿ����ӳص������ļ�
struct MysqlPoolConInfo
{
    int initSize          = 10;
    int maxSize           = 1024;
    int maxIdleTime       = 60;  ///< #������ʱ��Ĭ�ϵ�λ����
    int connectionTimeOut = 100; ///< #���ӳ�ʱʱ�䵥λ�Ǻ���
};


#ifdef _WIN32
#define MYSQL_CONFIG_PATH "mysql_init.conf"
#define MYSQL_POOL_CONFIG "mysql_pool.conf"
#else
#define MYSQL_CONFIG_PATH "/etc/mysql_pool.conf"
#define MYSQL_POOL_CONFIG ""
#endif

#endif // LXMYSQL_DEFINES_H
