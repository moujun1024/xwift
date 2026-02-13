#include "xwift/stdlib/Collections/Array.h"
#include <iostream>

namespace xwift {

// Explicit instantiation for common types
template class Array<int>;
template class Array<double>;
template class Array<std::string>;
template class Array<bool>;

// Helper function for XWift interpreter integration
extern "C" void* createArrayValue(void* elements, size_t count, const char* elementType);

}