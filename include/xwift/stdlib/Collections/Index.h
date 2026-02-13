#ifndef XWIFT_STDLIB_COLLECTIONS_INDEX_H
#define XWIFT_STDLIB_COLLECTIONS_INDEX_H

#include "xwift/AST/Type.h"
#include "xwift/Basic/LLVM.h"
#include <memory>

namespace xwift {

// Protocol for indexable collections
template<typename T>
class Indexable {
public:
    virtual ~Indexable() = default;
    virtual T& operator[](size_t index) = 0;
    virtual const T& operator[](size_t index) const = 0;
    virtual size_t count() const = 0;
};

// Protocol for collections with countable elements
template<typename T>
class Countable {
public:
    virtual ~Countable() = default;
    virtual size_t count() const = 0;
    virtual bool isEmpty() const = 0;
};

// Protocol for collections that can be iterated
template<typename T>
class Iterable {
public:
    virtual ~Iterable() = default;
    virtual void forEach(std::function<void(const T&)> callback) const = 0;
};

}

#endif