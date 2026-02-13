#ifndef XWIFT_INTERPRETER_INTERPRETER_H
#define XWIFT_INTERPRETER_INTERPRETER_H

#include "xwift/AST/Nodes.h"
#include "xwift/Basic/Diagnostic.h"
#include "xwift/Lexer/Lexer.h"
#include "xwift/Parser/SyntaxParser.h"
#include "xwift/stdlib/HTTP/HTTP.h"
#include "xwift/stdlib/JSON/JSON.h"
#include <map>
#include <iostream>
#include <sstream>
#include <fstream>
#include <variant>
#include <vector>
#include <set>
#include <cstdio>
#include <thread>
#include <chrono>

namespace xwift {

class Value {
private:
  std::variant<int64_t, double, std::string, bool, std::vector<Value>> data;
  
public:
  Value() : data(int64_t(0)) {}
  Value(int64_t val) : data(val) {}
  Value(double val) : data(val) {}
  Value(const std::string& val) : data(val) {}
  Value(bool val) : data(val) {}
  Value(const std::vector<Value>& val) : data(val) {}
  
  template<typename T>
  T* get() {
    return std::get_if<T>(&data);
  }
  
  template<typename T>
  const T* get() const {
    return std::get_if<T>(&data);
  }
  
  const auto& getData() const {
    return data;
  }
  
  bool operator==(const Value& other) const {
    return data == other.data;
  }
  
  bool operator!=(const Value& other) const {
    return data != other.data;
  }
};

std::string httpGet(const std::string& url);
std::string httpPost(const std::string& url, const std::string& data);
std::string httpPut(const std::string& url, const std::string& data);
std::string httpDelete(const std::string& url);
int httpStatusCode(const std::string& url);
std::string urlEncode(const std::string& str);
std::string urlDecode(const std::string& str);
std::string jsonParse(const std::string& jsonStr);
bool jsonHasKey(const std::string& jsonStr, const std::string& key);
std::string jsonGet(const std::string& jsonStr, const std::string& key);

class Interpreter {
public:
  std::vector<std::map<std::string, Value>> ScopeStack;
  std::map<std::string, std::function<Value(std::vector<Value>)>> Functions;
  std::map<std::string, FuncDecl*> UserFunctions;
  std::vector<std::unique_ptr<Program>> LoadedPrograms;
  size_t MaxSteps = 100000;
  size_t CurrentStep = 0;
  bool HasReturn = false;
  Value ReturnValue;
  
  void enterScope() {
    ScopeStack.push_back(std::map<std::string, Value>());
  }
  
  void exitScope() {
    if (!ScopeStack.empty()) {
      ScopeStack.pop_back();
    }
  }
  
  void setVariable(const std::string& name, const Value& value) {
    for (auto it = ScopeStack.rbegin(); it != ScopeStack.rend(); ++it) {
      auto varIt = it->find(name);
      if (varIt != it->end()) {
        varIt->second = value;
        return;
      }
    }
    if (!ScopeStack.empty()) {
      ScopeStack.back()[name] = value;
    }
  }
  
  Value* getVariable(const std::string& name) {
    for (auto it = ScopeStack.rbegin(); it != ScopeStack.rend(); ++it) {
      auto varIt = it->find(name);
      if (varIt != it->end()) {
        return &varIt->second;
      }
    }
    return nullptr;
  }
  
