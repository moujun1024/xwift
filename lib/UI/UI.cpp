#include "xwift/UI/UI.h"
#include <commctrl.h>

#pragma comment(lib, "comctl32.lib")

namespace XWift {
namespace UI {

std::map<HWND, Window*> Window::windowMap;

HWND Text::createWindow(HWND parent) {
    hwnd = CreateWindowEx(
        0,
        L"STATIC",
        std::wstring(text.begin(), text.end()).c_str(),
        WS_CHILD | WS_VISIBLE,
        0, 0, 100, 20,
        parent,
        nullptr,
        GetModuleHandle(nullptr),
        nullptr
    );
    
    update();
    return hwnd;
}

void Text::update() {
    if (!hwnd) return;
    
    SetWindowText(hwnd, std::wstring(text.begin(), text.end()).c_str());
    
    HFONT hFont = nullptr;
    switch (fontSize) {
        case FontSize::Small:
            hFont = CreateFont(12, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
                               DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
                               DEFAULT_QUALITY, DEFAULT_PITCH | FF_SWISS, L"Arial");
            break;
        case FontSize::Medium:
            hFont = CreateFont(14, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
                               DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
                               DEFAULT_QUALITY, DEFAULT_PITCH | FF_SWISS, L"Arial");
            break;
        case FontSize::Large:
            hFont = CreateFont(18, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE,
                               DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
                               DEFAULT_QUALITY, DEFAULT_PITCH | FF_SWISS, L"Arial");
            break;
        case FontSize::ExtraLarge:
            hFont = CreateFont(24, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE,
                               DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
                               DEFAULT_QUALITY, DEFAULT_PITCH | FF_SWISS, L"Arial");
            break;
    }
    
    if (hFont) {
        SendMessage(hwnd, WM_SETFONT, (WPARAM)hFont, TRUE);
    }
    
    DWORD style = GetWindowLong(hwnd, GWL_STYLE);
    switch (alignment) {
        case Alignment::Leading:
            style |= SS_LEFT;
            break;
        case Alignment::Center:
            style |= SS_CENTER;
            break;
        case Alignment::Trailing:
            style |= SS_RIGHT;
            break;
    }
    SetWindowLong(hwnd, GWL_STYLE, style);
}

void Text::destroy() {
    if (hwnd) {
        DestroyWindow(hwnd);
        hwnd = nullptr;
    }
}

LRESULT CALLBACK Button::buttonProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    Button* button = reinterpret_cast<Button*>(GetWindowLongPtr(hwnd, GWLP_USERDATA));
    
    switch (msg) {
        case WM_COMMAND:
            if (HIWORD(wParam) == BN_CLICKED) {
                if (button && button->action) {
                    button->action();
                }
            }
            break;
    }
    
