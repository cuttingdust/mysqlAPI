/**
 * @file   XCenter.h
 * @brief  
 *
 * @details   
 *
 * @author 31667
 * @date   2024-12-11
 */

#ifndef XCENTER_H
#define XCENTER_H

#include <memory>
#include <string>

class XCenter
{
public:
    static XCenter* get()
    {
        static XCenter c;
        return &c;
    }

private:
    XCenter();
    virtual ~XCenter();

public:
    auto init() -> bool;
    auto install(const std::string& ip) -> bool;
    auto addDevice(const std::string& ip, const std::string& name) -> bool;
    auto main() -> void;

private:
    class PImpl;
    std::unique_ptr<PImpl> impl_;
};


#endif // XCENTER_H
