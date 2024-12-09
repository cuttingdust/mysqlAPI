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

#include <map>
#include <memory>
#include <string>
#include <vector>


struct LXM_EXPORT LXData
{
public:
    enum LX_DATA_TYPE
    {
        LXD_TYPE_DECIMAL,
        LXD_TYPE_TINY,
        LXD_TYPE_SHORT,
        LXD_TYPE_LONG,
        LXD_TYPE_FLOAT,
        LXD_TYPE_DOUBLE,
        LXD_TYPE_NULL,
        LXD_TYPE_TIMESTAMP,
        LXD_TYPE_LONGLONG,
        LXD_TYPE_INT24,
        LXD_TYPE_DATE,
        LXD_TYPE_TIME,
        LXD_TYPE_DATETIME,
        LXD_TYPE_YEAR,
        LXD_TYPE_NEWDATE, /**< Internal to MySQL. Not used in protocol */
        LXD_TYPE_VARCHAR,
        LXD_TYPE_BIT,
        LXD_TYPE_TIMESTAMP2,
        LXD_TYPE_DATETIME2,   /**< Internal to MySQL. Not used in protocol */
        LXD_TYPE_TIME2,       /**< Internal to MySQL. Not used in protocol */
        LXD_TYPE_TYPED_ARRAY, /**< Used for replication only */
        LXD_TYPE_INVALID     = 243,
        LXD_TYPE_BOOL        = 244, /**< Currently just a placeholder */
        LXD_TYPE_JSON        = 245,
        LXD_TYPE_NEWDECIMAL  = 246,
        LXD_TYPE_ENUM        = 247,
        LXD_TYPE_SET         = 248,
        LXD_TYPE_TINY_BLOB   = 249,
        LXD_TYPE_MEDIUM_BLOB = 250,
        LXD_TYPE_LONG_BLOB   = 251,
        LXD_TYPE_BLOB        = 252,
        LXD_TYPE_VAR_STRING  = 253,
        LXD_TYPE_STRING      = 254,
        LXD_TYPE_GEOMETRY    = 255
    };

    LXData();
    LXData(const char *data); /// �ַ��� �Ƕ�����
    LXData(const char *data, int size, const LX_DATA_TYPE &type);
    LXData(const int *d);


public:
    const char  *data = nullptr;
    int          size = 0;
    LX_DATA_TYPE type = LX_DATA_TYPE::LXD_TYPE_INVALID;
    auto         loadFile(const char *fileName) -> bool;
    auto         drop() -> void;
};

using XDATA = std::map<std::string, LXData>;

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

    /// \brief Mysql�������趨
    /// \return
    auto option(const LX_OPT &opt, const void *arg) -> bool;

    /// \brief �������ݿ����ӳ�ʱʱ��
    /// \param sec ��ʱʱ��
    /// \return �Ƿ����óɹ�
    auto setConnectTimeout(unsigned int sec) -> bool;

    /// \brief �������ݿ��Զ�����
    /// \param bRe �Ƿ��Զ�����
    /// \return �Ƿ����óɹ�
    auto setReconnect(bool bRe) -> bool;

    /// \brief ping ���ݿ� ��������
    /// \return �Ƿ�ping�ɹ�
    auto ping() -> bool;

    /// \brief ��ȡ�����
    /// \return
    auto storeResult() -> bool;

    /// \brief ��ʼ���ս����ͨ��Fetch��ȡ
    /// \return
    auto useResult() -> bool;

    /// \brief �ͷŽ����ռ�õĿռ�
    auto freeResult() -> void;

    /// \brief ��ȡһ������
    /// \return
    auto fetchRow() -> std::vector<LXData>;

    /// \brief ���ɲ���sql���
    /// \param kv <�ֶ���,�ֶ�ֵ>
    /// \param table_name ����
    /// \return sql ���
    auto getInsertSql(const XDATA &kv, const std::string &table_name) -> std::string;

    /// \brief �������ݿ�(�Ƕ���������)
    /// \param kv <�ֶ���,�ֶ�ֵ>
    /// \param table_name ����
    /// \return �Ƿ����ɹ�
    auto insert(const XDATA &kv, const std::string &table_name) -> bool;

    /// \brief �������ݿ�(����������)
    /// \param kv <�ֶ����� �ֶ�ֵ>
    /// \param table_name ����
    /// \return �Ƿ����ɹ�
    auto insertBin(const XDATA &kv, const std::string &table_name) -> bool;

private:
    class PImpl;
    std::unique_ptr<PImpl> impl_;
};

#endif // LXMYSQL_H