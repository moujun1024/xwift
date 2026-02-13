#ifndef XWIFT_STDLIB_TERMINAL_TERMINAL_H
#define XWIFT_STDLIB_TERMINAL_TERMINAL_H

#include <string>
#include <cstdint>

namespace xwift {
namespace terminal {

enum class KeyCode {
    Unknown = 0,
    
    Character = 1,
    
    Up = 1000,
    Down,
    Left,
    Right,
    
    Home = 1010,
    End,
    PageUp,
    PageDown,
    
    F1 = 1020,
    F2,
    F3,
    F4,
    F5,
    F6,
    F7,
    F8,
    F9,
    F10,
    F11,
    F12,
    
    Enter = 2000,
    Tab,
    Backspace,
    Delete,
    Insert,
    Escape,
    
    Space = 3000
};

struct KeyEvent {
    KeyCode code;
    char character;
    bool pressed;
    
    KeyEvent() : code(KeyCode::Unknown), character('\0'), pressed(true) {}
    KeyEvent(KeyCode c, char ch = '\0') : code(c), character(ch), pressed(true) {}
};

class Terminal {
public:
    Terminal();
    ~Terminal();
    
    void init();
    void cleanup();
    
    bool hasInput();
    KeyEvent getKey();
    
    void setRawMode(bool enabled);
    void setEchoMode(bool enabled);
    
    void clearScreen();
    void moveCursor(int row, int col);
    void hideCursor();
    void showCursor();
    
    void setColor(int foreground, int background = -1);
    void resetColor();
    
    int getTerminalWidth();
    int getTerminalHeight();
    
    void flush();
    
private:
    bool rawMode;
    bool echoMode;
    bool initialized;
    
    void enableRawMode();
    void disableRawMode();
    
    KeyCode parseEscapeSequence();
    char readChar();
    bool charAvailable();
};

}
}

#endif
