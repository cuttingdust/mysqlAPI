#include <iostream>
#include <format>
#include <chrono>
#ifdef _WIN32
#include <windows.h>
#endif

std::string UTF8ToGBK(const char *data)
{
    std::string result = "";
#ifdef _WIN32
    /// 1 UFT8 转为unicode win utf16

    /// 1.1 统计转换后字节数
    int len = MultiByteToWideChar(CP_UTF8, /// 转换的格式
                                  0,       /// 默认的转换方式
                                  data,    /// 输入的字节
                                  -1,      /// 输入的字符串大小 -1 找\0
                                  0,       /// 输出
                                  0        /// 输出的空间大小
    );
    if (len <= 0)
        return result;
    std::wstring udata;
    udata.resize(len);
    MultiByteToWideChar(CP_UTF8, 0, data, -1, (wchar_t *)udata.data(), len);

    /// 2 unicode 转GBK
    len = WideCharToMultiByte(CP_ACP, 0, (wchar_t *)udata.data(), -1, 0, 0,
                              0, /// 失败默认替代字符
                              0  /// s是否使用默认替代
    );
    if (len <= 0)
        return result;
    result.resize(len);
    WideCharToMultiByte(CP_ACP, 0, (wchar_t *)udata.data(), -1, (char *)result.data(), len, 0, 0);
#endif
    return result;
}

std::string GBKToUTF8(const char *data)
{
    std::string result = "";
#ifdef _WIN32
    /// GBK转unicode

    /// 1.1 统计转换后字节数
    int len = MultiByteToWideChar(CP_ACP, /// 转换的格式
                                  0,      /// 默认的转换方式
                                  data,   /// 输入的字节
                                  -1,     /// 输入的字符串大小 -1 找\0
                                  0,      /// 输出
                                  0       /// 输出的空间大小
    );
    if (len <= 0)
        return result;
    std::wstring udata;
    udata.resize(len);
    MultiByteToWideChar(CP_ACP, 0, data, -1, (wchar_t *)udata.data(), len);

    /// 2 unicode 转utf-8
    len = WideCharToMultiByte(CP_UTF8, 0, (wchar_t *)udata.data(), -1, 0, 0,
                              0, /// 失败默认替代字符
                              0  /// s是否使用默认替代
    );
    if (len <= 0)
        return result;
    result.resize(len);
    WideCharToMultiByte(CP_UTF8, 0, (wchar_t *)udata.data(), -1, (char *)result.data(), len, 0, 0);
#endif
    return result;
}

int main(int argc, char *argv[])
{
    std::cout << "Hello, world!；你好，世界" << std::endl;

    /// 1 测试GBK 转换UTF-8,再转成GBK
    std::string gbk_string = "测试GBK 转换UTF-8,再转成GBK";
    std::cout << "GBK: " << gbk_string << std::endl;
    auto utf8_string = GBKToUTF8(gbk_string.c_str());
    std::cout << "UTF-8: " << utf8_string << std::endl;
    auto gbk_string2 = UTF8ToGBK(utf8_string.c_str());
    std::cout << "GBK: " << gbk_string2 << std::endl;

    return 0;
}
