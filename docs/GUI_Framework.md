# XWift GUI 框架 - 完整文档

## 概述

XWift GUI 框架是一个现代化的、声明式的用户界面库，采用 **XWift + C++ 混合开发**模式，提供类似 SwiftUI 的开发体验。

### 核心特性

- ✅ **声明式 UI** - 类似 SwiftUI 的简洁语法
- ✅ **跨平台支持** - Windows、macOS、Linux
- ✅ **动态加载** - 按需加载 UI 模块
- ✅ **高性能** - C++ 后端，XWift 前端
- ✅ **易于使用** - 直观的 API 设计
- ✅ **可扩展** - 支持自定义组件

## 架构设计

### 分层架构

```
┌─────────────────────────────────────┐
│   XWift UI DSL (声明式前端)         │
│   - Text, Button, TextField        │
│   - VStack, HStack, ZStack         │
│   - 修饰符系统                      │
└─────────────────────────────────────┘
              ↓
┌─────────────────────────────────────┐
│   C++ UI Backend (原生后端)        │
│   - Windows API                    │
│   - Cocoa (macOS)                   │
│   - GTK/Linux                       │
└─────────────────────────────────────┘
              ↓
┌─────────────────────────────────────┐
│   操作系统原生窗口系统              │
└─────────────────────────────────────┘
```

### 模块组织

```
include/xwift/UI/
├── UI.h              # C++ UI 后端头文件
└── ...

lib/UI/
├── UI.cpp            # C++ UI 后端实现
└── CMakeLists.txt

lib/stdlib/UI/
└── ui_dsl.xw         # XWift UI DSL 前端

test/
└── ui_examples.xw    # UI 示例程序

docs/
└── UI_DSL.md         # UI DSL 文档
```

## 快速开始

### 1. Hello World

```swift
import UI

let window = Window()
window.setTitle("Hello World")
window.setSize(width: 400, height: 300)

let content = VStack(spacing: 20) {
    Text("Hello, XWift!")
        .fontSize(.ExtraLarge)
        .foregroundColor(.Blue)
        .alignment(.Center)
    
    Text("Welcome to XWift UI")
        .fontSize(.Medium)
        .alignment(.Center)
}
.padding(30)

window.setContent(content)

let app = Application.shared
app.setMainWindow(window)
app.run()
```

### 2. 编译和运行

```bash
# 编译项目
mkdir build && cd build
cmake .. -G "Visual Studio 17 2022" -A x64
cmake --build . --config Release

# 运行 UI 示例
./Release/xwift.exe ../test/ui_examples.xw
```

## 核心组件

### 1. Text - 文本组件

```swift
// 基础用法
Text("Hello, World!")

// 带样式
Text("Styled Text")
    .foregroundColor(.Blue)
    .fontSize(.Large)
    .alignment(.Center)
    .padding(10)
```

**属性**:
- `foregroundColor(_:)` - 设置文本颜色
- `fontSize(_:)` - 设置字体大小
- `alignment(_:)` - 设置对齐方式
- `padding(_:)` - 设置内边距
- `margin(_:)` - 设置外边距

### 2. Button - 按钮组件

```swift
// 基础用法
Button("Click Me") {
    print("Button clicked!")
}

// 带样式
Button("Submit")
    .backgroundColor(.Blue)
    .foregroundColor(.White)
    .padding(10)
    .onAction {
        print("Form submitted!")
    }
```

**属性**:
- `backgroundColor(_:)` - 设置背景颜色
- `foregroundColor(_:)` - 设置文本颜色
- `padding(_:)` - 设置内边距
- `margin(_:)` - 设置外边距
- `onAction(_:)` - 设置点击事件

### 3. TextField - 文本输入框

```swift
// 基础用法
TextField("Enter text")

// 带占位符
TextField("Password")
    .placeholder("Enter your password")

// 获取输入
let textField = TextField("Email")
let email = textField.text
```

**属性**:
- `placeholder(_:)` - 设置占位符文本
- `text` - 获取/设置文本内容
- `padding(_:)` - 设置内边距
- `margin(_:)` - 设置外边距
- `onTextChanged(_:)` - 设置文本变化事件