    return DefWindowProc(hwnd, msg, wParam, lParam);
}

HWND Button::createWindow(HWND parent) {
    hwnd = CreateWindowEx(
        0,
        L"BUTTON",
        std::wstring(title.begin(), title.end()).c_str(),
        WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
        0, 0, 100, 30,
        parent,
        nullptr,
        GetModuleHandle(nullptr),
        nullptr
    );
    
    SetWindowLongPtr(hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(this));
    update();
    return hwnd;
}

void Button::update() {
    if (!hwnd) return;
    
    SetWindowText(hwnd, std::wstring(title.begin(), title.end()).c_str());
}

void Button::destroy() {
    if (hwnd) {
        DestroyWindow(hwnd);
        hwnd = nullptr;
    }
}

HWND TextField::createWindow(HWND parent) {
    hwnd = CreateWindowEx(
        WS_EX_CLIENTEDGE,
        L"EDIT",
        std::wstring(text.begin(), text.end()).c_str(),
        WS_CHILD | WS_VISIBLE | ES_AUTOHSCROLL,
        0, 0, 200, 24,
        parent,
        nullptr,
        GetModuleHandle(nullptr),
        nullptr
    );
    
    if (!placeholder.empty()) {
        SendMessage(hwnd, EM_SETCUEBANNER, TRUE, 
                   (LPARAM)std::wstring(placeholder.begin(), placeholder.end()).c_str());
    }
    
    return hwnd;
}

void TextField::update() {
    if (!hwnd) return;
    
    SetWindowText(hwnd, std::wstring(text.begin(), text.end()).c_str());
}

void TextField::destroy() {
    if (hwnd) {
        DestroyWindow(hwnd);
        hwnd = nullptr;
    }
}

HWND Container::createWindow(HWND parent) {
    hwnd = CreateWindowEx(
        0,
        L"STATIC",
        L"",
        WS_CHILD | WS_VISIBLE,
        0, 0, 100, 100,
        parent,
        nullptr,
        GetModuleHandle(nullptr),
        nullptr
    );
    
    return hwnd;
}

void Container::update() {
    if (!hwnd) return;
    
    for (auto& child : children) {
        if (child) {
            child->update();
        }
    }
}

void Container::destroy() {
    for (auto& child : children) {
        if (child) {
            child->destroy();
        }
    }
    
    if (hwnd) {
        DestroyWindow(hwnd);
        hwnd = nullptr;
    }
}

HWND VStack::createWindow(HWND parent) {
    Container::createWindow(parent);
    
    RECT parentRect;
    GetClientRect(parent, &parentRect);
    
    int yPos = static_cast<int>(margin.top);
    int maxWidth = 0;
    
    for (auto& child : children) {
        if (!child) continue;
        
        child->createWindow(hwnd);
        
        RECT childRect;
        GetClientRect(child->getHandle(), &childRect);
        
        SetWindowPos(child->getHandle(), nullptr,
                    static_cast<int>(margin.left),
                    yPos,
                    static_cast<int>(parentRect.right - margin.left - margin.right),
                    childRect.bottom,
                    SWP_NOZORDER);
        
        yPos += childRect.bottom + static_cast<int>(spacing);
        maxWidth = std::max(maxWidth, childRect.right);
    }
    
    SetWindowPos(hwnd, nullptr, 0, 0, maxWidth, yPos, SWP_NOZORDER);
    
    return hwnd;
}

HWND HStack::createWindow(HWND parent) {
    Container::createWindow(parent);
    
    RECT parentRect;
    GetClientRect(parent, &parentRect);
    
    int xPos = static_cast<int>(margin.left);
    int maxHeight = 0;
    
    for (auto& child : children) {
        if (!child) continue;
        
        child->createWindow(hwnd);
        
        RECT childRect;
        GetClientRect(child->getHandle(), &childRect);
        
        SetWindowPos(child->getHandle(), nullptr,
                    xPos,
                    static_cast<int>(margin.top),
                    childRect.right,
                    static_cast<int>(parentRect.bottom - margin.top - margin.bottom),
                    SWP_NOZORDER);
        
        xPos += childRect.right + static_cast<int>(spacing);
        maxHeight = std::max(maxHeight, childRect.bottom);
    }
    
    SetWindowPos(hwnd, nullptr, 0, 0, xPos, maxHeight, SWP_NOZORDER);
    
    return hwnd;
}

HWND ZStack::createWindow(HWND parent) {
    Container::createWindow(parent);
    
    RECT parentRect;
    GetClientRect(parent, &parentRect);
    
    for (auto& child : children) {
        if (!child) continue;
        
        child->createWindow(hwnd);
        
        SetWindowPos(child->getHandle(), nullptr,
                    static_cast<int>(margin.left),
                    static_cast<int>(margin.top),
                    static_cast<int>(parentRect.right - margin.left - margin.right),
                    static_cast<int>(parentRect.bottom - margin.top - margin.bottom),
                    SWP_NOZORDER);
    }
    
    SetWindowPos(hwnd, nullptr, 0, 0, 
                 static_cast<int>(parentRect.right), 
                 static_cast<int>(parentRect.bottom), 
                 SWP_NOZORDER);
    
    return hwnd;
}

Window::Window() {
    WNDCLASSW wc = {};
    wc.lpfnWndProc = windowProc;
    wc.hInstance = GetModuleHandle(nullptr);
    wc.lpszClassName = L"XWiftWindow";
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wc.hCursor = LoadCursor(nullptr, IDC_ARROW);
    
    RegisterClassW(&wc);
}

Window::~Window() {
    if (hwnd) {
        DestroyWindow(hwnd);
        windowMap.erase(hwnd);
    }
}

void Window::setTitle(const std::string& title) {
    this->title = title;
    if (hwnd) {
        SetWindowText(hwnd, std::wstring(title.begin(), title.end()).c_str());
    }
}

void Window::setSize(Size size) {
    this->size = size;
    if (hwnd) {
        SetWindowPos(hwnd, nullptr, 0, 0,
                    static_cast<int>(size.width),
                    static_cast<int>(size.height),
                    SWP_NOMOVE | SWP_NOZORDER);
    }
}

void Window::setPosition(Point position) {
    this->position = position;
    if (hwnd) {
        SetWindowPos(hwnd, nullptr,
                    static_cast<int>(position.x),
                    static_cast<int>(position.y),
                    0, 0,
                    SWP_NOSIZE | SWP_NOZORDER);
    }
}

void Window::setContent(std::shared_ptr<View> content) {
    this->content = content;
    if (hwnd && content) {
        content->createWindow(hwnd);
    }
}

void Window::show() {
    if (!hwnd) {
        hwnd = CreateWindowExW(
            0,
            L"XWiftWindow",
            std::wstring(title.begin(), title.end()).c_str(),
            WS_OVERLAPPEDWINDOW,
            static_cast<int>(position.x),
            static_cast<int>(position.y),
            static_cast<int>(size.width),
            static_cast<int>(size.height),
            nullptr,
            nullptr,
            GetModuleHandle(nullptr),
            this
        );
        
        if (content) {
            content->createWindow(hwnd);
        }
    }
    
    ShowWindow(hwnd, SW_SHOW);
    UpdateWindow(hwnd);
}

void Window::hide() {
    if (hwnd) {
        ShowWindow(hwnd, SW_HIDE);
    }
}

void Window::close() {
    if (hwnd) {
        DestroyWindow(hwnd);
        hwnd = nullptr;
    }
}

LRESULT CALLBACK Window::windowProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    Window* window = nullptr;
    
