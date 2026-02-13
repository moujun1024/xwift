#include "xwift/stdlib/Error/Error.h"
#include <iostream>

namespace xwift {

// Explicit instantiation for common error types
template class RuntimeError;
template class Result<int, RuntimeError>;
template class Result<std::string, RuntimeError>;
template class Result<bool, RuntimeError>;

// Helper function for XWift interpreter integration
extern "C" void* createErrorValue(const char* message, const char* code, bool isSuccess);

}