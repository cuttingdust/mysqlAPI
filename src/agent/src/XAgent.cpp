#include "XAgent.h"

#include <iostream>
#include <LXMysql.h>
#include <thread>

// #define LOGPATH "/var/log/system.log"
#define LOGPATH "test.log"

constexpr int FILE_LINE_LEN = 1024;
long          g_curr_offset = 0;

int32_t c_tail(const char *file)
{
    if (!file)
        return -1;
    FILE *fp = fopen(file, "rb");
    if (!fp)
    {
        printf("cant open file, file: %s\n", file);
        return -2;
    }

    fseek(fp, g_curr_offset, SEEK_SET);

    char     text[FILE_LINE_LEN];
    uint32_t len;
    while (!feof(fp))
    {
        memset(text, 0x0, FILE_LINE_LEN);
        fgets(text, FILE_LINE_LEN, fp);
        len = strlen(text);
        if (len == 0 || text[len - 1] != '\n')
            continue;
        text[len - 1] = 0;
        g_curr_offset += len;
        printf("%s\n", text);
    }

    fclose(fp);

    return 0;
}

class XAgent::PImpl
{
public:
    PImpl(XAgent *owenr);
    ~PImpl() = default;

public:
    XAgent  *owenr_ = nullptr;
    LXMysql *mysql_ = nullptr;
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
    FILE *fp = fopen(LOGPATH, "rb");
    if (!fp)
    {
        std::cerr << "open log " << LOGPATH << " failed!" << std::endl;
        return false;
    }
    std::cout << "open log " << LOGPATH << " success!" << std::endl;
    /// 只审计系统开始运行之后事件
    fclose(fp);

    return true;
}

auto XAgent::main() -> void
{
    for (;;) /// 无限循环
    {
        c_tail(LOGPATH);
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
}
