#include "xwift/stdlib/Concurrency/Async.h"
#include <iostream>

namespace xwift {

// Explicit instantiation for common types
template class Task<int>;
template class Task<std::string>;
template class Task<double>;

// Helper function for XWift interpreter integration
extern "C" void* createTaskValue(void* task, const char* resultType);

}