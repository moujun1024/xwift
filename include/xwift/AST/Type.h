#ifndef XWIFT_AST_TYPE_H
#define XWIFT_AST_TYPE_H

#include "xwift/Basic/LLVM.h"

namespace xwift {

class Type {
public:
  StringRef Name;
  
  Type() : Name("") {}
  Type(StringRef name) : Name(name) {}
  virtual ~Type() = default;
  
  virtual bool isInteger() const { return false; }
  virtual bool isFloat() const { return false; }
  virtual bool isVoid() const { return false; }
};

class BuiltinType : public Type {
public:
  enum Kind {
    Int,
    Int8,
    Int16,
    Int32,
    Int64,
    UInt,
    UInt8,
    UInt16,
    UInt32,
    UInt64,
    Float,
    Double,
    Void,
    Bool,
    String,
    Any,
  };
  
  Kind TyKind;
  
  BuiltinType(Kind K) : TyKind(K) {
    switch (K) {
      case Int: Name = "Int"; break;
      case Int8: Name = "Int8"; break;
      case Int16: Name = "Int16"; break;
      case Int32: Name = "Int32"; break;
      case Int64: Name = "Int64"; break;
      case UInt: Name = "UInt"; break;
      case UInt8: Name = "UInt8"; break;
      case UInt16: Name = "UInt16"; break;
      case UInt32: Name = "UInt32"; break;
      case UInt64: Name = "UInt64"; break;
      case Float: Name = "Float"; break;
      case Double: Name = "Double"; break;
      case Void: Name = "Void"; break;
      case Bool: Name = "Bool"; break;
      case String: Name = "String"; break;
      case Any: Name = "Any"; break;
    }
  }
  
  bool isInteger() const override {
    return TyKind >= Int && TyKind <= UInt64;
  }
  
  bool isFloat() const override {
    return TyKind == Float || TyKind == Double;
  }
  
  bool isVoid() const override {
    return TyKind == Void;
  }
};

}

#endif