  Interpreter() {
    Functions["setCursor"] = [](std::vector<Value> args) -> Value {
      return Value(int64_t(0));
    };
    
    Functions["clearLine"] = [](std::vector<Value> args) -> Value {
      return Value(int64_t(0));
    };
    
    Functions["print"] = [](std::vector<Value> args) -> Value {
      for (size_t i = 0; i < args.size(); i++) {
        std::string output;
        if (auto val = args[i].get<std::string>()) {
          output = *val;
        } else if (auto val = args[i].get<int64_t>()) {
          output = std::to_string(*val);
        } else if (auto val = args[i].get<double>()) {
          output = std::to_string(*val);
        } else if (auto val = args[i].get<bool>()) {
          output = (*val ? "true" : "false");
        } else if (auto arr = args[i].get<std::vector<Value>>()) {
          output = "[";
          for (size_t j = 0; j < arr->size(); j++) {
            if (auto v = (*arr)[j].get<std::string>()) {
              output += "\"" + *v + "\"";
            } else if (auto v = (*arr)[j].get<int64_t>()) {
              output += std::to_string(*v);
            } else if (auto v = (*arr)[j].get<double>()) {
              output += std::to_string(*v);
            } else if (auto v = (*arr)[j].get<bool>()) {
              output += (*v ? "true" : "false");
            } else if (auto v = (*arr)[j].get<std::vector<Value>>()) {
              output += "[...]";
            }
            if (j < arr->size() - 1) output += ", ";
          }
          output += "]";
        }
        if (i < args.size() - 1) output += " ";
        
        std::cout << output << std::flush;
      }
      return Value(int64_t(0));
    };
    
    Functions["println"] = [this](std::vector<Value> args) -> Value {
      for (size_t i = 0; i < args.size(); i++) {
        if (auto val = args[i].get<std::string>()) {
          std::cout << *val;
        } else if (auto val = args[i].get<int64_t>()) {
          std::cout << *val;
        } else if (auto val = args[i].get<double>()) {
          std::cout << *val;
        } else if (auto val = args[i].get<bool>()) {
          std::cout << (*val ? "true" : "false");
        } else if (auto arr = args[i].get<std::vector<Value>>()) {
          std::cout << "[";
          for (size_t j = 0; j < arr->size(); j++) {
            if (auto v = (*arr)[j].get<std::string>()) {
              std::cout << "\"" << *v << "\"";
            } else if (auto v = (*arr)[j].get<int64_t>()) {
              std::cout << *v;
            } else if (auto v = (*arr)[j].get<double>()) {
              std::cout << *v;
            } else if (auto v = (*arr)[j].get<bool>()) {
              std::cout << (*v ? "true" : "false");
            } else if (auto v = (*arr)[j].get<std::vector<Value>>()) {
              std::cout << "[...]";
            }
            if (j < arr->size() - 1) std::cout << ", ";
          }
          std::cout << "]";
        }
        if (i < args.size() - 1) std::cout << " ";
      }
      std::cout << std::endl;
      return Value(int64_t(0));
    };
    
    Functions["read"] = [this](std::vector<Value> args) -> Value {
      std::string input;
      std::getline(std::cin, input);
      return Value(input);
    };
    
    Functions["readInt"] = [this](std::vector<Value> args) -> Value {
      std::string input;
      std::getline(std::cin, input);
      try {
        return Value(std::stoll(input));
      } catch (...) {
        return Value(int64_t(0));
      }
    };
    
    Functions["sleep"] = [this](std::vector<Value> args) -> Value {
      if (args.empty()) return Value(int64_t(0));
      if (auto ms = args[0].get<int64_t>()) {
        std::this_thread::sleep_for(std::chrono::milliseconds(*ms));
      }
      return Value(int64_t(0));
    };
    
    Functions["httpGet"] = [this](std::vector<Value> args) -> Value {
      if (args.empty()) return Value("");
      if (auto url = args[0].get<std::string>()) {
        return Value(httpGet(*url));
      }
      return Value("");
    };
    
    Functions["httpPost"] = [this](std::vector<Value> args) -> Value {
      if (args.size() < 2) return Value("");
      if (auto url = args[0].get<std::string>()) {
        if (auto data = args[1].get<std::string>()) {
          return Value(httpPost(*url, *data));
        }
      }
      return Value("");
    };
    
    Functions["httpPut"] = [this](std::vector<Value> args) -> Value {
      if (args.size() < 2) return Value("");
      if (auto url = args[0].get<std::string>()) {
        if (auto data = args[1].get<std::string>()) {
          return Value(httpPut(*url, *data));
        }
      }
      return Value("");
    };
    
    Functions["httpDelete"] = [this](std::vector<Value> args) -> Value {
      if (args.empty()) return Value("");
      if (auto url = args[0].get<std::string>()) {
        return Value(httpDelete(*url));
      }
      return Value("");
    };
    
    Functions["httpStatusCode"] = [this](std::vector<Value> args) -> Value {
      if (args.empty()) return Value(int64_t(0));
      if (auto url = args[0].get<std::string>()) {
        return Value(int64_t(httpStatusCode(*url)));
      }
      return Value(int64_t(0));
    };
    
    Functions["urlEncode"] = [this](std::vector<Value> args) -> Value {
      if (args.empty()) return Value("");
      if (auto str = args[0].get<std::string>()) {
        return Value(http::urlEncode(*str));
      }
      return Value("");
    };
    
    Functions["urlDecode"] = [this](std::vector<Value> args) -> Value {
      if (args.empty()) return Value("");
      if (auto str = args[0].get<std::string>()) {
        return Value(http::urlDecode(*str));
      }
      return Value("");
    };
    
    Functions["len"] = [this](std::vector<Value> args) -> Value {
      if (args.empty()) return Value(int64_t(0));
      if (auto s = args[0].get<std::string>()) {
        return Value(int64_t(s->length()));
      }
      if (auto arr = args[0].get<std::vector<Value>>()) {
        return Value(int64_t(arr->size()));
      }
      return Value(int64_t(0));
    };
    
    Functions["append"] = [this](std::vector<Value> args) -> Value {
      if (args.size() < 2) return Value(int64_t(0));
      if (auto arr = args[0].get<std::vector<Value>>()) {
        std::vector<Value> newArr = *arr;
        newArr.push_back(args[1]);
        return Value(newArr);
      }
      return Value(int64_t(0));
    };
    
    Functions["remove"] = [this](std::vector<Value> args) -> Value {
      if (args.size() < 2) return Value(int64_t(0));
      if (auto arr = args[0].get<std::vector<Value>>()) {
        std::vector<Value> newArr = *arr;
        if (auto idx = args[1].get<int64_t>()) {
          if (*idx >= 0 && *idx < (int64_t)newArr.size()) {
            newArr.erase(newArr.begin() + *idx);
          }
        }
        return Value(newArr);
      }
      return Value(int64_t(0));
    };
    
    Functions["get"] = [this](std::vector<Value> args) -> Value {
      if (args.size() < 2) return Value(int64_t(0));
      if (auto arr = args[0].get<std::vector<Value>>()) {
        if (auto idx = args[1].get<int64_t>()) {
          if (*idx >= 0 && *idx < (int64_t)arr->size()) {
            return (*arr)[*idx];
          }
        }
      }
      return Value(int64_t(0));
    };
    
    Functions["set"] = [this](std::vector<Value> args) -> Value {
      if (args.size() < 3) return Value(int64_t(0));
      if (auto arr = args[0].get<std::vector<Value>>()) {
        std::vector<Value> newArr = *arr;
        if (auto idx = args[1].get<int64_t>()) {
          if (*idx >= 0 && *idx < (int64_t)newArr.size()) {
            newArr[*idx] = args[2];
          }
        }
        return Value(newArr);
      }
      return Value(int64_t(0));
    };
    
    Functions["contains"] = [this](std::vector<Value> args) -> Value {
      if (args.size() < 2) return Value(false);
      if (auto arr = args[0].get<std::vector<Value>>()) {
        for (const auto& item : *arr) {
          if (auto s1 = item.get<std::string>()) {
            if (auto s2 = args[1].get<std::string>()) {
              if (*s1 == *s2) return Value(true);
            }
          } else if (auto i1 = item.get<int64_t>()) {
            if (auto i2 = args[1].get<int64_t>()) {
              if (*i1 == *i2) return Value(true);
            }
          } else if (auto d1 = item.get<double>()) {
            if (auto d2 = args[1].get<double>()) {
              if (*d1 == *d2) return Value(true);
            }
          } else if (auto b1 = item.get<bool>()) {
            if (auto b2 = args[1].get<bool>()) {
              if (*b1 == *b2) return Value(true);
            }
          }
        }
      }
      return Value(false);
    };
    
    Functions["indexOf"] = [this](std::vector<Value> args) -> Value {
      if (args.size() < 2) return Value(int64_t(-1));
      if (auto arr = args[0].get<std::vector<Value>>()) {
        int64_t index = 0;
        for (const auto& item : *arr) {
          if (auto s1 = item.get<std::string>()) {
            if (auto s2 = args[1].get<std::string>()) {
              if (*s1 == *s2) return Value(index);
            }
          } else if (auto i1 = item.get<int64_t>()) {
            if (auto i2 = args[1].get<int64_t>()) {
              if (*i1 == *i2) return Value(index);
            }
          } else if (auto d1 = item.get<double>()) {
            if (auto d2 = args[1].get<double>()) {
              if (*d1 == *d2) return Value(index);
            }
          } else if (auto b1 = item.get<bool>()) {
            if (auto b2 = args[1].get<bool>()) {
              if (*b1 == *b2) return Value(index);
            }
          }
          index++;
        }
      }
      return Value(int64_t(-1));
    };
    
    Functions["toString"] = [this](std::vector<Value> args) -> Value {
      if (args.empty()) return Value(std::string(""));
      if (auto i = args[0].get<int64_t>()) {
        return Value(std::to_string(*i));
      }
      if (auto d = args[0].get<double>()) {
        return Value(std::to_string(*d));
      }
      if (auto b = args[0].get<bool>()) {
        return Value(std::string(*b ? "true" : "false"));
      }
      return Value(std::string(""));
    };
    
    Functions["toInt"] = [this](std::vector<Value> args) -> Value {
      if (args.empty()) return Value(int64_t(0));
      if (auto s = args[0].get<std::string>()) {
        try {
          return Value(std::stoll(*s));
        } catch (...) {
          return Value(int64_t(0));
        }
      }
      return Value(int64_t(0));
    };
    
    Functions["find"] = [this](std::vector<Value> args) -> Value {
      if (args.size() < 2) return Value(int64_t(-1));
      if (auto str = args[0].get<std::string>()) {
        if (auto substr = args[1].get<std::string>()) {
          size_t startPos = 0;
          if (args.size() >= 3) {
            if (auto start = args[2].get<int64_t>()) {
              if (*start >= 0) {
                startPos = (size_t)*start;
              }
            }
          }
          size_t pos = str->find(*substr, startPos);
          if (pos != std::string::npos) {
            return Value(int64_t(pos));
          }
          return Value(int64_t(-1));
        }
      }
      return Value(int64_t(-1));
    };
    
    Functions["substring"] = [this](std::vector<Value> args) -> Value {
      if (args.size() < 2) return Value("");
      if (auto str = args[0].get<std::string>()) {
        if (auto start = args[1].get<int64_t>()) {
          if (*start >= 0 && *start < (int64_t)str->length()) {
            if (args.size() >= 3) {
              if (auto len = args[2].get<int64_t>()) {
                if (*len > 0) {
                  return Value(str->substr(*start, *len));
                }
              }
            } else {
              return Value(str->substr(*start));
            }
          }
        }
      }
      return Value("");
    };
    
    Functions["jsonParse"] = [this](std::vector<Value> args) -> Value {
      if (args.empty()) return Value("");
      if (auto jsonStr = args[0].get<std::string>()) {
        json::JSONParser parser;
        json::JSONValue result = parser.parse(*jsonStr);
        if (parser.hasError()) {
          return Value("");
        }
        return Value(result.toString());
      }
      return Value("");
    };
    
    Functions["jsonGet"] = [this](std::vector<Value> args) -> Value {
      if (args.size() < 2) return Value("");
      if (auto jsonStr = args[0].get<std::string>()) {
        if (auto key = args[1].get<std::string>()) {
          json::JSONParser parser;
          json::JSONValue result = parser.parse(*jsonStr);
          if (parser.hasError()) {
            return Value("");
          }
          if (result.has(*key)) {
            json::JSONValue value = result.get(*key);
            if (value.getType() == json::JSONType::String) {
              if (auto strVal = value.asString()) {
                return Value(*strVal);
              }
            }
            return Value(value.toString());
          }
        }
      }
      return Value("");
    };
    
    Functions["jsonHasKey"] = [this](std::vector<Value> args) -> Value {
      if (args.size() < 2) return Value(false);
      if (auto jsonStr = args[0].get<std::string>()) {
        if (auto key = args[1].get<std::string>()) {
          json::JSONParser parser;
          json::JSONValue result = parser.parse(*jsonStr);
          if (parser.hasError()) {
            return Value(false);
          }
          return Value(result.has(*key));
        }
      }
      return Value(false);
    };
    
    Functions["split"] = [](std::vector<Value> args) -> Value {
      if (args.size() < 2) return Value(std::vector<Value>());
      if (auto str = args[0].get<std::string>()) {
        if (auto separator = args[1].get<std::string>()) {
          std::vector<Value> result;
          std::string current;
          for (char c : *str) {
            if (c == (*separator)[0]) {
              result.push_back(Value(current));
              current.clear();
            } else {
              current += c;
            }
          }
          result.push_back(Value(current));
          return Value(result);
        }
      }
      return Value(std::vector<Value>());
    };
    
    Functions["trim"] = [](std::vector<Value> args) -> Value {
      if (args.size() < 1) return Value("");
      if (auto str = args[0].get<std::string>()) {
        size_t start = str->find_first_not_of(" \t\n\r");
        if (start == std::string::npos) return Value("");
        size_t end = str->find_last_not_of(" \t\n\r");
        return Value(str->substr(start, end - start + 1));
      }
      return Value("");
    };
    
    Functions["join"] = [](std::vector<Value> args) -> Value {
      if (args.size() < 2) return Value("");
      if (auto arr = args[0].get<std::vector<Value>>()) {
        if (auto separator = args[1].get<std::string>()) {
          std::string result;
          for (size_t i = 0; i < arr->size(); i++) {
            if (auto str = (*arr)[i].get<std::string>()) {
              result += *str;
            }
            if (i < arr->size() - 1) {
              result += *separator;
            }
          }
          return Value(result);
        }
      }
      return Value("");
    };
  }
  