    if (msg == WM_NCCREATE) {
        CREATESTRUCT* cs = reinterpret_cast<CREATESTRUCT*>(lParam);
        window = reinterpret_cast<Window*>(cs->lpCreateParams);
        windowMap[hwnd] = window;
    } else {
        window = windowMap[hwnd];
    }
    
    switch (msg) {
        case WM_DESTROY:
            if (window) {
                window->hwnd = nullptr;
                windowMap.erase(hwnd);
            }
            PostQuitMessage(0);
            return 0;
            
        case WM_SIZE:
            if (window && window->content) {
                RECT rect;
                GetClientRect(hwnd, &rect);
                SetWindowPos(window->content->getHandle(), nullptr,
                            0, 0, rect.right, rect.bottom, SWP_NOZORDER);
            }
            break;
    }
    
    return DefWindowProc(hwnd, msg, wParam, lParam);
}

Application& Application::getInstance() {
    static Application instance;
    return instance;
}

void Application::run() {
    if (!mainWindow) {
        return;
    }
    
    running = true;
    mainWindow->show();
    
    MSG msg;
    while (running && GetMessage(&msg, nullptr, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
}

void Application::quit() {
    running = false;
    PostQuitMessage(0);
}

void Application::setMainWindow(std::shared_ptr<Window> window) {
    mainWindow = window;
}

} 
} 
