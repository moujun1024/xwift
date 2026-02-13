#pragma once

#include <string>
#include <map>
#include <memory>

namespace xwift {
namespace http {

enum class HTTPError {
  None,
  ConnectionFailed,
  Timeout,
  InvalidURL,
  SSLFailed,
  Unknown
};

struct Response {
  int statusCode;
  HTTPError error;
  std::string body;
  std::map<std::string, std::string> headers;
  
  Response() : statusCode(0), error(HTTPError::None) {}
  
  bool isSuccess() const {
    return statusCode >= 200 && statusCode < 300 && error == HTTPError::None;
  }
  
  bool isRedirect() const {
    return statusCode >= 300 && statusCode < 400;
  }
  
  bool isClientError() const {
    return statusCode >= 400 && statusCode < 500;
  }
  
  bool isServerError() const {
    return statusCode >= 500;
  }
  
  std::string getHeader(const std::string& key) const {
    auto it = headers.find(key);
    if (it != headers.end()) {
      return it->second;
    }
    return "";
  }
};

}
}
