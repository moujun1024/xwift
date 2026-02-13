#pragma once

#include <string>
#include <vector>
#include <memory>
#include <stdexcept>

namespace xwift {

enum class ErrorKind {
  Syntax,
  Semantic,
  Runtime,
  IO,
  Network,
  JSON,
  HTTP,
  Memory,
  Type,
  Unknown
};

class Error {
public:
  Error(ErrorKind kind, const std::string& message)
    : kind(kind), message(message) {}
  
  ErrorKind getKind() const { return kind; }
  const std::string& getMessage() const { return message; }
  const std::vector<std::string>& getStack() const { return stack; }
  
  void addStackFrame(const std::string& frame) {
    stack.push_back(frame);
  }
  
  std::string toString() const {
    std::string result;
    switch (kind) {
      case ErrorKind::Syntax: result = "[Syntax Error] "; break;
      case ErrorKind::Semantic: result = "[Semantic Error] "; break;
      case ErrorKind::Runtime: result = "[Runtime Error] "; break;
      case ErrorKind::IO: result = "[IO Error] "; break;
      case ErrorKind::Network: result = "[Network Error] "; break;
      case ErrorKind::JSON: result = "[JSON Error] "; break;
      case ErrorKind::HTTP: result = "[HTTP Error] "; break;
      case ErrorKind::Memory: result = "[Memory Error] "; break;
      case ErrorKind::Type: result = "[Type Error] "; break;
      default: result = "[Error] "; break;
    }
    result += message;
    
    if (!stack.empty()) {
      result += "\nStack trace:\n";
      for (const auto& frame : stack) {
        result += "  at " + frame + "\n";
      }
    }
    
    return result;
  }
  
  static Error syntax(const std::string& message) {
    return Error(ErrorKind::Syntax, message);
  }
  
  static Error semantic(const std::string& message) {
    return Error(ErrorKind::Semantic, message);
  }
  
  static Error runtime(const std::string& message) {
    return Error(ErrorKind::Runtime, message);
  }
  
  static Error io(const std::string& message) {
    return Error(ErrorKind::IO, message);
  }
  
  static Error network(const std::string& message) {
    return Error(ErrorKind::Network, message);
  }
  
  static Error json(const std::string& message) {
    return Error(ErrorKind::JSON, message);
  }
  
  static Error http(const std::string& message) {
    return Error(ErrorKind::HTTP, message);
  }
  
  static Error memory(const std::string& message) {
    return Error(ErrorKind::Memory, message);
  }
  
  static Error type(const std::string& message) {
    return Error(ErrorKind::Type, message);
  }
  
private:
  ErrorKind kind;
  std::string message;
  std::vector<std::string> stack;
};

class ErrorException : public std::runtime_error {
public:
  explicit ErrorException(const Error& error)
    : std::runtime_error(error.toString()), error(error) {}
  
  const Error& getError() const { return error; }
  
private:
  Error error;
};

}
