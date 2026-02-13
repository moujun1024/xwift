# XWift 多语言互操作 (FFI) 系统

## 概述

XWift FFI (Foreign Function Interface) 系统允许 XWift 代码与其他编程语言（如 Python、Objective-C、JavaScript、Rust、Go 等）进行无缝互操作。这使得开发者可以利用各语言的优势，构建更强大、更高效的应用程序。

## 核心特性

### 1. 多语言支持
- ✅ **Python**: 完整的 Python 解释器集成
- ✅ **Objective-C**: macOS/iOS 原生 API 调用
- 🚧 **JavaScript**: Node.js 运行时集成（开发中）
- 🚧 **Rust**: Rust 函数调用（开发中）
- 🚧 **Go**: Go 函数调用（开发中）
- ✅ **C**: 原生 C 函数调用

### 2. 类型系统
XWift FFI 提供了完整的类型映射系统：

| XWift 类型 | Python 类型 | Objective-C 类型 | C 类型 |
|-----------|-------------|------------------|--------|
| Int | int | NSInteger | int64_t |
| Float | float | double | double |
| Bool | bool | BOOL | bool |
| String | str | NSString | char* |
| Array | list | NSArray | void* |
| Object | object | id | void* |

### 3. 插件架构
基于现有的插件系统，每个语言实现都是一个独立的插件：
- `PythonFFIPlugin`: Python 互操作插件
- `ObjCFFIPlugin`: Objective-C 互操作插件
- 未来可扩展更多语言插件

## 使用方法

### Python 互操作

#### 基本用法

```swift
import Python

func main() {
    let python = PythonInterpreter()
    python.initialize()
    
    python.execute("""
def greet(name):
    return f"Hello, {name}!"
""")
    
    let result = python.callFunction("greet", args: ["XWift"])
    print(result)
    
    python.shutdown()
}

main()
```

#### 数据交换

```swift
import Python

func main() {
    let python = PythonInterpreter()
    python.initialize()
    
    python.execute("""
import numpy as np

def process_data(data):
    arr = np.array(data)
    return np.mean(arr)
""")
    
    let data = [1, 2, 3, 4, 5]
    python.setVariable("input_data", value: data)
    
    let mean = python.callFunction("process_data", args: [data])
    print("Mean: \(mean)")
    
    python.shutdown()
}

main()
```

### Objective-C 互操作

#### 基本用法

```swift
import ObjectiveC

func main() {
    let nsStringClass = ObjCClass(name: "NSString")
    
    if !nsStringClass.isValid() {
        print("NSString class not found")
        return
    }
    
    let str = nsStringClass.callClassMethod("stringWithString:", args: ["Hello from XWift!"])
    print(str)
}

main()
```

#### 创建实例和调用方法

```swift
import ObjectiveC

func main() {
    let dateFormatterClass = ObjCClass(name: "NSDateFormatter")
    let formatter = dateFormatterClass.createInstance([])
    
    formatter.callMethod("setDateFormat:", args: ["yyyy-MM-dd HH:mm:ss"])
    
    let dateClass = ObjCClass(name: "NSDate")
    let date = dateClass.callClassMethod("date", args: [])
    
    let dateString = formatter.callMethod("stringFromDate:", args: [date])
    print("Current date: \(dateString)")
}

main()
```

### 多语言混合编程

```swift
import Python
import ObjectiveC
import Terminal

func main() {
    clearScreen()
    
    let python = PythonInterpreter()
    python.initialize()
    
    python.execute("""
import numpy as np

def calculate_statistics(data):
    return {
        'mean': float(np.mean(data)),
        'std': float(np.std(data))
    }
""")
    
    let data = [1, 2, 3, 4, 5, 6, 7, 8, 9, 10]
    let stats = python.callFunction("calculate_statistics", args: [data])
    
    print("Statistics (Python NumPy):")
    print("  Mean: \(stats['mean'])")
    print("  Std: \(stats['std'])")
    
    let nsStringClass = ObjCClass(name: "NSString")
    let message = nsStringClass.callClassMethod("stringWithFormat:", 
                                                  args: ["计算完成！共 %d 个数据点", data.count])
    print("\(message)")
    
    python.shutdown()
}

main()
```

## 构建配置

### 启用 Python 支持

```cmake
cmake .. -DXWIFT_ENABLE_PYTHON=ON
```

### 启用 Objective-C 支持（仅 macOS）

```cmake
cmake .. -DXWIFT_ENABLE_OBJC=ON
```

### 完整配置

```cmake
cmake .. -DXWIFT_ENABLE_PYTHON=ON -DXWIFT_ENABLE_OBJC=ON
```

