#include "XAgent.h"

#include <iostream>
#include <LXMysql.h>

#define LOGPATH "/var/log/system.log"

class XAgent::PImpl
{
public:
    PImpl(XAgent *owenr);
    ~PImpl() = default;

public:
    XAgent  *owenr_ = nullptr;
    LXMysql *mysql_ = nullptr;
    FILE    *fp_    = nullptr;
};

XAgent::PImpl::PImpl(XAgent *owenr) : owenr_(owenr)
{
}

XAgent::XAgent()
{
    impl_ = std::make_unique<PImpl>(this);
}

XAgent::~XAgent()
{
    if (impl_->fp_)
    {
        fclose(impl_->fp_);
    }

    delete impl_->mysql_;
}

auto XAgent::init(const std::string &ip) -> bool
{
    if (ip.empty())
    {
        std::cerr << "XAgent::Init failed! ip is empty!" << std::endl;
        return false;
    }
    impl_->mysql_ = new LXMysql();
    /// �������ݿ�
    if (!impl_->mysql_->connect(ip.c_str(), "root", "System123@", "laoxiaketang"))
    {
        std::cerr << "XAgent::Init failed! Connect DB failed!" << std::endl;
        return false;
    }


    /// ��ȡ��־�ļ�
    impl_->fp_ = fopen(LOGPATH, "rb");
    if (!impl_->fp_)
    {
        std::cerr << "open log " << LOGPATH << " failed!" << std::endl;
        return false;
    }
    std::cout << "open log " << LOGPATH << " success!" << std::endl;
    /// ֻ���ϵͳ��ʼ����֮���¼�

    /// �ļ��ƶ�����β
    fseek(impl_->fp_, 0, SEEK_END);
    return true;
}
