# XWift 动态加载系统重构文档

## 概述

本文档描述了 XWift 编译器项目从硬编码实现到动态加载插件系统的重构过程。

## 重构目标

1. 将硬编码的 HTTP 和 JSON 实现改为动态加载的插件
2. 创建通用的插件管理系统
3. 提高代码的可扩展性和可维护性
4. 支持用户自定义标准库实现

## 架构设计

### 1. 插件系统核心

#### Plugin 基类
- 位置: `include/xwift/Plugin/Plugin.h`
- 功能: 定义插件接口
- 实现: `lib/Plugin/Plugin.cpp`

#### PluginManager
- 功能: 管理插件的加载、卸载和查询
- 支持跨平台动态库加载（Windows DLL / Linux SO）
- 单例模式实现

### 2. HTTP 插件

#### IHTTPBackend 接口
- 位置: `include/xwift/stdlib/HTTP/HTTPPlugin.h`
- 功能: 定义 HTTP 后端接口

#### HTTPPlugin
- 实现: `lib/stdlib/HTTP/HTTPPlugin.cpp`
- 功能: HTTP 插件包装器

#### Win32HTTPBackend
- 实现: `lib/stdlib/HTTP/Win32Backend.cpp`
- 功能: Windows 平台的 HTTP 实现

#### HTTPClient
- 位置: `include/xwift/stdlib/HTTP/HTTP.h`
- 实现: `lib/stdlib/HTTP/HTTPClient.cpp`
- 功能: HTTP 客户端，通过插件管理器加载插件

### 3. JSON 插件

#### IJSONEngine 接口
- 位置: `include/xwift/stdlib/JSON/JSONPlugin.h`
- 功能: 定义 JSON 引擎接口

#### JSONPlugin
- 实现: `lib/stdlib/JSON/JSONPlugin.cpp`
- 功能: JSON 插件包装器

#### NativeJSONEngine
- 实现: `lib/stdlib/JSON/JSON.cpp`
- 功能: 原生 JSON 解析实现

#### JSONClient
- 位置: `include/xwift/stdlib/JSON/JSONClient.h`
- 实现: `lib/stdlib/JSON/JSONClient.cpp`
- 功能: JSON 客户端，通过插件管理器加载插件

## 构建系统更新

### CMakeLists.txt 更新

1. 添加 `BUILD_PLUGINS` 选项
2. 创建 `lib/Plugin` 子目录
3. 更新 `lib/stdlib` 构建配置
4. 更新 `tools/xwift` 链接配置

### 动态库配置

- Windows: `.dll` 后缀
- Linux/macOS: `.so` 后缀
- 使用 `XWIFT_PLATFORM_SUFFIX` 宏自动适配

## 使用方式

### 加载插件

```cpp
#include "xwift/Plugin/Plugin.h"

auto& pluginManager = xwift::plugin::PluginManager::getInstance();
pluginManager.loadPlugin("libhttp_plugin.dll");
```

### 使用 HTTP 客户端

```cpp
#include "xwift/stdlib/HTTP/HTTP.h"

xwift::http::HTTPClient client;
auto response = client.get("https://api.example.com/data");
```

### 使用 JSON 客户端

```cpp
#include "xwift/stdlib/JSON/JSONClient.h"

xwift::json::JSONClient jsonClient;
auto value = jsonClient.parse("{\"key\": \"value\"}");
```

## 优势

1. **模块化**: 每个功能都是独立的插件
2. **可扩展**: 用户可以轻松添加新的插件
3. **可替换**: 可以替换不同的实现（如使用 nlohmann/json 替换原生实现）
4. **跨平台**: 自动适配不同平台的动态库格式
5. **向后兼容**: 保留了原有的 API，不影响现有代码

## 后续计划

1. 实现 CurlHTTPBackend 支持 Linux/macOS
2. 实现 NlohmannJSONEngine 使用 nlohmann/json 库
3. 添加更多标准库插件（文件系统、网络、加密等）
4. 实现插件配置系统
5. 添加插件热加载支持

## 注意事项

1. 插件必须实现 `createPlugin()` 和 `destroyPlugin()` 导出函数
2. 插件必须继承 `Plugin` 基类并实现所有虚函数
3. 插件动态库必须放在可执行文件同目录或系统库路径中
4. 插件加载失败时会回退到默认实现（如果有）
