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
    /// 1 UFT8 תΪunicode win utf16

    /// 1.1 ͳ��ת�����ֽ���
    int len = MultiByteToWideChar(CP_UTF8, /// ת���ĸ�ʽ
                                  0,       /// Ĭ�ϵ�ת����ʽ
                                  data,    /// ������ֽ�
                                  -1,      /// ������ַ�����С -1 ��\0
                                  0,       /// ���
                                  0        /// ����Ŀռ��С
    );
    if (len <= 0)
        return result;
    std::wstring udata;
    udata.resize(len);
    MultiByteToWideChar(CP_UTF8, 0, data, -1, (wchar_t *)udata.data(), len);

    /// 2 unicode תGBK
    len = WideCharToMultiByte(CP_ACP, 0, (wchar_t *)udata.data(), -1, 0, 0,
                              0, /// ʧ��Ĭ������ַ�
                              0  /// s�Ƿ�ʹ��Ĭ�����
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
    /// GBKתunicode

    /// 1.1 ͳ��ת�����ֽ���
    int len = MultiByteToWideChar(CP_ACP, /// ת���ĸ�ʽ
                                  0,      /// Ĭ�ϵ�ת����ʽ
                                  data,   /// ������ֽ�
                                  -1,     /// ������ַ�����С -1 ��\0
                                  0,      /// ���
                                  0       /// ����Ŀռ��С
    );
    if (len <= 0)
        return result;
    std::wstring udata;
    udata.resize(len);
    MultiByteToWideChar(CP_ACP, 0, data, -1, (wchar_t *)udata.data(), len);

    /// 2 unicode תutf-8
    len = WideCharToMultiByte(CP_UTF8, 0, (wchar_t *)udata.data(), -1, 0, 0,
                              0, /// ʧ��Ĭ������ַ�
                              0  /// s�Ƿ�ʹ��Ĭ�����
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
    std::cout << "Hello, world!����ã�����" << std::endl;

    /// 1 ����GBK ת��UTF-8,��ת��GBK
    std::string gbk_string = "����GBK ת��UTF-8,��ת��GBK";
    std::cout << "GBK: " << gbk_string << std::endl;
    auto utf8_string = GBKToUTF8(gbk_string.c_str());
    std::cout << "UTF-8: " << utf8_string << std::endl;
    auto gbk_string2 = UTF8ToGBK(utf8_string.c_str());
    std::cout << "GBK: " << gbk_string2 << std::endl;

    return 0;
}