## 布局容器

### 1. VStack - 垂直堆叠

```swift
VStack(spacing: 10) {
    Text("Title")
        .fontSize(.Large)
    
    TextField("Enter text")
    
    Button("Submit") {
        print("Submitted!")
    }
}
.padding(20)
```

**属性**:
- `spacing(_:)` - 设置子元素间距
- `padding(_:)` - 设置内边距
- `margin(_:)` - 设置外边距

### 2. HStack - 水平堆叠

```swift
HStack(spacing: 8) {
    Text("Username:")
    
    TextField("username")
        .padding(5)
    
    Button("Login") {
        print("Login clicked!")
    }
}
.padding(10)
```

### 3. ZStack - 层叠布局

```swift
ZStack {
    Text("Background")
        .fontSize(.ExtraLarge)
        .foregroundColor(.LightGray)
    
    Text("Foreground")
        .fontSize(.Medium)
        .foregroundColor(.Black)
}
```

## 样式系统

### 颜色

```swift
// 预定义颜色
Text("Red Text").foregroundColor(.Red)
Text("Blue Text").foregroundColor(.Blue)

// 自定义颜色（未来支持）
Text("Custom Color").foregroundColor(Color(255, 128, 0))
```

**可用颜色**:
- `.Black`, `.White`
- `.Red`, `.Green`, `.Blue`
- `.Yellow`, `.Cyan`, `.Magenta`
- `.Gray`, `.LightGray`, `.DarkGray`

### 字体大小

```swift
Text("Small").fontSize(.Small)
Text("Medium").fontSize(.Medium)
Text("Large").fontSize(.Large)
Text("Extra Large").fontSize(.ExtraLarge)
```

### 对齐方式

```swift
Text("Left").alignment(.Leading)
Text("Center").alignment(.Center)
Text("Right").alignment(.Trailing)
```

### 内边距和外边距

```swift
// 所有边相同
.padding(20)
.margin(10)

// 水平和垂直
.padding(horizontal: 10, vertical: 20)

// 每个边单独设置
.padding(top: 5, left: 10, bottom: 15, right: 20)
```

## 窗口管理

### 创建窗口

```swift
let window = Window()
window.setTitle("My App")
window.setSize(width: 800, height: 600)
window.setPosition(x: 100, y: 100)
```

### 设置内容

```swift
let content = VStack {
    Text("Hello, World!")
        .fontSize(.Large)
    
    Button("Click Me") {
        print("Button clicked!")
    }
}

window.setContent(content)
```

### 显示窗口

```swift
let app = Application.shared
app.setMainWindow(window)
app.run()
```

## 动态加载机制

XWift UI 支持动态加载，只在需要时加载 UI 模块：

```swift
// 懒加载窗口
lazy var settingsWindow = {
    let window = Window()
    window.setTitle("Settings")
    window.setSize(width: 600, height: 400)
    
    let content = VStack {
        Text("Settings")
            .fontSize(.Large)
        
        Button("Close") {
            settingsWindow.hide()
        }
    }
    
    window.setContent(content)
    return window
}()

// 只在需要时显示
Button("Open Settings") {
    settingsWindow.show()
}
```

### 动态加载的优势

1. **减少内存占用** - 不需要的 UI 模块不会被加载
2. **提高启动速度** - 只加载必需的组件
3. **按需加载** - 用户交互时才加载相关模块
4. **模块化设计** - 每个 UI 模块独立加载

## 事件处理

### 按钮点击事件

```swift
Button("Click Me") {
    print("Button clicked!")
    doSomething()
}
```

### 文本输入事件

```swift
let textField = TextField("Search")

textField.onTextChanged { text in
    print("Text changed: " + text)
    performSearch(text)
}
```

### 窗口事件

```swift
let window = Window()

window.onClose {
    print("Window closing")
    cleanup()
}
```

## 状态管理

### 简单状态

