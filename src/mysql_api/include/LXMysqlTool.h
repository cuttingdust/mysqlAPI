#ifndef LXMYSQLTOOL_H
#define LXMYSQLTOOL_H

#include <vector>
#include <string>

class LXMysqlTool
{
public:
    static auto join(const std::vector<std::string> &strings, const std::string &delimiter) -> std::string;

#ifndef _WIN32
    static auto convert(char *from_cha, char *to_cha, char *in, size_t inlen, char *out, size_t outlen) -> size_t;
#endif

    static auto getPassword(char *out, int out_size) -> int;
};


#endif // LXMYSQLTOOL_H
