#pragma once

#include "xwift/Plugin/Plugin.h"
#include "xwift/stdlib/HTTP/HTTP.h"
#include <memory>

namespace xwift {
namespace http {

class IHTTPBackend {
public:
  virtual ~IHTTPBackend() = default;
  
  virtual Response get(const std::string& url) = 0;
  virtual Response post(const std::string& url, const std::string& data) = 0;
  virtual Response put(const std::string& url, const std::string& data) = 0;
  virtual Response deleteRequest(const std::string& url) = 0;
  virtual void setHeader(const std::string& key, const std::string& value) = 0;
  virtual void setTimeout(int milliseconds) = 0;
  
  virtual std::string getName() const = 0;
  virtual std::string getVersion() const = 0;
};

class HTTPPlugin : public plugin::Plugin {
public:
  bool initialize() override;
  void shutdown() override;
  plugin::PluginInfo getInfo() const override;
  
  std::shared_ptr<IHTTPBackend> getBackend();
  
private:
  std::shared_ptr<IHTTPBackend> backend;
};

}
}
