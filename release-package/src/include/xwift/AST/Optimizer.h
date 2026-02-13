#ifndef XWIFT_AST_OPTIMIZER_H
#define XWIFT_AST_OPTIMIZER_H

#include "xwift/AST/Nodes.h"
#include "xwift/AST/Type.h"
#include <memory>
#include <vector>

namespace xwift {

class Optimizer {
public:
    Optimizer() {}
    
    void optimize(Program* program);
    
private:
    void optimizeDecl(Decl* decl);
    void optimizeStmt(Stmt* stmt);
    void optimizeExpr(Expr* expr);
    
    void constantFolding(Expr* expr);
    void deadCodeElimination(Program* program);
    void loopOptimization(Stmt* stmt);
    
    bool isConstant(Expr* expr);
    Value evaluateConstant(Expr* expr);
    
    bool isUnreachable(Stmt* stmt);
    bool hasSideEffects(Expr* expr);
    
    int OptimizationPasses;
};

}

#endif
