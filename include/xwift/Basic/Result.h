#pragma once

#include "xwift/Basic/Error.h"
#include <optional>
#include <variant>
#include <string>

namespace xwift {

template<typename T>
class Optional {
public:
  Optional() : hasValue(false) {}
  Optional(const T& value) : hasValue(true), value(value) {}
  Optional(T&& value) : hasValue(true), value(std::move(value)) {}
  Optional(std::nullopt_t) : hasValue(false) {}
  
  bool has_value() const { return hasValue; }
  explicit operator bool() const { return hasValue; }
  
  T& operator*() { return value; }
  const T& operator*() const { return value; }
  
  T* operator->() { return &value; }
  const T* operator->() const { return &value; }
  
  T value_or(const T& defaultValue) const {
    return hasValue ? value : defaultValue;
  }
  
  T unwrap() const {
    if (!hasValue) {
      throw ErrorException(Error::runtime("Attempted to unwrap an empty Optional"));
    }
    return value;
  }
  
  T unwrap_or(const T& defaultValue) const {
    return hasValue ? value : defaultValue;
  }
  
  template<typename F>
  auto map(F&& f) -> Optional<decltype(f(value))> {
    if (hasValue) {
      return Optional<decltype(f(value))>(f(value));
    }
    return Optional<decltype(f(value))>();
  }
  
  template<typename F>
  auto and_then(F&& f) -> decltype(f(value)) {
    if (hasValue) {
      return f(value);
    }
    return decltype(f(value))();
  }
  
private:
  bool hasValue;
  T value;
};

template<typename T>
class Result {
public:
  Result(const T& value) : data(value) {}
  Result(T&& value) : data(std::move(value)) {}
  Result(const Error& error) : data(error) {}
  Result(Error&& error) : data(std::move(error)) {}
  
  bool is_ok() const {
    return std::holds_alternative<T>(data);
  }
  
  bool is_error() const {
    return std::holds_alternative<Error>(data);
  }
  
  T& unwrap() {
    if (is_error()) {
      throw ErrorException(std::get<Error>(data));
    }
    return std::get<T>(data);
  }
  
  const T& unwrap() const {
    if (is_error()) {
      throw ErrorException(std::get<Error>(data));
    }
    return std::get<T>(data);
  }
  
  T unwrap_or(const T& defaultValue) {
    if (is_ok()) {
      return std::get<T>(data);
    }
    return defaultValue;
  }
  
  const T& unwrap_or(const T& defaultValue) const {
    if (is_ok()) {
      return std::get<T>(data);
    }
    return defaultValue;
  }
  
  const Error& error() const {
    if (is_ok()) {
      throw ErrorException(Error::runtime("Attempted to get error from a successful Result"));
    }
    return std::get<Error>(data);
  }
  
  template<typename F>
  auto map(F&& f) -> Result<decltype(f(std::get<T>(data)))> {
    if (is_ok()) {
      return Result<decltype(f(std::get<T>(data)))>(f(std::get<T>(data)));
    }
    return Result<decltype(f(std::get<T>(data)))>(std::get<Error>(data));
  }
  
  template<typename F>
  auto and_then(F&& f) -> decltype(f(std::get<T>(data))) {
    if (is_ok()) {
      return f(std::get<T>(data));
    }
    return decltype(f(std::get<T>(data)))(std::get<Error>(data));
  }
  
  static Result<T> ok(const T& value) {
    return Result<T>(value);
  }
  
  static Result<T> err(const Error& error) {
    return Result<T>(error);
  }
  
  static Result<T> err(const std::string& message) {
    return Result<T>(Error::runtime(message));
  }
  
private:
  std::variant<T, Error> data;
};

}
