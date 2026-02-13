#include "xwift/stdlib/HTTP/HTTPPlugin.h"
#include "xwift/Basic/Error.h"
#include <curl/curl.h>
#include <sstream>
#include <cstring>

namespace xwift {
namespace http {

class CurlHTTPBackend : public IHTTPBackend {
public:
  CurlHTTPBackend() : timeout(30000) {
    curl_global_init(CURL_GLOBAL_DEFAULT);
  }
  
  ~CurlHTTPBackend() override {
    curl_global_cleanup();
  }
  
  Result<Response> get(const std::string& url) override {
    return sendRequest("GET", url);
  }
  
  Result<Response> post(const std::string& url, const std::string& data) override {
    return sendRequest("POST", url, data);
  }
  
  Result<Response> put(const std::string& url, const std::string& data) override {
    return sendRequest("PUT", url, data);
  }
  
  Result<Response> deleteRequest(const std::string& url) override {
    return sendRequest("DELETE", url);
  }
  
  void setHeader(const std::string& key, const std::string& value) override {
    headers[key] = value;
  }
  
  void setTimeout(int milliseconds) override {
    timeout = milliseconds;
  }
  
  std::string getName() const override {
    return "Curl";
  }
  
  std::string getVersion() const override {
    return "1.0.0";
  }
  
private:
  int timeout;
  std::map<std::string, std::string> headers;
  
  static size_t WriteCallback(void* contents, size_t size, size_t nmemb, void* userp) {
    size_t totalSize = size * nmemb;
    std::string* response = static_cast<std::string*>(userp);
    response->append(static_cast<char*>(contents), totalSize);
    return totalSize;
  }
  
  static size_t HeaderCallback(char* buffer, size_t size, size_t nitems, void* userdata) {
    size_t totalSize = size * nitems;
    std::map<std::string, std::string>* headersMap = static_cast<std::map<std::string, std::string>*>(userdata);
    
    std::string header(buffer, totalSize);
    size_t colonPos = header.find(':');
    if (colonPos != std::string::npos) {
      std::string key = header.substr(0, colonPos);
      std::string value = header.substr(colonPos + 1);
      
      while (!value.empty() && (value[0] == ' ' || value[0] == '\t' || value[0] == '\r' || value[0] == '\n')) {
        value = value.substr(1);
      }
      while (!value.empty() && (value.back() == '\r' || value.back() == '\n')) {
        value.pop_back();
      }
      
      if (!key.empty()) {
        (*headersMap)[key] = value;
      }
    }
    
    return totalSize;
  }
  
  Result<Response> sendRequest(const std::string& method, const std::string& url, const std::string& data = "") {
    Response response;
    response.statusCode = 0;
    response.error = HTTPError::None;
    
    CURL* curl = curl_easy_init();
    if (!curl) {
      return Result<Response>::err(Error::network("Failed to initialize CURL"));
    }
    
    std::string responseBody;
    std::map<std::string, std::string> responseHeaders;
    
    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &responseBody);
    curl_easy_setopt(curl, CURLOPT_HEADERFUNCTION, HeaderCallback);
    curl_easy_setopt(curl, CURLOPT_HEADERDATA, &responseHeaders);
    curl_easy_setopt(curl, CURLOPT_TIMEOUT_MS, timeout);
    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 1L);
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 2L);
    
    if (method == "GET") {
      curl_easy_setopt(curl, CURLOPT_HTTPGET, 1L);
    } else if (method == "POST") {
      curl_easy_setopt(curl, CURLOPT_POST, 1L);
      curl_easy_setopt(curl, CURLOPT_POSTFIELDS, data.c_str());
      curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, data.length());
    } else if (method == "PUT") {
      curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, "PUT");
      curl_easy_setopt(curl, CURLOPT_POSTFIELDS, data.c_str());
      curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, data.length());
    } else if (method == "DELETE") {
      curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, "DELETE");
    }
    
    struct curl_slist* headerList = nullptr;
    for (const auto& header : headers) {
      std::string headerStr = header.first + ": " + header.second;
      headerList = curl_slist_append(headerList, headerStr.c_str());
    }
    
    if (!data.empty()) {
      headerList = curl_slist_append(headerList, "Content-Type: application/x-www-form-urlencoded");
    }
    
    if (headerList) {
      curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headerList);
    }
    
    CURLcode res = curl_easy_perform(curl);
    
    if (res != CURLE_OK) {
      if (res == CURLE_OPERATION_TIMEDOUT) {
        return Result<Response>::err(Error::network("Request timeout"));
      } else if (res == CURLE_URL_MALFORMAT) {
        return Result<Response>::err(Error::network("Invalid URL"));
      } else if (res == CURLE_SSL_CONNECT_ERROR) {
        return Result<Response>::err(Error::network("SSL connection failed"));
      } else {
        return Result<Response>::err(Error::network("Request failed"));
      }
    } else {
      long statusCode;
      curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &statusCode);
      response.statusCode = static_cast<int>(statusCode);
      response.body = responseBody;
      response.headers = responseHeaders;
    }
    
    if (headerList) {
      curl_slist_free_all(headerList);
    }
    
    curl_easy_cleanup(curl);
    return Result<Response>::ok(response);
  }
};

}
}
