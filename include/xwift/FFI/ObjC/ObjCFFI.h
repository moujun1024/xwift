#pragma once

#include "xwift/FFI/FFI.h"
#include <objc/runtime.h>
#include <objc/message.h>
#include <string>
#include <map>
#include <memory>

namespace xwift {
namespace objc {

class ObjCFFIPlugin : public plugin::Plugin {
private:
    bool initialized;
    
public:
    ObjCFFIPlugin() : initialized(false) {}
    ~ObjCFFIPlugin() override;
    
    bool initialize() override;
    void shutdown() override;
    plugin::PluginInfo getInfo() const override;
};

class ObjCClass {
private:
    Class objcClass;
    std::string className;
    
public:
    ObjCClass(const std::string& name);
    ~ObjCClass() = default;
    
    bool isValid() const;
    
    std::string getClassName() const { return className; }
    Class getClass() const { return objcClass; }
    
    bool hasMethod(const std::string& selector) const;
    bool hasClassMethod(const std::string& selector) const;
    
    ForeignValue callMethod(id instance, const std::string& selector, 
                             const std::vector<ForeignValue>& args);
    ForeignValue callClassMethod(const std::string& selector, 
                                  const std::vector<ForeignValue>& args);
    
    id createInstance(const std::vector<ForeignValue>& args);
};

class ObjCInstance {
private:
    id instance;
    std::shared_ptr<ObjCClass> instanceClass;
    
public:
    ObjCInstance(id obj, std::shared_ptr<ObjCClass> cls);
    ~ObjCInstance();
    
    id getInstance() const { return instance; }
    std::shared_ptr<ObjCClass> getClass() const { return instanceClass; }
    
    ForeignValue callMethod(const std::string& selector, 
                             const std::vector<ForeignValue>& args);
    
    ForeignValue getProperty(const std::string& name);
    bool setProperty(const std::string& name, const ForeignValue& value);
};

class ObjCFunction : public ForeignFunction {
private:
    std::shared_ptr<ObjCClass> targetClass;
    std::string selector;
    bool isClassMethod;
    ForeignFunctionSignature signature;
    
public:
    ObjCFunction(std::shared_ptr<ObjCClass> cls, const std::string& sel, bool isClass);
    
    ForeignValue call(const std::vector<ForeignValue>& args) override;
    ForeignFunctionSignature getSignature() const override;
    ForeignLanguage getLanguage() const override;
    
    void setSignature(const ForeignFunctionSignature& sig) {
        signature = sig;
    }
};

class ObjCLibrary : public ForeignLibrary {
private:
    std::map<std::string, std::shared_ptr<ObjCClass>> classes;
    
public:
    ObjCLibrary(const std::string& name);
    ~ObjCLibrary() override;
    
    bool load() override;
    void unload() override;
    
    std::shared_ptr<ObjCClass> getClass(const std::string& className);
    bool hasClass(const std::string& className) const;
    
    std::shared_ptr<ForeignFunction> getFunction(const std::string& name) override;
    
    std::vector<std::string> listClasses() const;
};

class ObjCValue {
private:
    id objcObject;
    
public:
    ObjCValue(id obj);
    ~ObjCValue();
    
    id getObjCObject() const { return objcObject; }
    
    ForeignValue toForeignValue() const;
    static ObjCValue fromForeignValue(const ForeignValue& value);
};

}
}
