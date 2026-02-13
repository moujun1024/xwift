#include "xwift/FFI/FFI.h"
#include "xwift/Basic/Diagnostic.h"
#include <dlfcn.h>
#include <stdexcept>

namespace xwift {
namespace ffi {

ForeignValue ForeignValue::toXwiftValue() const {
    switch (type) {
        case ForeignType::Int8:
        case ForeignType::Int16:
        case ForeignType::Int32:
        case ForeignType::Int64:
        case ForeignType::UInt8:
        case ForeignType::UInt16:
        case ForeignType::UInt32:
        case ForeignType::UInt64:
            return Value(*get<int64_t>());
        case ForeignType::Float32:
        case ForeignType::Float64:
            return Value(*get<double>());
        case ForeignType::Bool:
            return Value(*get<bool>());
        case ForeignType::String:
            return Value(*get<std::string>());
        case ForeignType::Array: {
            const auto& arr = *get<std::vector<ForeignValue>>();
            std::vector<Value> xwiftArr;
            for (const auto& item : arr) {
                xwiftArr.push_back(item.toXwiftValue());
            }
            return Value(xwiftArr);
        }
        case ForeignType::Void:
        case ForeignType::Object:
        case ForeignType::Pointer:
        default:
            return Value();
    }
}

ForeignValue ForeignValue::fromXwiftValue(const Value& val) {
    if (val.isNil()) {
        return ForeignValue();
    }
    
    if (auto intVal = val.get<int64_t>()) {
        return ForeignValue(*intVal);
    }
    
    if (auto floatVal = val.get<double>()) {
        return ForeignValue(*floatVal);
    }
    
    if (auto boolVal = val.get<bool>()) {
        return ForeignValue(*boolVal);
    }
    
    if (auto strVal = val.get<std::string>()) {
        return ForeignValue(*strVal);
    }
    
    if (auto arrVal = val.get<std::vector<Value>>()) {
        std::vector<ForeignValue> foreignArr;
        for (const auto& item : *arrVal) {
            foreignArr.push_back(fromXwiftValue(item));
        }
        return ForeignValue(foreignArr);
    }
    
    return ForeignValue();
}

ForeignLibrary::ForeignLibrary(const std::string& libName, ForeignLanguage lang)
    : name(libName), handle(nullptr), language(lang) {
}

ForeignLibrary::~ForeignLibrary() {
    unload();
}

bool ForeignLibrary::load() {
    if (handle) {
        return true;
    }
    
    std::string libPath;
    switch (language) {
        case ForeignLanguage::Python:
            libPath = "libpython3.so";
            break;
        case ForeignLanguage::ObjectiveC:
            libPath = "libobjc.so";
            break;
        case ForeignLanguage::JavaScript:
            libPath = "libnode.so";
            break;
        case ForeignLanguage::Rust:
            libPath = "librust.so";
            break;
        case ForeignLanguage::Go:
            libPath = "libgo.so";
            break;
        case ForeignLanguage::C:
        default:
            libPath = name;
            break;
    }
    
    handle = dlopen(libPath.c_str(), RTLD_LAZY);
    if (!handle) {
        return false;
    }
    
    return true;
}

void ForeignLibrary::unload() {
    if (handle) {
        dlclose(handle);
        handle = nullptr;
    }
    functions.clear();
}

bool ForeignLibrary::isLoaded() const {
    return handle != nullptr;
}

std::shared_ptr<ForeignFunction> ForeignLibrary::getFunction(const std::string& name) {
    auto it = functions.find(name);
    if (it != functions.end()) {
        return it->second;
    }
    
    if (!handle) {
        return nullptr;
    }
    
    void* funcPtr = dlsym(handle, name.c_str());
    if (!funcPtr) {
        return nullptr;
    }
    
    return nullptr;
}

bool ForeignLibrary::hasFunction(const std::string& name) const {
    return functions.find(name) != functions.end();
}

FFIManager& FFIManager::getInstance() {
    static FFIManager instance;
    return instance;
}

bool FFIManager::loadLibrary(const std::string& name, ForeignLanguage language) {
    if (libraries.find(name) != libraries.end()) {
        return true;
    }
    
    auto lib = std::make_shared<ForeignLibrary>(name, language);
    if (!lib->load()) {
        return false;
    }
    
    libraries[name] = lib;
    return true;
}

bool FFIManager::unloadLibrary(const std::string& name) {
    auto it = libraries.find(name);
    if (it == libraries.end()) {
        return false;
    }
    
    it->second->unload();
    libraries.erase(it);
    return true;
}

std::shared_ptr<ForeignLibrary> FFIManager::getLibrary(const std::string& name) {
    auto it = libraries.find(name);
    if (it != libraries.end()) {
        return it->second;
    }
    return nullptr;
}

ForeignValue FFIManager::callFunction(const std::string& libraryName,
                                        const std::string& functionName,
                                        const std::vector<ForeignValue>& args) {
    auto lib = getLibrary(libraryName);
    if (!lib) {
        return ForeignValue();
    }
    
    auto func = lib->getFunction(functionName);
    if (!func) {
        return ForeignValue();
    }
    
    return func->call(args);
}

std::vector<std::string> FFIManager::listLoadedLibraries() const {
    std::vector<std::string> result;
    for (const auto& pair : libraries) {
        result.push_back(pair.first);
    }
    return result;
}

ForeignType foreignTypeFromString(const std::string& typeStr) {
    if (typeStr == "Void") return ForeignType::Void;
    if (typeStr == "Int8") return ForeignType::Int8;
    if (typeStr == "Int16") return ForeignType::Int16;
    if (typeStr == "Int32") return ForeignType::Int32;
    if (typeStr == "Int64") return ForeignType::Int64;
    if (typeStr == "UInt8") return ForeignType::UInt8;
    if (typeStr == "UInt16") return ForeignType::UInt16;
    if (typeStr == "UInt32") return ForeignType::UInt32;
    if (typeStr == "UInt64") return ForeignType::UInt64;
    if (typeStr == "Float32") return ForeignType::Float32;
    if (typeStr == "Float64") return ForeignType::Float64;
    if (typeStr == "Bool") return ForeignType::Bool;
    if (typeStr == "String") return ForeignType::String;
    if (typeStr == "Array") return ForeignType::Array;
    if (typeStr == "Object") return ForeignType::Object;
    if (typeStr == "Pointer") return ForeignType::Pointer;
    return ForeignType::Void;
}

std::string foreignTypeToString(ForeignType type) {
    switch (type) {
        case ForeignType::Void: return "Void";
        case ForeignType::Int8: return "Int8";
        case ForeignType::Int16: return "Int16";
        case ForeignType::Int32: return "Int32";
        case ForeignType::Int64: return "Int64";
        case ForeignType::UInt8: return "UInt8";
        case ForeignType::UInt16: return "UInt16";
        case ForeignType::UInt32: return "UInt32";
        case ForeignType::UInt64: return "UInt64";
        case ForeignType::Float32: return "Float32";
        case ForeignType::Float64: return "Float64";
        case ForeignType::Bool: return "Bool";
        case ForeignType::String: return "String";
        case ForeignType::Array: return "Array";
        case ForeignType::Object: return "Object";
        case ForeignType::Pointer: return "Pointer";
        default: return "Void";
    }
}

}
}
