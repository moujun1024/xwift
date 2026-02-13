# XWift 错误处理机制实现文档

## 概述

本文档描述了 XWift 编译器错误处理机制的实现，包括错误类型体系、Result 类型和 Optional 类型。

## 设计目标

1. **类型安全的错误处理**: 使用类型系统强制错误处理
2. **清晰的错误分类**: 区分不同类型的错误
3. **堆栈跟踪**: 提供错误发生时的调用栈信息
4. **链式错误处理**: 支持 `map` 和 `and_then` 操作

## 核心组件

### 1. Error 类型体系

#### ErrorKind 枚举

定义了 10 种错误类型：

```cpp
enum class ErrorKind {
  Syntax,      // 语法错误
  Semantic,    // 语义错误
  Runtime,     // 运行时错误
  IO,          // IO 错误
  Network,     // 网络错误
  JSON,        // JSON 解析错误
  HTTP,        // HTTP 请求错误
  Memory,      // 内存错误
  Type,        // 类型错误
  Unknown      // 未知错误
};
```

#### Error 类

```cpp
class Error {
public:
  Error(ErrorKind kind, const std::string& message);
  
  ErrorKind getKind() const;
  const std::string& getMessage() const;
  const std::vector<std::string>& getStack() const;
  
  void addStackFrame(const std::string& frame);
  std::string toString() const;
  
  // 静态工厂方法
  static Error syntax(const std::string& message);
  static Error semantic(const std::string& message);
  static Error runtime(const std::string& message);
  static Error io(const std::string& message);
  static Error network(const std::string& message);
  static Error json(const std::string& message);
  static Error http(const std::string& message);
  static Error memory(const std::string& message);
  static Error type(const std::string& message);
};
```

**特性**:
- 支持错误堆栈跟踪
- 提供友好的错误信息格式
- 静态工厂方法简化错误创建

### 2. Optional 类型

```cpp
template<typename T>
class Optional {
public:
  Optional();
  Optional(const T& value);
  Optional(std::nullopt_t);
  
  bool has_value() const;
  explicit operator bool() const;
  
  T& operator*();
  const T& operator*() const;
  
  T* operator->();
  const T* operator->() const;
  
  T value_or(const T& defaultValue) const;
  T unwrap() const;
  T unwrap_or(const T& defaultValue) const;
  
  // 函数式操作
  template<typename F>
  auto map(F&& f) -> Optional<decltype(f(value))>;
  
  template<typename F>
  auto and_then(F&& f) -> decltype(f(value));
};
```

**使用示例**:

```cpp
Optional<int> findUser(int id) {
  if (id > 0) {
    return Optional<int>(id);
  }
  return Optional<int>();
}

auto user = findUser(42);
if (user) {
  std::cout << "User found: " << *user << std::endl;
}

// 使用 map
auto name = findUser(42).map([](int id) {
  return "User-" + std::to_string(id);
});
```

### 3. Result 类型

```cpp
template<typename T>
class Result {
public:
  Result(const T& value);
  Result(Error&& error);
  
  bool is_ok() const;
  bool is_error() const;
  
  T& unwrap();
  const T& unwrap() const;
  T unwrap_or(const T& defaultValue) const;
  
  const Error& error() const;
  
  // 函数式操作
  template<typename F>
  auto map(F&& f) -> Result<decltype(f(std::get<T>(data)))>;
  
  template<typename F>
  auto and_then(F&& f) -> decltype(f(std::get<T>(data)));
  
  static Result<T> ok(const T& value);
  static Result<T> err(const Error& error);
  static Result<T> err(const std::string& message);
};
```

**使用示例**:

```cpp
Result<Response> fetchData(const std::string& url) {
  HTTPClient client;
  return client.get(url);
}

auto result = fetchData("https://api.example.com/data");
if (result.is_ok()) {
  auto response = result.unwrap();
  std::cout << "Status: " << response.statusCode << std::endl;
} else {
  std::cerr << result.error().toString() << std::endl;
}

// 链式调用
auto data = fetchData(url)
  .map([](Response r) { return r.body; })
  .and_then([](std::string body) { return parseJSON(body); });
```

## HTTP 后端错误处理

### Win32HTTPBackend

所有 HTTP 方法现在返回 `Result<Response>`：

