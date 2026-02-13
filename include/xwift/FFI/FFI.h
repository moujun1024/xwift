#pragma once

#include "xwift/AST/Nodes.h"
#include "xwift/Interpreter/Interpreter.h"
#include <string>
#include <map>
#include <vector>
#include <functional>
#include <memory>
#include <variant>

namespace xwift {
namespace ffi {

enum class ForeignLanguage {
    Python,
    ObjectiveC,
    JavaScript,
    Rust,
    Go,
    C
};

enum class ForeignType {
    Void,
    Int8,
    Int16,
    Int32,
    Int64,
    UInt8,
    UInt16,
    UInt32,
    UInt64,
    Float32,
    Float64,
    Bool,
    String,
    Array,
    Object,
    Pointer
};

struct ForeignFunctionSignature {
    std::string name;
    ForeignType returnType;
    std::vector<ForeignType> parameterTypes;
    bool isVariadic;
};

class ForeignValue {
private:
    std::variant<
        int64_t,
        double,
        bool,
        std::string,
        std::vector<ForeignValue>,
        void*
    > data;
    
    ForeignType type;
    
public:
    ForeignValue() : type(ForeignType::Void), data(int64_t(0)) {}
    
    ForeignValue(int64_t val) : type(ForeignType::Int64), data(val) {}
    ForeignValue(double val) : type(ForeignType::Float64), data(val) {}
    ForeignValue(bool val) : type(ForeignType::Bool), data(val) {}
    ForeignValue(const std::string& val) : type(ForeignType::String), data(val) {}
    ForeignValue(const std::vector<ForeignValue>& val) : type(ForeignType::Array), data(val) {}
    ForeignValue(void* val) : type(ForeignType::Pointer), data(val) {}
    
    ForeignType getType() const { return type; }
    
    template<typename T>
    T* get() {
        return std::get_if<T>(&data);
    }
    
    template<typename T>
    const T* get() const {
        return std::get_if<T>(&data);
    }
    
    Value toXwiftValue() const;
    static ForeignValue fromXwiftValue(const Value& val);
};

class ForeignFunction {
public:
    virtual ~ForeignFunction() = default;
    
    virtual ForeignValue call(const std::vector<ForeignValue>& args) = 0;
    virtual ForeignFunctionSignature getSignature() const = 0;
    virtual ForeignLanguage getLanguage() const = 0;
};

class ForeignLibrary {
private:
    std::string name;
    void* handle;
    ForeignLanguage language;
    std::map<std::string, std::shared_ptr<ForeignFunction>> functions;
    
public:
    ForeignLibrary(const std::string& libName, ForeignLanguage lang);
    ~ForeignLibrary();
    
    bool load();
    void unload();
    bool isLoaded() const;
    
    std::shared_ptr<ForeignFunction> getFunction(const std::string& name);
    bool hasFunction(const std::string& name) const;
    
    std::string getName() const { return name; }
    ForeignLanguage getLanguage() const { return language; }
};

class FFICallExpr : public Expr {
public:
    std::string libraryName;
    std::string functionName;
    std::vector<ExprPtr> arguments;
    SourceLocation Loc;
    
    FFICallExpr(const std::string& lib, const std::string& func, 
                const std::vector<ExprPtr>& args, SourceLocation loc = SourceLocation())
        : libraryName(lib), functionName(func), arguments(args), Loc(loc) {}
};

class ForeignImportDecl : public Decl {
public:
    std::string libraryName;
    ForeignLanguage language;
    std::vector<std::string> functionNames;
    
    ForeignImportDecl(const std::string& lib, ForeignLanguage lang,
                     const std::vector<std::string>& funcs)
        : libraryName(lib), language(lang), functionNames(funcs) {}
    
    void accept(DeclVisitor& visitor) override {
        visitor.visit(this);
    }
};

class FFIManager {
public:
    static FFIManager& getInstance();
    
    bool loadLibrary(const std::string& name, ForeignLanguage language);
    bool unloadLibrary(const std::string& name);
    std::shared_ptr<ForeignLibrary> getLibrary(const std::string& name);
    
    ForeignValue callFunction(const std::string& libraryName, 
                              const std::string& functionName,
                              const std::vector<ForeignValue>& args);
    
    std::vector<std::string> listLoadedLibraries() const;
    
private:
    FFIManager() = default;
    ~FFIManager() = default;
    
    std::map<std::string, std::shared_ptr<ForeignLibrary>> libraries;
};

ForeignType foreignTypeFromString(const std::string& typeStr);
std::string foreignTypeToString(ForeignType type);

}
}
