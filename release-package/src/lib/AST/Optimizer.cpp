#include "xwift/AST/Optimizer.h"
#include "xwift/AST/Nodes.h"
#include <iostream>

namespace xwift {

void Optimizer::optimize(Program* program) {
    if (!program) {
        return;
    }
    
    OptimizationPasses = 0;
    
    deadCodeElimination(program);
    OptimizationPasses++;
    
    for (auto& decl : program->Declarations) {
        optimizeDecl(decl.get());
    }
    
    OptimizationPasses++;
}

void Optimizer::optimizeDecl(Decl* decl) {
    if (!decl) {
        return;
    }
    
    if (auto func = dynamic_cast<FuncDecl*>(decl)) {
        if (func->Body) {
            optimizeStmt(func->Body.get());
        }
    } else if (auto cls = dynamic_cast<ClassDecl*>(decl)) {
        for (auto& member : cls->Members) {
            optimizeDecl(member.get());
        }
    } else if (auto st = dynamic_cast<StructDecl*>(decl)) {
        for (auto& member : st->Members) {
            optimizeDecl(member.get());
        }
    }
}

void Optimizer::optimizeStmt(Stmt* stmt) {
    if (!stmt) {
        return;
    }
    
    if (auto exprStmt = dynamic_cast<ExprStmt*>(stmt)) {
        optimizeExpr(exprStmt->Expr.get());
    } else if (auto ret = dynamic_cast<ReturnStmt*>(stmt)) {
        if (ret->Value) {
            optimizeExpr(ret->Value.get());
        }
    } else if (auto ifStmt = dynamic_cast<IfStmt*>(stmt)) {
        optimizeExpr(ifStmt->Condition.get());
        optimizeStmt(ifStmt->ThenBranch.get());
        optimizeStmt(ifStmt->ElseBranch.get());
    } else if (auto whileStmt = dynamic_cast<WhileStmt*>(stmt)) {
        optimizeExpr(whileStmt->Condition.get());
        optimizeStmt(whileStmt->Body.get());
        loopOptimization(whileStmt);
    } else if (auto forStmt = dynamic_cast<ForStmt*>(stmt)) {
        optimizeExpr(forStmt->Start.get());
        optimizeExpr(forStmt->End.get());
        optimizeExpr(forStmt->Step.get());
        optimizeStmt(forStmt->Body.get());
        loopOptimization(forStmt);
    } else if (auto block = dynamic_cast<BlockStmt*>(stmt)) {
        for (auto& s : block->Statements) {
            optimizeStmt(s.get());
        }
    }
}

void Optimizer::optimizeExpr(Expr* expr) {
    if (!expr) {
        return;
    }
    
    if (auto binary = dynamic_cast<BinaryExpr*>(expr)) {
        optimizeExpr(binary->LHS.get());
        optimizeExpr(binary->RHS.get());
        constantFolding(binary);
    } else if (auto call = dynamic_cast<CallExpr*>(expr)) {
        for (auto& arg : call->Args) {
            optimizeExpr(arg.get());
        }
    } else if (auto arr = dynamic_cast<ArrayLiteralExpr*>(expr)) {
        for (auto& elem : arr->Elements) {
            optimizeExpr(elem.get());
        }
    } else if (auto arrIdx = dynamic_cast<ArrayIndexExpr*>(expr)) {
        optimizeExpr(arrIdx->Array.get());
        optimizeExpr(arrIdx->Index.get());
    }
}

void Optimizer::constantFolding(Expr* expr) {
    if (!expr) {
        return;
    }
    
    auto binary = dynamic_cast<BinaryExpr*>(expr);
    if (!binary) {
        return;
    }
    
    if (!isConstant(binary->LHS.get()) || !isConstant(binary->RHS.get())) {
        return;
    }
    
    Value lhs = evaluateConstant(binary->LHS.get());
    Value rhs = evaluateConstant(binary->RHS.get());
    
    if (lhs.isNil() || rhs.isNil()) {
        return;
    }
    
    Value result;
    
    if (binary->Op == "+") {
        if (auto l = lhs.get<int64_t>()) {
            if (auto r = rhs.get<int64_t>()) {
                result = Value(*l + *r);
            }
        }
        if (auto l = lhs.get<double>()) {
            if (auto r = rhs.get<double>()) {
                result = Value(*l + *r);
            }
        }
    } else if (binary->Op == "-") {
        if (auto l = lhs.get<int64_t>()) {
            if (auto r = rhs.get<int64_t>()) {
                result = Value(*l - *r);
            }
        }
        if (auto l = lhs.get<double>()) {
            if (auto r = rhs.get<double>()) {
                result = Value(*l - *r);
            }
        }
    } else if (binary->Op == "*") {
        if (auto l = lhs.get<int64_t>()) {
            if (auto r = rhs.get<int64_t>()) {
                result = Value(*l * *r);
            }
        }
        if (auto l = lhs.get<double>()) {
            if (auto r = rhs.get<double>()) {
                result = Value(*l * *r);
            }
        }
    } else if (binary->Op == "/") {
        if (auto l = lhs.get<int64_t>()) {
            if (auto r = rhs.get<int64_t>()) {
                if (*r != 0) {
                    result = Value(*l / *r);
                }
            }
        }
        if (auto l = lhs.get<double>()) {
            if (auto r = rhs.get<double>()) {
                if (*r != 0.0) {
                    result = Value(*l / *r);
                }
            }
        }
    } else if (binary->Op == "==") {
        result = Value(lhs == rhs);
    } else if (binary->Op == "!=") {
        result = Value(lhs != rhs);
    } else if (binary->Op == "<") {
        if (auto l = lhs.get<int64_t>()) {
            if (auto r = rhs.get<int64_t>()) {
                result = Value(*l < *r);
            }
        }
    } else if (binary->Op == ">") {
        if (auto l = lhs.get<int64_t>()) {
            if (auto r = rhs.get<int64_t>()) {
                result = Value(*l > *r);
            }
        }
    } else if (binary->Op == "<=") {
        if (auto l = lhs.get<int64_t>()) {
            if (auto r = rhs.get<int64_t>()) {
                result = Value(*l <= *r);
            }
        }
    } else if (binary->Op == ">=") {
        if (auto l = lhs.get<int64_t>()) {
            if (auto r = rhs.get<int64_t>()) {
                result = Value(*l >= *r);
            }
        }
    } else if (binary->Op == "&&") {
        if (auto l = lhs.get<bool>()) {
            if (auto r = rhs.get<bool>()) {
                result = Value(*l && *r);
            }
        }
    } else if (binary->Op == "||") {
        if (auto l = lhs.get<bool>()) {
            if (auto r = rhs.get<bool>()) {
                result = Value(*l || *r);
            }
        }
    }
    
    if (!result.isNil()) {
        if (auto val = result.get<int64_t>()) {
            auto lit = std::make_unique<IntegerLiteralExpr>(*val);
            *binary->LHS = std::move(lit);
        } else if (auto val = result.get<double>()) {
            auto lit = std::make_unique<FloatLiteralExpr>(*val);
            *binary->LHS = std::move(lit);
        } else if (auto val = result.get<bool>()) {
            auto lit = std::make_unique<BoolLiteralExpr>(*val);
            *binary->LHS = std::move(lit);
        }
    }
}

void Optimizer::deadCodeElimination(Program* program) {
    if (!program) {
        return;
    }
    
    for (auto& decl : program->Declarations) {
        if (auto func = dynamic_cast<FuncDecl*>(decl.get())) {
            if (!func->Body) {
                continue;
            }
            
            auto block = dynamic_cast<BlockStmt*>(func->Body.get());
            if (!block) {
                continue;
            }
            
            std::vector<StmtPtr> newStatements;
            for (auto& stmt : block->Statements) {
                if (!isUnreachable(stmt.get())) {
                    newStatements.push_back(std::move(stmt));
                }
            }
            
            block->Statements = std::move(newStatements);
        }
    }
}

void Optimizer::loopOptimization(Stmt* stmt) {
    if (!stmt) {
        return;
    }
    
    auto whileStmt = dynamic_cast<WhileStmt*>(stmt);
    auto forStmt = dynamic_cast<ForStmt*>(stmt);
    
    if (!whileStmt && !forStmt) {
        return;
    }
    
    Expr* condition = whileStmt ? whileStmt->Condition.get() : forStmt->End.get();
    
    if (!isConstant(condition)) {
        return;
    }
    
    Value cond = evaluateConstant(condition);
    
    if (auto val = cond.get<bool>()) {
        if (!*val) {
            return;
        }
    }
    
    if (auto val = cond.get<int64_t>()) {
        if (*val == 0) {
            return;
        }
    }
}

bool Optimizer::isConstant(Expr* expr) {
    if (!expr) {
        return false;
    }
    
    if (dynamic_cast<IntegerLiteralExpr*>(expr)) {
        return true;
    }
    if (dynamic_cast<FloatLiteralExpr*>(expr)) {
        return true;
    }
    if (dynamic_cast<BoolLiteralExpr*>(expr)) {
        return true;
    }
    if (dynamic_cast<StringLiteralExpr*>(expr)) {
        return true;
    }
    if (dynamic_cast<NilLiteralExpr*>(expr)) {
        return true;
    }
    
    return false;
}

Value Optimizer::evaluateConstant(Expr* expr) {
    if (!expr) {
        return Value();
    }
    
    if (auto lit = dynamic_cast<IntegerLiteralExpr*>(expr)) {
        return Value(lit->Value);
    }
    if (auto lit = dynamic_cast<FloatLiteralExpr*>(expr)) {
        return Value(lit->Value);
    }
    if (auto lit = dynamic_cast<BoolLiteralExpr*>(expr)) {
        return Value(lit->Value);
    }
    if (auto lit = dynamic_cast<StringLiteralExpr*>(expr)) {
        return Value(lit->Value);
    }
    if (dynamic_cast<NilLiteralExpr*>(expr)) {
        return Value();
    }
    
    return Value();
}

bool Optimizer::isUnreachable(Stmt* stmt) {
    if (!stmt) {
        return false;
    }
    
    auto ret = dynamic_cast<ReturnStmt*>(stmt);
    if (ret) {
        return true;
    }
    
    return false;
}

bool Optimizer::hasSideEffects(Expr* expr) {
    if (!expr) {
        return false;
    }
    
    if (dynamic_cast<CallExpr*>(expr)) {
        return true;
    }
    if (dynamic_cast<AssignExpr*>(expr)) {
        return true;
    }
    
    return false;
}

}