```swift
var count = 0

let countLabel = Text("Count: 0")
    .fontSize(.Large)

Button("Increment") {
    count = count + 1
    countLabel.text = "Count: " + String(count)
}
```

### 状态对象

```swift
class AppState {
    var count: Int = 0
    var text: String = ""
    var isLoggedIn: Bool = false
}

let state = AppState()

let countLabel = Text("Count: " + String(state.count))

Button("Increment") {
    state.count = state.count + 1
    countLabel.text = "Count: " + String(state.count)
}
```

## 示例应用

### 1. 登录界面

```swift
import UI

let window = Window()
window.setTitle("Login")
window.setSize(width: 400, height: 350)

let content = VStack(spacing: 15) {
    Text("Welcome Back")
        .fontSize(.ExtraLarge)
        .foregroundColor(.Blue)
        .alignment(.Center)
    
    TextField("Username")
        .placeholder("Enter your username")
        .padding(5)
    
    TextField("Password")
        .placeholder("Enter your password")
        .padding(5)
    
    HStack(spacing: 10) {
        Button("Login") {
            print("Login clicked!")
        }
        .backgroundColor(.Blue)
        .foregroundColor(.White)
        
        Button("Sign Up") {
            print("Sign up clicked!")
        }
        .backgroundColor(.Green)
        .foregroundColor(.White)
    }
}
.padding(30)

window.setContent(content)

let app = Application.shared
app.setMainWindow(window)
app.run()
```

### 2. 计数器应用

```swift
import UI

var count = 0

let countLabel = Text("Count: 0")
    .fontSize(.Large)
    .alignment(.Center)

let content = VStack(spacing: 20) {
    Text("Counter App")
        .fontSize(.ExtraLarge)
        .foregroundColor(.Blue)
        .alignment(.Center)
    
    countLabel
    
    HStack(spacing: 10) {
        Button("-") {
            count = count - 1
            countLabel.text = "Count: " + String(count)
        }
        .backgroundColor(.Red)
        .foregroundColor(.White)
        
        Button("Reset") {
            count = 0
            countLabel.text = "Count: 0"
        }
        .backgroundColor(.Gray)
        .foregroundColor(.White)
        
        Button("+") {
            count = count + 1
            countLabel.text = "Count: " + String(count)
        }
        .backgroundColor(.Green)
        .foregroundColor(.White)
    }
}
.padding(30)

let window = Window()
window.setTitle("Counter")
window.setSize(width: 300, height: 250)
window.setContent(content)

let app = Application.shared
app.setMainWindow(window)
app.run()
```

### 3. 待办事项列表

```swift
import UI

var todos: [String] = []

let todoList = VStack(spacing: 5) {
    Text("Todo List")
        .fontSize(.Large)
        .foregroundColor(.Blue)
}

let textField = TextField("New todo")

let content = VStack(spacing: 15) {
    Text("My Todos")
        .fontSize(.ExtraLarge)
        .foregroundColor(.Blue)
        .alignment(.Center)
    
    textField
        .padding(5)
    
    HStack(spacing: 10) {
        Button("Add Todo") {
            let text = textField.text
            if !text.isEmpty {
                todos.append(text)
                textField.text = ""
                print("Added: " + text)
            }
        }
        .backgroundColor(.Blue)
        .foregroundColor(.White)
        
        Button("Clear All") {
            todos.removeAll()
            print("Cleared all todos")
        }
        .backgroundColor(.Red)
        .foregroundColor(.White)
    }
    
    todoList
}
.padding(20)

let window = Window()
window.setTitle("Todo List")
window.setSize(width: 400, height: 500)
window.setContent(content)

let app = Application.shared
app.setMainWindow(window)
app.run()
```

## 性能优化

### 1. 避免频繁更新

```swift
var lastUpdateTime = 0

func updateUI() {
    let currentTime = timestamp()
    if currentTime - lastUpdateTime > 100 { // 100ms 节流
        updateContent()
        lastUpdateTime = currentTime
    }
}
```

### 2. 懒加载组件

```swift
lazy var heavyComponent = {
    // 只在第一次访问时创建
    return createComplexUI()
}()
```

