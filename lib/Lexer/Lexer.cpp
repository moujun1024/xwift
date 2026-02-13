#include "xwift/Lexer/Lexer.h"

namespace xwift {

std::unordered_map<std::string, TokenKind> Lexer::Keywords;
std::unordered_map<std::string, TokenKind> Lexer::Operators;
bool Lexer::IsInitialized = false;

Lexer::Lexer(const StringRef& buffer) 
  : Buffer(buffer), CurrentIndex(0), BufferStart(0), BufferEnd(buffer.size()),
    CurLine(1), CurCol(1), hasPeeked(false) {
  if (!IsInitialized) {
    initKeywords();
    IsInitialized = true;
  }
}

void Lexer::initKeywords() {
  Keywords = {
    {"func", TokenKind::kw_func},
    {"var", TokenKind::kw_var},
    {"let", TokenKind::kw_let},
    {"class", TokenKind::kw_class},
    {"struct", TokenKind::kw_struct},
    {"enum", TokenKind::kw_enum},
    {"protocol", TokenKind::kw_protocol},
    {"extension", TokenKind::kw_extension},
    {"import", TokenKind::kw_import},
    {"return", TokenKind::kw_return},
    {"if", TokenKind::kw_if},
    {"else", TokenKind::kw_else},
    {"switch", TokenKind::kw_switch},
    {"case", TokenKind::kw_case},
    {"default", TokenKind::kw_default},
    {"for", TokenKind::kw_for},
    {"while", TokenKind::kw_while},
    {"repeat", TokenKind::kw_repeat},
    {"break", TokenKind::kw_break},
    {"continue", TokenKind::kw_continue},
    {"fallthrough", TokenKind::kw_fallthrough},
    {"guard", TokenKind::kw_guard},
    {"defer", TokenKind::kw_defer},
    {"do", TokenKind::kw_do},
    {"try", TokenKind::kw_try},
    {"catch", TokenKind::kw_catch},
    {"throw", TokenKind::kw_throw},
    {"throws", TokenKind::kw_throws},
    {"rethrows", TokenKind::kw_rethrows},
    {"async", TokenKind::kw_async},
    {"await", TokenKind::kw_await},
    {"public", TokenKind::kw_public},
    {"private", TokenKind::kw_private},
    {"internal", TokenKind::kw_internal},
    {"fileprivate", TokenKind::kw_fileprivate},
    {"open", TokenKind::kw_open},
    {"static", TokenKind::kw_static},
    {"override", TokenKind::kw_override},
    {"final", TokenKind::kw_final},
    {"lazy", TokenKind::kw_lazy},
    {"weak", TokenKind::kw_weak},
    {"unowned", TokenKind::kw_unowned},
    {"mutating", TokenKind::kw_mutating},
    {"nonmutating", TokenKind::kw_nonmutating},
    {"inout", TokenKind::kw_inout},
    {"typealias", TokenKind::kw_typealias},
    {"associatedtype", TokenKind::kw_associatedtype},
    {"where", TokenKind::kw_where},
    {"self", TokenKind::kw_self},
    {"Self", TokenKind::kw_Self},
    {"init", TokenKind::kw_init},
    {"deinit", TokenKind::kw_deinit},
    {"subscript", TokenKind::kw_subscript},
    {"operator", TokenKind::kw_operator},
    {"prefix", TokenKind::kw_prefix},
    {"postfix", TokenKind::kw_postfix},
    {"infix", TokenKind::kw_infix},
    {"convention", TokenKind::kw_convention},
    {"Void", TokenKind::kw_void},
    {"Any", TokenKind::kw_any},
    {"some", TokenKind::kw_some},
    {"is", TokenKind::kw_is},
    {"as", TokenKind::kw_as},
    {"nil", TokenKind::kw_nil},
    {"true", TokenKind::kw_true},
    {"false", TokenKind::kw_false},
    {"in", TokenKind::kw_in},
    {"unsafe", TokenKind::kw_unsafe},
    {"optional", TokenKind::kw_optional},
    {"required", TokenKind::kw_required},
    {"willSet", TokenKind::kw_willSet},
    {"didSet", TokenKind::kw_didSet},
    {"get", TokenKind::kw_get},
    {"set", TokenKind::kw_set},
    {"actor", TokenKind::kw_actor},
    {"nonisolated", TokenKind::kw_nonisolated},
    {"isolated", TokenKind::kw_isolated},
    {"macro", TokenKind::kw_macro},
    {"type", TokenKind::kw_type},
    {"alias", TokenKind::kw_alias},
    {"each", TokenKind::kw_each},
  };
  
  Operators = {
    {"+", TokenKind::op_plus},
    {"-", TokenKind::op_minus},
    {"*", TokenKind::op_star},
    {"/", TokenKind::op_slash},
    {"%", TokenKind::op_percent},
    {"&", TokenKind::op_amp},
    {"|", TokenKind::op_bar},
    {"^", TokenKind::op_caret},
    {"~", TokenKind::op_tilde},
    {"!", TokenKind::op_bang},
    {"?", TokenKind::op_question},
    {"=", TokenKind::op_eq},
    {"<", TokenKind::op_lt},
    {">", TokenKind::op_gt},
    {"<=", TokenKind::op_le},
    {">=", TokenKind::op_ge},
    {"&&", TokenKind::op_amp_amp},
    {"||", TokenKind::op_bar_bar},
    {"<<", TokenKind::op_lt_lt},
    {">>", TokenKind::op_gt_gt},
    {"->", TokenKind::op_minus_gt},
    {"...", TokenKind::op_ellipsis},
    {"??", TokenKind::op_dot_question},
    {"!", TokenKind::op_bang},
  };
}

char Lexer::peekChar() const {
  if (CurrentIndex >= BufferEnd) return 0;
  return Buffer[CurrentIndex];
}

char Lexer::getChar() {
  if (CurrentIndex >= BufferEnd) return 0;
  char c = Buffer[CurrentIndex++];
  if (c == '\n') {
    CurLine++;
    CurCol = 1;
  } else {
    CurCol++;
  }
  return c;
}

void Lexer::consumeChar() {
  if (CurrentIndex >= BufferEnd) return;
  char c = Buffer[CurrentIndex++];
  if (c == '\n') {
    CurLine++;
    CurCol = 1;
  } else {
    CurCol++;
  }
}

bool Lexer::isWhitespace(char c) const {
  return c == ' ' || c == '\t' || c == '\n' || c == '\r';
}

bool Lexer::isDigit(char c) const {
  return c >= '0' && c <= '9';
}

bool Lexer::isAlpha(char c) const {
  return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || c == '_';
}

bool Lexer::isAlphaNumeric(char c) const {
  return isAlpha(c) || isDigit(c);
}

void Lexer::skipWhitespace() {
  while (isWhitespace(peekChar())) {
    consumeChar();
  }
}

void Lexer::skipLineComment() {
  consumeChar();
  consumeChar();
  while (peekChar() != '\n' && peekChar() != 0) {
    consumeChar();
  }
}

void Lexer::skipBlockComment() {
  consumeChar();
  consumeChar();
  while (true) {
    char c = peekChar();
    if (c == 0) break;
    if (c == '*' && Buffer[CurrentIndex + 1] == '/') {
      consumeChar();
      consumeChar();
      break;
    }
    consumeChar();
  }
}

SourceLocation Lexer::getCurLocation() const {
  return SourceLocation(CurLine, CurCol);
}

Token Lexer::lexIdentifier() {
  size_t start = CurrentIndex;
  while (isAlphaNumeric(peekChar())) {
    consumeChar();
  }
  
  StringRef text = Buffer.substr(start, CurrentIndex - start);
  SourceLocation loc = getCurLocation();
  loc.Col -= (CurrentIndex - start);
  
  auto it = Keywords.find(text);
  if (it != Keywords.end()) {
    return Token(it->second, loc, CurrentIndex - start, text);
  }
  
  return Token(TokenKind::Identifier, loc, CurrentIndex - start, text);
}

Token Lexer::lexNumber() {
  size_t start = CurrentIndex;
  bool hasDecimal = false;
  
  while (isDigit(peekChar())) {
    consumeChar();
  }
  
  if (peekChar() == '.') {
    if (Buffer[CurrentIndex + 1] == '.') {
    } else {
      hasDecimal = true;
      consumeChar();
      while (isDigit(peekChar())) {
        consumeChar();
      }
    }
  }
  
  if (peekChar() == 'e' || peekChar() == 'E') {
    consumeChar();
    if (peekChar() == '+' || peekChar() == '-') {
      consumeChar();
    }
    while (isDigit(peekChar())) {
      consumeChar();
    }
  }
  
  StringRef text = Buffer.substr(start, CurrentIndex - start);
  SourceLocation loc = getCurLocation();
  loc.Col -= (CurrentIndex - start);
  
  TokenKind kind = hasDecimal ? TokenKind::FloatLiteral : TokenKind::IntegerLiteral;
  return Token(kind, loc, CurrentIndex - start, text);
}

Token Lexer::lexString() {
  size_t start = CurrentIndex;
  char quote = peekChar();
  consumeChar();
  
  std::string result;
  
  while (peekChar() != quote && peekChar() != 0) {
    if (peekChar() == '\\') {
      consumeChar();
      if (peekChar() != 0) {
        char c = peekChar();
        switch (c) {
          case 'n': result += '\n'; break;
          case 'r': result += '\r'; break;
          case 't': result += '\t'; break;
          case 'b': result += '\b'; break;
          case 'f': result += '\f'; break;
          case '\\': result += '\\'; break;
          case '"': result += '"'; break;
          case '\'': result += '\''; break;
          default: result += c; break;
        }
        consumeChar();
      }
    } else {
      result += peekChar();
      consumeChar();
    }
  }
  
  if (peekChar() == quote) {
    consumeChar();
  }
  
  SourceLocation loc = getCurLocation();
  loc.Col -= (CurrentIndex - start);
  
  return Token(TokenKind::StringLiteral, loc, CurrentIndex - start, result);
}

Token Lexer::lexPunctuation() {
  size_t start = CurrentIndex;
  char c = getChar();
  SourceLocation loc = getCurLocation();
  loc.Col--;
  
  TokenKind kind = TokenKind::Unknown;
  
  switch (c) {
    case '(': kind = TokenKind::punct_l_paren; break;
    case ')': kind = TokenKind::punct_r_paren; break;
    case '{': kind = TokenKind::punct_l_brace; break;
    case '}': kind = TokenKind::punct_r_brace; break;
    case '[': kind = TokenKind::punct_l_bracket; break;
    case ']': kind = TokenKind::punct_r_bracket; break;
    case ',': kind = TokenKind::punct_comma; break;
    case ':': kind = TokenKind::punct_colon; break;
    case ';': kind = TokenKind::punct_semicolon; break;
    case '.': 
      if (peekChar() == '.') {
        consumeChar();
        return Token(TokenKind::punct_dot_dot, getCurLocation(), 2, "..");
      }
      kind = TokenKind::punct_dot; 
      break;
    case '?': kind = TokenKind::punct_question; break;
    case '!': kind = TokenKind::punct_exclaim; break;
    case '=': kind = TokenKind::punct_equal; break;
    default: break;
  }
  
  return Token(kind, loc, 1, StringRef(1, c));
}

Token Lexer::lexOperator() {
  size_t start = CurrentIndex;
  SourceLocation loc = getCurLocation();
  
  while (true) {
    char c = peekChar();
    if (c == 0) break;
    
    bool isOpChar = false;
    switch (c) {
      case '+': case '-': case '*': case '/': case '%':
      case '&': case '|': case '^': case '~': case '!':
      case '=': case '<': case '>': case '?':
        isOpChar = true;
        break;
    }
    
    if (!isOpChar) break;
    
    consumeChar();
  }
  
  loc.Col -= (CurrentIndex - start);
  StringRef text = Buffer.substr(start, CurrentIndex - start);
  
  auto it = Operators.find(text);
  if (it != Operators.end()) {
    return Token(it->second, loc, CurrentIndex - start, text);
  }
  
  return Token(TokenKind::Unknown, loc, CurrentIndex - start, text);
}

Token Lexer::nextToken() {
  if (hasPeeked) {
    hasPeeked = false;
    return curToken;
  }
  
  skipWhitespace();
  
  char c = peekChar();
  
  if (c == 0) {
    return Token(TokenKind::EndOfFile, getCurLocation(), 0);
  }
  
  if (c == '/' && Buffer[CurrentIndex + 1] == '/') {
    skipLineComment();
    return nextToken();
  }
  
  if (c == '/' && Buffer[CurrentIndex + 1] == '*') {
    skipBlockComment();
    return nextToken();
  }
  
  if (isAlpha(c)) {
    return lexIdentifier();
  }
  
  if (isDigit(c)) {
    return lexNumber();
  }
  
  if (c == '"' || c == '\'') {
    return lexString();
  }
  
  if (c == '(' || c == ')' || c == '{' || c == '}' ||
      c == '[' || c == ']' || c == ',' || c == ':' ||
      c == ';' || c == '.' || c == '?' || c == '!' || c == '=') {
    return lexPunctuation();
  }
  
  if (c == '-' && Buffer[CurrentIndex + 1] == '>') {
    consumeChar();
    consumeChar();
    return Token(TokenKind::op_minus_gt, getCurLocation(), 2, "->");
  }
  
  if (c == '.' && Buffer[CurrentIndex + 1] == '.' && Buffer[CurrentIndex + 2] == '.') {
    consumeChar();
    consumeChar();
    consumeChar();
    return Token(TokenKind::op_ellipsis, getCurLocation(), 3, "...");
  }
  
  return lexOperator();
}

Token Lexer::peekToken() {
  if (!hasPeeked) {
    curToken = nextToken();
    hasPeeked = true;
  }
  return curToken;
}

bool Token::isPunctuation() const {
  return Kind >= TokenKind::punct_l_paren && Kind <= TokenKind::punct_arrow;
}

bool Token::isOperator() const {
  return Kind >= TokenKind::op_plus && Kind <= TokenKind::op_ellipsis;
}

}
