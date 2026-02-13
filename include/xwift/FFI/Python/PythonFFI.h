#pragma once

#include "xwift/FFI/FFI.h"
#include <Python.h>
#include <string>
#include <map>
#include <memory>

namespace xwift {
namespace python {

class PythonFFIPlugin : public plugin::Plugin {
private:
    bool initialized;
    
public:
    PythonFFIPlugin() : initialized(false) {}
    ~PythonFFIPlugin() override;
    
    bool initialize() override;
    void shutdown() override;
    plugin::PluginInfo getInfo() const override;
};

class PythonInterpreter {
private:
    PyObject* mainModule;
    PyObject* globalDict;
    PyObject* localDict;
    
public:
    PythonInterpreter();
    ~PythonInterpreter();
    
    bool initialize();
    void shutdown();
    
    bool execute(const std::string& code);
    bool executeFile(const std::string& filename);
    
    ForeignValue callFunction(const std::string& functionName, 
                               const std::vector<ForeignValue>& args);
    
    bool setVariable(const std::string& name, const ForeignValue& value);
    ForeignValue getVariable(const std::string& name);
    
    bool hasFunction(const std::string& name) const;
    std::vector<std::string> listFunctions() const;
};

class PythonFunction : public ForeignFunction {
private:
    PythonInterpreter* interpreter;
    std::string functionName;
    ForeignFunctionSignature signature;
    
public:
    PythonFunction(PythonInterpreter* interp, const std::string& name);
    
    ForeignValue call(const std::vector<ForeignValue>& args) override;
    ForeignFunctionSignature getSignature() const override;
    ForeignLanguage getLanguage() const override;
    
    void setSignature(const ForeignFunctionSignature& sig) {
        signature = sig;
    }
};

class PythonValue {
private:
    PyObject* pyObject;
    
public:
    PythonValue(PyObject* obj);
    ~PythonValue();
    
    PyObject* getPyObject() const { return pyObject; }
    
    ForeignValue toForeignValue() const;
    static PythonValue fromForeignValue(const ForeignValue& value);
};

class PythonLibrary : public ForeignLibrary {
private:
    std::unique_ptr<PythonInterpreter> interpreter;
    
public:
    PythonLibrary(const std::string& name);
    ~PythonLibrary() override;
    
    bool load() override;
    void unload() override;
    
    std::shared_ptr<ForeignFunction> getFunction(const std::string& name) override;
};

}
}
