#ifndef XWIFT_HTTP_HTTP_H
#define XWIFT_HTTP_HTTP_H

#include "xwift/stdlib/HTTP/HTTPBackend.h"
#include <memory>

namespace xwift {
namespace http {

class HTTPClient {
public:
  HTTPClient();
  ~HTTPClient();
  
  Response get(const std::string& url);
  Response post(const std::string& url, const std::string& data);
  Response put(const std::string& url, const std::string& data);
  Response deleteRequest(const std::string& url);
  
  void setHeader(const std::string& key, const std::string& value);
  void setTimeout(int milliseconds);
  
private:
  std::unique_ptr<HTTPBackend> backend;
};

std::string urlEncode(const std::string& str);
std::string urlDecode(const std::string& str);

}
}

#endif
