#ifndef XWIFT_STDLIB_COLLECTIONS_DICTIONARY_H
#define XWIFT_STDLIB_COLLECTIONS_DICTIONARY_H

#include "xwift/AST/Type.h"
#include "xwift/Basic/LLVM.h"
#include <unordered_map>
#include <vector>
#include <memory>
#include <functional>

namespace xwift {

// Generic Dictionary implementation similar to Swift's Dictionary<Key, Value>
template<typename K, typename V>
class Dictionary {
private:
    std::unordered_map<K, V> elements;
    
public:
    Dictionary() = default;
    
    Dictionary(const std::unordered_map<K, V>& elems) : elements(elems) {}
    
    Dictionary(std::initializer_list<std::pair<K, V>> init) {
        for (const auto& pair : init) {
            elements[pair.first] = pair.second;
        }
    }
    
    // Count and capacity
    size_t count() const { return elements.size(); }
    bool isEmpty() const { return elements.empty(); }
    
    // Element access
    V& operator[](const K& key) {
        return elements[key]; // Creates default value if key doesn't exist
    }
    
    const V& operator[](const K& key) const {
        static V defaultValue; // Default constructed value
        auto it = elements.find(key);
        return (it != elements.end()) ? it->second : defaultValue;
    }
    
    // Key-value operations
    void set(const K& key, const V& value) {
        elements[key] = value;
    }
    
    void remove(const K& key) {
        elements.erase(key);
    }
    
    void removeAll() {
        elements.clear();
    }
    
    // Query operations
    bool containsKey(const K& key) const {
        return elements.find(key) != elements.end();
    }
    
    std::vector<K> keys() const {
        std::vector<K> result;
        result.reserve(elements.size());
        for (const auto& pair : elements) {
            result.push_back(pair.first);
        }
        return result;
    }
    
    std::vector<V> values() const {
        std::vector<V> result;
        result.reserve(elements.size());
        for (const auto& pair : elements) {
            result.push_back(pair.second);
        }
        return result;
    }
    
    // Iterator support
    typename std::unordered_map<K, V>::iterator begin() { return elements.begin(); }
    typename std::unordered_map<K, V>::iterator end() { return elements.end(); }
    typename std::unordered_map<K, V>::const_iterator begin() const { return elements.begin(); }
    typename std::unordered_map<K, V>::const_iterator end() const { return elements.end(); }
    
    // Conversion to std::unordered_map
    std::unordered_map<K, V> toStdMap() const { return elements; }
};

// Type alias for XWift's Dictionary type
template<typename K, typename V>
using XWiftDictionary = Dictionary<K, V>;

}

#endif