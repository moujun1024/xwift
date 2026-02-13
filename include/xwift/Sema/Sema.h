#ifndef XWIFT_SEMA_SEMA_H
#define XWIFT_SEMA_SEMA_H

#include "xwift/Basic/LLVM.h"
#include "xwift/AST/Nodes.h"
#include "xwift/AST/Type.h"
#include "xwift/Basic/Diagnostic.h"
#include "xwift/Lexer/Token.h"
#include <map>
#include <memory>
#include <set>

namespace xwift {

class Sema {
public:
  Sema(DiagnosticEngine& diag);
  
  void setFilename(const std::string& filename) {
    currentFilename = filename;
  }
  
  bool visit(Program* prog);
  bool visit(Decl* decl);
  bool visit(FuncDecl* func);
  bool visit(VarDeclStmt* var);
  bool visit(Expr* expr);
  bool visit(Stmt* stmt);
  bool visit(ReturnStmt* ret);
  bool visit(IfStmt* ifStmt);
  bool visit(WhileStmt* whileStmt);
  bool visit(ForStmt* forStmt);
  bool visit(SwitchStmt* switchStmt);
  bool visit(BlockStmt* block);
  bool visit(CallExpr* call);
  bool visit(BinaryExpr* binary);
  bool visit(AssignExpr* assign);
  bool visit(IdentifierExpr* ident);
  bool visit(IntegerLiteralExpr* lit);
  bool visit(BoolLiteralExpr* lit);
  bool visit(FloatLiteralExpr* lit);
  bool visit(StringLiteralExpr* lit);
  bool visit(ArrayLiteralExpr* lit);
  bool visit(ArrayIndexExpr* expr);
  
  std::shared_ptr<Type> getExprType(Expr* expr);
  bool isTypeCompatible(std::shared_ptr<Type> from, std::shared_ptr<Type> to);
  
private:
  DiagnosticEngine& Diags;
  std::vector<std::map<std::string, std::shared_ptr<Type>>> ScopeStack;
  std::map<std::string, FuncDecl*> FunctionTable;
  std::set<std::string> BuiltinFunctions;
  std::shared_ptr<Type> CurrentFuncReturnType;
  std::string currentFilename;
  
  void initBuiltinFunctions();
  bool isBuiltinFunction(const std::string& name);
  
  void enterScope();
  void exitScope();
  std::shared_ptr<Type> lookupType(const std::string& name);
  bool addSymbol(const std::string& name, std::shared_ptr<Type> type);
  std::shared_ptr<Type> lookupSymbol(const std::string& name);
  int getIntegerBitWidth(BuiltinType::Kind kind);
};

}

#endif
