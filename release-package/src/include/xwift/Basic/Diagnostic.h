#ifndef XWIFT_BASIC_DIAGNOSTIC_H
#define XWIFT_BASIC_DIAGNOSTIC_H

#include <string>
#include <vector>
#include <memory>
#include <iostream>
#include <sstream>
#include <map>
#include <set>
#include "xwift/Lexer/Token.h"

namespace xwift {

enum class DiagLevel {
    Note,
    Warning,
    Error,
    Fatal
};

enum class ErrorCategory {
    Syntax,
    Semantic,
    Type,
    Runtime,
    IO,
    Network,
    JSON,
    HTTP,
    Memory,
    Unknown
};

struct DiagnosticContext {
    std::string CodeSnippet;
    unsigned StartLine;
    unsigned StartCol;
    unsigned EndLine;
    unsigned EndCol;
    std::vector<std::string> Suggestions;
    std::vector<std::string> RelatedMessages;
    
    DiagnosticContext() : StartLine(0), StartCol(0), EndLine(0), EndCol(0) {}
};

struct StackFrame {
    std::string FunctionName;
    std::string FileName;
    unsigned Line;
    unsigned Column;
    
    StackFrame() : Line(0), Column(0) {}
    StackFrame(const std::string& func, const std::string& file, unsigned line, unsigned col)
        : FunctionName(func), FileName(file), Line(line), Column(col) {}
    
    std::string format() const {
        std::stringstream result;
        result << "  at " << FunctionName;
        if (!FileName.empty()) {
            result << " (" << FileName << ":" << Line << ":" << Column << ")";
        }
        result << "\n";
        return result.str();
    }
};

class DiagnosticError {
public:
    DiagLevel Level;
    ErrorCategory Category;
    unsigned Line;
    unsigned Column;
    std::string FileName;
    std::string Message;
    std::string ErrorID;
    DiagnosticContext Context;
    std::vector<DiagnosticError> Notes;
    
    DiagnosticError() 
        : Level(DiagLevel::Error), Category(ErrorCategory::Unknown),
          Line(1), Column(1) {}
    
    DiagnosticError(const std::string& msg, const std::string& errorID = "E0000")
        : Level(DiagLevel::Error), Category(ErrorCategory::Unknown),
          Line(1), Column(1), Message(msg), ErrorID(errorID) {}
    
    DiagnosticError(DiagLevel level, ErrorCategory category, 
                  const std::string& msg, const std::string& errorID = "E0000")
        : Level(level), Category(category), Line(1), Column(1), 
          Message(msg), ErrorID(errorID) {}
    
    DiagnosticError(const std::string& msg, unsigned line, unsigned col, 
                  const std::string& file = "", const std::string& errorID = "E0000")
        : Level(DiagLevel::Error), Category(ErrorCategory::Unknown),
          Line(line), Column(col), FileName(file), Message(msg), ErrorID(errorID) {}
    
    DiagnosticError(const std::string& msg, const SourceLocation& loc, 
                  const std::string& file = "", const std::string& errorID = "E0000")
        : Level(DiagLevel::Error), Category(ErrorCategory::Unknown),
          Line(loc.Line), Column(loc.Col), FileName(file), Message(msg), ErrorID(errorID) {}
    
    void addNote(const std::string& note) {
        DiagnosticError noteDiag;
        noteDiag.Level = DiagLevel::Note;
        noteDiag.Line = Line;
        noteDiag.Column = Column;
        noteDiag.FileName = FileName;
        noteDiag.Message = note;
        Notes.push_back(noteDiag);
    }
    
    void addSuggestion(const std::string& suggestion) {
        Context.Suggestions.push_back(suggestion);
    }
    
    void addRelatedMessage(const std::string& message) {
        Context.RelatedMessages.push_back(message);
    }
    
    void setCodeSnippet(const std::string& code, unsigned startLine, unsigned startCol,
                      unsigned endLine, unsigned endCol) {
        Context.CodeSnippet = code;
        Context.StartLine = startLine;
        Context.StartCol = startCol;
        Context.EndLine = endLine;
        Context.EndCol = endCol;
    }
    
