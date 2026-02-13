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
};

}
}
