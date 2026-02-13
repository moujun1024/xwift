#include "xwift/stdlib/HTTP/HTTPBackend.h"
#include <windows.h>
#include <winhttp.h>

#pragma comment(lib, "winhttp.lib")

namespace xwift {
namespace http {

class Win32HTTPBackend : public HTTPBackend {
public:
  Win32HTTPBackend() : hSession(nullptr), hConnect(nullptr), timeout(30000) {
    hSession = WinHttpOpen(
      L"XWift HTTP Client/1.0",
      WINHTTP_ACCESS_TYPE_DEFAULT_PROXY,
      WINHTTP_NO_PROXY_NAME,
      WINHTTP_NO_PROXY_BYPASS,
      0
    );
    
    if (hSession) {
      WinHttpSetTimeouts(hSession, timeout, timeout, timeout, timeout);
    }
  }
  
  ~Win32HTTPBackend() override {
    if (hConnect) {
      WinHttpCloseHandle(hConnect);
    }
    if (hSession) {
      WinHttpCloseHandle(hSession);
    }
  }
  
  Response get(const std::string& url) override {
    return sendRequest("GET", url);
  }
  
  Response post(const std::string& url, const std::string& data) override {
    return sendRequest("POST", url, data);
  }
  
  Response put(const std::string& url, const std::string& data) override {
    return sendRequest("PUT", url, data);
  }
  
  Response deleteRequest(const std::string& url) override {
    return sendRequest("DELETE", url);
  }
  
  void setHeader(const std::string& key, const std::string& value) override {
    headers[key] = value;
  }
  
  void setTimeout(int milliseconds) override {
    timeout = milliseconds;
    if (hSession) {
      WinHttpSetTimeouts(hSession, timeout, timeout, timeout, timeout);
    }
  }
  
private:
  HINTERNET hSession;
  HINTERNET hConnect;
  int timeout;
  std::map<std::string, std::string> headers;
  