    std::string format() const {
        std::stringstream result;
        
        if (!FileName.empty()) {
            result << FileName << ":";
        }
        result << Line << ":" << Column << ": ";
        
        switch (Level) {
            case DiagLevel::Note:
                result << "note: ";
                break;
            case DiagLevel::Warning:
                result << "warning: ";
                break;
            case DiagLevel::Error:
                result << "error: ";
                break;
            case DiagLevel::Fatal:
                result << "fatal error: ";
                break;
        }
        
        result << Message;
        
        if (!ErrorID.empty()) {
            result << " [" << ErrorID << "]";
        }
        
        result << "\n";
        
        if (!Context.CodeSnippet.empty()) {
            result << formatCodeSnippet();
        }
        
        for (const auto& note : Notes) {
            result << "    " << note.format();
        }
        
        for (const auto& suggestion : Context.Suggestions) {
            result << "    suggestion: " << suggestion << "\n";
        }
        
        for (const auto& related : Context.RelatedMessages) {
            result << "    note: " << related << "\n";
        }
        
        return result.str();
    }
    
private:
    std::string formatCodeSnippet() const {
        std::stringstream result;
        std::vector<std::string> lines;
        std::stringstream ss(Context.CodeSnippet);
        std::string line;
        
        while (std::getline(ss, line)) {
            lines.push_back(line);
        }
        
        for (size_t i = 0; i < lines.size(); i++) {
            unsigned lineNum = Context.StartLine + i;
            if (lineNum < Context.StartLine || lineNum > Context.EndLine) {
                continue;
            }
            
            result << " " << lineNum << " | " << lines[i] << "\n";
            
            if (lineNum == Context.StartLine && lineNum == Context.EndLine) {
                result << "   | ";
                for (unsigned j = 1; j < Context.StartCol; j++) {
                    result << " ";
                }
                for (unsigned j = Context.StartCol; j <= Context.EndCol; j++) {
                    result << "^";
                }
                result << "\n";
            }
        }
        
        return result.str();
    }
};

class DiagnosticEngine {
public:
    DiagnosticEngine() : errorCount(0), warningCount(0), noteCount(0) {}
    
    void pushStackFrame(const std::string& funcName, const std::string& fileName = "", 
                      unsigned line = 0, unsigned col = 0) {
        StackFrame frame(funcName, fileName, line, col);
        callStack.push_back(frame);
    }
    
    void popStackFrame() {
        if (!callStack.empty()) {
            callStack.pop_back();
        }
    }
    
    void clearStack() {
        callStack.clear();
    }
    
    std::string formatStackTrace() const {
        if (callStack.empty()) {
            return "";
        }
        
        std::stringstream result;
        result << "Stack trace:\n";
        for (const auto& frame : callStack) {
            result << frame.format();
        }
        return result.str();
    }
    
    void report(const DiagnosticError& error) {
        diagnostics.push_back(error);
        
        switch (error.Level) {
            case DiagLevel::Error:
                errorCount++;
                break;
            case DiagLevel::Warning:
                warningCount++;
                break;
            case DiagLevel::Note:
                noteCount++;
                break;
            case DiagLevel::Fatal:
                errorCount++;
                break;
        }
        
        std::cout << error.format();
        
        if (error.Level == DiagLevel::Fatal) {
            std::cout << formatStackTrace();
        }
    }
    
    void report(DiagLevel level, const std::string& message, 
               const SourceLocation& loc = SourceLocation(), 
               const std::string& filename = "",
               const std::string& errorID = "E0000") {
        DiagnosticError error;
        error.Level = level;
        error.Message = message;
        error.Line = loc.Line;
        error.Column = loc.Col;
        error.FileName = filename;
        error.ErrorID = errorID;
        report(error);
    }
    
    void reportWithCode(DiagLevel level, ErrorCategory category,
                      const std::string& message, const std::string& errorID,
                      const SourceLocation& loc = SourceLocation(),
                      const std::string& filename = "",
                      const std::string& codeSnippet = "",
                      unsigned startLine = 0, unsigned startCol = 0,
                      unsigned endLine = 0, unsigned endCol = 0) {
        DiagnosticError error(level, category, message, errorID);
        error.Line = loc.Line;
        error.Column = loc.Col;
        error.FileName = filename;
        
        if (!codeSnippet.empty()) {
            error.setCodeSnippet(codeSnippet, startLine, startCol, endLine, endCol);
        }
        
        report(error);
    }
    
    void setFilename(const std::string& filename) {
        currentFilename = filename;
    }
    
    void setSourceCode(const std::string& source) {
        sourceCode = source;
        std::stringstream ss(source);
        std::string line;
        sourceLines.clear();
        while (std::getline(ss, line)) {
            sourceLines.push_back(line);
        }
    }
    
