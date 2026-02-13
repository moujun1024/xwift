#include "xwift/Parser/SyntaxParser.h"
#include "xwift/AST/Nodes.h"

namespace xwift {

std::unique_ptr<Program> SyntaxParser::parseProgram() {
  auto program = std::make_unique<Program>();
  
  while (CurrentToken.Kind != TokenKind::EndOfFile) {
    auto decl = parseDeclaration();
    if (decl) {
      program->addDecl(std::move(decl));
    } else {
      break;
    }
  }
  
  return program;
}

std::unique_ptr<Decl> SyntaxParser::parseDeclaration() {
  if (CurrentToken.is(TokenKind::kw_import)) {
    return parseImportDeclaration();
  }
  if (CurrentToken.is(TokenKind::kw_func)) {
    return parseFunctionDeclaration();
  }
  if (CurrentToken.is(TokenKind::kw_class)) {
    return parseClassDeclaration();
  }
  if (CurrentToken.is(TokenKind::kw_var) || CurrentToken.is(TokenKind::kw_let)) {
    return parseVariableDeclaration();
  }
  
  return nullptr;
}

std::unique_ptr<Decl> SyntaxParser::parseImportDeclaration() {
  consume(TokenKind::kw_import);
  std::string moduleName = CurrentToken.Text;
  expect(TokenKind::Identifier);
  consume(TokenKind::punct_semicolon);
  return std::make_unique<ImportDecl>(moduleName);
}

std::unique_ptr<FuncDecl> SyntaxParser::parseFunctionDeclaration() {
  consume(TokenKind::kw_func);
  
  std::string name = CurrentToken.Text;
  expect(TokenKind::Identifier);
  
  expect(TokenKind::punct_l_paren);
  
  std::vector<std::pair<std::string, std::string>> params;
  while (!CurrentToken.is(TokenKind::punct_r_paren)) {
    if (!params.empty()) {
      consume(TokenKind::punct_comma);
    }
    std::string paramName = CurrentToken.Text;
    expect(TokenKind::Identifier);
    std::string paramType = "Any";
    if (CurrentToken.is(TokenKind::punct_colon)) {
      consume(TokenKind::punct_colon);
      paramType = CurrentToken.Text;
      expect(TokenKind::Identifier);
    }
    params.push_back({paramName, paramType});
  }
  expect(TokenKind::punct_r_paren);
  
  std::string returnType = "Void";
  if (CurrentToken.is(TokenKind::op_minus_gt)) {
    advance();
    returnType = CurrentToken.Text;
    expect(TokenKind::Identifier);
  }
  
  auto body = parseBlock();
  
  auto funcDecl = std::make_unique<FuncDecl>(name, returnType, std::move(body));
  for (auto& p : params) {
    funcDecl->addParam(p.first, p.second);
  }
  return std::move(funcDecl);
}

std::unique_ptr<ClassDecl> SyntaxParser::parseClassDeclaration() {
  consume(TokenKind::kw_class);
  
  std::string name = CurrentToken.Text;
  expect(TokenKind::Identifier);
  
  expect(TokenKind::punct_l_brace);
  
  auto classDecl = std::make_unique<ClassDecl>(name);
  
  while (!CurrentToken.is(TokenKind::punct_r_brace) && 
         CurrentToken.Kind != TokenKind::EndOfFile) {
    auto member = parseDeclaration();
    if (member) {
      classDecl->addMember(std::move(member));
    } else {
      advance();
    }
  }
  
  expect(TokenKind::punct_r_brace);
  
  return classDecl;
}

std::unique_ptr<VarDeclStmt> SyntaxParser::parseVariableDeclaration() {
  bool isMutable = CurrentToken.is(TokenKind::kw_var);
  advance();
  
  std::string name = CurrentToken.Text;
  expect(TokenKind::Identifier);
  
  std::string type;
  if (CurrentToken.is(TokenKind::punct_colon)) {
    advance();
    type = CurrentToken.Text;
    expect(TokenKind::Identifier);
  }
  
  std::unique_ptr<Expr> init;
  if (CurrentToken.is(TokenKind::punct_equal)) {
    advance();
    init = parseExpression();
  }
  
  consume(TokenKind::punct_semicolon);
  
  return std::make_unique<VarDeclStmt>(name, type, std::move(init), isMutable);
}

std::unique_ptr<Stmt> SyntaxParser::parseStatement() {
  if (CurrentToken.is(TokenKind::kw_return)) {
    advance();
    auto value = parseExpression();
    consume(TokenKind::punct_semicolon);
    return std::make_unique<ReturnStmt>(std::move(value));
  }
  
  if (CurrentToken.is(TokenKind::kw_if)) {
    return parseIfStatement();
  }
  
  if (CurrentToken.is(TokenKind::kw_while)) {
    return parseWhileStatement();
  }
  
  if (CurrentToken.is(TokenKind::kw_for)) {
    return parseForStatement();
  }
  
  if (CurrentToken.is(TokenKind::kw_switch)) {
    return parseSwitchStatement();
  }
  
  if (CurrentToken.is(TokenKind::punct_l_brace)) {
    return parseBlock();
  }
  
  if (CurrentToken.is(TokenKind::kw_var) || CurrentToken.is(TokenKind::kw_let)) {
    return parseVariableDeclaration();
  }
  
  auto expr = parseExpression();
  if (!expr) {
    advance();
    return nullptr;
  }
  consume(TokenKind::punct_semicolon);
  return expr;
}

std::unique_ptr<Stmt> SyntaxParser::parseIfStatement() {
  consume(TokenKind::kw_if);
  expect(TokenKind::punct_l_paren);
  auto cond = parseExpression();
  expect(TokenKind::punct_r_paren);
  
  auto thenBranch = parseStatement();
  
  std::unique_ptr<Stmt> elseBranch;
  if (CurrentToken.is(TokenKind::kw_else)) {
    advance();
    elseBranch = parseStatement();
  }
  
  return std::make_unique<IfStmt>(std::move(cond), std::move(thenBranch), std::move(elseBranch));
}

std::unique_ptr<Stmt> SyntaxParser::parseWhileStatement() {
  consume(TokenKind::kw_while);
  expect(TokenKind::punct_l_paren);
  auto cond = parseExpression();
  expect(TokenKind::punct_r_paren);
  
  auto body = parseStatement();
  
  return std::make_unique<WhileStmt>(std::move(cond), std::move(body));
}

std::unique_ptr<Stmt> SyntaxParser::parseForStatement() {
  consume(TokenKind::kw_for);
  expect(TokenKind::punct_l_paren);
  
  std::string varName;
  if (CurrentToken.is(TokenKind::Identifier)) {
    varName = CurrentToken.Text;
    advance();
  }
  
  consume(TokenKind::kw_in);
  
  auto start = parseExpression();
  
  consume(TokenKind::punct_dot_dot);
  
  auto end = parseExpression();
  
  std::unique_ptr<Expr> step;
  if (CurrentToken.is(TokenKind::punct_semicolon)) {
    consume(TokenKind::punct_semicolon);
    step = parseExpression();
  } else {
    step = std::make_unique<IntegerLiteralExpr>(1, CurrentToken.Loc);
  }
  
  expect(TokenKind::punct_r_paren);
  
  auto body = parseStatement();
  
  return std::make_unique<ForStmt>(varName, std::move(start), std::move(end), std::move(step), std::move(body));
}

std::unique_ptr<Stmt> SyntaxParser::parseSwitchStatement() {
  consume(TokenKind::kw_switch);
  expect(TokenKind::punct_l_paren);
  auto cond = parseExpression();
  expect(TokenKind::punct_r_paren);
  expect(TokenKind::punct_l_brace);
  
  auto switchStmt = std::make_unique<SwitchStmt>(std::move(cond));
  
  while (!CurrentToken.is(TokenKind::punct_r_brace) && 
         CurrentToken.Kind != TokenKind::EndOfFile) {
    if (CurrentToken.is(TokenKind::kw_case)) {
      consume(TokenKind::kw_case);
      std::vector<ExprPtr> patterns;
      patterns.push_back(parseExpression());
      while (CurrentToken.is(TokenKind::punct_comma)) {
        consume(TokenKind::punct_comma);
        patterns.push_back(parseExpression());
      }
      consume(TokenKind::punct_colon);
      auto body = parseStatement();
      switchStmt->addCase(std::move(patterns), std::move(body));
    } else if (CurrentToken.is(TokenKind::kw_default)) {
      consume(TokenKind::kw_default);
      consume(TokenKind::punct_colon);
      std::vector<ExprPtr> patterns;
      auto body = parseStatement();
      switchStmt->addCase(std::move(patterns), std::move(body));
    } else {
      advance();
    }
  }
  
  expect(TokenKind::punct_r_brace);
  
  return std::move(switchStmt);
}

std::unique_ptr<BlockStmt> SyntaxParser::parseBlock() {
  expect(TokenKind::punct_l_brace);
  
  auto block = std::make_unique<BlockStmt>();
  
  while (!CurrentToken.is(TokenKind::punct_r_brace) && 
         CurrentToken.Kind != TokenKind::EndOfFile) {
    auto stmt = parseStatement();
    if (stmt) {
      block->addStmt(std::move(stmt));
    }
  }
  
  expect(TokenKind::punct_r_brace);
  
  return block;
}

std::unique_ptr<Expr> SyntaxParser::parseExpression() {
  auto lhs = parseBinaryExpression();
  
  if (CurrentToken.is(TokenKind::punct_equal)) {
    advance();
    auto rhs = parseExpression();
    return std::make_unique<AssignExpr>(std::move(lhs), std::move(rhs));
  }
  
  return lhs;
}

std::unique_ptr<Expr> SyntaxParser::parsePrimaryExpression() {
  if (CurrentToken.is(TokenKind::punct_l_paren)) {
    advance();
    auto expr = parseExpression();
    expect(TokenKind::punct_r_paren);
    return expr;
  }
  
  if (CurrentToken.is(TokenKind::punct_l_bracket)) {
    advance();
    std::vector<std::unique_ptr<Expr>> elements;
    while (!CurrentToken.is(TokenKind::punct_r_bracket)) {
      if (!elements.empty()) {
        consume(TokenKind::punct_comma);
      }
      elements.push_back(parseExpression());
    }
    expect(TokenKind::punct_r_bracket);
    return std::make_unique<ArrayLiteralExpr>(std::move(elements), CurrentToken.Loc);
  }
  
  if (CurrentToken.is(TokenKind::IntegerLiteral)) {
    auto val = std::stoll(CurrentToken.Text);
    auto loc = CurrentToken.Loc;
    advance();
    return std::make_unique<IntegerLiteralExpr>(val, loc);
  }
  
  if (CurrentToken.is(TokenKind::FloatLiteral)) {
    auto val = std::stod(CurrentToken.Text);
    auto loc = CurrentToken.Loc;
    advance();
    return std::make_unique<FloatLiteralExpr>(val, loc);
  }
  
  if (CurrentToken.is(TokenKind::StringLiteral)) {
    std::string val = CurrentToken.Text;
    auto loc = CurrentToken.Loc;
    advance();
    return std::make_unique<StringLiteralExpr>(val, loc);
  }
  
  if (CurrentToken.is(TokenKind::Identifier)) {
    std::string name = CurrentToken.Text;
    advance();
    
    if (CurrentToken.is(TokenKind::punct_l_paren)) {
      advance();
      std::vector<std::unique_ptr<Expr>> args;
      while (!CurrentToken.is(TokenKind::punct_r_paren)) {
        if (!args.empty()) {
          consume(TokenKind::punct_comma);
        }
        args.push_back(parseExpression());
      }
      expect(TokenKind::punct_r_paren);
      return std::make_unique<CallExpr>(name, std::move(args));
    }
    
    if (CurrentToken.is(TokenKind::punct_l_bracket)) {
      advance();
      auto index = parseExpression();
      expect(TokenKind::punct_r_bracket);
      return std::make_unique<ArrayIndexExpr>(std::make_unique<IdentifierExpr>(name), std::move(index));
    }
    
    return std::make_unique<IdentifierExpr>(name);
  }
  
  if (CurrentToken.is(TokenKind::kw_true) || CurrentToken.is(TokenKind::kw_false)) {
    bool val = CurrentToken.is(TokenKind::kw_true);
    auto loc = CurrentToken.Loc;
    advance();
    return std::make_unique<BoolLiteralExpr>(val, loc);
  }
  
  if (CurrentToken.is(TokenKind::kw_nil)) {
    auto loc = CurrentToken.Loc;
    advance();
    return std::make_unique<IntegerLiteralExpr>(0, loc);
  }
  
  return nullptr;
}

std::unique_ptr<Expr> SyntaxParser::parseBinaryExpression() {
  return parseBinaryExpression(0);
}

std::unique_ptr<Expr> SyntaxParser::parseBinaryExpression(int minPrecedence) {
  auto lhs = parsePrimaryExpression();
  
  while (true) {
    if (!CurrentToken.isOperator()) {
      break;
    }
    
    std::string op = CurrentToken.Text;
    int precedence = getPrecedence(op);
    
    if (precedence < minPrecedence) {
      break;
    }
    
    advance();
    auto rhs = parseBinaryExpression(precedence + 1);
    lhs = std::make_unique<BinaryExpr>(op, std::move(lhs), std::move(rhs));
  }
  
  return lhs;
}

int SyntaxParser::getPrecedence(const std::string& op) {
  static std::map<std::string, int> prec = {
    {"||", 10},
    {"&&", 20},
    {"==", 30}, {"!=", 30},
    {"<", 40},{">", 40}, {"<=", 40}, {">=", 40},
    {"+", 50}, {"-", 50},
    {"*", 60}, {"/", 60}, {"%", 60},
  };
  auto it = prec.find(op);
  return it != prec.end() ? it->second : 0;
}

}
