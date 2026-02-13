#ifndef XWIFT_DIAGNOSTIC_H
#define XWIFT_DIAGNOSTIC_H

#include <string>
#include <stdexcept>
#include <memory>
#include <iostream>
#include "xwift/Lexer/Token.h"

namespace xwift {

enum class DiagLevel {
    Error,
    Warning,
    Note
};

class DiagnosticError : public std::exception {
public:
    DiagLevel Level;
    unsigned Line;
    unsigned Column;
    std::string FileName;
    std::string Message;
    std::string ErrorID;
    
    DiagnosticError(const std::string& msg, const std::string& errorID = "unknown")
        : Level(DiagLevel::Error), Line(1), Column(1), Message(msg), ErrorID(errorID) {}
    
    DiagnosticError(const std::string& msg, unsigned line, unsigned col, 
                    const std::string& file = "", const std::string& errorID = "unknown")
        : Level(DiagLevel::Error), Line(line), Column(col), 
          FileName(file), Message(msg), ErrorID(errorID) {}
    
    DiagnosticError(const std::string& msg, const SourceLocation& loc, const std::string& errorID = "unknown")
        : Level(DiagLevel::Error), Line(loc.Line), Column(loc.Col), 
          Message(msg), ErrorID(errorID) {}
    
    const char* what() const noexcept override {
        return Message.c_str();
    }
    
    std::string format() const {
        std::string result;
        
        if (!FileName.empty()) {
            result += FileName + ":";
        }
        result += std::to_string(Line) + ":" + std::to_string(Column) + ": ";
        
        if (Level == DiagLevel::Error) {
            result += "error: ";
        } else if (Level == DiagLevel::Warning) {
            result += "warning: ";
        }
        
        result += Message;
        
        if (!ErrorID.empty()) {
            result += " [" + ErrorID + "]";
        }
        
        return result;
    }
};

inline std::string formatError(const std::string& msg, const std::string& errorID = "E0000") {
    return "error: " + msg + " [" + errorID + "]";
}

namespace diag {
    inline DiagnosticError undefinedVariable(const std::string& name, unsigned line = 1, unsigned col = 1) {
        return DiagnosticError("cannot find '" + name + "' in scope", line, col, "", "E0425");
    }
    
    inline DiagnosticError invalidOperation(const std::string& msg, unsigned line = 1, unsigned col = 1) {
        return DiagnosticError(msg, line, col, "", "E0001");
    }
    
    inline DiagnosticError typeMismatch(const std::string& expected, const std::string& actual, 
                                        unsigned line = 1, unsigned col = 1) {
        return DiagnosticError("cannot convert '"
            + actual + "' to type '" + expected + "'", line, col, "", "E0006");
    }
}

class DiagnosticEngine {
public:
    void report(const DiagnosticError& error) {
        std::cout << error.format() << "\n";
        if (error.Level == DiagLevel::Error) {
            errorCount++;
        }
    }
    
    void report(DiagLevel level, const std::string& message, const SourceLocation& loc = SourceLocation(), const std::string& filename = "") {
        DiagnosticError error(message, loc);
        error.Level = level;
        if (!filename.empty()) {
            error.FileName = filename;
        }
        report(error);
    }
    
    bool hasErrors() const {
        return errorCount > 0;
    }
    
    void clear() {
        errorCount = 0;
    }
    
    void setFilename(const std::string& filename) {
        currentFilename = filename;
    }
    
private:
    int errorCount = 0;
    std::string currentFilename;
};

}

#endif