    std::string getCodeSnippet(unsigned line, unsigned col, unsigned length = 20) {
        if (line > 0 && line <= sourceLines.size()) {
            const std::string& sourceLine = sourceLines[line - 1];
            unsigned start = (col > length) ? col - length : 0;
            unsigned end = std::min(start + length * 2, (unsigned)sourceLine.length());
            return sourceLine.substr(start, end - start);
        }
        return "";
    }
    
    void setWarningAsError(bool enabled) {
        warningAsError = enabled;
    }
    
    void setIgnoreWarnings(bool enabled) {
        ignoreWarnings = enabled;
    }
    
    void setMaxErrors(int max) {
        maxErrors = max;
    }
    
    bool hasErrors() const {
        return errorCount > 0;
    }
    
    bool hasWarnings() const {
        return warningCount > 0;
    }
    
    int getErrorCount() const {
        return errorCount;
    }
    
    int getWarningCount() const {
        return warningCount;
    }
    
    void clear() {
        diagnostics.clear();
        errorCount = 0;
        warningCount = 0;
        noteCount = 0;
    }
    
    void dumpAll() {
        for (const auto& diag : diagnostics) {
            std::cout << diag.format();
        }
    }
    
    void dumpErrors() {
        for (const auto& diag : diagnostics) {
            if (diag.Level == DiagLevel::Error || diag.Level == DiagLevel::Fatal) {
                std::cout << diag.format();
            }
        }
    }
    
    void dumpWarnings() {
        for (const auto& diag : diagnostics) {
            if (diag.Level == DiagLevel::Warning) {
                std::cout << diag.format();
            }
        }
    }
    
private:
    std::vector<DiagnosticError> diagnostics;
    std::vector<StackFrame> callStack;
    int errorCount;
    int warningCount;
    int noteCount;
    std::string currentFilename;
    std::string sourceCode;
    std::vector<std::string> sourceLines;
    bool warningAsError = false;
    bool ignoreWarnings = false;
    int maxErrors = 100;
};

namespace ErrorCodes {
    namespace Syntax {
        constexpr const char* InvalidToken = "S0001";
        constexpr const char* UnexpectedToken = "S0002";
        constexpr const char* MissingSemicolon = "S0003";
        constexpr const char* MissingParenthesis = "S0004";
        constexpr const char* MissingBrace = "S0005";
        constexpr const char* MissingBracket = "S0006";
        constexpr const char* InvalidIdentifier = "S0007";
        constexpr const char* InvalidLiteral = "S0008";
        constexpr const char* UnterminatedString = "S0009";
        constexpr const char* UnterminatedComment = "S0010";
    }
    
    namespace Semantic {
        constexpr const char* UndefinedVariable = "S0100";
        constexpr const char* UndefinedFunction = "S0101";
        constexpr const char* Redefinition = "S0102";
        constexpr const char* InvalidOperation = "S0103";
        constexpr const char* TypeMismatch = "S0104";
        constexpr const char* InvalidAssignment = "S0105";
        constexpr const char* InvalidReturn = "S0106";
        constexpr const char* InvalidBreak = "S0107";
        constexpr const char* InvalidContinue = "S0108";
        constexpr const char* InvalidParameterCount = "S0109";
    }
    
    namespace Type {
        constexpr const char* CannotConvert = "T0200";
        constexpr const char* InvalidOptional = "T0201";
        constexpr const char* InvalidForceUnwrap = "T0202";
        constexpr const char* InvalidOptionalChain = "T0203";
        constexpr const char* MissingTypeAnnotation = "T0204";
        constexpr const char* InvalidType = "T0205";
    }
    
    namespace Runtime {
        constexpr const char* DivisionByZero = "R0300";
        constexpr const char* IndexOutOfBounds = "R0301";
        constexpr const char* NullPointer = "R0302";
        constexpr const char* StackOverflow = "R0303";
        constexpr const char* MemoryAllocation = "R0304";
    }
    
    namespace IO {
        constexpr const char* FileNotFound = "I0400";
        constexpr const char* PermissionDenied = "I0401";
        constexpr const char* InvalidPath = "I0402";
        constexpr const char* ReadError = "I0403";
        constexpr const char* WriteError = "I0404";
    }
    
