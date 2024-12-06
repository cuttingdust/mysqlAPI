/**
 * @file   LXMysql.h
 * @brief  
 *
 * @details   
 *
 * @author 31667
 * @date   2024-12-06
 */

#ifndef LXMYSQL_H
#define LXMYSQL_H

#include "LXMysql_Global.h"

#include <memory>

class LXM_EXPORT LXMysql
{
public:
    LXMysql();
    virtual ~LXMysql();
    enum LX_OPT
    {
        LX_OPT_CONNECT_TIMEOUT,
        LX_OPT_COMPRESS,
        LX_OPT_NAMED_PIPE,
        LX_INIT_COMMAND,
        LX_READ_DEFAULT_FILE,
        LX_READ_DEFAULT_GROUP,
        LX_SET_CHARSET_DIR,
        LX_SET_CHARSET_NAME,
        LX_OPT_LOCAL_INFILE,
        LX_OPT_PROTOCOL,
        LX_SHARED_MEMORY_BASE_NAME,
        LX_OPT_READ_TIMEOUT,
        LX_OPT_WRITE_TIMEOUT,
        LX_OPT_USE_RESULT,
        LX_REPORT_DATA_TRUNCATION,
        LX_OPT_RECONNECT,
        LX_PLUGIN_DIR,
        LX_DEFAULT_AUTH,
        LX_OPT_BIND,
        LX_OPT_SSL_KEY,
        LX_OPT_SSL_CERT,
        LX_OPT_SSL_CA,
        LX_OPT_SSL_CAPATH,
        LX_OPT_SSL_CIPHER,
        LX_OPT_SSL_CRL,
        LX_OPT_SSL_CRLPATH,
        LX_OPT_CONNECT_ATTR_RESET,
        LX_OPT_CONNECT_ATTR_ADD,
        LX_OPT_CONNECT_ATTR_DELETE,
        LX_SERVER_PUBLIC_KEY,
        LX_ENABLE_CLEARTEXT_PLUGIN,
        LX_OPT_CAN_HANDLE_EXPIRED_PASSWORDS,
        LX_OPT_MAX_ALLOWED_PACKET,
        LX_OPT_NET_BUFFER_LENGTH,
        LX_OPT_TLS_VERSION,
        LX_OPT_SSL_MODE,
        LX_OPT_GET_SERVER_PUBLIC_KEY,
        LX_OPT_RETRY_COUNT,
        LX_OPT_OPTIONAL_RESULTSET_METADATA,
        LX_OPT_SSL_FIPS_MODE
    };

public:
    /// \brief ��ʼ�����ݿ�
    /// \return
    auto init() -> bool;

    /// \brief �ر����ݿ�
    auto close() -> void;

    /// \brief �������ݿ�
    /// \param host ������ַ
    /// \param user �û���
    /// \param pass ����
    /// \param db   ���ݿ�
    /// \param port �˿�
    /// \param flag ���ӱ�־
    /// \return �Ƿ����ӳɹ�
    auto connect(const char *host, const char *user, const char *pass, const char *db, unsigned short port = 3306,
                 unsigned long flag = 0) -> bool;

    /// \brief ִ��sql ���
    /// \param sql sql ���
    /// \param sql_len sql ��䳤��
    /// \return
    auto query(const char *sql, unsigned long sql_len = 0) -> bool;

private:
    class PImpl;
    std::unique_ptr<PImpl> impl_;
};

#endif // LXMYSQL_H
