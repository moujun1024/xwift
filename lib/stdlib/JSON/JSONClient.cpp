#include "xwift/stdlib/JSON/JSONClient.h"
#include "xwift/stdlib/JSON/JSONPlugin.h"
#include "xwift/Plugin/Plugin.h"
#include "xwift/Basic/Error.h"

namespace xwift {
namespace json {

JSONClient::JSONClient() {
  auto& pluginManager = plugin::PluginManager::getInstance();
  auto jsonPlugin = dynamic_cast<JSONPlugin*>(pluginManager.getPlugin("JSON"));
  
  if (jsonPlugin) {
    engine = jsonPlugin->getEngine();
  } else {
    if (pluginManager.loadPlugin("libjson_plugin" XWIFT_PLATFORM_SUFFIX)) {
      jsonPlugin = dynamic_cast<JSONPlugin*>(pluginManager.getPlugin("JSON"));
      if (jsonPlugin) {
        engine = jsonPlugin->getEngine();
      }
    }
  }
}

Result<JSONValue> JSONClient::parse(const std::string& jsonStr) {
  if (engine) {
    return engine->parse(jsonStr);
  }
  return Result<JSONValue>::err(Error::json("JSON engine not initialized"));
}

Result<std::string> JSONClient::stringify(const JSONValue& value) {
  if (engine) {
    return engine->stringify(value);
  }
  return Result<std::string>::err(Error::json("JSON engine not initialized"));
}

}
}