    namespace Network {
        constexpr const char* ConnectionFailed = "N0500";
        constexpr const char* Timeout = "N0501";
        constexpr const char* InvalidURL = "N0502";
        constexpr const char* SSLFailed = "N0503";
        constexpr const char* HTTPError = "N0504";
    }
    
    namespace JSON {
        constexpr const char* ParseError = "J0600";
        constexpr const char* InvalidStructure = "J0601";
        constexpr const char* MissingKey = "J0602";
        constexpr const char* TypeMismatch = "J0603";
        constexpr const char* SerializationError = "J0604";
    }
}

namespace diag {
    inline DiagnosticError undefinedVariable(const std::string& name, 
                                        const SourceLocation& loc = SourceLocation(),
                                        const std::string& filename = "") {
        DiagnosticError error("cannot find '" + name + "' in scope", loc, filename, 
                          ErrorCodes::Semantic::UndefinedVariable);
        error.Category = ErrorCategory::Semantic;
        
        error.addSuggestion("Check the spelling of '" + name + "'");
        error.addSuggestion("Make sure '" + name + "' is declared before use");
        
        return error;
    }
    
    inline DiagnosticError undefinedFunction(const std::string& name,
                                        const SourceLocation& loc = SourceLocation(),
                                        const std::string& filename = "") {
        DiagnosticError error("cannot find '" + name + "' in scope", loc, filename,
                          ErrorCodes::Semantic::UndefinedFunction);
        error.Category = ErrorCategory::Semantic;
        
        error.addSuggestion("Check the spelling of '" + name + "'");
        error.addSuggestion("Make sure '" + name + "' is imported or defined");
        
        return error;
    }
    
    inline DiagnosticError typeMismatch(const std::string& expected, const std::string& actual,
                                   const SourceLocation& loc = SourceLocation(),
                                   const std::string& filename = "") {
        DiagnosticError error("cannot convert '" + actual + "' to type '" + expected + "'", 
                          loc, filename, ErrorCodes::Type::CannotConvert);
        error.Category = ErrorCategory::Type;
        
        error.addNote("Expected type: " + expected);
        error.addNote("Actual type: " + actual);
        
        return error;
    }
    
    inline DiagnosticError invalidOperation(const std::string& msg,
                                       const SourceLocation& loc = SourceLocation(),
                                       const std::string& filename = "") {
        DiagnosticError error(msg, loc, filename, ErrorCodes::Semantic::InvalidOperation);
        error.Category = ErrorCategory::Semantic;
        return error;
    }
    
    inline DiagnosticError invalidForceUnwrap(const SourceLocation& loc = SourceLocation(),
                                          const std::string& filename = "") {
        DiagnosticError error("cannot force unwrap non-optional value", loc, filename,
                          ErrorCodes::Type::InvalidForceUnwrap);
        error.Category = ErrorCategory::Type;
        
        error.addSuggestion("Use optional chaining (?.) instead");
        error.addSuggestion("Check if the value is optional before force unwrapping");
        
        return error;
    }
    
    inline DiagnosticError invalidOptionalChain(const SourceLocation& loc = SourceLocation(),
                                           const std::string& filename = "") {
        DiagnosticError error("cannot use optional chaining on non-optional value", loc, filename,
                          ErrorCodes::Type::InvalidOptionalChain);
        error.Category = ErrorCategory::Type;
        
        error.addSuggestion("Remove the '?' from the expression");
        error.addSuggestion("Make sure the value is of optional type");
        
        return error;
    }
    
    inline DiagnosticError redefinition(const std::string& name,
                                    const SourceLocation& loc = SourceLocation(),
                                    const std::string& filename = "") {
        DiagnosticError error("redefinition of '" + name + "'", loc, filename,
                          ErrorCodes::Semantic::Redefinition);
        error.Category = ErrorCategory::Semantic;
        
        error.addSuggestion("Rename one of the variables");
        error.addSuggestion("Use a different name for the variable");
        
        return error;
    }
    
    inline DiagnosticError missingSemicolon(const SourceLocation& loc = SourceLocation(),
                                          const std::string& filename = "") {
        DiagnosticError error("expected ';' after expression", loc, filename,
                          ErrorCodes::Syntax::MissingSemicolon);
        error.Category = ErrorCategory::Syntax;
        
        error.addSuggestion("Add ';' at the end of the statement");
        
        return error;
    }
    
