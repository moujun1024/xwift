# XWift 跨平台支持文档

## 概述

XWift 编译器现已支持全平台编译和运行，包括 Windows、Linux 和 macOS。本文档描述了跨平台实现的细节。

## 支持的平台

- **Windows**: x86_64-pc-windows-msvc
- **Linux**: x86_64-unknown-linux-gnu
- **macOS**: x86_64-apple-darwin

## 架构设计

### 1. 平台抽象层

XWift 使用条件编译来适配不同平台：

```cpp
#ifdef _WIN32
  // Windows 特定代码
#elif __linux__
  // Linux 特定代码
#elif __APPLE__
  // macOS 特定代码
#endif
```

### 2. HTTP 后端实现

#### Windows 平台
- **实现**: `Win32HTTPBackend`
- **依赖**: WinHTTP API
- **文件**: `lib/stdlib/HTTP/Win32Backend.cpp`

#### Linux/macOS 平台
- **实现**: `CurlHTTPBackend`
- **依赖**: libcurl
- **文件**: `lib/stdlib/HTTP/CurlBackend.cpp`

### 3. HTTP 插件自动选择

`HTTPPlugin` 根据编译平台自动选择合适的后端：

```cpp
bool HTTPPlugin::initialize() {
#ifdef _WIN32
  backend = std::make_shared<Win32HTTPBackend>();
#else
  backend = std::make_shared<CurlHTTPBackend>();
#endif
  return backend != nullptr;
}
```

## 构建系统

### CMake 配置

#### Windows
```cmake
if(WIN32)
  set(HTTP_SOURCES
    HTTP/Win32Backend.cpp
  )
  target_link_libraries(XWiftHTTP winhttp)
endif()
```

#### Linux/macOS
```cmake
else()
  set(HTTP_SOURCES
    HTTP/CurlBackend.cpp
  )
  find_package(CURL REQUIRED)
  target_link_libraries(XWiftHTTP ${CURL_LIBRARIES})
endif()
```

### 依赖管理

#### Windows
- WinHTTP (系统自带)

#### Linux/macOS
- libcurl (需要安装)

#### 安装 libcurl

**Ubuntu/Debian:**
```bash
sudo apt-get install libcurl4-openssl-dev
```

**macOS:**
```bash
brew install curl
```

**Arch Linux:**
```bash
sudo pacman -S curl
```

## 编译指南

### Windows

```bash
mkdir build
cd build
cmake .. -G "Visual Studio 17 2022" -A x64
cmake --build . --config Release
```

### Linux

```bash
mkdir build
cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
make -j$(nproc)
```

### macOS

```bash
mkdir build
cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
make -j$(sysctl -n hw.ncpu)
```

## 平台特定功能

### Windows

#### 控制台编码设置
```cpp
void setConsoleEncoding() {
#ifdef _WIN32
  SetConsoleOutputCP(CP_UTF8);
  SetConsoleCP(CP_UTF8);
#endif
}
```

#### 动态库加载
- 使用 `LoadLibrary` / `FreeLibrary`
- 后缀: `.dll`

### Linux/macOS

#### 动态库加载
- 使用 `dlopen` / `dlclose`
- 后缀: `.so` (Linux) / `.dylib` (macOS)

## 测试

### 单元测试

```bash
# 运行所有测试
ctest

# 运行特定测试
ctest -R HTTPTest
```

### 集成测试

```bash
# 测试 HTTP 功能
xwift test/test_http.xw

# 测试 JSON 功能
xwift test/test_json.xw
```

## 故障排除

### Linux/macOS: 找不到 libcurl

**错误信息:**
```
Could not find CURL
```

**解决方案:**
```bash
# Ubuntu/Debian
sudo apt-get install libcurl4-openssl-dev

# macOS
brew install curl
```

### Windows: 找不到 WinHTTP

WinHTTP 是 Windows 系统自带库，通常不会出现此问题。如果出现，请确保：

1. 使用的是 Windows Vista 或更高版本
2. 正确链接了 `winhttp.lib`

### 动态库加载失败

**错误信息:**
```
Failed to load plugin: libhttp_plugin.dll
```

**解决方案:**
1. 确保动态库文件在可执行文件同目录
2. 检查动态库的依赖项是否都已安装
3. 使用 `ldd` (Linux) 或 `Dependency Walker` (Windows) 检查依赖

## 贡献指南

### 添加新平台支持

1. 在 `lib/stdlib/HTTP/` 下创建新的后端实现
2. 在 `lib/stdlib/HTTP/HTTPPlugin.cpp` 中添加平台检测
3. 在 `lib/stdlib/CMakeLists.txt` 中添加构建配置
4. 在 `lib/Plugin/CMakeLists.txt` 中添加插件构建配置
5. 更新本文档

### 代码规范

- 使用 `#ifdef _WIN32` 检测 Windows
- 使用 `#elif __linux__` 检测 Linux
- 使用 `#elif __APPLE__` 检测 macOS
- 避免使用平台特定的 API，尽可能使用标准库
- 必须使用平台特定 API 时，用条件编译包裹

## 未来计划

1. **支持 ARM 架构**
   - Windows ARM64
   - Linux ARM64
   - macOS Apple Silicon (M1/M2)

2. **更多平台**
   - FreeBSD
   - OpenBSD
   - Android

3. **性能优化**
   - 使用平台特定的优化
   - 减少条件编译的性能开销

4. **测试覆盖**
   - 在 CI/CD 中添加多平台测试
   - 自动化跨平台构建
