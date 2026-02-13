#ifndef XWIFT_STDLIB_HTTP_BACKEND_H
#define XWIFT_STDLIB_HTTP_BACKEND_H

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

class HTTPBackend {
public:
  virtual ~HTTPBackend() = default;
  
  virtual Response get(const std::string& url) = 0;
  virtual Response post(const std::string& url, const std::string& data) = 0;
  virtual Response put(const std::string& url, const std::string& data) = 0;
  virtual Response deleteRequest(const std::string& url) = 0;
  
  virtual void setHeader(const std::string& key, const std::string& value) = 0;
  virtual void setTimeout(int milliseconds) = 0;
};

std::unique_ptr<HTTPBackend> createHTTPBackend();

}
}

#endif
