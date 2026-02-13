#ifndef XWIFT_AST_MODULE_H
#define XWIFT_AST_MODULE_H

#include "xwift/AST/Nodes.h"
#include "xwift/AST/Type.h"
#include <string>
#include <map>
#include <set>
#include <memory>

namespace xwift {

class Module {
public:
    std::string Name;
    std::string FilePath;
    std::vector<DeclPtr> Declarations;
    std::set<std::string> Exports;
    std::map<std::string, std::shared_ptr<Type>> ExportedTypes;
    std::map<std::string, FuncDecl*> ExportedFunctions;
    std::map<std::string, ClassDecl*> ExportedClasses;
    std::map<std::string, StructDecl*> ExportedStructs;
    bool IsLoaded;
    bool IsLoading;
    
    Module(const std::string& name, const std::string& filePath)
        : Name(name), FilePath(filePath), IsLoaded(false), IsLoading(false) {}
    
    void addExport(const std::string& symbol) {
        Exports.insert(symbol);
    }
    
    void exportType(const std::string& name, std::shared_ptr<Type> type) {
        ExportedTypes[name] = type;
        addExport(name);
    }
    
    void exportFunction(const std::string& name, FuncDecl* func) {
        ExportedFunctions[name] = func;
        addExport(name);
    }
    
    void exportClass(const std::string& name, ClassDecl* cls) {
        ExportedClasses[name] = cls;
        addExport(name);
    }
    
    void exportStruct(const std::string& name, StructDecl* st) {
        ExportedStructs[name] = st;
        addExport(name);
    }
    
    bool hasExport(const std::string& symbol) const {
        return Exports.find(symbol) != Exports.end();
    }
    
    std::shared_ptr<Type> getType(const std::string& name) const {
        auto it = ExportedTypes.find(name);
        if (it != ExportedTypes.end()) {
            return it->second;
        }
        return nullptr;
    }
    
    FuncDecl* getFunction(const std::string& name) const {
        auto it = ExportedFunctions.find(name);
        if (it != ExportedFunctions.end()) {
            return it->second;
        }
        return nullptr;
    }
    
    ClassDecl* getClass(const std::string& name) const {
        auto it = ExportedClasses.find(name);
        if (it != ExportedClasses.end()) {
            return it->second;
        }
        return nullptr;
    }
    
    StructDecl* getStruct(const std::string& name) const {
        auto it = ExportedStructs.find(name);
        if (it != ExportedStructs.end()) {
            return it->second;
        }
        return nullptr;
    }
};

class ModuleManager {
public:
    ModuleManager() {}
    
    Module* loadModule(const std::string& moduleName, const std::string& basePath);
    Module* getModule(const std::string& moduleName);
    bool isModuleLoaded(const std::string& moduleName) const;
    void unloadModule(const std::string& moduleName);
    void clear();
    
private:
    std::map<std::string, std::unique_ptr<Module>> Modules;
    std::vector<std::string> SearchPaths;
    
    std::string findModuleFile(const std::string& moduleName, const std::string& basePath);
    bool parseModule(Module* module);
};

}

#endif
