#ifndef XWIFT_HTTP_HTTP_H
#define XWIFT_HTTP_HTTP_H

#include <string>
#include <map>
#include <vector>
#include <functional>

namespace xwift {
namespace http {

struct Response {
  int statusCode;
  std::string body;
  std::map<std::string, std::string> headers;
};

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
  void* hSession;
  void* hConnect;
  std::map<std::string, std::string> headers;
  int timeout;
  
  Response sendRequest(const std::string& method, const std::string& url, const std::string& data = "");
};

std::string urlEncode(const std::string& str);
std::string urlDecode(const std::string& str);

}
}

#endif
