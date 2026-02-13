#include "xwift/stdlib/HTTP/HTTPClient.h"
#include "xwift/stdlib/HTTP/HTTPPlugin.h"
#include "xwift/stdlib/HTTP/URLParser.h"
#include "xwift/stdlib/HTTP/BodyEncoder.h"
#include "xwift/Plugin/Plugin.h"
#include "xwift/Basic/Error.h"
#include <sstream>
#include <iomanip>
#include <map>

namespace xwift {
namespace http {

HTTPClient::HTTPClient() {
  auto& pluginManager = plugin::PluginManager::getInstance();
  auto httpPlugin = dynamic_cast<HTTPPlugin*>(pluginManager.getPlugin("HTTP"));
  
  if (httpPlugin) {
    backend = httpPlugin->getBackend();
  } else {
    if (pluginManager.loadPlugin("libhttp_plugin" XWIFT_PLATFORM_SUFFIX)) {
      httpPlugin = dynamic_cast<HTTPPlugin*>(pluginManager.getPlugin("HTTP"));
      if (httpPlugin) {
        backend = httpPlugin->getBackend();
      }
    }
  }
}

HTTPClient::~HTTPClient() = default;

Result<Response> HTTPClient::get(const std::string& url) {
  URL parsedUrl = URLParser::parse(url);
  if (!parsedUrl.isValid()) {
    return Result<Response>::err(Error::http("Invalid URL: " + url));
  }
  
  if (backend) {
    return backend->get(parsedUrl.toString());
  }
  return Result<Response>::err(Error::http("HTTP backend not initialized"));
}

Result<Response> HTTPClient::post(const std::string& url, const std::string& data) {
  URL parsedUrl = URLParser::parse(url);
  if (!parsedUrl.isValid()) {
    return Result<Response>::err(Error::http("Invalid URL: " + url));
  }
  
  if (backend) {
    return backend->post(parsedUrl.toString(), data);
  }
  return Result<Response>::err(Error::http("HTTP backend not initialized"));
}

Result<Response> HTTPClient::postJSON(const std::string& url, const std::string& json) {
  URL parsedUrl = URLParser::parse(url);
  if (!parsedUrl.isValid()) {
    return Result<Response>::err(Error::http("Invalid URL: " + url));
  }
  
  if (backend) {
    setHeader("Content-Type", "application/json");
    return backend->post(parsedUrl.toString(), json);
  }
  return Result<Response>::err(Error::http("HTTP backend not initialized"));
}

Result<Response> HTTPClient::postForm(const std::string& url, const std::map<std::string, std::string>& params) {
  URL parsedUrl = URLParser::parse(url);
  if (!parsedUrl.isValid()) {
    return Result<Response>::err(Error::http("Invalid URL: " + url));
  }
  
  std::string body = BodyEncoder::encodeFormURLEncoded(params);
  
  if (backend) {
    setHeader("Content-Type", "application/x-www-form-urlencoded");
    return backend->post(parsedUrl.toString(), body);
  }
  return Result<Response>::err(Error::http("HTTP backend not initialized"));
}

Result<Response> HTTPClient::put(const std::string& url, const std::string& data) {
  if (backend) {
    return backend->put(url, data);
  }
  return Result<Response>::err(Error::http("HTTP backend not initialized"));
}

Result<Response> HTTPClient::deleteRequest(const std::string& url) {
  if (backend) {
    return backend->deleteRequest(url);
  }
  return Result<Response>::err(Error::http("HTTP backend not initialized"));
}

void HTTPClient::setHeader(const std::string& key, const std::string& value) {
  if (backend) {
    backend->setHeader(key, value);
  }
}

void HTTPClient::setTimeout(int milliseconds) {
  if (backend) {
    backend->setTimeout(milliseconds);
  }
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
