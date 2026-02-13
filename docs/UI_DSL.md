# XWift UI DSL - 声明式用户界面

## 概述

XWift UI 提供类似 SwiftUI 的声明式 UI 编写方式，让你用简洁的代码创建美观的桌面应用程序。

## 基础组件

### 1. Text - 文本组件

```swift
import UI

// 简单文本
Text("Hello, XWift!")

// 带样式的文本
Text("Welcome to XWift")
    .foregroundColor(.Blue)
    .fontSize(.Large)
    .alignment(.Center)

// 多行文本
Text("This is a long text\nthat spans multiple lines")
    .alignment(.Leading)
```

### 2. Button - 按钮组件

```swift
import UI

// 简单按钮
Button("Click Me") {
    print("Button clicked!")
}

// 带样式的按钮
Button("Submit")
    .backgroundColor(.Blue)
    .foregroundColor(.White)
    .onAction {
        print("Form submitted!")
    }
```

### 3. TextField - 文本输入框

```swift
import UI

// 简单输入框
TextField("Enter your name")

// 带占位符的输入框
TextField("Password")
    .placeholder("Enter your password")

// 获取输入内容
let textField = TextField("Email")
let email = textField.getText()
```

## 布局容器

### 1. VStack - 垂直堆叠

```swift
import UI

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

### 2. HStack - 水平堆叠

```swift
import UI

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
import UI

ZStack {
    Text("Background")
        .fontSize(.ExtraLarge)
        .foregroundColor(.LightGray)
    
    Text("Foreground")
        .fontSize(.Medium)
        .foregroundColor(.Black)
}
```

## 样式修饰符

### 颜色

```swift
Text("Colored Text")
    .foregroundColor(.Red)

Text("Custom Color")
    .foregroundColor(Color(255, 128, 0))
```

### 内边距和外边距

```swift
VStack {
    Text("Content")
}
.padding(20)           // 所有边
.padding(10, 20)       // 水平，垂直
.padding(5, 10, 15, 20) // 上，左，下，右
.margin(10)
```

### 字体大小

```swift
Text("Small Text")
    .fontSize(.Small)

Text("Medium Text")
    .fontSize(.Medium)

Text("Large Text")
    .fontSize(.Large)

Text("Extra Large Text")
    .fontSize(.ExtraLarge)
```

### 对齐方式

```swift
Text("Left Aligned")
    .alignment(.Leading)

Text("Center Aligned")
    .alignment(.Center)

Text("Right Aligned")
    .alignment(.Trailing)
```

## 窗口管理

### 创建窗口

```swift
import UI

let window = Window()
window.setTitle("My App")
window.setSize(Size(800, 600))
window.setPosition(Point(100, 100))
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
import UI

let app = Application()
app.setMainWindow(window)
app.run()
```

## 完整示例

### 示例 1: 登录界面

```swift
import UI

let window = Window()
window.setTitle("Login")
window.setSize(Size(400, 300))

let content = VStack(spacing: 15) {
    Text("Welcome Back")
        .fontSize(.ExtraLarge)
        .alignment(.Center)
    
    TextField("Username")
        .placeholder("Enter your username")
    
    TextField("Password")
        .placeholder("Enter your password")
    
    Button("Login") {
        print("Login button clicked!")
    }
    .backgroundColor(.Blue)
    
    Button("Sign Up") {
        print("Sign up button clicked!")
    }
}
.padding(30)

window.setContent(content)

let app = Application()
app.setMainWindow(window)
app.run()
```

### 示例 2: 计数器应用

```swift
import UI

var count = 0

let countLabel = Text("Count: 0")
    .fontSize(.Large)
    .alignment(.Center)

let content = VStack(spacing: 20) {
    Text("Counter App")
        .fontSize(.ExtraLarge)
    
    countLabel
    
    HStack(spacing: 10) {
        Button("-") {
            count = count - 1
            countLabel.setText("Count: " + String(count))
        }
        
        Button("+") {
            count = count + 1
            countLabel.setText("Count: " + String(count))
        }
    }
}
.padding(30)

let window = Window()
window.setTitle("Counter")
window.setSize(Size(300, 250))
window.setContent(content)

let app = Application()
app.setMainWindow(window)
app.run()
```

### 示例 3: 待办事项列表

```swift
import UI

