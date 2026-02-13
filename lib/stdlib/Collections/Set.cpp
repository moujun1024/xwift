#include "xwift/stdlib/Collections/Set.h"
#include <iostream>

namespace xwift {

// Explicit instantiation for common types
template class Set<int>;
template class Set<std::string>;
template class Set<double>;

// Helper function for XWift interpreter integration
extern "C" void* createSetValue(void* elements, size_t count, const char* elementType);

}