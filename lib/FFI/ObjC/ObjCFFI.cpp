#include "xwift/FFI/ObjC/ObjCFFI.h"
#include <stdexcept>
#include <iostream>

namespace xwift {
namespace objc {

ObjCFFIPlugin::~ObjCFFIPlugin() {
    shutdown();
}

bool ObjCFFIPlugin::initialize() {
    if (initialized) {
        return true;
    }
    
    initialized = true;
    return true;
}

void ObjCFFIPlugin::shutdown() {
    if (initialized) {
        initialized = false;
    }
}

plugin::PluginInfo ObjCFFIPlugin::getInfo() const {
    return {
        "ObjCFFI",
        "1.0.0",
        "Objective-C Foreign Function Interface for XWift",
        "XWift Team"
    };
}

ObjCClass::ObjCClass(const std::string& name) 
    : className(name), objcClass(nil) {
    objcClass = objc_getClass(name.c_str());
}

bool ObjCClass::isValid() const {
    return objcClass != nil;
}

bool ObjCClass::hasMethod(const std::string& selector) const {
    if (!objcClass) {
        return false;
    }
    
    SEL sel = sel_registerName(selector.c_str());
    return class_respondsToSelector(objcClass, sel);
}

bool ObjCClass::hasClassMethod(const std::string& selector) const {
    if (!objcClass) {
        return false;
    }
    
    SEL sel = sel_registerName(selector.c_str());
    return class_respondsToSelector(objc_getMetaClass(className.c_str()), sel);
}

ForeignValue ObjCClass::callMethod(id instance, const std::string& selector,
                                     const std::vector<ForeignValue>& args) {
    if (!instance || !objcClass) {
        return ForeignValue();
    }
    
    SEL sel = sel_registerName(selector.c_str());
    
    NSMethodSignature* sig = [instance methodSignatureForSelector:sel];
    if (!sig) {
        return ForeignValue();
    }
    
    NSInvocation* invocation = [NSInvocation invocationWithMethodSignature:sig];
    [invocation setSelector:sel];
    [invocation setTarget:instance];
    
    for (size_t i = 0; i < args.size(); i++) {
        ForeignValue arg = args[i];
        switch (arg.getType()) {
            case ForeignType::Int8:
            case ForeignType::Int16:
            case ForeignType::Int32:
            case ForeignType::Int64: {
                int64_t val = *arg.get<int64_t>();
                [invocation setArgument:&val atIndex:i + 2];
                break;
            }
            case ForeignType::Float32:
            case ForeignType::Float64: {
                double val = *arg.get<double>();
                [invocation setArgument:&val atIndex:i + 2];
                break;
            }
            case ForeignType::Bool: {
                bool val = *arg.get<bool>();
                [invocation setArgument:&val atIndex:i + 2];
                break;
            }
            case ForeignType::String: {
                std::string str = *arg.get<std::string>();
                NSString* nsStr = [NSString stringWithUTF8String:str.c_str()];
                [invocation setArgument:&nsStr atIndex:i + 2];
                break;
            }
            default:
                break;
        }
    }
    
    [invocation invoke];
    
    const char* returnType = [sig methodReturnType];
    if (strcmp(returnType, @encode(void)) == 0) {
        return ForeignValue();
    }
    
    if (strcmp(returnType, @encode(id)) == 0) {
        id result;
        [invocation getReturnValue:&result];
        return ObjCValue(result).toForeignValue();
    }
    
    if (strcmp(returnType, @encode(NSInteger)) == 0) {
        NSInteger result;
        [invocation getReturnValue:&result];
        return ForeignValue(static_cast<int64_t>(result));
    }
    
    if (strcmp(returnType, @encode(double)) == 0) {
        double result;
        [invocation getReturnValue:&result];
        return ForeignValue(result);
    }
    
    if (strcmp(returnType, @encode(BOOL)) == 0) {
        BOOL result;
        [invocation getReturnValue:&result];
        return ForeignValue(result != 0);
    }
    
    return ForeignValue();
}

ForeignValue ObjCClass::callClassMethod(const std::string& selector,
                                          const std::vector<ForeignValue>& args) {
    if (!objcClass) {
        return ForeignValue();
    }
    
    return callMethod((id)objcClass, selector, args);
}

id ObjCClass::createInstance(const std::vector<ForeignValue>& args) {
    if (!objcClass) {
        return nil;
    }
    
    ForeignValue result = callClassMethod("alloc", {});
    if (result.getType() != ForeignType::Object) {
        return nil;
    }
    
    id instance = *result.get<void*>();
    
    if (!args.empty()) {
        ForeignValue initResult = callMethod(instance, "initWith:", args);
        if (initResult.getType() == ForeignType::Object) {
            instance = *initResult.get<void*>();
        }
    } else {
        ForeignValue initResult = callMethod(instance, "init", {});
        if (initResult.getType() == ForeignType::Object) {
            instance = *initResult.get<void*>();
        }
    }
    
    return instance;
}

ObjCInstance::ObjCInstance(id obj, std::shared_ptr<ObjCClass> cls)
    : instance(obj), instanceClass(cls) {
    if (instance) {
        [instance retain];
    }
}

ObjCInstance::~ObjCInstance() {
    if (instance) {
        [instance release];
    }
}

ForeignValue ObjCInstance::callMethod(const std::string& selector,
                                        const std::vector<ForeignValue>& args) {
    if (!instance || !instanceClass) {
        return ForeignValue();
    }
    
    return instanceClass->callMethod(instance, selector, args);
}

ForeignValue ObjCInstance::getProperty(const std::string& name) {
    if (!instance) {
        return ForeignValue();
    }
    
    NSString* key = [NSString stringWithUTF8String:name.c_str()];
    id value = [instance valueForKey:key];
    
    return ObjCValue(value).toForeignValue();
}

bool ObjCInstance::setProperty(const std::string& name, const ForeignValue& value) {
    if (!instance) {
        return false;
    }
    
    NSString* key = [NSString stringWithUTF8String:name.c_str()];
    ObjCValue objcValue = ObjCValue::fromForeignValue(value);
    id objValue = objcValue.getObjCObject();
    
    [instance setValue:objValue forKey:key];
    return true;
}

ObjCFunction::ObjCFunction(std::shared_ptr<ObjCClass> cls, const std::string& sel, bool isClass)
    : targetClass(cls), selector(sel), isClassMethod(isClass) {
    signature.name = sel;
    signature.returnType = ForeignType::Void;
    signature.isVariadic = false;
}

ForeignValue ObjCFunction::call(const std::vector<ForeignValue>& args) {
    if (!targetClass) {
        return ForeignValue();
    }
    
    if (isClassMethod) {
        return targetClass->callClassMethod(selector, args);
    } else {
        id instance = targetClass->createInstance({});
        if (!instance) {
            return ForeignValue();
        }
        
        auto objInstance = std::make_shared<ObjCInstance>(instance, targetClass);
        return objInstance->callMethod(selector, args);
    }
}

ForeignFunctionSignature ObjCFunction::getSignature() const {
    return signature;
}

ForeignLanguage ObjCFunction::getLanguage() const {
    return ForeignLanguage::ObjectiveC;
}

ObjCLibrary::ObjCLibrary(const std::string& name)
    : ForeignLibrary(name, ForeignLanguage::ObjectiveC) {
}

ObjCLibrary::~ObjCLibrary() {
    unload();
}

bool ObjCLibrary::load() {
    if (isLoaded()) {
        return true;
    }
    
    return true;
}

void ObjCLibrary::unload() {
    classes.clear();
}

std::shared_ptr<ObjCClass> ObjCLibrary::getClass(const std::string& className) {
    auto it = classes.find(className);
    if (it != classes.end()) {
        return it->second;
    }
    
    auto cls = std::make_shared<ObjCClass>(className);
    if (!cls->isValid()) {
        return nullptr;
    }
    
    classes[className] = cls;
    return cls;
}

bool ObjCLibrary::hasClass(const std::string& className) const {
    return classes.find(className) != classes.end();
}

std::shared_ptr<ForeignFunction> ObjCLibrary::getFunction(const std::string& name) {
    return nullptr;
}

std::vector<std::string> ObjCLibrary::listClasses() const {
    std::vector<std::string> result;
    for (const auto& pair : classes) {
        result.push_back(pair.first);
    }
    return result;
}

ObjCValue::ObjCValue(id obj) : objcObject(obj) {
    if (objcObject) {
        [objcObject retain];
    }
}

ObjCValue::~ObjCValue() {
    if (objcObject) {
        [objcObject release];
    }
}

ForeignValue ObjCValue::toForeignValue() const {
    if (!objcObject) {
        return ForeignValue();
    }
    
    if ([objcObject isKindOfClass:[NSNumber class]]) {
        NSNumber* num = (NSNumber*)objcObject;
        
        if (strcmp([num objCType], @encode(BOOL)) == 0) {
            return ForeignValue([num boolValue]);
        }
        
        if (strcmp([num objCType], @encode(NSInteger)) == 0) {
            return ForeignValue(static_cast<int64_t>([num integerValue]));
        }
        
        if (strcmp([num objCType], @encode(double)) == 0) {
            return ForeignValue([num doubleValue]);
        }
        
        return ForeignValue(static_cast<int64_t>([num longValue]));
    }
    
    if ([objcObject isKindOfClass:[NSString class]]) {
        NSString* str = (NSString*)objcObject;
        const char* cstr = [str UTF8String];
        if (cstr) {
            return ForeignValue(std::string(cstr));
        }
    }
    
    if ([objcObject isKindOfClass:[NSArray class]]) {
        NSArray* arr = (NSArray*)objcObject;
        std::vector<ForeignValue> result;
        
        for (id item in arr) {
            result.push_back(ObjCValue(item).toForeignValue());
        }
        
        return ForeignValue(result);
    }
    
    return ForeignValue(static_cast<void*>(objcObject));
}

ObjCValue ObjCValue::fromForeignValue(const ForeignValue& value) {
    id obj = nil;
    
    switch (value.getType()) {
        case ForeignType::Int8:
        case ForeignType::Int16:
        case ForeignType::Int32:
        case ForeignType::Int64:
        case ForeignType::UInt8:
        case ForeignType::UInt16:
        case ForeignType::UInt32:
        case ForeignType::UInt64: {
            int64_t intVal = *value.get<int64_t>();
            obj = [NSNumber numberWithLongLong:intVal];
            break;
        }
        case ForeignType::Float32:
        case ForeignType::Float64: {
            double floatVal = *value.get<double>();
            obj = [NSNumber numberWithDouble:floatVal];
            break;
        }
        case ForeignType::Bool: {
            bool boolVal = *value.get<bool>();
            obj = [NSNumber numberWithBool:boolVal];
            break;
        }
        case ForeignType::String: {
            const std::string& strVal = *value.get<std::string>();
            obj = [NSString stringWithUTF8String:strVal.c_str()];
            break;
        }
        case ForeignType::Array: {
            const std::vector<ForeignValue>& arr = *value.get<std::vector<ForeignValue>>();
            NSMutableArray* nsArr = [NSMutableArray arrayWithCapacity:arr.size()];
            
            for (const auto& item : arr) {
                ObjCValue objcItem = ObjCValue::fromForeignValue(item);
                [nsArr addObject:objcItem.getObjCObject()];
            }
            
            obj = nsArr;
            break;
        }
        case ForeignType::Object:
        case ForeignType::Pointer: {
            obj = (__bridge id)(*value.get<void*>());
            break;
        }
        default:
            obj = [NSNull null];
            break;
    }
    
    return ObjCValue(obj);
}

}
}
