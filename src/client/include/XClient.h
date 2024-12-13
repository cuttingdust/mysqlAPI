
/**
 * @file   XClient.h
 * @brief  
 *
 * @details   
 *
 * @author 31667
 * @date   2024-12-13
 */

#ifndef XCLIENT_H
#define XCLIENT_H

#include <memory>
#include <string>

class XClient
{
public:
    static XClient* get()
    {
        static XClient c;
        return &c;
    }

private:
    XClient();
    virtual ~XClient();

public:
    auto init(const std::string& ip) -> bool;

private:
    class PImpl;
    std::unique_ptr<PImpl> impl_;
};

#endif // XCLIENT_H
