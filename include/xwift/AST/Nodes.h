#ifndef XWIFT_AST_NODES_H
#define XWIFT_AST_NODES_H

#include "xwift/Basic/LLVM.h"
#include "xwift/Lexer/Token.h"
#include "xwift/AST/Type.h"
#include <memory>
#include <vector>
#include <map>

namespace xwift {

class ASTNode {
public:
  virtual ~ASTNode() = default;
};

using ASTNodePtr = std::unique_ptr<ASTNode>;

class Stmt : public ASTNode {
public:
  virtual ~Stmt() = default;
};
using StmtPtr = std::unique_ptr<Stmt>;

class Expr : public Stmt {
public:
  std::shared_ptr<Type> ExprType;
  virtual ~Expr() = default;
};
using ExprPtr = std::unique_ptr<Expr>;

class Decl : public Stmt {
public:
  virtual ~Decl() = default;
};
using DeclPtr = std::unique_ptr<Decl>;

class ImportDecl : public Decl {
public:
  std::string ModuleName;
  ImportDecl(const std::string& name) : ModuleName(name) {}
};

class IntegerLiteralExpr : public Expr {
public:
  int64_t Value;
  SourceLocation Loc;
  IntegerLiteralExpr(int64_t val, SourceLocation loc = SourceLocation()) : Value(val), Loc(loc) {}
};

class FloatLiteralExpr : public Expr {
public:
  double Value;
  SourceLocation Loc;
  FloatLiteralExpr(double val, SourceLocation loc = SourceLocation()) : Value(val), Loc(loc) {}
};

class BoolLiteralExpr : public Expr {
public:
  bool Value;
  SourceLocation Loc;
  BoolLiteralExpr(bool val, SourceLocation loc = SourceLocation()) : Value(val), Loc(loc) {}
};

class StringLiteralExpr : public Expr {
public:
  std::string Value;
  SourceLocation Loc;
  StringLiteralExpr(const std::string& val, SourceLocation loc = SourceLocation()) : Value(val), Loc(loc) {}
};

class ArrayLiteralExpr : public Expr {
public:
  std::vector<ExprPtr> Elements;
  SourceLocation Loc;
  ArrayLiteralExpr(std::vector<ExprPtr> elements, SourceLocation loc = SourceLocation()) 
    : Elements(std::move(elements)), Loc(loc) {}
};

class IdentifierExpr : public Expr {
public:
  std::string Name;
  IdentifierExpr(const std::string& name) : Name(name) {}
};

class AssignExpr : public Expr {
public:
  ExprPtr Target;
  ExprPtr Value;
  AssignExpr(ExprPtr target, ExprPtr value) : Target(std::move(target)), Value(std::move(value)) {}
};

class BinaryExpr : public Expr {
public:
  std::string Op;
  ExprPtr LHS, RHS;
  BinaryExpr(const std::string& op, ExprPtr lhs, ExprPtr rhs)
    : Op(op), LHS(std::move(lhs)), RHS(std::move(rhs)) {}
};

class ArrayIndexExpr : public Expr {
public:
  ExprPtr Array;
  ExprPtr Index;
  ArrayIndexExpr(ExprPtr array, ExprPtr index) : Array(std::move(array)), Index(std::move(index)) {}
};

class CallExpr : public Expr {
public:
  std::string Callee;
  std::vector<ExprPtr> Args;
  CallExpr(const std::string& callee, std::vector<ExprPtr> args)
    : Callee(callee), Args(std::move(args)) {}
};

class VarDeclStmt : public Decl {
public:
  std::string Name;
  std::string Type;
  ExprPtr Init;
  bool IsMutable;
  VarDeclStmt(const std::string& name, const std::string& type, ExprPtr init, bool mut)
    : Name(name), Type(type), Init(std::move(init)), IsMutable(mut) {}
};

class ReturnStmt : public Stmt {
public:
  ExprPtr Value;
  ReturnStmt(ExprPtr val) : Value(std::move(val)) {}
};

class IfStmt : public Stmt {
public:
  ExprPtr Condition;
  StmtPtr ThenBranch;
  StmtPtr ElseBranch;
  IfStmt(ExprPtr cond, StmtPtr thenBranch, StmtPtr elseBranch = nullptr)
    : Condition(std::move(cond)), ThenBranch(std::move(thenBranch)), ElseBranch(std::move(elseBranch)) {}
};

class WhileStmt : public Stmt {
public:
  ExprPtr Condition;
  StmtPtr Body;
  WhileStmt(ExprPtr cond, StmtPtr body)
    : Condition(std::move(cond)), Body(std::move(body)) {}
};

class ForStmt : public Stmt {
public:
  std::string VarName;
  ExprPtr Start;
  ExprPtr End;
  ExprPtr Step;
  StmtPtr Body;
  ForStmt(const std::string& var, ExprPtr start, ExprPtr end, ExprPtr step, StmtPtr body)
    : VarName(var), Start(std::move(start)), End(std::move(end)), 
      Step(std::move(step)), Body(std::move(body)) {}
};

class SwitchStmt : public Stmt {
public:
  ExprPtr Condition;
  std::vector<std::pair<std::vector<ExprPtr>, StmtPtr>> Cases;
  SwitchStmt(ExprPtr cond) : Condition(std::move(cond)) {}
  void addCase(std::vector<ExprPtr> patterns, StmtPtr body) {
    Cases.push_back({std::move(patterns), std::move(body)});
  }
};

class BlockStmt : public Stmt {
public:
  std::vector<StmtPtr> Statements;
  void addStmt(StmtPtr stmt) { Statements.push_back(std::move(stmt)); }
};

class FuncDecl : public Decl {
public:
  std::string Name;
  std::string ReturnType;
  std::vector<std::pair<std::string, std::string>> Params;
  StmtPtr Body;
  FuncDecl(const std::string& name, const std::string& retType, StmtPtr body)
    : Name(name), ReturnType(retType), Body(std::move(body)) {}
  void addParam(const std::string& name, const std::string& type) {
    Params.push_back({name, type});
  }
};

class ClassDecl : public Decl {
public:
  std::string Name;
  std::vector<DeclPtr> Members;
  ClassDecl(const std::string& name) : Name(name) {}
  void addMember(DeclPtr member) { Members.push_back(std::move(member)); }
};

class Program : public ASTNode {
public:
  std::vector<DeclPtr> Declarations;
  void addDecl(DeclPtr decl) { Declarations.push_back(std::move(decl)); }
  std::vector<DeclPtr>& getDecls() { return Declarations; }
};

}

#endif
