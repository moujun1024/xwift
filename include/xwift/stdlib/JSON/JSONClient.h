#pragma once

#include "xwift/stdlib/JSON/JSONPlugin.h"
#include "xwift/Plugin/Plugin.h"
#include <memory>

#ifdef _WIN32
#define XWIFT_PLATFORM_SUFFIX ".dll"
#else
#define XWIFT_PLATFORM_SUFFIX ".so"
#endif

namespace xwift {
namespace json {

class JSONClient {
public:
  JSONClient();
  ~JSONClient() = default;
  
  JSONValue parse(const std::string& jsonStr);
  std::string stringify(const JSONValue& value);
  
private:
  std::shared_ptr<IJSONEngine> engine;
};

}
}
