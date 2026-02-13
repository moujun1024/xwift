#include "xwift/Sema/Sema.h"
#include <iostream>

namespace xwift {

Sema::Sema(DiagnosticEngine& diag) : Diags(diag) {
  initBuiltinFunctions();
}

void Sema::initBuiltinFunctions() {
  BuiltinFunctions.insert("print");
  BuiltinFunctions.insert("println");
  BuiltinFunctions.insert("read");
  BuiltinFunctions.insert("readInt");
  BuiltinFunctions.insert("len");
  BuiltinFunctions.insert("toString");
  BuiltinFunctions.insert("toInt");
  BuiltinFunctions.insert("split");
  BuiltinFunctions.insert("join");
  BuiltinFunctions.insert("append");
  BuiltinFunctions.insert("trim");
  BuiltinFunctions.insert("httpGet");
  BuiltinFunctions.insert("httpPost");
  BuiltinFunctions.insert("httpPut");
  BuiltinFunctions.insert("httpDelete");
  BuiltinFunctions.insert("httpStatusCode");
  BuiltinFunctions.insert("urlEncode");
  BuiltinFunctions.insert("urlDecode");
  BuiltinFunctions.insert("jsonParse");
  BuiltinFunctions.insert("jsonHasKey");
  BuiltinFunctions.insert("jsonGet");
  BuiltinFunctions.insert("jsonGetString");
  BuiltinFunctions.insert("jsonGetInt");
  BuiltinFunctions.insert("jsonGetDouble");
  BuiltinFunctions.insert("jsonGetBool");
  BuiltinFunctions.insert("jsonGetArray");
  BuiltinFunctions.insert("jsonGetObject");
  BuiltinFunctions.insert("jsonToString");
}

bool Sema::isBuiltinFunction(const std::string& name) {
  return BuiltinFunctions.find(name) != BuiltinFunctions.end();
}

std::shared_ptr<Type> Sema::lookupType(const std::string& name) {
  if (name.empty()) {
    return nullptr;
  }
  
  bool isOptional = false;
  std::string baseName = name;
  
  if (name.back() == '?') {
    isOptional = true;
    baseName = name.substr(0, name.length() - 1);
  }
  
  std::shared_ptr<Type> baseType;
  
  if (baseName == "Int" || baseName == "Int64") {
    baseType = std::make_shared<BuiltinType>(BuiltinType::Int64);
  }
  else if (baseName == "Int32") {
    baseType = std::make_shared<BuiltinType>(BuiltinType::Int32);
  }
  else if (baseName == "Int16") {
    baseType = std::make_shared<BuiltinType>(BuiltinType::Int16);
  }
  else if (baseName == "Int8") {
    baseType = std::make_shared<BuiltinType>(BuiltinType::Int8);
  }
  else if (baseName == "UInt" || baseName == "UInt64") {
    baseType = std::make_shared<BuiltinType>(BuiltinType::UInt64);
  }
  else if (baseName == "UInt32") {
    baseType = std::make_shared<BuiltinType>(BuiltinType::UInt32);
  }
  else if (baseName == "UInt16") {
    baseType = std::make_shared<BuiltinType>(BuiltinType::UInt16);
  }
  else if (baseName == "UInt8") {
    baseType = std::make_shared<BuiltinType>(BuiltinType::UInt8);
  }
  else if (baseName == "Float") {
    baseType = std::make_shared<BuiltinType>(BuiltinType::Float);
  }
  else if (baseName == "Double") {
    baseType = std::make_shared<BuiltinType>(BuiltinType::Double);
  }
  else if (baseName == "Bool") {
    baseType = std::make_shared<BuiltinType>(BuiltinType::Bool);
  }
  else if (baseName == "String") {
    baseType = std::make_shared<BuiltinType>(BuiltinType::String);
  }
  else if (baseName == "Void") {
    baseType = std::make_shared<BuiltinType>(BuiltinType::Void);
  }
  else if (baseName == "Any") {
    baseType = std::make_shared<BuiltinType>(BuiltinType::Any);
  }
  else {
    return nullptr;
  }
  
  if (isOptional) {
    return std::make_shared<OptionalType>(baseType);
  }
  
  return baseType;
}

bool Sema::addSymbol(const std::string& name, std::shared_ptr<Type> type) {
  if (!ScopeStack.empty()) {
    auto& currentScope = ScopeStack.back();
    if (currentScope.find(name) != currentScope.end()) {
      Diags.report(diag::redefinition(name, SourceLocation(), currentFilename));
      return false;
    }
    currentScope[name] = type;
  }
  return true;
}

std::shared_ptr<Type> Sema::lookupSymbol(const std::string& name) {
  for (auto it = ScopeStack.rbegin(); it != ScopeStack.rend(); ++it) {
    auto symbolIt = it->find(name);
    if (symbolIt != it->end()) {
      return symbolIt->second;
    }
  }
  return nullptr;
}

void Sema::enterScope() {
  ScopeStack.push_back(std::map<std::string, std::shared_ptr<Type>>());
}

void Sema::exitScope() {
  if (!ScopeStack.empty()) {
    ScopeStack.pop_back();
  }
}

std::shared_ptr<Type> Sema::getExprType(Expr* expr) {
  if (!expr) {
    return nullptr;
  }
  return expr->ExprType;
}

bool Sema::isTypeCompatible(std::shared_ptr<Type> from, std::shared_ptr<Type> to) {
  if (!from || !to) {
    return false;
  }
  
  auto fromBuiltin = std::dynamic_pointer_cast<BuiltinType>(from);
  auto toBuiltin = std::dynamic_pointer_cast<BuiltinType>(to);
  
  if (!fromBuiltin || !toBuiltin) {
    return false;
  }
  
  if (fromBuiltin->TyKind == BuiltinType::Any || toBuiltin->TyKind == BuiltinType::Any) {
    return true;
  }
  
  if (fromBuiltin->TyKind == toBuiltin->TyKind) {
    return true;
  }
  
  if (fromBuiltin->isInteger() && toBuiltin->isInteger()) {
    int fromBits = getIntegerBitWidth(fromBuiltin->TyKind);
    int toBits = getIntegerBitWidth(toBuiltin->TyKind);
    
    bool fromSigned = (fromBuiltin->TyKind >= BuiltinType::Int && fromBuiltin->TyKind <= BuiltinType::Int64);
    bool toSigned = (toBuiltin->TyKind >= BuiltinType::Int && toBuiltin->TyKind <= BuiltinType::Int64);
    
    if (fromSigned && toSigned) {
      return fromBits <= toBits;
    }
    
    if (!fromSigned && !toSigned) {
      return fromBits <= toBits;
    }
    
    if (fromSigned && !toSigned) {
      return fromBits < toBits;
    }
    
    return false;
  }
  
  if (fromBuiltin->isFloat() && toBuiltin->isFloat()) {
    return fromBuiltin->TyKind == toBuiltin->TyKind || 
           (fromBuiltin->TyKind == BuiltinType::Float && toBuiltin->TyKind == BuiltinType::Double);
  }
  
  if (fromBuiltin->TyKind == BuiltinType::String && toBuiltin->TyKind == BuiltinType::String) {
    return true;
  }
  
  if (fromBuiltin->TyKind == BuiltinType::Bool && toBuiltin->TyKind == BuiltinType::Bool) {
    return true;
  }
  
  return false;
}

int Sema::getIntegerBitWidth(BuiltinType::Kind kind) {
  switch (kind) {
    case BuiltinType::Int8:
    case BuiltinType::UInt8:
      return 8;
    case BuiltinType::Int16:
    case BuiltinType::UInt16:
      return 16;
    case BuiltinType::Int32:
    case BuiltinType::UInt32:
      return 32;
    case BuiltinType::Int:
    case BuiltinType::Int64:
    case BuiltinType::UInt:
    case BuiltinType::UInt64:
      return 64;
    default:
      return 0;
  }
}

bool Sema::visit(Program* prog) {
  if (!prog) {
    return false;
  }
  
  for (auto& decl : prog->getDecls()) {
    if (!visit(decl.get())) {
      return false;
    }
  }
  
  return true;
}

bool Sema::visit(Decl* decl) {
  if (!decl) {
    return false;
  }
  
  if (auto func = dynamic_cast<FuncDecl*>(decl)) {
    return visit(func);
  }
  
  if (auto var = dynamic_cast<VarDeclStmt*>(decl)) {
    return visit(var);
  }
  
  if (auto import = dynamic_cast<ImportDecl*>(decl)) {
    return true;
  }
  
  if (auto cls = dynamic_cast<ClassDecl*>(decl)) {
    return true;
  }
  
  return true;
}

bool Sema::visit(FuncDecl* func) {
  if (!func) {
    return false;
  }
  
  if (FunctionTable.find(func->Name) != FunctionTable.end()) {
    Diags.report(diag::redefinition(func->Name, SourceLocation(), currentFilename));
    return false;
  }
  
  FunctionTable[func->Name] = func;
  
  auto prevReturnType = CurrentFuncReturnType;
  
  if (func->ReturnType.empty()) {
    CurrentFuncReturnType = std::make_shared<BuiltinType>(BuiltinType::Void);
  } else {
    CurrentFuncReturnType = lookupType(func->ReturnType);
  }
  
  if (!CurrentFuncReturnType) {
    DiagnosticError error;
    error.Level = DiagLevel::Error;
    error.Category = ErrorCategory::Type;
    error.Message = "unknown type '" + func->ReturnType + "'";
    error.ErrorID = ErrorCodes::Type::InvalidType;
    Diags.report(error);
    return false;
  }
  
  enterScope();
  
  for (const auto& param : func->Params) {
    auto paramType = lookupType(param.second);
    if (!paramType) {
      DiagnosticError error;
      error.Level = DiagLevel::Error;
      error.Category = ErrorCategory::Type;
      error.Message = "unknown type '" + param.second + "' for parameter '" + param.first + "'";
      error.ErrorID = ErrorCodes::Type::InvalidType;
      Diags.report(error);
      CurrentFuncReturnType = prevReturnType;
      return false;
    }
    addSymbol(param.first, paramType);
  }
  
  if (func->Body) {
    visit(func->Body.get());
  }
  
  exitScope();
  
  CurrentFuncReturnType = prevReturnType;
  
  return true;
}

bool Sema::visit(VarDeclStmt* var) {
  if (!var) {
    return false;
  }
  
  std::shared_ptr<Type> type;
  
  if (!var->Type.empty()) {
    type = lookupType(var->Type);
    if (!type) {
      DiagnosticError error;
      error.Level = DiagLevel::Error;
      error.Category = ErrorCategory::Type;
      error.Message = "unknown type '" + var->Type + "'";
      error.ErrorID = ErrorCodes::Type::InvalidType;
      Diags.report(error);
      return false;
    }
  }
  
  if (var->Init) {
    visit(var->Init.get());
    auto initType = getExprType(var->Init.get());
    
    if (!initType) {
      return false;
    }
    
    if (type) {
      if (!isTypeCompatible(initType, type)) {
        Diags.report(diag::typeMismatch(var->Type, initType->Name, SourceLocation(), currentFilename));
        return false;
      }
    } else {
      type = initType;
    }
  }
  
  if (!type) {
    DiagnosticError error;
    error.Level = DiagLevel::Error;
    error.Category = ErrorCategory::Type;
    error.Message = "cannot infer type for variable '" + var->Name + "'";
    error.ErrorID = ErrorCodes::Type::MissingTypeAnnotation;
    Diags.report(error);
    return false;
  }
  
  // Add symbol to table after determining type
  if (!addSymbol(var->Name, type)) {
    return false;
  }
  
  return true;
}

bool Sema::visit(Expr* expr) {
  if (!expr) {
    return false;
  }
  
  if (auto call = dynamic_cast<CallExpr*>(expr)) {
    return visit(call);
  }
  
  if (auto binary = dynamic_cast<BinaryExpr*>(expr)) {
    return visit(binary);
  }
  
  if (auto assign = dynamic_cast<AssignExpr*>(expr)) {
    return visit(assign);
  }
  
  if (auto ident = dynamic_cast<IdentifierExpr*>(expr)) {
    return visit(ident);
  }
  
  if (auto intLit = dynamic_cast<IntegerLiteralExpr*>(expr)) {
    return visit(intLit);
  }
  
  if (auto boolLit = dynamic_cast<BoolLiteralExpr*>(expr)) {
    return visit(boolLit);
  }
  
  if (auto floatLit = dynamic_cast<FloatLiteralExpr*>(expr)) {
    return visit(floatLit);
  }
  
  if (auto strLit = dynamic_cast<StringLiteralExpr*>(expr)) {
    return visit(strLit);
  }
  
  if (auto arrLit = dynamic_cast<ArrayLiteralExpr*>(expr)) {
    return visit(arrLit);
  }
  
  if (auto arrIdx = dynamic_cast<ArrayIndexExpr*>(expr)) {
    return visit(arrIdx);
  }
  
  if (auto nilLit = dynamic_cast<NilLiteralExpr*>(expr)) {
    return visit(nilLit);
  }
  
  if (auto optUnwrap = dynamic_cast<OptionalUnwrapExpr*>(expr)) {
    return visit(optUnwrap);
  }
  
  if (auto optChain = dynamic_cast<OptionalChainExpr*>(expr)) {
    return visit(optChain);
  }
  
  return true;
}

bool Sema::visit(Stmt* stmt) {
  if (!stmt) {
    return false;
  }
  
  if (auto ret = dynamic_cast<ReturnStmt*>(stmt)) {
    return visit(ret);
  }
  
  if (auto ifStmt = dynamic_cast<IfStmt*>(stmt)) {
    return visit(ifStmt);
  }
  
  if (auto ifLetStmt = dynamic_cast<IfLetStmt*>(stmt)) {
    return visit(ifLetStmt);
  }
  
  if (auto guardStmt = dynamic_cast<GuardStmt*>(stmt)) {
    return visit(guardStmt);
  }
  
  if (auto whileStmt = dynamic_cast<WhileStmt*>(stmt)) {
    return visit(whileStmt);
  }
  
  if (auto forStmt = dynamic_cast<ForStmt*>(stmt)) {
    return visit(forStmt);
  }
  
  if (auto switchStmt = dynamic_cast<SwitchStmt*>(stmt)) {
    return visit(switchStmt);
  }
  
  if (auto block = dynamic_cast<BlockStmt*>(stmt)) {
    return visit(block);
  }
  
  if (auto var = dynamic_cast<VarDeclStmt*>(stmt)) {
    return visit(var);
  }
  
  if (auto expr = dynamic_cast<Expr*>(stmt)) {
    return visit(expr);
  }
  
  return true;
}

bool Sema::visit(ReturnStmt* ret) {
  if (!ret) {
    return false;
  }
  
  if (ret->Value) {
    visit(ret->Value.get());
    auto exprType = getExprType(ret->Value.get());
    
    if (!exprType) {
      return false;
    }
    
    if (CurrentFuncReturnType && !isTypeCompatible(exprType, CurrentFuncReturnType)) {
      Diags.report(diag::typeMismatch(CurrentFuncReturnType->Name, exprType->Name, SourceLocation(), currentFilename));
      return false;
    }
  } else {
    if (CurrentFuncReturnType && CurrentFuncReturnType->Name != "Void") {
      DiagnosticError error;
      error.Level = DiagLevel::Error;
      error.Category = ErrorCategory::Semantic;
      error.Message = "non-void function must return a value";
      error.ErrorID = ErrorCodes::Semantic::InvalidReturn;
      Diags.report(error);
      return false;
    }
  }
  
  return true;
}

bool Sema::visit(IfStmt* ifStmt) {
  if (!ifStmt) {
    return false;
  }
  
  visit(ifStmt->Condition.get());
  
  auto condType = getExprType(ifStmt->Condition.get());
  if (condType) {
    auto boolType = std::make_shared<BuiltinType>(BuiltinType::Bool);
    if (!isTypeCompatible(condType, boolType)) {
      DiagnosticError error;
      error.Level = DiagLevel::Error;
      error.Category = ErrorCategory::Type;
      error.Message = "condition must be of type Bool";
      error.ErrorID = ErrorCodes::Type::CannotConvert;
      Diags.report(error);
    }
  }
  
  visit(ifStmt->ThenBranch.get());
  
  if (ifStmt->ElseBranch) {
    visit(ifStmt->ElseBranch.get());
  }
  
  return true;
}

bool Sema::visit(WhileStmt* whileStmt) {
  if (!whileStmt) {
    return false;
  }
  
  visit(whileStmt->Condition.get());
  
  auto condType = getExprType(whileStmt->Condition.get());
  if (condType) {
    auto boolType = std::make_shared<BuiltinType>(BuiltinType::Bool);
    if (!isTypeCompatible(condType, boolType)) {
      DiagnosticError error;
      error.Level = DiagLevel::Error;
      error.Category = ErrorCategory::Type;
      error.Message = "condition must be of type Bool";
      error.ErrorID = ErrorCodes::Type::CannotConvert;
      Diags.report(error);
    }
  }
  
  visit(whileStmt->Body.get());
  
  return true;
}

bool Sema::visit(ForStmt* forStmt) {
  if (!forStmt) {
    return false;
  }
  
  visit(forStmt->Start.get());
  visit(forStmt->End.get());
  visit(forStmt->Step.get());
  
  auto intType = std::make_shared<BuiltinType>(BuiltinType::Int64);
  addSymbol(forStmt->VarName, intType);
  
  visit(forStmt->Body.get());
  
  return true;
}

bool Sema::visit(SwitchStmt* switchStmt) {
  if (!switchStmt) {
    return false;
  }
  
  visit(switchStmt->Condition.get());
  
  for (const auto& casePair : switchStmt->Cases) {
    for (const auto& pattern : casePair.first) {
      visit(pattern.get());
    }
    visit(casePair.second.get());
  }
  
  return true;
}

bool Sema::visit(BlockStmt* block) {
  if (!block) {
    return false;
  }
  
  enterScope();
  
  for (const auto& stmt : block->Statements) {
    visit(stmt.get());
  }
  
  exitScope();
  
  return true;
}

bool Sema::visit(CallExpr* call) {
  if (!call) {
    return false;
  }
  
  for (auto& arg : call->Args) {
    visit(arg.get());
  }
  
  if (isBuiltinFunction(call->Callee)) {
    if (call->Callee == "len") {
      call->ExprType = std::make_shared<BuiltinType>(BuiltinType::Int64);
    } else if (call->Callee == "toString") {
      call->ExprType = std::make_shared<BuiltinType>(BuiltinType::String);
    } else if (call->Callee == "toInt") {
      call->ExprType = std::make_shared<BuiltinType>(BuiltinType::Int64);
    } else if (call->Callee == "split") {
      call->ExprType = std::make_shared<BuiltinType>(BuiltinType::Any);
    } else if (call->Callee == "join" || call->Callee == "append") {
      call->ExprType = std::make_shared<BuiltinType>(BuiltinType::Any);
    } else if (call->Callee == "trim") {
      call->ExprType = std::make_shared<BuiltinType>(BuiltinType::String);
    } else {
      call->ExprType = std::make_shared<BuiltinType>(BuiltinType::Any);
    }
    return true;
  }
  
  auto it = FunctionTable.find(call->Callee);
  if (it == FunctionTable.end()) {
    Diags.report(diag::undefinedFunction(call->Callee, SourceLocation(), currentFilename));
    return false;
  }
  
  auto func = it->second;
  
  auto returnType = lookupType(func->ReturnType);
  if (returnType) {
    call->ExprType = returnType;
  }
  
  return true;
}

bool Sema::visit(BinaryExpr* binary) {
  if (!binary) {
    return false;
  }
  
  visit(binary->LHS.get());
  visit(binary->RHS.get());
  
  auto lhsType = getExprType(binary->LHS.get());
  auto rhsType = getExprType(binary->RHS.get());
  
  if (binary->Op == "+" || binary->Op == "-" || binary->Op == "*" || binary->Op == "/") {
    if (lhsType && rhsType) {
      if (lhsType->Name == "Bool" || rhsType->Name == "Bool") {
        DiagnosticError error;
        error.Level = DiagLevel::Error;
        error.Category = ErrorCategory::Semantic;
        error.Message = "cannot perform arithmetic operations on Bool type";
        error.ErrorID = ErrorCodes::Semantic::InvalidOperation;
        Diags.report(error);
        return false;
      }
      if (lhsType->isInteger() && rhsType->isInteger()) {
        binary->ExprType = lhsType;
      } else if (lhsType->isFloat() || rhsType->isFloat()) {
        binary->ExprType = std::make_shared<BuiltinType>(BuiltinType::Double);
      } else if (lhsType->Name == "String" || rhsType->Name == "String") {
        binary->ExprType = std::make_shared<BuiltinType>(BuiltinType::String);
      } else if (lhsType->Name == "Any" || rhsType->Name == "Any") {
        binary->ExprType = std::make_shared<BuiltinType>(BuiltinType::Any);
      }
    }
  } else if (binary->Op == "==" || binary->Op == "!=" || binary->Op == "<" || binary->Op == ">" || binary->Op == "<=" || binary->Op == ">=") {
    if (lhsType && rhsType) {
      if (!isTypeCompatible(lhsType, rhsType) && !isTypeCompatible(rhsType, lhsType)) {
        DiagnosticError error;
        error.Level = DiagLevel::Error;
        error.Category = ErrorCategory::Type;
        error.Message = "cannot compare '" + lhsType->Name + "' with '" + rhsType->Name + "'";
        error.ErrorID = ErrorCodes::Type::CannotConvert;
        Diags.report(error);
        return false;
      }
    }
    binary->ExprType = std::make_shared<BuiltinType>(BuiltinType::Bool);
  } else if (binary->Op == "&&" || binary->Op == "||") {
    if (lhsType && lhsType->Name != "Bool") {
      DiagnosticError error;
      error.Level = DiagLevel::Error;
      error.Category = ErrorCategory::Type;
      error.Message = "left operand of '" + binary->Op + "' must be of type Bool";
      error.ErrorID = ErrorCodes::Type::CannotConvert;
      Diags.report(error);
      return false;
    }
    if (rhsType && rhsType->Name != "Bool") {
      DiagnosticError error;
      error.Level = DiagLevel::Error;
      error.Category = ErrorCategory::Type;
      error.Message = "right operand of '" + binary->Op + "' must be of type Bool";
      error.ErrorID = ErrorCodes::Type::CannotConvert;
      Diags.report(error);
      return false;
    }
    binary->ExprType = std::make_shared<BuiltinType>(BuiltinType::Bool);
  }
  
  return true;
}

bool Sema::visit(AssignExpr* assign) {
  if (!assign) {
    return false;
  }
  
  visit(assign->Target.get());
  visit(assign->Value.get());
  
  auto varType = getExprType(assign->Target.get());
  auto valueType = getExprType(assign->Value.get());
  
  if (!varType) {
    DiagnosticError error;
    error.Level = DiagLevel::Error;
    error.Category = ErrorCategory::Semantic;
    error.Message = "cannot assign to expression";
    error.ErrorID = ErrorCodes::Semantic::InvalidAssignment;
    Diags.report(error);
    return false;
  }
  
  if (valueType) {
    if (!isTypeCompatible(valueType, varType)) {
      Diags.report(diag::typeMismatch(varType->Name, valueType->Name, SourceLocation(), currentFilename));
      return false;
    }
  }
  
  assign->ExprType = varType;
  
  return true;
}

bool Sema::visit(IdentifierExpr* ident) {
  if (!ident) {
    return false;
  }
  
  auto type = lookupSymbol(ident->Name);
  if (!type) {
    Diags.report(diag::undefinedVariable(ident->Name, SourceLocation(), currentFilename));
    return false;
  }
  
  ident->ExprType = type;
  
  return true;
}

bool Sema::visit(IntegerLiteralExpr* lit) {
  if (!lit) {
    return false;
  }
  
  lit->ExprType = std::make_shared<BuiltinType>(BuiltinType::Int64);
  
  return true;
}

bool Sema::visit(BoolLiteralExpr* lit) {
  if (!lit) {
    return false;
  }
  
  lit->ExprType = std::make_shared<BuiltinType>(BuiltinType::Bool);
  
  return true;
}

bool Sema::visit(FloatLiteralExpr* lit) {
  if (!lit) {
    return false;
  }
  
  lit->ExprType = std::make_shared<BuiltinType>(BuiltinType::Double);
  
  return true;
}

bool Sema::visit(StringLiteralExpr* lit) {
  if (!lit) {
    return false;
  }
  
  lit->ExprType = std::make_shared<BuiltinType>(BuiltinType::String);
  
  return true;
}

bool Sema::visit(ArrayLiteralExpr* lit) {
  if (!lit) {
    return false;
  }
  
  if (lit->Elements.empty()) {
    lit->ExprType = std::make_shared<BuiltinType>(BuiltinType::Any);
    return true;
  }
  
  visit(lit->Elements[0].get());
  auto elemType = getExprType(lit->Elements[0].get());
  
  if (!elemType) {
    return false;
  }
  
  for (size_t i = 1; i < lit->Elements.size(); i++) {
    visit(lit->Elements[i].get());
    auto currentType = getExprType(lit->Elements[i].get());
    
    if (!currentType) {
      return false;
    }
    
    if (!isTypeCompatible(currentType, elemType)) {
      DiagnosticError error;
      error.Level = DiagLevel::Error;
      error.Category = ErrorCategory::Type;
      error.Message = "array elements must have the same type";
      error.ErrorID = ErrorCodes::Type::CannotConvert;
      Diags.report(error);
      return false;
    }
  }
  
  lit->ExprType = elemType;
  
  return true;
}

bool Sema::visit(ArrayIndexExpr* expr) {
  if (!expr) {
    return false;
  }
  
  visit(expr->Array.get());
  visit(expr->Index.get());
  
  auto arrayType = getExprType(expr->Array.get());
  auto indexType = getExprType(expr->Index.get());
  
  if (!arrayType) {
    return false;
  }
  
  if (!indexType) {
    return false;
  }
  
  if (!indexType->isInteger()) {
    DiagnosticError error;
    error.Level = DiagLevel::Error;
    error.Category = ErrorCategory::Type;
    error.Message = "array index must be of integer type";
    error.ErrorID = ErrorCodes::Type::CannotConvert;
    Diags.report(error);
    return false;
  }
  
  expr->ExprType = arrayType;
  
  return true;
}

bool Sema::visit(NilLiteralExpr* lit) {
  if (!lit) {
    return false;
  }
  
  lit->ExprType = std::make_shared<BuiltinType>(BuiltinType::Any);
  
  return true;
}

bool Sema::visit(OptionalUnwrapExpr* expr) {
  if (!expr) {
    return false;
  }
  
  visit(expr->Target.get());
  
  auto targetType = getExprType(expr->Target.get());
  if (!targetType) {
    return false;
  }
  
  if (!targetType->isOptional()) {
    if (expr->IsForceUnwrap) {
      Diags.report(diag::invalidForceUnwrap(expr->Loc, currentFilename));
    } else {
      Diags.report(diag::invalidOptionalChain(expr->Loc, currentFilename));
    }
    return false;
  }
  
  auto optionalType = std::dynamic_pointer_cast<OptionalType>(targetType);
  if (optionalType) {
    expr->ExprType = optionalType->getWrappedType();
  } else {
    expr->ExprType = targetType;
  }
  
  return true;
}

bool Sema::visit(OptionalChainExpr* expr) {
  if (!expr) {
    return false;
  }
  
  visit(expr->Target.get());
  
  auto targetType = getExprType(expr->Target.get());
  if (!targetType) {
    return false;
  }
  
  if (!targetType->isOptional()) {
    Diags.report(diag::invalidOptionalChain(expr->Loc, currentFilename));
    return false;
  }
  
  for (auto& arg : expr->CallArgs) {
    visit(arg.get());
  }
  
  expr->ExprType = std::make_shared<OptionalType>(std::make_shared<BuiltinType>(BuiltinType::Any));
  
  return true;
}

bool Sema::visit(IfLetStmt* stmt) {
  if (!stmt) {
    return false;
  }
  
  visit(stmt->OptionalExpr.get());
  
  auto optionalType = getExprType(stmt->OptionalExpr.get());
  if (!optionalType) {
    return false;
  }
  
  if (!optionalType->isOptional()) {
    DiagnosticError error;
    error.Level = DiagLevel::Error;
    error.Category = ErrorCategory::Type;
    error.Message = "if let requires an optional value";
    error.ErrorID = ErrorCodes::Type::InvalidOptional;
    error.Line = stmt->OptionalExpr->Loc.Line;
    error.Column = stmt->OptionalExpr->Loc.Col;
    error.FileName = currentFilename;
    Diags.report(error);
    return false;
  }
  
  enterScope();
  
  auto optionalTypeCast = std::dynamic_pointer_cast<OptionalType>(optionalType);
  if (optionalTypeCast) {
    addSymbol(stmt->VarName, optionalTypeCast->getWrappedType());
  } else {
    addSymbol(stmt->VarName, optionalType);
  }
  
  visit(stmt->ThenBranch.get());
  exitScope();
  
  if (stmt->ElseBranch) {
    visit(stmt->ElseBranch.get());
  }
  
  return true;
}

bool Sema::visit(GuardStmt* stmt) {
  if (!stmt) {
    return false;
  }
  
  visit(stmt->OptionalExpr.get());
  
  auto optionalType = getExprType(stmt->OptionalExpr.get());
  if (!optionalType) {
    return false;
  }
  
  if (!optionalType->isOptional()) {
    DiagnosticError error;
    error.Level = DiagLevel::Error;
    error.Category = ErrorCategory::Type;
    error.Message = "guard let requires an optional value";
    error.ErrorID = ErrorCodes::Type::InvalidOptional;
    error.Line = stmt->OptionalExpr->Loc.Line;
    error.Column = stmt->OptionalExpr->Loc.Col;
    error.FileName = currentFilename;
    Diags.report(error);
    return false;
  }
  
  enterScope();
  
  auto optionalTypeCast = std::dynamic_pointer_cast<OptionalType>(optionalType);
  if (optionalTypeCast) {
    addSymbol(stmt->VarName, optionalTypeCast->getWrappedType());
  } else {
    addSymbol(stmt->VarName, optionalType);
  }
  
  exitScope();
  
  visit(stmt->ElseBranch.get());
  
  return true;
}

}
