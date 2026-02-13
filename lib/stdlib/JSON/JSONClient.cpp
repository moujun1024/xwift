#include "xwift/stdlib/JSON/JSONClient.h"
#include "xwift/stdlib/JSON/JSONPlugin.h"
#include "xwift/Plugin/Plugin.h"

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

JSONValue JSONClient::parse(const std::string& jsonStr) {
  if (engine) {
    return engine->parse(jsonStr);
  }
  return JSONValue();
}

std::string JSONClient::stringify(const JSONValue& value) {
  if (engine) {
    return engine->stringify(value);
  }
  return "";
}

}
}
