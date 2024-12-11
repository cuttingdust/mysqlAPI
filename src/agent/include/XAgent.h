/**
 * @file   XAgengt.h
 * @brief  
 *
 * @details   
 *
 * @author 31667
 * @date   2024-12-11
 */

#ifndef XAGENGT_H
#define XAGENGT_H

#include <memory>
#include <string>

class XAgent
{
public:
    XAgent();
    virtual ~XAgent();
    static XAgent* get()
    {
        static XAgent a;
        return &a;
    }

public:
    auto init(const std::string& ip) -> bool;

private:
    class PImpl;
    std::unique_ptr<PImpl> impl_;
};


#endif // XAGENGT_H
