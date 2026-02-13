#pragma once

#include <string>
#include <functional>
#include <memory>
#include <vector>
#include <map>
#include <windows.h>

namespace XWift {
namespace UI {

enum class Color {
    Black,
    White,
    Red,
    Green,
    Blue,
    Yellow,
    Cyan,
    Magenta,
    Gray,
    LightGray,
    DarkGray,
    Custom
};

enum class Alignment {
    Leading,
    Center,
    Trailing
};

enum class FontSize {
    Small,
    Medium,
    Large,
    ExtraLarge
};

struct ColorValue {
    uint8_t r;
    uint8_t g;
    uint8_t b;
    uint8_t a;
    
    ColorValue() : r(0), g(0), b(0), a(255) {}
    ColorValue(uint8_t r, uint8_t g, uint8_t b, uint8_t a = 255)
        : r(r), g(g), b(b), a(a) {}
    
    static ColorValue fromEnum(Color color) {
        switch (color) {
            case Color::Black: return ColorValue(0, 0, 0);
            case Color::White: return ColorValue(255, 255, 255);
            case Color::Red: return ColorValue(255, 0, 0);
            case Color::Green: return ColorValue(0, 255, 0);
            case Color::Blue: return ColorValue(0, 0, 255);
            case Color::Yellow: return ColorValue(255, 255, 0);
            case Color::Cyan: return ColorValue(0, 255, 255);
            case Color::Magenta: return ColorValue(255, 0, 255);
            case Color::Gray: return ColorValue(128, 128, 128);
            case Color::LightGray: return ColorValue(211, 211, 211);
            case Color::DarkGray: return ColorValue(64, 64, 64);
            default: return ColorValue(0, 0, 0);
        }
    }
};

struct Size {
    double width;
    double height;
    
    Size() : width(0), height(0) {}
    Size(double width, double height) : width(width), height(height) {}
};

struct Point {
    double x;
    double y;
    
    Point() : x(0), y(0) {}
    Point(double x, double y) : x(x), y(y) {}
};

struct EdgeInsets {
    double top;
    double left;
    double bottom;
    double right;
    
    EdgeInsets() : top(0), left(0), bottom(0), right(0) {}
    EdgeInsets(double all) : top(all), left(all), bottom(all), right(all) {}
    EdgeInsets(double horizontal, double vertical) 
        : top(vertical), left(horizontal), bottom(vertical), right(horizontal) {}
    EdgeInsets(double top, double left, double bottom, double right)
        : top(top), left(left), bottom(bottom), right(right) {}
};

class View {
public:
    virtual ~View() = default;
    
    virtual HWND createWindow(HWND parent) = 0;
    virtual void update() = 0;
    virtual void destroy() = 0;
    
    void setBackgroundColor(Color color) { bgColor = color; }
    void setForegroundColor(Color color) { fgColor = color; }
    void setPadding(EdgeInsets padding) { this->padding = padding; }
    void setMargin(EdgeInsets margin) { this->margin = margin; }
    
protected:
    HWND hwnd = nullptr;
    ColorValue bgColor = ColorValue::fromEnum(Color::White);
    ColorValue fgColor = ColorValue::fromEnum(Color::Black);
    EdgeInsets padding;
    EdgeInsets margin;
};

class Text : public View {
public:
    Text(const std::string& text) : text(text) {}
    
    void setText(const std::string& text) { this->text = text; update(); }
    void setFontSize(FontSize size) { fontSize = size; update(); }
    void setAlignment(Alignment align) { alignment = align; update(); }
    
    HWND createWindow(HWND parent) override;
    void update() override;
    void destroy() override;
    
private:
    std::string text;
    FontSize fontSize = FontSize::Medium;
    Alignment alignment = Alignment::Leading;
};

class Button : public View {
public:
    Button(const std::string& title) : title(title) {}
    
    void setTitle(const std::string& title) { this->title = title; update(); }
    void setAction(std::function<void()> action) { this->action = action; }
    
    HWND createWindow(HWND parent) override;
    void update() override;
    void destroy() override;
    
private:
    std::string title;
    std::function<void()> action;
    
    static LRESULT CALLBACK buttonProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
};

class TextField : public View {
public:
    TextField() {}
    TextField(const std::string& placeholder) : placeholder(placeholder) {}
    
    void setPlaceholder(const std::string& placeholder) { this->placeholder = placeholder; }
    void setText(const std::string& text) { this->text = text; update(); }
    std::string getText() const { return text; }
    
    HWND createWindow(HWND parent) override;
    void update() override;
    void destroy() override;
    
private:
    std::string placeholder;
    std::string text;
};

class Container : public View {
public:
    Container() {}
    
    void addChild(std::shared_ptr<View> child) { children.push_back(child); }
    void setSpacing(double spacing) { this->spacing = spacing; }
    
    HWND createWindow(HWND parent) override;
    void update() override;
    void destroy() override;
    
protected:
    std::vector<std::shared_ptr<View>> children;
    double spacing = 8.0;
};

class VStack : public Container {
public:
    VStack() {}
    
    HWND createWindow(HWND parent) override;
};

class HStack : public Container {
public:
    HStack() {}
    
    HWND createWindow(HWND parent) override;
};

class ZStack : public Container {
public:
    ZStack() {}
    
    HWND createWindow(HWND parent) override;
};

class Window {
public:
    Window();
    ~Window();
    
    void setTitle(const std::string& title);
    void setSize(Size size);
    void setPosition(Point position);
    void setContent(std::shared_ptr<View> content);
    void show();
    void hide();
    void close();
    
    HWND getHandle() const { return hwnd; }
    
private:
    HWND hwnd = nullptr;
    std::string title = "XWift Window";
    Size size = Size(800, 600);
    Point position = Point(100, 100);
    std::shared_ptr<View> content;
    
    static LRESULT CALLBACK windowProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
    static std::map<HWND, Window*> windowMap;
};

class Application {
public:
    static Application& getInstance();
    
    void run();
    void quit();
    
    void setMainWindow(std::shared_ptr<Window> window);
    
private:
    Application() = default;
    ~Application() = default;
    Application(const Application&) = delete;
    Application& operator=(const Application&) = delete;
    
    std::shared_ptr<Window> mainWindow;
    bool running = false;
};

} 
} 
