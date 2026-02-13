#include "xwift/stdlib/JSON/JSONPlugin.h"
#include "xwift/stdlib/JSON/JSON.h"
#include "xwift/Basic/Error.h"

namespace xwift {
namespace json {

class NativeJSONEngine : public IJSONEngine {
public:
  Result<JSONValue> parse(const std::string& jsonStr) override {
    JSONParser parser;
    JSONValue result = parser.parse(jsonStr);
    if (parser.hasError()) {
      return Result<JSONValue>::err(Error::json(parser.getError()));
    }
    return Result<JSONValue>::ok(result);
  }
  
  Result<std::string> stringify(const JSONValue& value) override {
    try {
      return Result<std::string>::ok(value.toString());
    } catch (const std::exception& e) {
      return Result<std::string>::err(Error::json(e.what()));
    }
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
