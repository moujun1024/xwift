#ifndef XWIFT_STDLIB_PROTOCOLS_ITERABLE_H
#define XWIFT_STDLIB_PROTOCOLS_ITERABLE_H

#include "xwift/AST/Type.h"
#include "xwift/Basic/LLVM.h"
#include <memory>
#include <functional>

namespace xwift {

// Protocol for iterable collections
template<typename T>
class Iterable {
public:
    virtual ~Iterable() = default;
    virtual void forEach(std::function<void(const T&)> callback) const = 0;
    virtual bool contains(std::function<bool(const T&)> predicate) const = 0;
    virtual std::shared_ptr<Iterable<T>> filter(std::function<bool(const T&)> predicate) const = 0;
    virtual bool all(std::function<bool(const T&)> predicate) const = 0;
    virtual bool any(std::function<bool(const T&)> predicate) const = 0;
};

}

#endif