## API 参考

### PythonInterpreter

```swift
class PythonInterpreter {
    func initialize() -> Bool
    func shutdown() -> Void
    func execute(_ code: String) -> Bool
    func executeFile(_ filename: String) -> Bool
    func callFunction(_ name: String, args: [ForeignValue]) -> ForeignValue
    func setVariable(_ name: String, value: ForeignValue) -> Bool
    func getVariable(_ name: String) -> ForeignValue
    func hasFunction(_ name: String) -> Bool
    func listFunctions() -> [String]
}
```

### ObjCClass

```swift
class ObjCClass {
    init(name: String)
    func isValid() -> Bool
    func hasMethod(_ selector: String) -> Bool
    func hasClassMethod(_ selector: String) -> Bool
    func callMethod(_ instance: id, _ selector: String, args: [ForeignValue]) -> ForeignValue
    func callClassMethod(_ selector: String, args: [ForeignValue]) -> ForeignValue
    func createInstance(_ args: [ForeignValue]) -> id
}
```

### ObjCInstance

```swift
class ObjCInstance {
    init(_ obj: id, _ cls: ObjCClass)
    func callMethod(_ selector: String, args: [ForeignValue]) -> ForeignValue
    func getProperty(_ name: String) -> ForeignValue
    func setProperty(_ name: String, value: ForeignValue) -> Bool
}
```

## 性能考虑

### 1. 初始化开销
- Python 解释器初始化: ~50-100ms
- Objective-C 运行时初始化: ~10-20ms

**建议**: 在应用启动时初始化，避免重复初始化

### 2. 数据转换开销
- 简单类型（Int, Float, Bool）: ~1-5μs
- 字符串: ~10-50μs
- 数组: ~50-500μs（取决于大小）

**建议**: 批量传递数据，减少跨语言调用次数

### 3. 调用开销
- Python 函数调用: ~10-100μs
- Objective-C 方法调用: ~1-10μs

**建议**: 在性能关键路径上使用 Objective-C，复杂计算使用 Python

## 最佳实践

### 1. 语言选择指南

| 场景 | 推荐语言 | 原因 |
|------|---------|------|
| 主逻辑、流程控制 | XWift | 类型安全、编译时检查 |
| 数据处理、科学计算 | Python | 丰富的生态系统（NumPy, Pandas） |
| macOS/iOS 原生功能 | Objective-C | 原生 API 支持 |
| 高性能计算 | Rust | 零成本抽象、内存安全 |
| 网络服务 | Go | 并发模型优秀 |

### 2. 错误处理

```swift
import Python

func main() {
    let python = PythonInterpreter()
    
    if !python.initialize() {
        print("Failed to initialize Python")
        return
    }
    
    defer { python.shutdown() }
    
    let result = python.callFunction("some_function", args: [])
    
    if result.isNil() {
        print("Function call failed")
        return
    }
    
    print("Result: \(result)")
}
```

### 3. 资源管理

```swift
import Python

func processData() {
    let python = PythonInterpreter()
    python.initialize()
    defer { python.shutdown() }
    
    python.execute("""
def process(data):
    return [x * 2 for x in data]
""")
    
    let data = [1, 2, 3, 4, 5]
    let result = python.callFunction("process", args: [data])
    
    print("Result: \(result)")
}
```

## 示例程序

### 1. test_python_ffi.xw
演示 Python 互操作，包括函数调用、数据交换、算法实现

### 2. test_objc_ffi.xw
演示 Objective-C 互操作，包括类方法调用、实例创建、属性访问

### 3. test_multilang.xw
演示多语言混合编程，结合 Python、Objective-C 和 XWift 的优势

## 未来计划

### 短期（v0.2.0）
- [ ] 完善 JavaScript 互操作
- [ ] 添加 Rust 互操作
- [ ] 改进错误处理和诊断信息

### 中期（v0.3.0）
- [ ] 添加 Go 互操作
- [ ] 实现异步调用支持
- [ ] 添加性能分析工具

### 长期（v1.0.0）
- [ ] 支持更多语言（C#, Java, Kotlin）
- [ ] 自动类型推断和转换
- [ ] 跨语言调试支持

## 注意事项

1. **平台限制**: Objective-C 互操作仅支持 macOS 和 iOS 平台
2. **线程安全**: Python 解释器不是线程安全的，需要在主线程调用
3. **内存管理**: 跨语言传递对象时注意生命周期管理
4. **性能**: 频繁的跨语言调用会影响性能，合理规划调用策略

## 贡献

欢迎贡献新的语言支持！请参考现有实现（PythonFFI, ObjCFFI）添加新的语言插件。

## 许可证

MIT License
