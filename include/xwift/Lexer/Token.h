#ifndef XWIFT_LEXER_TOKEN_H
#define XWIFT_LEXER_TOKEN_H

#include "xwift/Basic/LLVM.h"

namespace xwift {

enum class TokenKind {
  EndOfFile,
  
  Unknown,
  
  Identifier,
  IntegerLiteral,
  FloatLiteral,
  StringLiteral,
  CharacterLiteral,
  
  keyword_start,
  kw_func,
  kw_var,
  kw_let,
  kw_class,
  kw_struct,
  kw_enum,
  kw_protocol,
  kw_extension,
  kw_import,
  kw_return,
  kw_if,
  kw_else,
  kw_switch,
  kw_case,
  kw_default,
  kw_for,
  kw_while,
  kw_repeat,
  kw_break,
  kw_continue,
  kw_fallthrough,
  kw_guard,
  kw_defer,
  kw_do,
  kw_try,
  kw_catch,
  kw_throw,
  kw_throws,
  kw_rethrows,
  kw_async,
  kw_await,
  kw_public,
  kw_private,
  kw_internal,
  kw_fileprivate,
  kw_open,
  kw_static,
  kw_override,
  kw_final,
  kw_lazy,
  kw_weak,
  kw_unowned,
  kw_mutating,
  kw_nonmutating,
  kw_inout,
  kw_typealias,
  kw_associatedtype,
  kw_where,
  kw_self,
  kw_Self,
  kw_init,
  kw_deinit,
  kw_subscript,
  kw_operator,
  kw_prefix,
  kw_postfix,
  kw_infix,
  kw_convention,
  kw_void,
  kw_any,
  kw_some,
  kw_is,
  kw_as,
  kw_nil,
  kw_true,
  kw_false,
  kw_in,
  kw_unsafe,
  kw_optional,
  kw_required,
  kw_willSet,
  kw_didSet,
  kw_get,
  kw_set,
  kw_actor,
  kw_nonisolated,
  kw_isolated,
  kw_macro,
  kw_unknown,
  kw_type,
  kw_alias,
  kw_each,
  kw_willMove,
  kw_didMove,
  kw_willObserve,
  kw_didObserve,
  kw_on,
  keyword_end,
  
  punct_l_paren,
  punct_r_paren,
  punct_l_brace,
  punct_r_brace,
  punct_l_bracket,
  punct_r_bracket,
  punct_comma,
  punct_colon,
  punct_semicolon,
  punct_dot,
  punct_dot_dot,
  punct_question,
  punct_exclaim,
  punct_equal,
  punct_arrow,
  
  op_plus,
  op_minus,
  op_star,
  op_slash,
  op_percent,
  op_amp,
  op_bar,
  op_caret,
  op_tilde,
  op_bang,
  op_question,
  
  op_eq,
  op_plus_eq,
  op_minus_eq,
  op_star_eq,
  op_slash_eq,
  op_percent_eq,
  op_amp_eq,
  op_bar_eq,
  op_caret_eq,
  
  op_lt,
  op_gt,
  op_le,
  op_ge,
  
  op_amp_amp,
  op_bar_bar,
  
  op_lt_lt,
  op_gt_gt,
  
  op_dot_question,
  op_dot_exclaim,
  
  op_minus_gt,
  
  op_ellipsis,
};

class SourceLocation {
public:
  unsigned Line;
  unsigned Col;
  unsigned FileID;
  
  SourceLocation() : Line(0), Col(0), FileID(0) {}
  SourceLocation(unsigned line, unsigned col, unsigned fileID = 0) 
    : Line(line), Col(col), FileID(fileID) {}
};

class SourceRange {
public:
  SourceLocation Start;
  SourceLocation End;
  
  SourceRange() {}
  SourceRange(SourceLocation start, SourceLocation end) 
    : Start(start), End(end) {}
};

class Token {
public:
  TokenKind Kind;
  SourceLocation Loc;
  unsigned Length;
  StringRef Text;
  
  Token() : Kind(TokenKind::Unknown), Length(0) {}
  Token(TokenKind kind, SourceLocation loc, unsigned len, StringRef text = "")
    : Kind(kind), Loc(loc), Length(len), Text(text) {}
  
  bool is(TokenKind K) const { return Kind == K; }
  bool isNot(TokenKind K) const { return Kind != K; }
  
  bool isKeyword() const {
    return Kind > TokenKind::keyword_start && Kind < TokenKind::keyword_end;
  }
  
  bool isIdentifier() const {
    return Kind == TokenKind::Identifier;
  }
  
  bool isLiteral() const {
    return Kind == TokenKind::IntegerLiteral ||
           Kind == TokenKind::FloatLiteral ||
           Kind == TokenKind::StringLiteral ||
           Kind == TokenKind::CharacterLiteral;
  }
  
  bool isPunctuation() const;
  
  bool isOperator() const;
};

}

#endif
