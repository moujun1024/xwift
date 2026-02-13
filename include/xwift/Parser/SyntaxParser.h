#ifndef XWIFT_PARSER_SYNTAX_PARSER_H
#define XWIFT_PARSER_SYNTAX_PARSER_H

#include "xwift/Lexer/Lexer.h"
#include "xwift/AST/Nodes.h"
#include <memory>
#include <vector>

namespace xwift {

class SyntaxParser {
public:
  SyntaxParser(Lexer& lexer) : Lex(lexer), CurrentToken(), HasPeeked(false) {
    advance();
  }
  
  std::unique_ptr<Program> parseProgram();
  
private:
  Lexer& Lex;
  Token CurrentToken;
  bool HasPeeked;
  
  void advance() {
    if (HasPeeked) {
      HasPeeked = false;
      return;
    }
    CurrentToken = Lex.nextToken();
  }
  
  Token peek() {
    if (!HasPeeked) {
      CurrentToken = Lex.nextToken();
      HasPeeked = true;
    }
    return CurrentToken;
  }
  
  bool consume(TokenKind kind) {
    if (CurrentToken.is(kind)) {
      advance();
      return true;
    }
    return false;
  }
  
  bool expect(TokenKind kind) {
    if (CurrentToken.is(kind)) {
      advance();
      return true;
    }
    return false;
  }
  
  std::unique_ptr<Decl> parseDeclaration();
  std::unique_ptr<Decl> parseImportDeclaration();
  std::unique_ptr<FuncDecl> parseFunctionDeclaration();
  std::unique_ptr<ClassDecl> parseClassDeclaration();
  std::unique_ptr<VarDeclStmt> parseVariableDeclaration();
  std::unique_ptr<Stmt> parseStatement();
  std::unique_ptr<Stmt> parseIfStatement();
  std::unique_ptr<Stmt> parseWhileStatement();
  std::unique_ptr<Stmt> parseForStatement();
  std::unique_ptr<Stmt> parseSwitchStatement();
  std::unique_ptr<BlockStmt> parseBlock();
  std::unique_ptr<Expr> parseExpression();
  std::unique_ptr<Expr> parsePrimaryExpression();
  std::unique_ptr<Expr> parseBinaryExpression();
  std::unique_ptr<Expr> parseBinaryExpression(int minPrecedence);
  int getPrecedence(const std::string& op);
};

}

#endif