  void run(Program* program, const std::string& basePath = ".") {
    BasePath = basePath;
    CurrentStep = 0;
    enterScope();
    for (auto& decl : program->Declarations) {
      runDecl(decl.get());
      CurrentStep++;
      if (CurrentStep > MaxSteps) {
        throw std::runtime_error("Execution timeout: infinite loop detected");
      }
    }
    exitScope();
  }
  
  void setBasePath(const std::string& path) { BasePath = path; }
  
private:
  std::string BasePath = ".";
  void runDecl(Decl* decl) {
    if (auto importDecl = dynamic_cast<ImportDecl*>(decl)) {
      loadModule(importDecl->ModuleName);
    } else if (auto funcDecl = dynamic_cast<FuncDecl*>(decl)) {
      UserFunctions[funcDecl->Name] = funcDecl;
      if (funcDecl->Name == "main") {
        if (funcDecl->Body) {
          auto* block = dynamic_cast<BlockStmt*>(funcDecl->Body.get());
          if (block) {
            runBlock(block);
          }
        }
      }
    } else if (auto classDecl = dynamic_cast<ClassDecl*>(decl)) {
      runClassDecl(classDecl);
    }
  }
  
  void loadModule(const std::string& moduleName) {
    auto it = LoadedModules.find(moduleName);
    if (it != LoadedModules.end()) {
      return;
    }
    
    LoadedModules.insert(moduleName);
    
    std::vector<std::string> searchPaths = {
      BasePath + "/lib/" + moduleName + ".xw",
      BasePath + "/" + moduleName + ".xw", 
      BasePath + "/test/" + moduleName + ".xw",
      "lib/" + moduleName + ".xw",
      moduleName + ".xw"
    };
    
    std::string filename;
    std::ifstream file;
    for (const auto& path : searchPaths) {
      file.open(path);
      if (file.is_open()) {
        filename = path;
        break;
      }
    }
    
    if (!file.is_open()) {
      return;
    }
    
    std::stringstream buffer;
    buffer << file.rdbuf();
    std::string source = buffer.str();
    
    Lexer lexer(source);
    SyntaxParser parser(lexer);
    auto program = parser.parseProgram();
    
    LoadedPrograms.push_back(std::move(program));
    
    for (auto& decl : LoadedPrograms.back()->Declarations) {
      runDecl(decl.get());
    }
  }
  
