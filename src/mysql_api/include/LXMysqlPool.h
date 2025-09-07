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
    /// \brief 获取连接池对象实例
    /// \return
    static auto getConnectionPool() -> LXMysqlPool *;

    /// \brief 给外部提供接口，从连接池中获取一个可用的空闲连接
    /// \return
    auto getConnection() const -> LXMysql::Ptr;

private:
    LXMysqlPool();
    virtual ~LXMysqlPool();

    class PImpl;
    std::unique_ptr<PImpl> impl_;
};


#endif // LXMYSQLPOOL_H
