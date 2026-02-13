#ifndef XWIFT_STDLIB_COLLECTIONS_ARRAY_H
#define XWIFT_STDLIB_COLLECTIONS_ARRAY_H

#include "xwift/AST/Type.h"
#include "xwift/Basic/LLVM.h"
#include <vector>
#include <memory>
#include <algorithm>

namespace xwift {

// Generic Array implementation similar to Swift's Array<T>
template<typename T>
class Array {
private:
    std::vector<T> elements;
    
public:
    Array() = default;
    
    Array(const std::vector<T>& elems) : elements(elems) {}
    
    Array(std::initializer_list<T> init) : elements(init) {}
    
    // Capacity and count
    size_t count() const { return elements.size(); }
    size_t capacity() const { return elements.capacity(); }
    
    // Element access
    T& operator[](size_t index) {
        if (index >= elements.size()) {
            throw std::out_of_range("Array index out of range");
        }
        return elements[index];
    }
    
    const T& operator[](size_t index) const {
        if (index >= elements.size()) {
            throw std::out_of_range("Array index out of range");
        }
        return elements[index];
    }
    
    // First and last elements
    T& first() { 
        if (elements.empty()) {
            throw std::out_of_range("Array is empty");
        }
        return elements[0]; 
    }
    
    const T& first() const { 
        if (elements.empty()) {
            throw std::out_of_range("Array is empty");
        }
        return elements[0]; 
    }
    
    T& last() { 
        if (elements.empty()) {
            throw std::out_of_range("Array is empty");
        }
        return elements.back(); 
    }
    
    const T& last() const { 
        if (elements.empty()) {
            throw std::out_of_range("Array is empty");
        }
        return elements.back(); 
    }
    
    // Subarray
    Array<T> subarray(size_t start, size_t length) const {
        if (start >= elements.size()) {
            return Array<T>();
        }
        
        size_t end = std::min(start + length, elements.size());
        std::vector<T> subElements(elements.begin() + start, elements.begin() + end);
        return Array<T>(subElements);
    }
    
    // Array operations
    void append(const T& element) {
        elements.push_back(element);
    }
    
    void append(const Array<T>& other) {
        elements.insert(elements.end(), other.elements.begin(), other.elements.end());
    }
    
    void insert(const T& element, size_t index) {
        if (index > elements.size()) {
            throw std::out_of_range("Insert index out of range");
        }
        elements.insert(elements.begin() + index, element);
    }
    
    void insert(const Array<T>& other, size_t index) {
        if (index > elements.size()) {
            throw std::out_of_range("Insert index out of range");
        }
        elements.insert(elements.begin() + index, other.elements.begin(), other.elements.end());
    }
    
    void remove(size_t index) {
        if (index >= elements.size()) {
            throw std::out_of_range("Remove index out of range");
        }
        elements.erase(elements.begin() + index);
    }
    
    void remove(size_t start, size_t length) {
        if (start >= elements.size()) {
            return;
        }
        
        size_t end = std::min(start + length, elements.size());
        elements.erase(elements.begin() + start, elements.begin() + end);
    }
    
    void removeAll() {
        elements.clear();
    }
    
    // Sorting and searching
    void sort() {
        std::sort(elements.begin(), elements.end());
    }
    
    bool contains(const T& element) const {
        return std::find(elements.begin(), elements.end(), element) != elements.end();
    }
    
    size_t find(const T& element) const {
        auto it = std::find(elements.begin(), elements.end(), element);
        return (it != elements.end()) ? std::distance(elements.begin(), it) : -1;
    }
    
    // Iterator support
    typename std::vector<T>::iterator begin() { return elements.begin(); }
    typename std::vector<T>::iterator end() { return elements.end(); }
    typename std::vector<T>::const_iterator begin() const { return elements.begin(); }
    typename std::vector<T>::const_iterator end() const { return elements.end(); }
    
    // Conversion to std::vector
    std::vector<T> toStdVector() const { return elements; }
};

// Type alias for XWift's Array type
template<typename T>
using XWiftArray = Array<T>;

}

#endif