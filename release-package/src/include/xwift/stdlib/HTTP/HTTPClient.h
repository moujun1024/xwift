#ifndef XWIFT_HTTP_HTTPCLIENT_H
#define XWIFT_HTTP_HTTPCLIENT_H

#include "xwift/Basic/Result.h"
#include "xwift/stdlib/HTTP/HTTPBackend.h"
#include <string>
#include <map>

namespace xwift {
namespace http {

class HTTPClient {
public:
  HTTPClient();
  ~HTTPClient();
  
  Result<Response> get(const std::string& url);
  Result<Response> post(const std::string& url, const std::string& data);
  Result<Response> postJSON(const std::string& url, const std::string& json);
  Result<Response> postForm(const std::string& url, const std::map<std::string, std::string>& params);
  Result<Response> put(const std::string& url, const std::string& data);
  Result<Response> deleteRequest(const std::string& url);
  
  void setHeader(const std::string& key, const std::string& value);
  void setTimeout(int milliseconds);
  
private:
  std::shared_ptr<IHTTPBackend> backend;
};

}
}

#endif
