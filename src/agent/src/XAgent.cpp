#include "XAgent.h"

#include <iostream>
#include <LXMysql.h>
#include <thread>

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
    /// 连接数据库
    if (!impl_->mysql_->connect(ip.c_str(), "root", "System123@", "laoxiaketang"))
    {
        std::cerr << "XAgent::Init failed! Connect DB failed!" << std::endl;
        return false;
    }


    /// 读取日志文件
    impl_->fp_ = fopen(LOGPATH, "rb");
    if (!impl_->fp_)
    {
        std::cerr << "open log " << LOGPATH << " failed!" << std::endl;
        return false;
    }
    std::cout << "open log " << LOGPATH << " success!" << std::endl;
    /// 只审计系统开始运行之后事件

    /// 文件移动到结尾
    fseek(impl_->fp_, 0, SEEK_END);
    return true;
}

auto XAgent::main() -> void
{
    if (!impl_->fp_)
    {
        std::cerr << "XAgent::main failed! log file is not open!" << std::endl;
        return;
    }

    /// 读取最新的日志
    std::string log = "";
    for (;;) /// 无限循环
    {
        char c = fgetc(impl_->fp_);
        /// 耗费cpu
        if (c <= 0) /// 没有字符或者读到结尾
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
            continue;
        }
        if (c == '\n')
        {
            std::cout << log << std::endl;
            log = "";
            continue;
        }
        log += c;
    }
}
