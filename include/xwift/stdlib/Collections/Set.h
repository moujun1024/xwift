#ifndef XWIFT_STDLIB_COLLECTIONS_SET_H
#define XWIFT_STDLIB_COLLECTIONS_SET_H

#include "xwift/AST/Type.h"
#include "xwift/Basic/LLVM.h"
#include <unordered_set>
#include <vector>
#include <memory>
#include <algorithm>

namespace xwift {

// Generic Set implementation similar to Swift's Set<T>
template<typename T>
class Set {
private:
    std::unordered_set<T> elements;
    
public:
    Set() = default;
    
    Set(const std::unordered_set<T>& elems) : elements(elems) {}
    
    Set(std::initializer_list<T> init) {
        for (const auto& elem : init) {
            elements.insert(elem);
        }
    }
    
    // Count and capacity
    size_t count() const { return elements.size(); }
    bool isEmpty() const { return elements.empty(); }
    
    // Element operations
    bool insert(const T& element) {
        return elements.insert(element).second;
    }
    
    bool remove(const T& element) {
        return elements.erase(element) > 0;
    }
    
    void removeAll() {
        elements.clear();
    }
    
    // Query operations
    bool contains(const T& element) const {
        return elements.find(element) != elements.end();
    }
    
    // Set operations
    Set<T> intersection(const Set<T>& other) const {
        Set<T> result;
        for (const auto& elem : elements) {
            if (other.contains(elem)) {
                result.insert(elem);
            }
        }
        return result;
    }
    
    Set<T> unionSet(const Set<T>& other) const {
        Set<T> result;
        for (const auto& elem : elements) {
            result.insert(elem);
        }
        for (const auto& elem : other.elements) {
            result.insert(elem);
        }
        return result;
    }
    
    Set<T> difference(const Set<T>& other) const {
        Set<T> result;
        for (const auto& elem : elements) {
            if (!other.contains(elem)) {
                result.insert(elem);
            }
        }
        return result;
    }
    
    bool isSubsetOf(const Set<T>& other) const {
        for (const auto& elem : elements) {
            if (!other.contains(elem)) {
                return false;
            }
        }
        return true;
    }
    
    // Iterator support
    typename std::unordered_set<T>::iterator begin() { return elements.begin(); }
    typename std::unordered_set<T>::iterator end() { return elements.end(); }
    typename std::unordered_set<T>::const_iterator begin() const { return elements.begin(); }
    typename std::unordered_set<T>::const_iterator end() const { return elements.end(); }
    
    // Conversion to std::unordered_set
    std::unordered_set<T> toStdSet() const { return elements; }
};

// Type alias for XWift's Set type
template<typename T>
using XWiftSet = Set<T>;

}

#endif