  std::set<std::string> LoadedModules;
  
  void runClassDecl(ClassDecl* cls) {
    for (auto& member : cls->Members) {
      runDecl(member.get());
    }
  }
  
  void runBlock(BlockStmt* block, Value* retVal = nullptr) {
    enterScope();
    for (auto& stmt : block->Statements) {
      if (!stmt) continue;
      
      if (auto ret = dynamic_cast<ReturnStmt*>(stmt.get())) {
        if (retVal && ret->Value) {
          *retVal = evaluate(ret->Value.get());
        }
        HasReturn = true;
        exitScope();
        return;
      }
      runStmt(stmt.get(), retVal);
      if (HasReturn) {
        exitScope();
        return;
      }
    }
    exitScope();
  }
  
  void runStmt(Stmt* stmt, Value* retVal = nullptr) {
    if (!stmt) return;
    
    CurrentStep++;
    if (CurrentStep > MaxSteps) {
      throw std::runtime_error("Execution timeout: infinite loop detected");
    }
    
    if (auto ret = dynamic_cast<ReturnStmt*>(stmt)) {
      return;
    }
    
    if (auto varDecl = dynamic_cast<VarDeclStmt*>(stmt)) {
      if (varDecl->Init) {
        Value val = evaluate(varDecl->Init.get());
        setVariable(varDecl->Name, val);
      }
      return;
    }
    
    if (auto ifStmt = dynamic_cast<IfStmt*>(stmt)) {
      Value cond = evaluate(ifStmt->Condition.get());
      bool condBool = false;
      if (auto boolVal = cond.get<bool>()) {
        condBool = *boolVal;
      } else if (auto intVal = cond.get<int64_t>()) {
        condBool = *intVal != 0;
      } else if (auto doubleVal = cond.get<double>()) {
        condBool = *doubleVal != 0.0;
      } else if (auto strVal = cond.get<std::string>()) {
        condBool = !strVal->empty();
      }
      
      if (condBool) {
        if (ifStmt->ThenBranch) {
          runStmt(ifStmt->ThenBranch.get(), retVal);
        }
      } else if (ifStmt->ElseBranch) {
        runStmt(ifStmt->ElseBranch.get(), retVal);
      }
      return;
    }
    
    if (auto whileStmt = dynamic_cast<WhileStmt*>(stmt)) {
      while (true) {
        CurrentStep++;
        if (CurrentStep > MaxSteps) {
          throw std::runtime_error("Execution timeout: infinite loop detected");
        }
        
        Value cond = evaluate(whileStmt->Condition.get());
        bool condBool = false;
        if (auto boolVal = cond.get<bool>()) {
          condBool = *boolVal;
        } else if (auto intVal = cond.get<int64_t>()) {
          condBool = *intVal != 0;
        } else if (auto doubleVal = cond.get<double>()) {
          condBool = *doubleVal != 0.0;
        } else if (auto strVal = cond.get<std::string>()) {
          condBool = !strVal->empty();
        }
        
        if (!condBool) break;
        
        if (whileStmt->Body) {
          runStmt(whileStmt->Body.get(), retVal);
        }
      }
      return;
    }
    
    if (auto forStmt = dynamic_cast<ForStmt*>(stmt)) {
      Value startVal = evaluate(forStmt->Start.get());
      Value endVal = evaluate(forStmt->End.get());
      Value stepVal = evaluate(forStmt->Step.get());
      
      auto getInt = [](const Value& v) -> int64_t {
        if (auto i = v.get<int64_t>()) return *i;
        if (auto d = v.get<double>()) return (int64_t)*d;
        return 0;
      };
      
      int64_t start = getInt(startVal);
      int64_t end = getInt(endVal);
      int64_t step = getInt(stepVal);
      
      if (step == 0) {
        throw std::runtime_error("For loop step cannot be zero");
      }
      
      enterScope();
      for (int64_t i = start; (step > 0 ? i < end : i > end); i += step) {
        CurrentStep++;
        if (CurrentStep > MaxSteps) {
          throw std::runtime_error("Execution timeout: infinite loop detected");
        }
        
        setVariable(forStmt->VarName, Value(i));
        if (forStmt->Body) {
          runStmt(forStmt->Body.get(), retVal);
        }
      }
      exitScope();
      return;
    }
    
    if (auto switchStmt = dynamic_cast<SwitchStmt*>(stmt)) {
      Value condVal = evaluate(switchStmt->Condition.get());
      
      bool matched = false;
      for (auto& casePair : switchStmt->Cases) {
        auto& patterns = casePair.first;
        auto& body = casePair.second;
        
        if (patterns.empty()) {
          if (body) {
            runStmt(body.get(), retVal);
            matched = true;
          }
          break;
        }
        
        for (auto& pattern : patterns) {
          Value patternVal = evaluate(pattern.get());
          if (condVal == patternVal) {
            if (body) {
              runStmt(body.get(), retVal);
              matched = true;
            }
            break;
          }
        }
        if (matched) break;
      }
      return;
    }
    
    if (auto block = dynamic_cast<BlockStmt*>(stmt)) {
      runBlock(block, retVal);
      return;
    }
    
    if (auto expr = dynamic_cast<Expr*>(stmt)) {
      evaluate(expr);
      return;
    }
  }
  
