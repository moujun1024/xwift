#pragma once

#include "xwift/Plugin/Plugin.h"
#include "xwift/stdlib/JSON/JSON.h"
#include <memory>

namespace xwift {
namespace json {

class IJSONEngine {
public:
  virtual ~IJSONEngine() = default;
  
  virtual JSONValue parse(const std::string& jsonStr) = 0;
  virtual std::string stringify(const JSONValue& value) = 0;
  
  virtual std::string getName() const = 0;
  virtual std::string getVersion() const = 0;
};

class JSONPlugin : public plugin::Plugin {
public:
  bool initialize() override;
  void shutdown() override;
  plugin::PluginInfo getInfo() const override;
  
  std::shared_ptr<IJSONEngine> getEngine();
  
private:
  std::shared_ptr<IJSONEngine> engine;
};

}
}
