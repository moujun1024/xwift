#include "xwift/Parser/Parser.h"
#include "xwift/Lexer/Lexer.h"
#include <iostream>

namespace xwift {

void printTokenKind(TokenKind kind) {
  switch (kind) {
    case TokenKind::EndOfFile: std::cout << "EOF"; break;
    case TokenKind::Identifier: std::cout << "Identifier"; break;
    case TokenKind::IntegerLiteral: std::cout << "IntegerLiteral"; break;
    case TokenKind::FloatLiteral: std::cout << "FloatLiteral"; break;
    case TokenKind::StringLiteral: std::cout << "StringLiteral"; break;
    default:
      if (kind > TokenKind::keyword_start && kind < TokenKind::keyword_end) {
        std::cout << "Keyword";
      } else {
        std::cout << "Token(" << static_cast<int>(kind) << ")";
      }
      break;
  }
}

void testLexer(const std::string& source) {
  std::cout << "=== Testing Lexer ===" << std::endl;
  std::cout << "Source: " << source << std::endl << std::endl;
  
  Lexer lexer(source);
  Token token;
  
  int count = 0;
  do {
    token = lexer.nextToken();
    std::cout << "[Token " << count << "] ";
    printTokenKind(token.Kind);
    if (!token.Text.empty()) {
      std::cout << " -> \"" << token.Text << "\"";
    }
    std::cout << " (line " << token.Loc.Line << ", col " << token.Loc.Col << ")" << std::endl;
    count++;
    if (count > 100) {
      std::cout << "Too many tokens!" << std::endl;
      break;
    }
  } while (token.Kind != TokenKind::EndOfFile);
  
  std::cout << std::endl << "Total tokens: " << count << std::endl;
}

}