var todos: [String] = []

let todoList = VStack(spacing: 5) {
    Text("Todo List")
        .fontSize(.Large)
}

let textField = TextField("New todo")

let content = VStack(spacing: 15) {
    Text("My Todos")
        .fontSize(.ExtraLarge)
    
    textField
    
    Button("Add Todo") {
        let text = textField.getText()
        if !text.isEmpty {
            todos.append(text)
            textField.setText("")
            print("Added: " + text)
        }
    }
    
    todoList
}
.padding(20)

let window = Window()
window.setTitle("Todo List")
window.setSize(Size(400, 500))
window.setContent(content)

let app = Application()
app.setMainWindow(window)
app.run()
```

## 动态加载机制

XWift UI 支持动态加载，只在需要时加载 UI 模块：

```swift
import UI

// 懒加载 UI
lazy var myWindow = {
    let window = Window()
    window.setTitle("Lazy Loaded Window")
    window.setSize(Size(600, 400))
    
    let content = VStack {
        Text("This window was loaded on demand!")
            .fontSize(.Large)
    }
    
    window.setContent(content)
    return window
}()

// 只在需要时显示
Button("Show Window") {
    myWindow.show()
}
```

## 事件处理

### 按钮点击事件

```swift
Button("Click Me") {
    print("Button clicked!")
    
    // 执行其他操作
    doSomething()
}
```

### 文本输入事件

```swift
let textField = TextField("Search")

textField.onTextChanged { text in
    print("Text changed: " + text)
    
    // 实时搜索
    performSearch(text)
}
```

## 状态管理

### 简单状态

```swift
var isToggled = false

let button = Button(isToggled ? "ON" : "OFF") {
    isToggled = !isToggled
    button.setTitle(isToggled ? "ON" : "OFF")
}
```

### 使用状态对象

```swift
class State {
    var count: Int = 0
    var text: String = ""
}

let state = State()

let countLabel = Text("Count: " + String(state.count))

Button("Increment") {
    state.count = state.count + 1
    countLabel.setText("Count: " + String(state.count))
}
```

## 主题和样式

### 定义主题

```swift
struct Theme {
    static let primaryColor = Color.Blue
    static let secondaryColor = Color.Gray
    static let backgroundColor = Color.White
    static let textColor = Color.Black
}
```

### 应用主题

```swift
Text("Themed Text")
    .foregroundColor(Theme.primaryColor)

Button("Themed Button") {
    print("Clicked!")
}
.backgroundColor(Theme.primaryColor)
.foregroundColor(Theme.backgroundColor)
```

## 性能优化

### 避免频繁更新

```swift
// 使用节流
var lastUpdateTime = 0

func updateUI() {
    let currentTime = timestamp()
    if currentTime - lastUpdateTime > 100 { // 100ms 节流
        updateContent()
        lastUpdateTime = currentTime
    }
}
```

### 懒加载组件

```swift
lazy var heavyComponent = {
    // 只在第一次访问时创建
    return createComplexUI()
}()
```

## 跨平台支持

XWift UI 支持多平台，自动适配：

```swift
import UI

let window = Window()
window.setTitle("Cross-Platform App")

#if os(Windows)
// Windows 特定代码
window.setSize(Size(800, 600))
#elseif os(macOS)
// macOS 特定代码
window.setSize(Size(900, 700))
#elseif os(Linux)
// Linux 特定代码
window.setSize(Size(800, 600))
#endif

let content = VStack {
    Text("This works on all platforms!")
        .fontSize(.Large)
}

window.setContent(content)

let app = Application()
app.setMainWindow(window)
app.run()
```

## 最佳实践

1. **使用声明式布局** - 优先使用 VStack、HStack 等容器
2. **保持组件简单** - 每个组件只做一件事
3. **合理使用状态** - 避免过度复杂的状态管理
4. **性能优先** - 避免频繁更新 UI
5. **响应式设计** - 考虑不同窗口大小

## 下一步

- 学习更多组件：Image、ListView、ScrollView 等
- 探索动画效果
- 了解数据绑定
- 实现自定义组件

## 相关文档

- [UI API Reference](UI_API.md)
- [Component Library](ComponentLibrary.md)
- [Theming Guide](Theming.md)
- [Performance Guide](Performance.md)
