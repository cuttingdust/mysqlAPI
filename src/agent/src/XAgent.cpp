#include "XAgent.h"

#include <format>
#include <iostream>
#include <LXMysql.h>
#include <thread>

#ifndef _WIN32
#include <ifaddrs.h>
#include <arpa/inet.h>
#else
#include <winsock2.h>
#include <iphlpapi.h>
#include <ws2tcpip.h>
#pragma comment(lib, "ws2_32.lib")
#pragma comment(lib, "iphlpapi.lib")
#endif

// #define LOGPATH "/var/log/system.log"
#define LOGPATH "test.log"

constexpr auto FILE_LINE_LEN = 1024;
long           g_curr_offset = 0;

constexpr auto tabele_name = "t_log";
constexpr auto col_id      = "id";
constexpr auto col_ip      = "ip";
constexpr auto col_log     = "log";
constexpr auto col_time    = "log_time";

class XAgent::PImpl
{
public:
    PImpl(XAgent *owenr);
    ~PImpl() = default;

public:
    XAgent     *owenr_ = nullptr;
    LXMysql    *mysql_ = nullptr;
    std::string local_ip_;
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
    impl_->local_ip_ = getLocalIp();
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

    fseek(fp, 0, SEEK_END);
    g_curr_offset = ftell(fp);
    fseek(fp, 0, 0);

    /// 只审计系统开始运行之后事件
    fclose(fp);

    /// 创建t_log日志表

    const auto sql = std::format("CREATE TABLE IF NOT EXISTS `{0}` ("
                                 "`{1}` INT AUTO_INCREMENT,"
                                 "`{2}` VARCHAR(16),"
                                 "`{3}` VARCHAR(2048),"
                                 "`{4}` datetime, PRIMARY KEY(`{1}`))",
                                 tabele_name, col_id, col_ip, col_log, col_time);
    impl_->mysql_->query(sql.c_str());

    return true;
}

auto XAgent::main() -> void
{
    for (;;) /// 无限循环
    {
        if (std::string log = tail(LOGPATH); !log.empty())
        {
            saveLog(log);
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
}

auto XAgent::tail(const char *file) -> std::string
{
    std::string result;
    if (!file)
        return result;
    FILE *fp = fopen(file, "rb");
    if (!fp)
    {
        return result;
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
        result = text;
        // printf("%s\n", text);
    }

    fclose(fp);

    return result;
}

auto XAgent::getLocalIp() -> std::string
{
    std::string ip;
#ifndef _WIN32
    ifaddrs *ifadd = 0;
    if (getifaddrs(&ifadd) != 0)
        return "";
    /// 遍历地址
    ifaddrs *iter = ifadd;

    while (iter != NULL)
    {
        /// ipv4
        if (iter->ifa_addr->sa_family == AF_INET)
            if (strcmp(iter->ifa_name, "lo") != 0) /// 去掉回环地址 127.0.0.1
            {
                /// 转换整形ip为字符串

                void *tmp = &((sockaddr_in *)iter->ifa_addr)->sin_addr;
                inet_ntop(AF_INET, tmp, ip, INET_ADDRSTRLEN);
                break;
            }
        iter = iter->ifa_next;
    }
    freeifaddrs(ifadd);
#else

    WSADATA wsaData;

    /// 初始化 Winsock
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
    {
        printf("WSAStartup failed.\n");
        return ip;
    }
    ULONG                 outBufLen  = 0;
    IP_ADAPTER_ADDRESSES *pAddresses = NULL;

    /// 第一次调用以获取所需的缓冲区大小
    DWORD dwRetVal = GetAdaptersAddresses(AF_INET, 0, NULL, pAddresses, &outBufLen);
    if (dwRetVal == ERROR_BUFFER_OVERFLOW)
    {
        /// 分配足够的内存
        pAddresses = (IP_ADAPTER_ADDRESSES *)malloc(outBufLen);
        if (pAddresses == NULL)
        {
            printf("Memory allocation failed.\n");
            WSACleanup();
            return ip;
        }
    }
    else
    {
        printf("GetAdaptersAddresses failed with initial call: %lu\n", dwRetVal);
        WSACleanup();
        return ip; /// 不处理其他错误
    }

    /// 实际调用获取地址信息
    dwRetVal = GetAdaptersAddresses(AF_INET, 0, NULL, pAddresses, &outBufLen);
    if (dwRetVal == NO_ERROR)
    {
        IP_ADAPTER_ADDRESSES *pCurrAddresses = pAddresses;
        while (pCurrAddresses)
        {
            /// 检查网络接口状态
            if (pCurrAddresses->OperStatus == IfOperStatusUp)
            {
                IP_ADAPTER_UNICAST_ADDRESS *pUnicast = pCurrAddresses->FirstUnicastAddress;
                while (pUnicast)
                {
                    /// 只处理 IPv4 地址
                    if (pUnicast->Address.lpSockaddr->sa_family == AF_INET)
                    {
                        struct sockaddr_in *sa_in = (struct sockaddr_in *)pUnicast->Address.lpSockaddr;
                        /// 检查是否为回环地址
                        if (sa_in->sin_addr.S_un.S_addr != htonl(INADDR_LOOPBACK))
                        {
                            ip = inet_ntoa(sa_in->sin_addr);
                            // printf("Local IP Address: %s\n", ip);
                        }
                    }
                    pUnicast = pUnicast->Next;
                }
            }
            pCurrAddresses = pCurrAddresses->Next;
        }
    }
    else
    {
        printf("GetAdaptersAddresses failed with error: %lu\n", dwRetVal);
    }

    /// 释放分配的内存
    if (pAddresses)
    {
        free(pAddresses);
    }

    WSACleanup();
#endif
    return ip;
}

auto XAgent::saveLog(const std::string &log) -> bool
{
    if (!impl_->mysql_)
        return false;

    std::cout << log << std::endl;
    XDATA data;
    data[col_log] = log.c_str();
    data[col_ip]  = impl_->local_ip_.c_str();

    //插入时间，用mysql now（）
    //@表示 字段内容不加引号，@会自动去除
    data[col_time] = "@now()";
    impl_->mysql_->insert(data, tabele_name);
    return true;
}