    inline DiagnosticError divisionByZero(const SourceLocation& loc = SourceLocation(),
                                       const std::string& filename = "") {
        DiagnosticError error("division by zero", loc, filename,
                          ErrorCodes::Runtime::DivisionByZero);
        error.Category = ErrorCategory::Runtime;
        error.Level = DiagLevel::Fatal;
        
        error.addSuggestion("Check the divisor before performing division");
        error.addSuggestion("Add a guard clause to prevent zero division");
        
        return error;
    }
    
    inline DiagnosticError indexOutOfBounds(const SourceLocation& loc = SourceLocation(),
                                        const std::string& filename = "") {
        DiagnosticError error("array index out of bounds", loc, filename,
                          ErrorCodes::Runtime::IndexOutOfBounds);
        error.Category = ErrorCategory::Runtime;
        error.Level = DiagLevel::Fatal;
        
        error.addSuggestion("Check the array length before accessing");
        error.addSuggestion("Use bounds checking or safe access methods");
        
        return error;
    }
    
    inline DiagnosticError invalidType(const std::string& typeName,
                                    const SourceLocation& loc = SourceLocation(),
                                    const std::string& filename = "") {
        DiagnosticError error("unknown type '" + typeName + "'", loc, filename,
                          ErrorCodes::Type::InvalidType);
        error.Category = ErrorCategory::Type;
        
        error.addSuggestion("Check the spelling of '" + typeName + "'");
        error.addSuggestion("Make sure the type is imported or defined");
        
        return error;
    }
    
    inline DiagnosticError missingTypeAnnotation(const std::string& varName,
                                            const SourceLocation& loc = SourceLocation(),
                                            const std::string& filename = "") {
        DiagnosticError error("cannot infer type for variable '" + varName + "'", loc, filename,
                          ErrorCodes::Type::MissingTypeAnnotation);
        error.Category = ErrorCategory::Type;
        
        error.addSuggestion("Add an explicit type annotation");
        error.addSuggestion("Provide an initial value to help type inference");
        
        return error;
    }
    
    inline DiagnosticError invalidReturn(const SourceLocation& loc = SourceLocation(),
                                       const std::string& filename = "") {
        DiagnosticError error("non-void function must return a value", loc, filename,
                          ErrorCodes::Semantic::InvalidReturn);
        error.Category = ErrorCategory::Semantic;
        
        error.addSuggestion("Add a return statement with a value");
        error.addSuggestion("Change the function return type to Void");
        
        return error;
    }
    
    inline DiagnosticError conditionNotBool(const SourceLocation& loc = SourceLocation(),
                                           const std::string& filename = "") {
        DiagnosticError error("condition must be of type Bool", loc, filename,
                          ErrorCodes::Type::CannotConvert);
        error.Category = ErrorCategory::Type;
        
        error.addSuggestion("Use a boolean expression");
        error.addSuggestion("Convert the condition to Bool");
        
        return error;
    }
    
    inline DiagnosticError cannotCompare(const std::string& type1, const std::string& type2,
                                     const SourceLocation& loc = SourceLocation(),
                                     const std::string& filename = "") {
        DiagnosticError error("cannot compare '" + type1 + "' with '" + type2 + "'", loc, filename,
                          ErrorCodes::Type::CannotConvert);
        error.Category = ErrorCategory::Type;
        
        error.addSuggestion("Convert one of the types to match the other");
        error.addSuggestion("Use compatible types for comparison");
        
        return error;
    }
    
    inline DiagnosticError operandNotBool(const std::string& op, const std::string& side,
                                        const SourceLocation& loc = SourceLocation(),
                                        const std::string& filename = "") {
        DiagnosticError error(side + " operand of '" + op + "' must be of type Bool", loc, filename,
                          ErrorCodes::Type::CannotConvert);
        error.Category = ErrorCategory::Type;
        
        error.addSuggestion("Use a boolean expression");
        error.addSuggestion("Convert the operand to Bool");
        
        return error;
    }
    
    inline DiagnosticError arrayIndexNotInt(const SourceLocation& loc = SourceLocation(),
                                           const std::string& filename = "") {
        DiagnosticError error("array index must be of integer type", loc, filename,
                          ErrorCodes::Type::CannotConvert);
        error.Category = ErrorCategory::Type;
        
        error.addSuggestion("Use an integer expression for the index");
        error.addSuggestion("Convert the index to Int64");
        
        return error;
    }
    
