#include "xwift/stdlib/HTTP/HTTPPlugin.h"
#include "xwift/stdlib/HTTP/HTTPBackend.h"
#include "xwift/stdlib/HTTP/Win32Backend.h"

namespace xwift {
namespace http {

bool HTTPPlugin::initialize() {
#ifdef _WIN32
  backend = std::make_shared<Win32HTTPBackend>();
#else
  return false;
#endif
  return backend != nullptr;
}

void HTTPPlugin::shutdown() {
  backend.reset();
}

plugin::PluginInfo HTTPPlugin::getInfo() const {
  return {
    "HTTP",
    "1.0.0",
    "HTTP client plugin for XWift",
    "XWift Team"
  };
}

std::shared_ptr<IHTTPBackend> HTTPPlugin::getBackend() {
  return backend;
}

}
}

extern "C" {
  xwift::http::HTTPPlugin* createPlugin() {
    return new xwift::http::HTTPPlugin();
  }
  
  void destroyPlugin(xwift::http::HTTPPlugin* plugin) {
    delete plugin;
  }
}
