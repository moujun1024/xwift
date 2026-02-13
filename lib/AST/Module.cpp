#include "xwift/AST/Module.h"
#include "xwift/Lexer/Lexer.h"
#include "xwift/Parser/SyntaxParser.h"
#include "xwift/Sema/Sema.h"
#include "xwift/Basic/Diagnostic.h"
#include <fstream>
#include <sstream>
#include <iostream>

namespace xwift {

Module* ModuleManager::loadModule(const std::string& moduleName, const std::string& basePath) {
    if (isModuleLoaded(moduleName)) {
        return getModule(moduleName);
    }
    
    std::string filePath = findModuleFile(moduleName, basePath);
    if (filePath.empty()) {
        return nullptr;
    }
    
    auto module = std::make_unique<Module>(moduleName, filePath);
    Module* modulePtr = module.get();
    Modules[moduleName] = std::move(module);
    
    if (!parseModule(modulePtr)) {
        return nullptr;
    }
    
    modulePtr->IsLoaded = true;
    
    return modulePtr;
}

Module* ModuleManager::getModule(const std::string& moduleName) {
    auto it = Modules.find(moduleName);
    if (it != Modules.end()) {
        return it->second.get();
    }
    return nullptr;
}

bool ModuleManager::isModuleLoaded(const std::string& moduleName) const {
    auto it = Modules.find(moduleName);
    if (it != Modules.end()) {
        return it->second->IsLoaded;
    }
    return false;
}

void ModuleManager::unloadModule(const std::string& moduleName) {
    auto it = Modules.find(moduleName);
    if (it != Modules.end()) {
        it->second->IsLoaded = false;
    }
}

void ModuleManager::clear() {
    Modules.clear();
}

std::string ModuleManager::findModuleFile(const std::string& moduleName, const std::string& basePath) {
    std::vector<std::string> searchPaths = {
        basePath + "/lib/" + moduleName + ".xw",
        basePath + "/" + moduleName + ".xw", 
        basePath + "/test/" + moduleName + ".xw",
        "lib/" + moduleName + ".xw",
        moduleName + ".xw"
    };
    
    for (const auto& path : searchPaths) {
        std::ifstream file(path);
        if (file.is_open()) {
            return path;
        }
    }
    
    return "";
}

bool ModuleManager::parseModule(Module* module) {
    if (module->IsLoading) {
        return false;
    }
    
    module->IsLoading = true;
    
    std::ifstream file(module->FilePath);
    if (!file.is_open()) {
        return false;
    }
    
    std::stringstream buffer;
    buffer << file.rdbuf();
    std::string source = buffer.str();
    
    Lexer lexer(source);
    SyntaxParser parser(lexer);
    auto program = parser.parseProgram();
    
    if (!program) {
        module->IsLoading = false;
        return false;
    }
    
    DiagnosticEngine diag;
    diag.setFilename(module->FilePath);
    diag.setSourceCode(source);
    
    Sema sema(diag);
    sema.setFilename(module->FilePath);
    
    for (auto& decl : program->Declarations) {
        if (!sema.visit(decl.get())) {
            module->IsLoading = false;
            return false;
        }
        
        module->Declarations.push_back(std::move(decl));
    }
    
    if (diag.hasErrors()) {
        module->IsLoading = false;
        return false;
    }
    
    module->IsLoading = false;
    return true;
}

}
