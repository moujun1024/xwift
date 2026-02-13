#include "xwift/stdlib/Terminal/Terminal.h"
#include <iostream>
#include <chrono>
#include <thread>

#ifdef _WIN32
#include <conio.h>
#include <windows.h>
#else
#include <termios.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#endif

namespace xwift {
namespace terminal {

Terminal::Terminal() : rawMode(false), echoMode(true), initialized(false) {
}

Terminal::~Terminal() {
    cleanup();
}

void Terminal::init() {
    if (initialized) {
        return;
    }
    
    setRawMode(true);
    setEchoMode(false);
    initialized = true;
}

void Terminal::cleanup() {
    if (!initialized) {
        return;
    }
    
    setRawMode(false);
    setEchoMode(true);
    showCursor();
    resetColor();
    initialized = false;
}

void Terminal::setRawMode(bool enabled) {
    rawMode = enabled;
    
    if (enabled) {
        enableRawMode();
    } else {
        disableRawMode();
    }
}

void Terminal::setEchoMode(bool enabled) {
    echoMode = enabled;
    
#ifdef _WIN32
    HANDLE hStdin = GetStdHandle(STD_INPUT_HANDLE);
    DWORD mode;
    GetConsoleMode(hStdin, &mode);
    
    if (enabled) {
        mode |= ENABLE_ECHO_INPUT;
    } else {
        mode &= ~ENABLE_ECHO_INPUT;
    }
    
    SetConsoleMode(hStdin, mode);
#else
    struct termios term;
    tcgetattr(STDIN_FILENO, &term);
    
    if (enabled) {
        term.c_lflag |= ECHO;
    } else {
        term.c_lflag &= ~ECHO;
    }
    
    tcsetattr(STDIN_FILENO, TCSANOW, &term);
#endif
}

void Terminal::enableRawMode() {
#ifdef _WIN32
    HANDLE hStdin = GetStdHandle(STD_INPUT_HANDLE);
    DWORD mode;
    GetConsoleMode(hStdin, &mode);
    
    mode &= ~(ENABLE_LINE_INPUT | ENABLE_ECHO_INPUT | ENABLE_PROCESSED_INPUT);
    mode |= ENABLE_WINDOW_INPUT;
    
    SetConsoleMode(hStdin, mode);
#else
    struct termios term;
    tcgetattr(STDIN_FILENO, &term);
    
    term.c_lflag &= ~(ICANON | ECHO);
    term.c_cc[VMIN] = 0;
    term.c_cc[VTIME] = 0;
    
    tcsetattr(STDIN_FILENO, TCSANOW, &term);
#endif
}

void Terminal::disableRawMode() {
#ifdef _WIN32
    HANDLE hStdin = GetStdHandle(STD_INPUT_HANDLE);
    DWORD mode;
    GetConsoleMode(hStdin, &mode);
    
    mode |= ENABLE_LINE_INPUT | ENABLE_ECHO_INPUT | ENABLE_PROCESSED_INPUT;
    mode &= ~ENABLE_WINDOW_INPUT;
    
    SetConsoleMode(hStdin, mode);
#else
    struct termios term;
    tcgetattr(STDIN_FILENO, &term);
    
    term.c_lflag |= ICANON | ECHO;
    term.c_cc[VMIN] = 1;
    term.c_cc[VTIME] = 0;
    
    tcsetattr(STDIN_FILENO, TCSANOW, &term);
#endif
}

bool Terminal::hasInput() {
#ifdef _WIN32
    return _kbhit() != 0;
#else
    struct timeval tv = {0, 0};
    fd_set fds;
    FD_ZERO(&fds);
    FD_SET(STDIN_FILENO, &fds);
    
    return select(STDIN_FILENO + 1, &fds, nullptr, nullptr, &tv) > 0;
#endif
}

char Terminal::readChar() {
#ifdef _WIN32
    return static_cast<char>(_getch());
#else
    char c;
    read(STDIN_FILENO, &c, 1);
    return c;
#endif
}

bool Terminal::charAvailable() {
    return hasInput();
}

KeyEvent Terminal::getKey() {
    if (!hasInput()) {
        return KeyEvent(KeyCode::Unknown);
    }
    
    char ch = readChar();
    
    if (ch == '\033') {
        return KeyEvent(parseEscapeSequence());
    }
    
    if (ch == '\r' || ch == '\n') {
        return KeyEvent(KeyCode::Enter);
    }
    
    if (ch == '\t') {
        return KeyEvent(KeyCode::Tab);
    }
    
    if (ch == 127 || ch == 8) {
        return KeyEvent(KeyCode::Backspace);
    }
    
    if (ch == ' ') {
        return KeyEvent(KeyCode::Space);
    }
    
    return KeyEvent(KeyCode::Character, ch);
}

KeyCode Terminal::parseEscapeSequence() {
#ifdef _WIN32
    if (charAvailable()) {
        char ch = readChar();
        
        if (ch == '[' || ch == 'O') {
            if (charAvailable()) {
                ch = readChar();
                
                switch (ch) {
                    case 'A': return KeyCode::Up;
                    case 'B': return KeyCode::Down;
                    case 'C': return KeyCode::Right;
                    case 'D': return KeyCode::Left;
                    case 'H': return KeyCode::Home;
                    case 'F': return KeyCode::End;
                    case '5': 
                        if (charAvailable() && readChar() == '~') return KeyCode::PageUp;
                        break;
                    case '6':
                        if (charAvailable() && readChar() == '~') return KeyCode::PageDown;
                        break;
                    case '1':
                        if (charAvailable() && readChar() == '~') return KeyCode::Home;
                        break;
                    case '4':
                        if (charAvailable() && readChar() == '~') return KeyCode::End;
                        break;
                    case '2':
                        if (charAvailable() && readChar() == '~') return KeyCode::Insert;
                        break;
                    case '3':
                        if (charAvailable() && readChar() == '~') return KeyCode::Delete;
                        break;
                    default:
                        break;
                }
            }
        }
    }
#else
    if (charAvailable()) {
        char ch = readChar();
        
        if (ch == '[') {
            if (charAvailable()) {
                ch = readChar();
                
                switch (ch) {
                    case 'A': return KeyCode::Up;
                    case 'B': return KeyCode::Down;
                    case 'C': return KeyCode::Right;
                    case 'D': return KeyCode::Left;
                    case 'H': return KeyCode::Home;
                    case 'F': return KeyCode::End;
                    case '5': 
                        if (charAvailable() && readChar() == '~') return KeyCode::PageUp;
                        break;
                    case '6':
                        if (charAvailable() && readChar() == '~') return KeyCode::PageDown;
                        break;
                    case '1':
                        if (charAvailable()) {
                            char next = readChar();
                            if (next == '~') return KeyCode::Home;
                            if (next == ';') {
                                if (charAvailable()) {
                                    char modifier = readChar();
                                    if (modifier == '5' && charAvailable() && readChar() == '~') return KeyCode::PageUp;
                                    if (modifier == '6' && charAvailable() && readChar() == '~') return KeyCode::PageDown;
                                }
                            }
                        }
                        break;
                    case '4':
                        if (charAvailable()) {
                            char next = readChar();
                            if (next == '~') return KeyCode::End;
                            if (next == ';') {
                                if (charAvailable()) {
                                    char modifier = readChar();
                                    if (modifier == '1' && charAvailable() && readChar() == '~') return KeyCode::End;
                                }
                            }
                        }
                        break;
                    case '2':
                        if (charAvailable() && readChar() == '~') return KeyCode::Insert;
                        break;
                    case '3':
                        if (charAvailable() && readChar() == '~') return KeyCode::Delete;
                        break;
                    default:
                        break;
                }
            }
        } else if (ch == 'O') {
            if (charAvailable()) {
                ch = readChar();
                
                switch (ch) {
                    case 'A': return KeyCode::Up;
                    case 'B': return KeyCode::Down;
                    case 'C': return KeyCode::Right;
                    case 'D': return KeyCode::Left;
                    case 'H': return KeyCode::Home;
                    case 'F': return KeyCode::End;
                    case 'P': return KeyCode::F1;
                    case 'Q': return KeyCode::F2;
                    case 'R': return KeyCode::F3;
                    case 'S': return KeyCode::F4;
                    default:
                        break;
                }
            }
        }
    }
#endif
    
    return KeyCode::Unknown;
}

void Terminal::clearScreen() {
    std::cout << "\033[2J\033[H";
    flush();
}

void Terminal::moveCursor(int row, int col) {
    std::cout << "\033[" << row << ";" << col << "H";
    flush();
}

void Terminal::hideCursor() {
    std::cout << "\033[?25l";
    flush();
}

void Terminal::showCursor() {
    std::cout << "\033[?25h";
    flush();
}

void Terminal::setColor(int foreground, int background) {
    if (background == -1) {
        std::cout << "\033[" << (30 + foreground) << "m";
    } else {
        std::cout << "\033[" << (30 + foreground) << ";" << (40 + background) << "m";
    }
    flush();
}

void Terminal::resetColor() {
    std::cout << "\033[0m";
    flush();
}

int Terminal::getTerminalWidth() {
#ifdef _WIN32
    CONSOLE_SCREEN_BUFFER_INFO csbi;
    GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &csbi);
    return csbi.srWindow.Right - csbi.srWindow.Left + 1;
#else
    struct winsize w;
    ioctl(STDOUT_FILENO, TIOCGWINSZ, &w);
    return w.ws_col;
#endif
}

int Terminal::getTerminalHeight() {
#ifdef _WIN32
    CONSOLE_SCREEN_BUFFER_INFO csbi;
    GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &csbi);
    return csbi.srWindow.Bottom - csbi.srWindow.Top + 1;
#else
    struct winsize w;
    ioctl(STDOUT_FILENO, TIOCGWINSZ, &w);
    return w.ws_row;
#endif
}

void Terminal::flush() {
    std::cout << std::flush;
}

}
}
