#include "xwift/Plugin/Plugin.h"
#ifdef _WIN32
#include <windows.h>
#else
#include <dlfcn.h>
#endif

namespace xwift {
namespace plugin {

PluginManager& PluginManager::getInstance() {
  static PluginManager instance;
  return instance;
}

bool PluginManager::loadPlugin(const std::string& path) {
#ifdef _WIN32
  HMODULE handle = LoadLibraryA(path.c_str());
  if (!handle) {
    return false;
  }
  
  typedef Plugin* (*CreatePluginFunc)();
  CreatePluginFunc createFunc = (CreatePluginFunc)GetProcAddress(handle, "createPlugin");
  if (!createFunc) {
    FreeLibrary(handle);
    return false;
  }
  
  Plugin* plugin = createFunc();
  if (!plugin) {
    FreeLibrary(handle);
    return false;
  }
  
  if (!plugin->initialize()) {
    delete plugin;
    FreeLibrary(handle);
    return false;
  }
  
  PluginInfo info = plugin->getInfo();
  plugins[info.name] = std::unique_ptr<Plugin>(plugin);
  pluginHandles[info.name] = handle;
  
  return true;
#else
  void* handle = dlopen(path.c_str(), RTLD_LAZY);
  if (!handle) {
    return false;
  }
  
  typedef Plugin* (*CreatePluginFunc)();
  CreatePluginFunc createFunc = (CreatePluginFunc)dlsym(handle, "createPlugin");
  if (!createFunc) {
    dlclose(handle);
    return false;
  }
  
  Plugin* plugin = createFunc();
  if (!plugin) {
    dlclose(handle);
    return false;
  }
  
  if (!plugin->initialize()) {
    delete plugin;
    dlclose(handle);
    return false;
  }
  
  PluginInfo info = plugin->getInfo();
  plugins[info.name] = std::unique_ptr<Plugin>(plugin);
  pluginHandles[info.name] = handle;
  
  return true;
#endif
}

bool PluginManager::unloadPlugin(const std::string& name) {
  auto it = plugins.find(name);
  if (it == plugins.end()) {
    return false;
  }
  
  it->second->shutdown();
  it->second.reset();
  
  auto handleIt = pluginHandles.find(name);
  if (handleIt != pluginHandles.end()) {
#ifdef _WIN32
    FreeLibrary((HMODULE)handleIt->second);
#else
    dlclose(handleIt->second);
#endif
    pluginHandles.erase(handleIt);
  }
  
  plugins.erase(it);
  return true;
}

Plugin* PluginManager::getPlugin(const std::string& name) {
  auto it = plugins.find(name);
  if (it != plugins.end()) {
    return it->second.get();
  }
  return nullptr;
}

std::vector<PluginInfo> PluginManager::listPlugins() const {
  std::vector<PluginInfo> result;
  for (const auto& pair : plugins) {
    result.push_back(pair.second->getInfo());
  }
  return result;
}

}
}