  Value evaluate(Expr* expr) {
    if (!expr) return Value(int64_t(0));
    
    if (auto lit = dynamic_cast<IntegerLiteralExpr*>(expr)) {
      return Value(lit->Value);
    }
    
    if (auto bl = dynamic_cast<BoolLiteralExpr*>(expr)) {
      return Value(bl->Value);
    }
    
    if (auto flt = dynamic_cast<FloatLiteralExpr*>(expr)) {
      return Value(flt->Value);
    }
    
    if (auto str = dynamic_cast<StringLiteralExpr*>(expr)) {
      return Value(str->Value);
    }
    
    if (auto arr = dynamic_cast<ArrayLiteralExpr*>(expr)) {
      std::vector<Value> elements;
      for (auto& elem : arr->Elements) {
        elements.push_back(evaluate(elem.get()));
      }
      return Value(elements);
    }
    
    if (auto id = dynamic_cast<IdentifierExpr*>(expr)) {
      auto var = getVariable(id->Name);
      if (var) {
        return *var;
      }
      
      auto fnIt = Functions.find(id->Name);
      if (fnIt != Functions.end()) {
        return Value(int64_t(0));
      }
      
      throw diag::undefinedVariable(id->Name);
    }
    
    if (auto arrIdx = dynamic_cast<ArrayIndexExpr*>(expr)) {
      Value arrayVal = evaluate(arrIdx->Array.get());
      Value indexVal = evaluate(arrIdx->Index.get());
      
      if (auto arr = arrayVal.get<std::vector<Value>>()) {
        if (auto idx = indexVal.get<int64_t>()) {
          if (*idx >= 0 && *idx < (int64_t)arr->size()) {
            return (*arr)[*idx];
          }
        }
      }
      
      return Value(int64_t(0));
    }
    
    if (auto assign = dynamic_cast<AssignExpr*>(expr)) {
      Value rhs = evaluate(assign->Value.get());
      
      if (auto id = dynamic_cast<IdentifierExpr*>(assign->Target.get())) {
        setVariable(id->Name, rhs);
      }
      
      return rhs;
    }
    
    if (auto binary = dynamic_cast<BinaryExpr*>(expr)) {
      Value lhs = evaluate(binary->LHS.get());
      Value rhs = evaluate(binary->RHS.get());
      
      if (binary->Op == "+") {
        if (auto l = lhs.get<std::string>()) {
          if (auto r = rhs.get<std::string>()) {
            return Value(*l + *r);
          }
        }
        if (auto l = lhs.get<int64_t>()) {
          if (auto r = rhs.get<int64_t>()) {
            return Value(*l + *r);
          }
        }
      }
      if (binary->Op == "-") {
        if (auto l = lhs.get<int64_t>()) {
          if (auto r = rhs.get<int64_t>()) {
            return Value(*l - *r);
          }
        }
      }
      if (binary->Op == "*") {
        if (auto l = lhs.get<int64_t>()) {
          if (auto r = rhs.get<int64_t>()) {
            return Value(*l * *r);
          }
        }
      }
      if (binary->Op == "/") {
        if (auto l = lhs.get<int64_t>()) {
          if (auto r = rhs.get<int64_t>()) {
            return Value(*r != 0 ? *l / *r : 0);
          }
        }
      }
      if (binary->Op == "==") {
        return Value(lhs == rhs);
      }
      if (binary->Op == "!=") {
        return Value(lhs != rhs);
      }
      if (binary->Op == "<") {
        if (auto l = lhs.get<int64_t>()) {
          if (auto r = rhs.get<int64_t>()) {
            return Value(*l < *r);
          }
        }
      }
      if (binary->Op == ">") {
        if (auto l = lhs.get<int64_t>()) {
          if (auto r = rhs.get<int64_t>()) {
            return Value(*l > *r);
          }
        }
      }
      if (binary->Op == "<=") {
        if (auto l = lhs.get<int64_t>()) {
          if (auto r = rhs.get<int64_t>()) {
            return Value(*l <= *r);
          }
        }
      }
      if (binary->Op == ">=") {
        if (auto l = lhs.get<int64_t>()) {
          if (auto r = rhs.get<int64_t>()) {
            return Value(*l >= *r);
          }
        }
      }
      if (binary->Op == "&&") {
        return Value(lhs.getData().index() == 3 && rhs.getData().index() == 3 && std::get<bool>(lhs.getData()) && std::get<bool>(rhs.getData()));
      }
      if (binary->Op == "||") {
        return Value(lhs.getData().index() == 3 && rhs.getData().index() == 3 && (std::get<bool>(lhs.getData()) || std::get<bool>(rhs.getData())));
      }
      
      return Value(int64_t(0));
    }
    
    if (auto call = dynamic_cast<CallExpr*>(expr)) {
      auto it = Functions.find(call->Callee);
      if (it != Functions.end()) {
        std::vector<Value> args;
        for (auto& arg : call->Args) {
          args.push_back(evaluate(arg.get()));
        }
        return it->second(args);
      }
      
      auto userIt = UserFunctions.find(call->Callee);
      if (userIt != UserFunctions.end()) {
        FuncDecl* func = userIt->second;
        if (func->Body) {
          auto* block = dynamic_cast<BlockStmt*>(func->Body.get());
          if (block) {
            bool savedHasReturn = HasReturn;
            HasReturn = false;
            enterScope();
            for (size_t i = 0; i < func->Params.size() && i < call->Args.size(); i++) {
              setVariable(func->Params[i].first, evaluate(call->Args[i].get()));
            }
            Value retVal(int64_t(0));
            runBlock(block, &retVal);
            HasReturn = savedHasReturn;
            exitScope();
            return retVal;
          }
        }
        return Value(int64_t(0));
      }
    }
    
    return Value(int64_t(0));
  }
};

std::string httpGet(const std::string& url) {
  http::HTTPClient client;
  http::Response response = client.get(url);
  return response.body;
}

std::string httpPost(const std::string& url, const std::string& data) {
  http::HTTPClient client;
  http::Response response = client.post(url, data);
  return response.body;
}

std::string httpPut(const std::string& url, const std::string& data) {
  http::HTTPClient client;
  http::Response response = client.put(url, data);
  return response.body;
}

std::string httpDelete(const std::string& url) {
  http::HTTPClient client;
  http::Response response = client.deleteRequest(url);
  return response.body;
}

int httpStatusCode(const std::string& url) {
  http::HTTPClient client;
  http::Response response = client.get(url);
  return response.statusCode;
}

std::string urlEncode(const std::string& str) {
  return http::urlEncode(str);
}

std::string urlDecode(const std::string& str) {
  return http::urlDecode(str);
}

std::string jsonParse(const std::string& jsonStr) {
  json::JSONParser parser;
  json::JSONValue result = parser.parse(jsonStr);
  return result.toString();
}

bool jsonHasKey(const std::string& jsonStr, const std::string& key) {
  json::JSONParser parser;
  parser.parse(jsonStr);
  return parser.has(key);
}

std::string jsonGet(const std::string& jsonStr, const std::string& key) {
  json::JSONParser parser;
  parser.parse(jsonStr);
  return parser.get(key);
}

}

#endif
