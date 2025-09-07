#include "LXMysqlPool.h"
#include "LXMysql.h"
#include "LXMysqlTool.h"
#include "LXMysql_Defines.h"

#include <condition_variable>
#include <iostream>
#include <fstream>
#include <functional>
#include <queue>

class LXMysqlPool::PImpl
{
public:
    PImpl(LXMysqlPool *owenr);
    ~PImpl() = default;

public:
    auto inputDBConfig() -> bool;
    bool addNewConSqlToPool();


    auto produceConnectionTask() -> void;

    auto scannerConnectionTask() -> void;

public:
    LXMysqlPool     *owenr_ = nullptr;
    MysqlConInfo     con_info_;
    MysqlPoolConInfo pool_con_info_;

    std::queue<LXMysql *>   connectionQue_; ///< 存储mysql连接的队列
    std::mutex              queueMutex_;    ///< 维护连接队列的线程安全互斥锁
    std::atomic_int         connectionCnt_; ///< 记录连接所创建的connection连接的总数量
    std::condition_variable cv_;            ///< 设置条件变量，用于连接生产线程和连接消费线程的通信
};

LXMysqlPool::PImpl::PImpl(LXMysqlPool *owenr) : owenr_(owenr)
{
    /// 加载配置项了
    if (!inputDBConfig())
    {
        return;
    }

    /// 创建初始数量的连接
    for (int i = 0; i < pool_con_info_.initSize; ++i)
    {
        if (!addNewConSqlToPool())
        {
            std::cout << "LXMysqlPool::PImpl::PImpl addNewConSqlToPool failed!" << std::endl;
            continue;
        }
    }

    /// 启动一个新的线程，作为连接的生产者 linux thread => pthread_create
    std::thread produce(&LXMysqlPool::PImpl::produceConnectionTask, this);
    produce.detach();

    /// 启动一个新的定时线程，扫描超过maxIdleTime时间的空闲连接，进行对于的连接回收
    std::thread scanner(&LXMysqlPool::PImpl::scannerConnectionTask, this);
    scanner.detach();
}

auto LXMysqlPool::PImpl::inputDBConfig() -> bool
{
    bool          bReadConInf  = false;
    bool          bReadPoolInf = false;
    std::ifstream ifs;
    ifs.open(MYSQL_CONFIG_PATH, std::ios::binary);
    if (ifs.is_open())
    {
        ifs.read(reinterpret_cast<char *>(&con_info_), sizeof(con_info_));
        if (ifs.gcount() == sizeof(con_info_))
        {
            ifs.close();
            bReadConInf = true;
        }
        ifs.close();
    }
    ifs.open(MYSQL_POOL_CONFIG, std::ios::binary);
    if (ifs.is_open())
    {
        ifs.read(reinterpret_cast<char *>(&pool_con_info_), sizeof(pool_con_info_));
        if (ifs.gcount() == sizeof(pool_con_info_))
        {
            ifs.close();
            bReadPoolInf = true;
        }
        ifs.close();
    }

    if (!bReadConInf)
    {
        /// 数据库配置
        std::cout << "==============================" << std::endl;
        std::cout << "input the db set" << std::endl;
        std::cout << "input db host:";
        std::cin >> con_info_.host;
        std::cout << "input db user:";
        std::cin >> con_info_.user;
        std::cout << "input db pass:";
        LXMysqlTool::getPassword(con_info_.pass, sizeof(con_info_.pass) - 1);
        std::cout << std::endl;
        std::cout << "input db dbname(xms):";
        std::cin >> con_info_.db_name;
        std::cout << "input db port(3306):";
        std::cin >> con_info_.port;

        std::ofstream ofs;
        ofs.open(MYSQL_CONFIG_PATH, std::ios::binary);
        if (ofs.is_open())
        {
            ofs.write(reinterpret_cast<char *>(&con_info_), sizeof(con_info_));
            ofs.close();
        }
    }

    if (!bReadPoolInf)
    {
        /// 连接池配置
        std::cout << "==============================" << std::endl;
        std::cout << "input the pool set" << std::endl;
        std::cout << "input pool initSize(10):";
        std::cin >> pool_con_info_.initSize;
        std::cout << "input pool maxSize(1024):";
        std::cin >> pool_con_info_.maxSize;
        std::cout << "input pool maxIdleTime(60s):";
        std::cin >> pool_con_info_.maxIdleTime;
        std::cout << "input pool connectionTimeOut(100ms):";
        std::cin >> pool_con_info_.connectionTimeOut;

        std::ofstream ofs;
        ofs.open(MYSQL_POOL_CONFIG, std::ios::binary);
        if (ofs.is_open())
        {
            ofs.write(reinterpret_cast<char *>(&pool_con_info_), sizeof(pool_con_info_));
            ofs.close();
        }
    }

    return true;
}

