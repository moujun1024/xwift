#ifndef XWIFT_AST_DECL_H
#define XWIFT_AST_DECL_H

#include "xwift/Basic/LLVM.h"

namespace xwift {

enum class DeclKind {
  Import,
  Func,
  Var,
  Param,
  Struct,
  Class,
  Enum,
};

class Decl {
public:
  DeclKind Kind;
  
  Decl(DeclKind K) : Kind(K) {}
  virtual ~Decl() = default;
};

class FuncDecl : public Decl {
public:
  StringRef Name;
  
  FuncDecl(StringRef name) : Decl(DeclKind::Func), Name(name) {}
};

}

#endif
