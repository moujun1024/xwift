#include "xwift/stdlib/HTTP/HTTP.h"
#include "xwift/stdlib/HTTP/HTTPBackend.h"
#include <sstream>
#include <iomanip>

namespace xwift {
namespace http {

HTTPClient::HTTPClient() : backend(createHTTPBackend()) {
}

HTTPClient::~HTTPClient() = default;

Response HTTPClient::get(const std::string& url) {
  return backend->get(url);
}

Response HTTPClient::post(const std::string& url, const std::string& data) {
  return backend->post(url, data);
}

Response HTTPClient::put(const std::string& url, const std::string& data) {
  return backend->put(url, data);
}

Response HTTPClient::deleteRequest(const std::string& url) {
  return backend->deleteRequest(url);
}

void HTTPClient::setHeader(const std::string& key, const std::string& value) {
  backend->setHeader(key, value);
}

void HTTPClient::setTimeout(int milliseconds) {
  backend->setTimeout(milliseconds);
}

std::string urlEncode(const std::string& str) {
  std::ostringstream escaped;
  escaped.fill('0');
  escaped << std::hex;
  
  for (char c : str) {
    if (isalnum(c) || c == '-' || c == '_' || c == '.' || c == '~') {
      escaped << c;
    } else {
      escaped << std::uppercase;
      escaped << '%' << std::setw(2) << int((unsigned char)c);
      escaped << std::nouppercase;
    }
  }
  
  return escaped.str();
}

std::string urlDecode(const std::string& str) {
  std::string result;
  
  for (size_t i = 0; i < str.length(); ++i) {
    if (str[i] == '%' && i + 2 < str.length()) {
      std::string hex = str.substr(i + 1, 2);
      char c = static_cast<char>(std::stoi(hex, nullptr, 16));
      result += c;
      i += 2;
    } else if (str[i] == '+') {
      result += ' ';
    } else {
      result += str[i];
    }
  }
  
  return result;
}

}
}
