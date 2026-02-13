#ifndef XWIFT_STDLIB_PROTOCOLS_COMPARABLE_H
#define XWIFT_STDLIB_PROTOCOLS_COMPARABLE_H

#include "xwift/AST/Type.h"
#include "xwift/Basic/LLVM.h"
#include <memory>

namespace xwift {

// Protocol for comparable types
template<typename T>
class Comparable {
public:
    virtual ~Comparable() = default;
    virtual bool operator==(const T& other) const = 0;
    virtual bool operator!=(const T& other) const = 0;
    virtual bool operator<(const T& other) const = 0;
    virtual bool operator<=(const T& other) const = 0;
    virtual bool operator>(const T& other) const = 0;
    virtual bool operator>=(const T& other) const = 0;
};

}

#endif