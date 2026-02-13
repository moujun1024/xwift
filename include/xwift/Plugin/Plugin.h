#pragma once

#include <string>
#include <map>
#include <functional>
#include <memory>
#include <vector>

namespace xwift {
namespace plugin {

struct PluginInfo {
  std::string name;
  std::string version;
  std::string description;
  std::string author;
};

class Plugin {
public:
  virtual ~Plugin() = default;
  
  virtual bool initialize() = 0;
  virtual void shutdown() = 0;
  virtual PluginInfo getInfo() const = 0;
};

class PluginManager {
public:
  static PluginManager& getInstance();
  
  bool loadPlugin(const std::string& path);
  bool unloadPlugin(const std::string& name);
  Plugin* getPlugin(const std::string& name);
  std::vector<PluginInfo> listPlugins() const;
  
private:
  PluginManager() = default;
  ~PluginManager() = default;
  
  std::map<std::string, std::unique_ptr<Plugin>> plugins;
  std::map<std::string, void*> pluginHandles;
};

}
}
