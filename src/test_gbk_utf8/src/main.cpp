#include <iostream>
#include <format>
#include <chrono>
#ifdef _WIN32
#include <windows.h>
#else
#include <iconv.h>
#endif

#ifndef _WIN32
static size_t convert(char *from_cha, char *to_cha, char *in, size_t inlen, char *out, size_t outlen)
{
    /// ת��������
    iconv_t cd;
    cd = iconv_open(to_cha, from_cha);
    if (cd == 0)
        return -1;
    memset(out, 0, outlen);
    char **pin  = &in;
    char **pout = &out;
    // std::cout << "in = " << in << std::endl;
    // std::cout << "inlen = " << inlen << std::endl;
    // std::cout << "outlen = " << outlen << std::endl;
    //����ת���ֽ���������������תGBKʱ��������ȷ >=0�ͳɹ�
    size_t re = iconv(cd, pin, &inlen, pout, &outlen);
    iconv_close(cd);
    // std::cout << "result = " << (int)result << std::endl;
    return re;
}
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
#else
    result.resize(1024);
    int inlen = strlen(data);
    // std::cout << "inlen=" << inlen << std::endl;
    convert((char *)"utf-8", (char *)"gbk", (char *)data, inlen, (char *)result.data(), result.size());
    int outlen = strlen(result.data());
    //std::cout << "outlen = " << outlen << std::endl;
    result.resize(outlen);
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
#else
    result.resize(1024);
    int inlen = strlen(data);
    convert((char *)"gbk", (char *)"utf-8", (char *)data, inlen, (char *)result.data(), result.size());
    int outlen = strlen(result.data());
    // std::cout << "outlen = " << outlen << std::endl;
    result.resize(outlen);
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

    // auto utf8_mac = u8"����UTF8ת��GBK, ��ת��UTF8";
    // std::cout << utf8_mac << std::endl;

    return 0;
}
