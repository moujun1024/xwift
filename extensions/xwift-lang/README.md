# XWift Language VSCode Extension

VSCode extension for XWift language support.

## Directory Structure

```
xwift-lang/
├── package.json              # Extension configuration
├── language-configuration.json  # Language settings
├── syntaxes/
│   └── xwift.tmLanguage.json # Syntax highlighting
├── snippets/
│   └── xwift.code-snippets  # Code snippets
├── images/
│   └── icon.png             # Extension icon (128x128)
└── README.md               # This file
```

## Features

- Syntax highlighting for `.xw` files
- Code snippets for common XWift patterns
- Bracket matching and auto-closing
- Comment support (line and block)

## Installation

1. Open VSCode
2. Press `Ctrl+Shift+P` to open command palette
3. Run `Extensions: Install from VSIX`
4. Select the `xwift-lang.vsix` file

Or install in development mode:

1. Navigate to `extensions/xwift-lang`
2. Run `npm install -g vsce`
3. Run `vsce package`
4. Install the generated `.vsix` file

## Supported Features

### Syntax Highlighting
- Keywords (func, var, let, if, else, for, while, etc.)
- Types (Int, String, Bool, Double, etc.)
- Strings and numbers
- Comments
- Functions

### Code Snippets
- `main` - Main function template
- `func` - Function declaration
- `var` - Variable declaration
- `let` - Constant declaration
- `if` - If statement
- `ifelse` - If-else statement
- `while` - While loop
- `for` - For loop
- `switch` - Switch statement
- `class` - Class declaration
- `import` - Import module
- `print` - Print function
- `println` - Println function

## File Association

The extension automatically associates with `.xw` files.

## Language Server

For enhanced features like IntelliSense and error reporting, the XWift language server will be added in a future version.