  Response sendRequest(const std::string& method, const std::string& url, const std::string& data = "") {
    Response response;
    response.statusCode = 0;
    response.error = HTTPError::None;
    
    if (!hSession) {
      response.statusCode = -1;
      response.error = HTTPError::ConnectionFailed;
      return response;
    }
    
    int urlLen = MultiByteToWideChar(CP_UTF8, 0, url.c_str(), -1, NULL, 0);
    std::wstring wUrl(urlLen, 0);
    MultiByteToWideChar(CP_UTF8, 0, url.c_str(), -1, &wUrl[0], urlLen);
    
    URL_COMPONENTS urlComp = {0};
    urlComp.dwStructSize = sizeof(urlComp);
    urlComp.dwHostNameLength = (DWORD)wUrl.length();
    urlComp.dwUrlPathLength = (DWORD)wUrl.length();
    urlComp.dwSchemeLength = (DWORD)wUrl.length();
    
    std::wstring hostName;
    std::wstring urlPath;
    hostName.resize(wUrl.length());
    urlPath.resize(wUrl.length());
    urlComp.lpszHostName = &hostName[0];
    urlComp.lpszUrlPath = &urlPath[0];
    
    if (!WinHttpCrackUrl(wUrl.c_str(), (DWORD)wUrl.length(), 0, &urlComp)) {
      response.statusCode = -1;
      response.error = HTTPError::InvalidURL;
      return response;
    }
    
    INTERNET_PORT port = urlComp.nPort;
    if (port == 0) {
      port = (urlComp.nScheme == INTERNET_SCHEME_HTTPS) ? 443 : 80;
    }
    
    if (hConnect) {
      WinHttpCloseHandle(hConnect);
    }
    
    hConnect = WinHttpConnect(hSession, hostName.c_str(), port, 0);
    if (!hConnect) {
      response.statusCode = -1;
      response.error = HTTPError::ConnectionFailed;
      return response;
    }
    
    int methodLen = MultiByteToWideChar(CP_UTF8, 0, method.c_str(), -1, NULL, 0);
    std::wstring wMethod(methodLen, 0);
    MultiByteToWideChar(CP_UTF8, 0, method.c_str(), -1, &wMethod[0], methodLen);
    
    HINTERNET hRequest = WinHttpOpenRequest(
      hConnect,
      wMethod.c_str(),
      urlPath.c_str(),
      NULL,
      WINHTTP_NO_REFERER,
      WINHTTP_DEFAULT_ACCEPT_TYPES,
      (urlComp.nScheme == INTERNET_SCHEME_HTTPS) ? WINHTTP_FLAG_SECURE : 0
    );
    
    if (!hRequest) {
      response.statusCode = -1;
      response.error = HTTPError::ConnectionFailed;
      return response;
    }
    
    std::wstring headersStr;
    for (const auto& header : headers) {
      int nameLen = MultiByteToWideChar(CP_UTF8, 0, header.first.c_str(), -1, NULL, 0);
      std::wstring wName(nameLen, 0);
      MultiByteToWideChar(CP_UTF8, 0, header.first.c_str(), -1, &wName[0], nameLen);
      
      int valueLen = MultiByteToWideChar(CP_UTF8, 0, header.second.c_str(), -1, NULL, 0);
      std::wstring wValue(valueLen, 0);
      MultiByteToWideChar(CP_UTF8, 0, header.second.c_str(), -1, &wValue[0], valueLen);
      
      headersStr += wName + L": " + wValue + L"\r\n";
    }
    
    if (!data.empty()) {
      headersStr += L"Content-Type: application/x-www-form-urlencoded\r\n";
      headersStr += L"Content-Length: " + std::to_wstring(data.length()) + L"\r\n";
    }
    
    BOOL result = WinHttpSendRequest(
      hRequest,
      headersStr.empty() ? WINHTTP_NO_ADDITIONAL_HEADERS : headersStr.c_str(),
      headersStr.empty() ? 0 : headersStr.length(),
      (LPVOID)data.c_str(),
      data.length(),
      data.length(),
      0
    );
    
    if (!result) {
      WinHttpCloseHandle(hRequest);
      response.statusCode = -1;
      response.error = HTTPError::ConnectionFailed;
      return response;
    }
    
    result = WinHttpReceiveResponse(hRequest, NULL);
    if (!result) {
      WinHttpCloseHandle(hRequest);
      response.statusCode = -1;
      response.error = HTTPError::ConnectionFailed;
      return response;
    }
    
    DWORD statusCode = 0;
    DWORD statusCodeSize = sizeof(statusCode);
    WinHttpQueryHeaders(
      hRequest,
      WINHTTP_QUERY_STATUS_CODE | WINHTTP_QUERY_FLAG_NUMBER,
      WINHTTP_HEADER_NAME_BY_INDEX,
      &statusCode,
      &statusCodeSize,
      WINHTTP_NO_HEADER_INDEX
    );
    response.statusCode = statusCode;
    
    DWORD headerSize = 0;
    WinHttpQueryHeaders(
      hRequest,
      WINHTTP_QUERY_RAW_HEADERS_CRLF,
      WINHTTP_HEADER_NAME_BY_INDEX,
      NULL,
      &headerSize,
      WINHTTP_NO_HEADER_INDEX
    );
    
    if (headerSize > 0) {
      std::vector<wchar_t> headerBuffer(headerSize / sizeof(wchar_t));
      if (WinHttpQueryHeaders(
        hRequest,
        WINHTTP_QUERY_RAW_HEADERS_CRLF,
        WINHTTP_HEADER_NAME_BY_INDEX,
        headerBuffer.data(),
        &headerSize,
        WINHTTP_NO_HEADER_INDEX
      )) {
        std::wstring rawHeaders(headerBuffer.data());
        size_t pos = 0;
        while (pos < rawHeaders.length()) {
          size_t endLine = rawHeaders.find(L"\r\n", pos);
          if (endLine == std::wstring::npos) {
            break;
          }
          
          std::wstring line = rawHeaders.substr(pos, endLine - pos);
          size_t colonPos = line.find(L':');
          if (colonPos != std::wstring::npos) {
            std::wstring name = line.substr(0, colonPos);
            std::wstring value = line.substr(colonPos + 1);
            
            while (!value.empty() && (value[0] == L' ' || value[0] == L'\t')) {
              value = value.substr(1);
            }
            
            int nameLen = WideCharToMultiByte(CP_UTF8, 0, name.c_str(), -1, NULL, 0, NULL, NULL);
            std::string nameUtf8(nameLen, 0);
            WideCharToMultiByte(CP_UTF8, 0, name.c_str(), -1, &nameUtf8[0], nameLen, NULL, NULL);
            
            int valueLen = WideCharToMultiByte(CP_UTF8, 0, value.c_str(), -1, NULL, 0, NULL, NULL);
            std::string valueUtf8(valueLen, 0);
            WideCharToMultiByte(CP_UTF8, 0, value.c_str(), -1, &valueUtf8[0], valueLen, NULL, NULL);
            
            response.headers[nameUtf8] = valueUtf8;
          }
          
          pos = endLine + 2;
        }
      }
    }
    
    DWORD bytesAvailable = 0;
    DWORD bytesRead = 0;
    std::string body;
    
    while (WinHttpQueryDataAvailable(hRequest, &bytesAvailable) && bytesAvailable > 0) {
      std::vector<char> buffer(bytesAvailable + 1);
      if (WinHttpReadData(hRequest, buffer.data(), bytesAvailable, &bytesRead)) {
        buffer[bytesRead] = '\0';
        body += buffer.data();
      }
    }
    
    response.body = body;
    
    WinHttpCloseHandle(hRequest);
    return response;
  }
};

std::unique_ptr<HTTPBackend> createHTTPBackend() {
  return std::make_unique<Win32HTTPBackend>();
}

}
}
