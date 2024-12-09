#include <iostream>
#include <format>
#include <chrono>
#include <windows.h>

std::string UTF8ToGBK(const char *data)
{
    std::string result = "";
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
    return result;
}

std::string GBKToUTF8(const char *data)
{
    std::string result = "";
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
    return result;
}

int main(int argc, char *argv[])
{
    std::cout << "Hello, world!����ã�����" << std::endl;

    /// 1 ����UTF-8תGBK
    std::cout << UTF8ToGBK(reinterpret_cast<const char *>(u8"����UTF-8תGBK")) << std::endl;

    /// 2 ����GBK��UTF-8��ת��
    std::string uft8 = GBKToUTF8("����GBKתUTF-8��תΪGBK");
    std::cout << "utf8=" << uft8 << std::endl;
    std::cout << UTF8ToGBK(uft8.c_str()) << std::endl;
    return 0;
}
