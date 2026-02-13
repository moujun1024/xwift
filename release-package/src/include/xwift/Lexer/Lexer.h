#ifndef XWIFT_LEXER_LEXER_H
#define XWIFT_LEXER_LEXER_H

#include "xwift/Lexer/Token.h"
#include <cctype>
#include <unordered_map>

namespace xwift {

class Lexer {
public:
  Lexer(const StringRef& buffer);
  
  Token nextToken();
  Token peekToken();
  
  void setIndex(size_t idx) { CurrentIndex = idx; }
  size_t getIndex() const { return CurrentIndex; }
  
private:
  StringRef Buffer;
  size_t CurrentIndex;
  size_t BufferStart;
  size_t BufferEnd;
  
  unsigned CurLine;
  unsigned CurCol;
  
  Token curToken;
  bool hasPeeked;
  
  static std::unordered_map<std::string, TokenKind> Keywords;
  static std::unordered_map<std::string, TokenKind> Operators;
  static bool IsInitialized;
  
  static void initKeywords();
  
  char peekChar() const;
  char getChar();
  void consumeChar();
  
  bool isWhitespace(char c) const;
  bool isDigit(char c) const;
  bool isAlpha(char c) const;
  bool isAlphaNumeric(char c) const;
  
  void skipWhitespace();
  void skipLineComment();
  void skipBlockComment();
  
  Token lexIdentifier();
  Token lexNumber();
  Token lexString();
  Token lexPunctuation();
  Token lexOperator();
  
  SourceLocation getCurLocation() const;
};

}

#endif
