#include "xwift/stdlib/JSON/JSONPlugin.h"
#include "xwift/stdlib/JSON/JSON.h"

namespace xwift {
namespace json {

class NativeJSONEngine : public IJSONEngine {
public:
  JSONValue parse(const std::string& jsonStr) override {
    JSONParser parser;
    return parser.parse(jsonStr);
  }
  
  std::string stringify(const JSONValue& value) override {
    return value.toString();
  }
  
  std::string getName() const override {
    return "Native";
  }
  
  std::string getVersion() const override {
    return "1.0.0";
  }
};

bool JSONPlugin::initialize() {
  engine = std::make_shared<NativeJSONEngine>();
  return engine != nullptr;
}

void JSONPlugin::shutdown() {
  engine.reset();
}

plugin::PluginInfo JSONPlugin::getInfo() const {
  return {
    "JSON",
    "1.0.0",
    "JSON parser plugin for XWift",
    "XWift Team"
  };
}

std::shared_ptr<IJSONEngine> JSONPlugin::getEngine() {
  return engine;
}

}
}

extern "C" {
  xwift::json::JSONPlugin* createPlugin() {
    return new xwift::json::JSONPlugin();
  }
  
  void destroyPlugin(xwift::json::JSONPlugin* plugin) {
    delete plugin;
  }
}