    inline DiagnosticError guardNotOptional(const SourceLocation& loc = SourceLocation(),
                                           const std::string& filename = "") {
        DiagnosticError error("guard let requires an optional value", loc, filename,
                          ErrorCodes::Type::InvalidOptional);
        error.Category = ErrorCategory::Type;
        
        error.addSuggestion("Use an optional value with guard let");
        error.addSuggestion("Check if the value is of optional type");
        
        return error;
    }
    
    inline DiagnosticError classRedefinition(const std::string& className,
                                             const SourceLocation& loc = SourceLocation(),
                                             const std::string& filename = "") {
        DiagnosticError error("redefinition of class '" + className + "'", loc, filename,
                          ErrorCodes::Semantic::Redefinition);
        error.Category = ErrorCategory::Semantic;
        
        error.addSuggestion("Rename class");
        error.addSuggestion("Remove duplicate class definition");
        
        return error;
    }
    
    inline DiagnosticError structRedefinition(const std::string& structName,
                                              const SourceLocation& loc = SourceLocation(),
                                              const std::string& filename = "") {
        DiagnosticError error("redefinition of struct '" + structName + "'", loc, filename,
                          ErrorCodes::Semantic::Redefinition);
        error.Category = ErrorCategory::Semantic;
        
        error.addSuggestion("Rename struct");
        error.addSuggestion("Remove duplicate struct definition");
        
        return error;
    }
    
    inline DiagnosticError arithmeticOnBool(const SourceLocation& loc = SourceLocation(),
                                           const std::string& filename = "") {
        DiagnosticError error("cannot perform arithmetic operations on Bool type", loc, filename,
                          ErrorCodes::Semantic::InvalidOperation);
        error.Category = ErrorCategory::Semantic;
        
        error.addSuggestion("Use logical operators (&&, ||) instead");
        error.addSuggestion("Convert Bool to Int64 if arithmetic is needed");
        
        return error;
    }
    
    inline DiagnosticError cannotAssignToExpr(const SourceLocation& loc = SourceLocation(),
                                             const std::string& filename = "") {
        DiagnosticError error("cannot assign to expression", loc, filename,
                          ErrorCodes::Semantic::InvalidAssignment);
        error.Category = ErrorCategory::Semantic;
        
        error.addSuggestion("Assign to a variable or property");
        error.addSuggestion("Check if the target is assignable");
        
        return error;
    }
    
    inline DiagnosticError arrayElementsNotSameType(const SourceLocation& loc = SourceLocation(),
                                                   const std::string& filename = "") {
        DiagnosticError error("array elements must have the same type", loc, filename,
                          ErrorCodes::Type::CannotConvert);
        error.Category = ErrorCategory::Type;
        
        error.addSuggestion("Ensure all elements have compatible types");
        error.addSuggestion("Use explicit type conversion if needed");
        
        return error;
    }
    
    inline DiagnosticError ifLetNotOptional(const SourceLocation& loc = SourceLocation(),
                                             const std::string& filename = "") {
        DiagnosticError error("if let requires an optional value", loc, filename,
                          ErrorCodes::Type::InvalidOptional);
        error.Category = ErrorCategory::Type;
        
        error.addSuggestion("Use an optional value with if let");
        error.addSuggestion("Check if the value is of optional type");
        
        return error;
    }
    
    inline DiagnosticError cannotAssignToImmutable(const std::string& varName,
                                                  const SourceLocation& loc = SourceLocation(),
                                                  const std::string& filename = "") {
        DiagnosticError error("cannot assign to value: '" + varName + "' is a 'let' constant", loc, filename,
                          ErrorCodes::Semantic::InvalidAssignment);
        error.Category = ErrorCategory::Semantic;
        
        error.addSuggestion("Use 'var' instead of 'let' to make it mutable");
        error.addSuggestion("Declare a new variable with a different name");
        
        return error;
    }
    
    inline DiagnosticError wrongArgCount(const std::string& funcName, int expected, int actual,
                                        const SourceLocation& loc = SourceLocation(),
                                        const std::string& filename = "") {
        DiagnosticError error(funcName + "() expects " + std::to_string(expected) + 
                           " argument(s), but got " + std::to_string(actual), loc, filename,
                          ErrorCodes::Semantic::InvalidOperation);
        error.Category = ErrorCategory::Semantic;
        
        error.addSuggestion("Check the number of arguments");
        error.addSuggestion("Refer to the function signature");
        
        return error;
    }
}

}

#endif
