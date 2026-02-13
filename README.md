# XWift

XWift 是一款高性能编程语言编译器框架，采用与 Swift 类似的语法结构，为开发者提供跨平台的编程体验。

## 核心理念

- **Swift 兼容语法**: 采用与 Apple Swift 相似的语法结构，让 Swift 开发者能够无缝迁移
- **全平台支持**: 支持 Windows、Mac、Linux 等主流操作系统，真正的跨平台体验
- **去中心化**: 不依赖特定厂商或平台，人人都可以参与开发和定制
- **高性能**: 采用 C/C++ 实现，提供卓越的编译性能和运行时效率

## 为谁而来

- ✅ 熟悉 Apple Swift 开发的开发者
- ✅ 寻求跨平台编程方案的团队
- ✅ 希望掌控自己编程语言工具的爱好者
- ✅ 需要定制化编译器的企业用户

## 构建

### 环境要求

- CMake 3.20 或更高版本
- C++20 兼容编译器 (Clang, GCC, MSVC)

### 构建步骤

```bash
mkdir build
cd build
cmake ..
cmake --build . --config Release
```

### 运行测试

```bash
cmake --build . --config Release
ctest
```

## 项目结构

```
xwift/
├── include/xwift/     # 公共头文件
├── lib/               # 编译器核心库
│   ├── Basic/         # 基础工具
│   ├── AST/           # 抽象语法树
│   ├── Parser/        # 解析器
│   ├── Sema/          # 语义分析
│   ├── CodeGen/       # 代码生成
│   └── Frontend/      # 前端驱动
├── tools/             # 编译器驱动
├── utils/             # 工具脚本
├── cmake/             # CMake 模块
├── docs/              # 文档
└── test/              # 测试套件
```

## 使用方法

```bash
# 查看版本
xwift --version

# 查看帮助
xwift --help

# 编译文件
xwift input.xw
```

## 开源协议

MIT License - 详见 [LICENSE](LICENSE) 文件。

## 参与贡献

欢迎提交 Issue 和 Pull Request！
