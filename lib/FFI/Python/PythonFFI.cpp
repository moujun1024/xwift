#include "xwift/FFI/Python/PythonFFI.h"
#include <stdexcept>
#include <iostream>

namespace xwift {
namespace python {

PythonFFIPlugin::~PythonFFIPlugin() {
    shutdown();
}

bool PythonFFIPlugin::initialize() {
    if (initialized) {
        return true;
    }
    
    Py_Initialize();
    if (!Py_IsInitialized()) {
        return false;
    }
    
    initialized = true;
    return true;
}

void PythonFFIPlugin::shutdown() {
    if (initialized) {
        Py_Finalize();
        initialized = false;
    }
}

plugin::PluginInfo PythonFFIPlugin::getInfo() const {
    return {
        "PythonFFI",
        "1.0.0",
        "Python Foreign Function Interface for XWift",
        "XWift Team"
    };
}

PythonInterpreter::PythonInterpreter() 
    : mainModule(nullptr), globalDict(nullptr), localDict(nullptr) {
}

PythonInterpreter::~PythonInterpreter() {
    shutdown();
}

bool PythonInterpreter::initialize() {
    if (!Py_IsInitialized()) {
        return false;
    }
    
    mainModule = PyImport_ImportModule("__main__");
    if (!mainModule) {
        return false;
    }
    
    globalDict = PyModule_GetDict(mainModule);
    localDict = PyDict_New();
    
    return globalDict != nullptr && localDict != nullptr;
}

void PythonInterpreter::shutdown() {
    if (localDict) {
        Py_DECREF(localDict);
        localDict = nullptr;
    }
    if (mainModule) {
        Py_DECREF(mainModule);
        mainModule = nullptr;
    }
    globalDict = nullptr;
}

bool PythonInterpreter::execute(const std::string& code) {
    if (!globalDict || !localDict) {
        return false;
    }
    
    PyObject* result = PyRun_String(code.c_str(), Py_file_input, 
                                     globalDict, localDict);
    if (!result) {
        PyErr_Print();
        return false;
    }
    
    Py_DECREF(result);
    return true;
}

bool PythonInterpreter::executeFile(const std::string& filename) {
    FILE* file = fopen(filename.c_str(), "r");
    if (!file) {
        return false;
    }
    
    int result = PyRun_SimpleFile(file, filename.c_str());
    fclose(file);
    
    return result == 0;
}

ForeignValue PythonInterpreter::callFunction(const std::string& functionName,
                                               const std::vector<ForeignValue>& args) {
    if (!globalDict) {
        return ForeignValue();
    }
    
    PyObject* func = PyDict_GetItemString(globalDict, functionName.c_str());
    if (!func || !PyCallable_Check(func)) {
        return ForeignValue();
    }
    
    PyObject* pyArgs = PyTuple_New(args.size());
    for (size_t i = 0; i < args.size(); i++) {
        PyObject* arg = PythonValue::fromForeignValue(args[i]).getPyObject();
        PyTuple_SetItem(pyArgs, i, arg);
    }
    
    PyObject* result = PyObject_CallObject(func, pyArgs);
    Py_DECREF(pyArgs);
    
    if (!result) {
        PyErr_Print();
        return ForeignValue();
    }
    
    ForeignValue foreignResult = PythonValue(result).toForeignValue();
    Py_DECREF(result);
    
    return foreignResult;
}

bool PythonInterpreter::setVariable(const std::string& name, 
                                      const ForeignValue& value) {
    if (!globalDict) {
        return false;
    }
    
    PyObject* pyValue = PythonValue::fromForeignValue(value).getPyObject();
    if (!pyValue) {
        return false;
    }
    
    PyDict_SetItemString(globalDict, name.c_str(), pyValue);
    Py_DECREF(pyValue);
    
    return true;
}

ForeignValue PythonInterpreter::getVariable(const std::string& name) {
    if (!globalDict) {
        return ForeignValue();
    }
    
    PyObject* value = PyDict_GetItemString(globalDict, name.c_str());
    if (!value) {
        return ForeignValue();
    }
    
    return PythonValue(value).toForeignValue();
}

bool PythonInterpreter::hasFunction(const std::string& name) const {
    if (!globalDict) {
        return false;
    }
    
    PyObject* func = PyDict_GetItemString(globalDict, name.c_str());
    return func != nullptr && PyCallable_Check(func);
}

std::vector<std::string> PythonInterpreter::listFunctions() const {
    std::vector<std::string> result;
    
    if (!globalDict) {
        return result;
    }
    
    PyObject *key, *value;
    Py_ssize_t pos = 0;
    
    while (PyDict_Next(globalDict, &pos, &key, &value)) {
        if (PyCallable_Check(value)) {
            const char* name = PyUnicode_AsUTF8(key);
            if (name) {
                result.push_back(std::string(name));
            }
        }
    }
    
    return result;
}

PythonFunction::PythonFunction(PythonInterpreter* interp, const std::string& name)
    : interpreter(interp), functionName(name) {
    signature.name = name;
    signature.returnType = ForeignType::Void;
    signature.isVariadic = false;
}

ForeignValue PythonFunction::call(const std::vector<ForeignValue>& args) {
    if (!interpreter) {
        return ForeignValue();
    }
    
    return interpreter->callFunction(functionName, args);
}

ForeignFunctionSignature PythonFunction::getSignature() const {
    return signature;
}

ForeignLanguage PythonFunction::getLanguage() const {
    return ForeignLanguage::Python;
}

PythonValue::PythonValue(PyObject* obj) : pyObject(obj) {
    if (pyObject) {
        Py_INCREF(pyObject);
    }
}

PythonValue::~PythonValue() {
    if (pyObject) {
        Py_DECREF(pyObject);
    }
}

ForeignValue PythonValue::toForeignValue() const {
    if (!pyObject) {
        return ForeignValue();
    }
    
    if (PyLong_Check(pyObject)) {
        long value = PyLong_AsLong(pyObject);
        return ForeignValue(static_cast<int64_t>(value));
    }
    
    if (PyFloat_Check(pyObject)) {
        double value = PyFloat_AsDouble(pyObject);
        return ForeignValue(value);
    }
    
    if (PyBool_Check(pyObject)) {
        bool value = pyObject == Py_True;
        return ForeignValue(value);
    }
    
    if (PyUnicode_Check(pyObject)) {
        const char* str = PyUnicode_AsUTF8(pyObject);
        if (str) {
            return ForeignValue(std::string(str));
        }
    }
    
    if (PyList_Check(pyObject) || PyTuple_Check(pyObject)) {
        std::vector<ForeignValue> arr;
        Py_ssize_t size = PySequence_Size(pyObject);
        
        for (Py_ssize_t i = 0; i < size; i++) {
            PyObject* item = PySequence_GetItem(pyObject, i);
            if (item) {
                arr.push_back(PythonValue(item).toForeignValue());
                Py_DECREF(item);
            }
        }
        
        return ForeignValue(arr);
    }
    
    return ForeignValue();
}

PythonValue PythonValue::fromForeignValue(const ForeignValue& value) {
    PyObject* obj = nullptr;
    
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
            obj = PyLong_FromLong(static_cast<long>(intVal));
            break;
        }
        case ForeignType::Float32:
        case ForeignType::Float64: {
            double floatVal = *value.get<double>();
            obj = PyFloat_FromDouble(floatVal);
            break;
        }
        case ForeignType::Bool: {
            bool boolVal = *value.get<bool>();
            obj = boolVal ? Py_True : Py_False;
            Py_INCREF(obj);
            break;
        }
        case ForeignType::String: {
            const std::string& strVal = *value.get<std::string>();
            obj = PyUnicode_FromString(strVal.c_str());
            break;
        }
        case ForeignType::Array: {
            const std::vector<ForeignValue>& arr = *value.get<std::vector<ForeignValue>>();
            obj = PyList_New(arr.size());
            for (size_t i = 0; i < arr.size(); i++) {
                PyObject* item = PythonValue::fromForeignValue(arr[i]).getPyObject();
                PyList_SetItem(obj, i, item);
            }
            break;
        }
        default:
            obj = Py_None;
            Py_INCREF(obj);
            break;
    }
    
    return PythonValue(obj);
}

PythonLibrary::PythonLibrary(const std::string& name) 
    : ForeignLibrary(name, ForeignLanguage::Python) {
}

PythonLibrary::~PythonLibrary() {
    unload();
}

bool PythonLibrary::load() {
    if (isLoaded()) {
        return true;
    }
    
    interpreter = std::make_unique<PythonInterpreter>();
    if (!interpreter->initialize()) {
        interpreter.reset();
        return false;
    }
    
    return true;
}

void PythonLibrary::unload() {
    if (interpreter) {
        interpreter->shutdown();
        interpreter.reset();
    }
}

std::shared_ptr<ForeignFunction> PythonLibrary::getFunction(const std::string& name) {
    if (!interpreter || !interpreter->hasFunction(name)) {
        return nullptr;
    }
    
    auto func = std::make_shared<PythonFunction>(interpreter.get(), name);
    return func;
}

}
}
