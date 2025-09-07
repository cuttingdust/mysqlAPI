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

    std::queue<LXMysql *>   connectionQue_; ///< �洢mysql���ӵĶ���
    std::mutex              queueMutex_;    ///< ά�����Ӷ��е��̰߳�ȫ������
    std::atomic_int         connectionCnt_; ///< ��¼������������connection���ӵ�������
    std::condition_variable cv_;            ///< ���������������������������̺߳����������̵߳�ͨ��
};

LXMysqlPool::PImpl::PImpl(LXMysqlPool *owenr) : owenr_(owenr)
{
    /// ������������
    if (!inputDBConfig())
    {
        return;
    }

    /// ������ʼ����������
    for (int i = 0; i < pool_con_info_.initSize; ++i)
    {
        if (!addNewConSqlToPool())
        {
            std::cout << "LXMysqlPool::PImpl::PImpl addNewConSqlToPool failed!" << std::endl;
            continue;
        }
    }

    /// ����һ���µ��̣߳���Ϊ���ӵ������� linux thread => pthread_create
    std::thread produce(&LXMysqlPool::PImpl::produceConnectionTask, this);
    produce.detach();

    /// ����һ���µĶ�ʱ�̣߳�ɨ�賬��maxIdleTimeʱ��Ŀ������ӣ����ж��ڵ����ӻ���
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
        /// ���ݿ�����
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
        /// ���ӳ�����
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
    // con->setConnectTimeout(3); /// ���ӳ�ʱ��
    // con->setReconnect(true);   /// �Զ�����

    if (!con->connect(con_info_.host, con_info_.user, con_info_.pass, con_info_.db_name))
    {
        std::cerr << "my.Connect failed!" << std::endl;
        return false;
    }
    // std::cout << "my.Connect success��" << std::endl;
    con->refreshAliveTime(); /// ˢ��һ�¿�ʼ���е���ʼʱ��

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
            cv_.wait(lock); /// ���в��գ��˴������߳̽���ȴ�״̬
        }

        /// ��������û�е������ޣ����������µ�����
        if (connectionCnt_ < pool_con_info_.maxSize)
        {
            if (!addNewConSqlToPool())
            {
                return;
            }
        }

        /// ֪ͨ�������̣߳���������������
        cv_.notify_all();
    }
}

/// ɨ�賬��maxIdleTimeʱ��Ŀ������ӣ����ж��ڵ����ӻ���
auto LXMysqlPool::PImpl::scannerConnectionTask() -> void
{
    for (;;)
    {
        /// ͨ��sleepģ�ⶨʱЧ��
        std::this_thread::sleep_for(std::chrono::seconds(pool_con_info_.maxIdleTime));

        /// ɨ���������У��ͷŶ��������
        std::unique_lock<std::mutex> lock(queueMutex_);
        while (connectionCnt_ > pool_con_info_.initSize)
        {
            auto con = connectionQue_.front();
            if (con->getAliveTime() >= (pool_con_info_.maxIdleTime * 1000))
            {
                connectionQue_.pop();
                connectionCnt_--;
                delete con; /// ����~Connection()�ͷ�����
            }
            else
            {
                break; /// ��ͷ������û�г���_maxIdleTime���������ӿ϶�û��
            }
        }
    }
}


///  �̰߳�ȫ���������������ӿ�
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
                std::cout << "��ȡ�������ӳ�ʱ��...��ȡ����ʧ��!" << std::endl;
                return nullptr;
            }
        }
    }


    /// shared_ptr����ָ������ʱ�����connection��Դֱ��delete�����൱��
    /// ����connection������������connection�ͱ�close���ˡ�
    /// ������Ҫ�Զ���shared_ptr���ͷ���Դ�ķ�ʽ����connectionֱ�ӹ黹��queue����
    std::shared_ptr<LXMysql> sp(impl_->connectionQue_.front(),
                                [&](LXMysql *con)
                                {
                                    /// �������ڷ�����Ӧ���߳��е��õģ�����һ��Ҫ���Ƕ��е��̰߳�ȫ����
                                    std::unique_lock<std::mutex> lock(impl_->queueMutex_);
                                    con->refreshAliveTime(); /// ˢ��һ�¿�ʼ���е���ʼʱ��
                                    impl_->connectionQue_.push(con);
                                });

    impl_->connectionQue_.pop();
    impl_->cv_.notify_all(); /// �����������Ժ�֪ͨ�������̼߳��һ�£��������Ϊ���ˣ��Ͻ���������

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