bool LXMysqlPool::PImpl::addNewConSqlToPool()
{
    auto con = new LXMysql;
    con->init();
    // con->setConnectTimeout(3); /// 连接超时秒
    // con->setReconnect(true);   /// 自动重连

    if (!con->connect(con_info_.host, con_info_.user, con_info_.pass, con_info_.db_name))
    {
        std::cerr << "my.Connect failed!" << std::endl;
        return false;
    }
    // std::cout << "my.Connect success！" << std::endl;
    con->refreshAliveTime(); /// 刷新一下开始空闲的起始时间

    connectionQue_.emplace(con);
    connectionCnt_++;
    return true;
}

auto LXMysqlPool::PImpl::produceConnectionTask() -> void
{
    for (;;)
    {
        std::unique_lock<std::mutex> lock(queueMutex_);
        while (!connectionQue_.empty())
        {
            cv_.wait(lock); /// 队列不空，此处生产线程进入等待状态
        }

        /// 连接数量没有到达上限，继续创建新的连接
        if (connectionCnt_ < pool_con_info_.maxSize)
        {
            if (!addNewConSqlToPool())
            {
                return;
            }
        }

        /// 通知消费者线程，可以消费连接了
        cv_.notify_all();
    }
}

/// 扫描超过maxIdleTime时间的空闲连接，进行对于的连接回收
auto LXMysqlPool::PImpl::scannerConnectionTask() -> void
{
    for (;;)
    {
        /// 通过sleep模拟定时效果
        std::this_thread::sleep_for(std::chrono::seconds(pool_con_info_.maxIdleTime));

        /// 扫描整个队列，释放多余的连接
        std::unique_lock<std::mutex> lock(queueMutex_);
        while (connectionCnt_ > pool_con_info_.initSize)
        {
            auto con = connectionQue_.front();
            if (con->getAliveTime() >= (pool_con_info_.maxIdleTime * 1000))
            {
                connectionQue_.pop();
                connectionCnt_--;
                delete con; /// 调用~Connection()释放连接
            }
            else
            {
                break; /// 队头的连接没有超过_maxIdleTime，其它连接肯定没有
            }
        }
    }
}


///  线程安全的懒汉单例函数接口
auto LXMysqlPool::getConnectionPool() -> LXMysqlPool *
{
    static LXMysqlPool pool;
    return &pool;
}

auto LXMysqlPool::getConnection() const -> LXMysql::Ptr
{
    std::unique_lock<std::mutex> lock(impl_->queueMutex_);
    while (impl_->connectionQue_.empty())
    {
        /// sleep
        if (std::cv_status::timeout ==
            impl_->cv_.wait_for(lock, std::chrono::milliseconds(impl_->pool_con_info_.connectionTimeOut)))
        {
            if (impl_->connectionQue_.empty())
            {
                std::cout << "获取空闲连接超时了...获取连接失败!" << std::endl;
                return nullptr;
            }
        }
    }


    /// shared_ptr智能指针析构时，会把connection资源直接delete掉，相当于
    /// 调用connection的析构函数，connection就被close掉了。
    /// 这里需要自定义shared_ptr的释放资源的方式，把connection直接归还到queue当中
    std::shared_ptr<LXMysql> sp(impl_->connectionQue_.front(),
                                [&](LXMysql *con)
                                {
                                    /// 这里是在服务器应用线程中调用的，所以一定要考虑队列的线程安全操作
                                    std::unique_lock<std::mutex> lock(impl_->queueMutex_);
                                    con->refreshAliveTime(); /// 刷新一下开始空闲的起始时间
                                    impl_->connectionQue_.push(con);
                                });

    impl_->connectionQue_.pop();
    impl_->cv_.notify_all(); /// 消费完连接以后，通知生产者线程检查一下，如果队列为空了，赶紧生产连接

    return sp;
}

LXMysqlPool::LXMysqlPool()
{
    impl_ = std::make_unique<PImpl>(this);
}

LXMysqlPool::~LXMysqlPool()
{
    while (!impl_->connectionQue_.empty())
    {
        auto con = impl_->connectionQue_.front();
        con->close();
        impl_->connectionQue_.pop();
    }
}
