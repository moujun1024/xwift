#ifndef XWIFT_STDLIB_ERROR_ERROR_H
#define XWIFT_STDLIB_ERROR_ERROR_H

#include "xwift/AST/Type.h"
#include "xwift/Basic/LLVM.h"
#include <string>
#include <memory>
#include <vector>

namespace xwift {

// Error protocol similar to Swift's Error
class Error {
public:
    virtual ~Error() = default;
    virtual std::string getMessage() const = 0;
    virtual std::string getCode() const = 0;
};

// Concrete error implementation
class RuntimeError : public Error {
private:
    std::string message;
    std::string code;
    
public:
    RuntimeError(const std::string& msg, const std::string& errorCode = "")
        : message(msg), code(errorCode) {}
    
    std::string getMessage() const override { return message; }
    std::string getCode() const override { return code; }
};

// Result type similar to Swift's Result<Success, Error>
template<typename T, typename E = RuntimeError>
class Result {
private:
    bool isSuccess;
    union {
        T successValue;
        E errorValue;
    };
    
public:
    Result(const T& value) : isSuccess(true) {
        new(&successValue) T(value);
    }
    
    Result(const E& error) : isSuccess(false) {
        new(&errorValue) E(error);
    }
    
    ~Result() {
        if (isSuccess) {
            successValue.~T();
        } else {
            errorValue.~E();
        }
    }
    
    bool isSuccessful() const { return isSuccess; }
    bool isError() const { return !isSuccess; }
    
    const T& getValue() const {
        if (!isSuccess) {
            throw std::runtime_error("Cannot get value from error result");
        }
        return successValue;
    }
    
    const E& getError() const {
        if (isSuccess) {
            throw std::runtime_error("Cannot get error from success result");
        }
        return errorValue;
    }
    
    // Optional access methods
    bool getValue(T& outValue) const {
        if (isSuccess) {
            outValue = successValue;
            return true;
        }
        return false;
    }
    
    bool getError(E& outError) const {
        if (!isSuccess) {
            outError = errorValue;
            return true;
        }
        return false;
    }
};

}

#endif