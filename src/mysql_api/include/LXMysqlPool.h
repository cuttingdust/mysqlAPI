/**
 * @file   LXMysqlPool.h
 * @brief  
 *
 * @details   
 *
 * @author 31667
 * @date   2025-09-04
 */

#ifndef LXMYSQLPOOL_H
#define LXMYSQLPOOL_H

#include "LXMysql.h"
#include "LXMysql_Global.h"

#include <memory>

class LXMysql;

#define MySqlPool LXMysqlPool::getConnectionPool()

class LXM_EXPORT LXMysqlPool
{
public:
    /// \brief ��ȡ���ӳض���ʵ��
    /// \return
    static auto getConnectionPool() -> LXMysqlPool *;

    /// \brief ���ⲿ�ṩ�ӿڣ������ӳ��л�ȡһ�����õĿ�������
    /// \return
    auto getConnection() const -> LXMysql::Ptr;

private:
    LXMysqlPool();
    virtual ~LXMysqlPool();

    class PImpl;
    std::unique_ptr<PImpl> impl_;
};


#endif // LXMYSQLPOOL_H
