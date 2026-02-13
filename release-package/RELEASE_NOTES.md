# XWift v0.1.0 - 第一个发行版

## 版本信息
- 版本号: 0.1.0
- 发布日期: 2026-02-13
- 许可证: MIT License

## 发行说明

这是 XWift 语言的第一个发行版。XWift 是一款高性能编程语言编译器框架，采用与 Swift 类似的语法结构，为开发者提供跨平台的编程体验。

## 核心特性

### 已实现的功能
- ✅ Swift 兼容语法（85%）
- ✅ 枚举、结构体、类
- ✅ 泛型类型
- ✅ 错误处理机制
- ✅ Optional 类型系统
- ✅ 终端控制编程
- ✅ HTTP 客户端
- ✅ JSON 解析和序列化
- ✅ 文件系统操作
- ✅ 单元测试框架
- ✅ 日志系统

### 标准库
- **Collections**: Array, Dictionary, Set
- **Concurrency**: Async tasks, Actor model
- **Error**: Error handling, Result type
- **Filesystem**: Path handling, file I/O
- **HTTP**: HTTP client, URL parsing, request encoding
- **JSON**: JSON parsing, serialization
- **Terminal**: Terminal control, cursor movement, colors

### 示例程序
- `hello.xw` - Hello World 示例
- `snake_game_fixed.xw` - 终端贪吃蛇游戏
- `test_http.xw` - HTTP 请求示例
- `test_optional.xw` - Optional 类型示例
- `test_terminal.xw` - 终端控制示例

## 构建说明

### 环境要求
- CMake 3.20 或更高版本
- C++20 兼容编译器 (Clang, GCC, MSVC)

### Windows 构建
```bash
mkdir build
cd build
cmake .. -G "Visual Studio 17 2022" -A x64
cmake --build . --config Release
```

### Linux/Mac 构建
```bash
mkdir build
cd build
cmake ..
make -j$(nproc)
```

## 使用方法

```bash
# 查看版本
xwift --version

# 查看帮助
xwift --help

# 编译文件
xwift input.xw

# 运行示例
xwift examples/hello.xw
xwift examples/snake_game_fixed.xw
```

## 项目结构

```
release-package/
├── src/                 # 源代码
│   ├── include/xwift/   # 公共头文件
│   ├── lib/             # 编译器核心库
│   ├── tools/           # 编译器驱动
│   ├── cmake/           # CMake 模块
│   ├── CMakeLists.txt   # 主构建文件
│   └── LICENSE          # 许可证
├── examples/            # 示例程序
│   ├── hello.xw
│   ├── snake_game_fixed.xw
│   └── ...
├── docs/                # 文档
│   ├── CrossPlatform.md
│   ├── ErrorHandling.md
│   └── PluginSystem.md
├── extensions/          # VS Code 扩展
│   └── xwift-lang/
└── README.md            # 项目说明
```

## 当前进度

- 语法解析: 90%
- 类型系统: 45%
- 标准库-终端: 70%
- 标准库-集合: 40%
- 标准库-随机: 60%
- 运行时: 35%
- **总体进度: 42%**

## 已知限制

### 未实现的功能
- ❌ if let / guard let（Sema 已实现但未使用）
- ❌ 闭包
- ❌ 协议
- ❌ 泛型方法（只有泛型类型）
- ❌ 初始化器完整性检查
- ❌ 枚举关联值
- ❌ 模块化（缺少 import 机制）

### 技术说明
- 当前版本为源码发行版，需要自行编译
- 终端函数目前为全局函数，未来将改为模块化设计
- 部分内置函数为简化实现，后续将完善

## 贡献

欢迎提交 Issue 和 Pull Request！

## 许可证

MIT License - 详见 LICENSE 文件。

## 联系方式

- GitHub: https://github.com/moujun1024/xwift
- 问题反馈: https://github.com/moujun1024/xwift/issues

---

感谢您使用 XWift！