### 3. 重用组件

```swift
// 创建可重用的按钮
func createStyledButton(title: String, action: @escaping () -> Void) -> Button {
    return Button(title)
        .backgroundColor(.Blue)
        .foregroundColor(.White)
        .padding(10)
        .onAction(action)
}

// 使用
let button1 = createStyledButton("OK") { print("OK") }
let button2 = createStyledButton("Cancel") { print("Cancel") }
```

## 跨平台支持

### 平台特定代码

```swift
#if os(Windows)
// Windows 特定代码
window.setSize(width: 800, height: 600)
#elseif os(macOS)
// macOS 特定代码
window.setSize(width: 900, height: 700)
#elseif os(Linux)
// Linux 特定代码
window.setSize(width: 800, height: 600)
#endif
```

### 平台适配

XWift UI 自动适配不同平台：

- **Windows** - 使用 Windows API
- **macOS** - 使用 Cocoa 框架
- **Linux** - 使用 GTK

## 最佳实践

### 1. 使用声明式布局

```swift
// ✅ 推荐：使用 VStack
VStack {
    Text("Title")
    TextField("Input")
    Button("Submit")
}

// ❌ 不推荐：手动计算位置
let text = Text("Title")
text.setPosition(x: 10, y: 10)
```

### 2. 保持组件简单

```swift
// ✅ 推荐：每个组件只做一件事
struct LoginView {
    func build() -> View {
        VStack {
            TextField("Username")
            TextField("Password")
            Button("Login")
        }
    }
}

// ❌ 不推荐：一个组件做太多事情
struct ComplexView {
    func build() -> View {
        // 太多逻辑...
    }
}
```

### 3. 合理使用状态

```swift
// ✅ 推荐：使用状态对象
class AppState {
    var count: Int = 0
}

// ❌ 不推荐：全局变量
var count = 0
```

### 4. 性能优先

```swift
// ✅ 推荐：避免频繁更新
var lastUpdate = 0
if timestamp() - lastUpdate > 100 {
    updateUI()
}

// ❌ 不推荐：每次都更新
updateUI()
```

### 5. 响应式设计

```swift
// ✅ 推荐：考虑不同窗口大小
VStack {
    Text("Content")
}
.padding(window.width * 0.1)
```

## 未来计划

### 短期（v0.2.0）
- [ ] 添加 Image 组件
- [ ] 添加 ListView 组件
- [ ] 添加 ScrollView 组件
- [ ] 添加 ProgressBar 组件
- [ ] 添加 Toggle/Switch 组件

### 中期（v0.3.0）
- [ ] 添加动画效果
- [ ] 添加数据绑定
- [ ] 添加主题系统
- [ ] 添加国际化支持
- [ ] 添加无障碍支持

### 长期（v1.0.0）
- [ ] 完整的组件库
- [ ] 可视化设计工具
- [ ] 热重载支持
- [ ] 性能优化
- [ ] 完整的文档和示例

## 相关文档

- [UI DSL 文档](UI_DSL.md) - 详细的 DSL 语法说明
- [UI API 参考](UI_API.md) - API 参考手册
- [组件库](ComponentLibrary.md) - 所有组件的详细说明
- [主题指南](Theming.md) - 主题和样式系统
- [性能指南](Performance.md) - 性能优化技巧

## 贡献指南

如果你想为 XWift UI 贡献代码：

1. Fork 项目
2. 创建特性分支
3. 提交更改
4. 推送到分支
5. 创建 Pull Request

## 许可证

MIT License

## 总结

XWift GUI 框架提供了一个现代化、声明式的 UI 开发体验：

- ✅ **类似 SwiftUI** - 简洁直观的语法
- ✅ **高性能** - C++ 后端，XWift 前端
- ✅ **跨平台** - 支持 Windows、macOS、Linux
- ✅ **动态加载** - 按需加载 UI 模块
- ✅ **易于扩展** - 支持自定义组件
- ✅ **完善文档** - 详细的文档和示例

开始使用 XWift UI，创建美观的桌面应用程序吧！🚀
