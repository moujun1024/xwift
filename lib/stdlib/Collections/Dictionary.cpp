#include "xwift/stdlib/Collections/Dictionary.h"
#include <iostream>

namespace xwift {

// Explicit instantiation for common types
template class Dictionary<std::string, int>;
template class Dictionary<std::string, std::string>;
template class Dictionary<std::string, double>;
template class Dictionary<int, std::string>;

// Helper function for XWift interpreter integration
extern "C" void* createDictionaryValue(void* pairs, size_t count, const char* keyType, const char* valueType);

}