```cpp
Result<Response> get(const std::string& url) override;
Result<Response> post(const std::string& url, const std::string& data) override;
Result<Response> put(const std::string& url, const std::string& data) override;
Result<Response> deleteRequest(const std::string& url) override;
```

**错误转换**:

| WinHTTP 错误 | XWift 错误 |
|--------------|-------------|
| 连接失败 | Error::network("Failed to connect to server") |
| 无效 URL | Error::network("Invalid URL") |
| 请求失败 | Error::network("Failed to send HTTP request") |
| 响应失败 | Error::network("Failed to receive HTTP response") |

### CurlHTTPBackend

所有 HTTP 方法返回 `Result<Response>`：

```cpp
Result<Response> get(const std::string& url) override;
Result<Response> post(const std::string& url, const std::string& data) override;
Result<Response> put(const std::string& url, const std::string& data) override;
Result<Response> deleteRequest(const std::string& url) override;
```

**错误转换**:

| CURL 错误 | XWift 错误 |
|------------|-------------|
| CURLE_OPERATION_TIMEDOUT | Error::network("Request timeout") |
| CURLE_URL_MALFORMAT | Error::network("Invalid URL") |
| CURLE_SSL_CONNECT_ERROR | Error::network("SSL connection failed") |
| 其他错误 | Error::network("Request failed") |

## JSON 引擎错误处理

### IJSONEngine 接口

```cpp
class IJSONEngine {
public:
  virtual Result<JSONValue> parse(const std::string& jsonStr) = 0;
  virtual Result<std::string> stringify(const JSONValue& value) = 0;
};
```

### NativeJSONEngine 实现

**解析错误**:

```cpp
Result<JSONValue> parse(const std::string& jsonStr) override {
  JSONParser parser;
  JSONValue result = parser.parse(jsonStr);
  if (parser.hasError()) {
    return Result<JSONValue>::err(Error::json(parser.getError()));
  }
  return Result<JSONValue>::ok(result);
}
```

**序列化错误**:

```cpp
Result<std::string> stringify(const JSONValue& value) override {
  try {
    return Result<std::string>::ok(value.toString());
  } catch (const std::exception& e) {
    return Result<std::string>::err(Error::json(e.what()));
  }
}
```

## 错误信息格式

### 基本格式

```
[ErrorKind] Error message
```

### 带堆栈跟踪的格式

```
[ErrorKind] Error message
Stack trace:
  at function1()
  at function2()
  at function3()
```

### 示例

```
[Network Error] Failed to connect to server
Stack trace:
  at HTTPClient::get()
  at fetchData()
  at main()
```

## 优势

1. **类型安全**: 编译时强制错误处理，避免未处理的错误
2. **函数式编程**: 支持 `map` 和 `and_then` 操作，便于链式调用
3. **清晰的错误分类**: 区分不同类型的错误，便于调试
4. **堆栈跟踪**: 提供错误发生时的调用栈，快速定位问题
5. **零成本抽象**: Optional 和 Result 在优化后无运行时开销

## 使用建议

### 1. 优先使用 Result 而非异常

```cpp
// 推荐
Result<int> parseNumber(const std::string& str);

// 不推荐
int parseNumber(const std::string& str); // 可能抛出异常
```

### 2. 使用链式操作简化代码

```cpp
// 推荐
auto result = fetch(url)
  .map([](Response r) { return r.body; })
  .and_then([](std::string body) { return parseJSON(body); });

// 不推荐
auto response = fetch(url);
if (response.is_ok()) {
  auto body = response.unwrap().body;
  auto json = parseJSON(body);
  if (json.is_ok()) {
    // ...
  }
}
```

### 3. 为关键操作添加堆栈帧

```cpp
Error error = Error::network("Connection failed");
error.addStackFrame("HTTPClient::get()");
error.addStackFrame("fetchData()");
throw ErrorException(error);
```

## 未来计划

1. **实现 do-catch 语法**: 在语言层面支持错误捕获
2. **实现 throws/try 语法**: 在函数签名中声明可能抛出的错误
3. **错误上下文**: 添加更多错误上下文信息（如文件名、行号）
4. **错误恢复**: 支持错误恢复机制
5. **错误日志**: 集成日志系统，记录所有错误

## 相关文档

- [PluginSystem.md](PluginSystem.md) - 插件系统文档
- [CrossPlatform.md](CrossPlatform.md) - 跨平台支持文档
