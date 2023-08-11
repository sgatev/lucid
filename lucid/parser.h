#pragma once

#include <cstddef>
#include <string>
#include <string_view>
#include <utility>
#include <variant>

#include "lucid/arena.h"
#include "lucid/ast.h"
#include "lucid/lexer.h"
#include "lucid/token.h"

namespace lucid {

// An error that occurred while parsing Lucid code.
class ParserError {
 public:
  enum class Kind : uint8_t {
    End,
    ExpectedIdent,
    UnexpectedToken,
    ExpectedLet,
  };

  explicit ParserError(Kind kind, std::size_t line, std::size_t col)
      : kind_(kind), line_(line), col_(col) {}

  Kind GetKind() const { return kind_; }

  std::string ToString() const {
    return "parse error: " + KindString() + " at line " +
           std::to_string(line_) + ", column " + std::to_string(col_);
  }

 private:
  std::string KindString() const {
    switch (kind_) {
      case Kind::End:
        return "end";
      case Kind::ExpectedIdent:
        return "expected identifier";
      case Kind::UnexpectedToken:
        return "unexpected token";
      case Kind::ExpectedLet:
        return "expected `let` keyword";
    }
  }

  Kind kind_;
  std::size_t line_;
  std::size_t col_;
};

// Converts a string of Lucid code into a stream of AST nodes.
template <typename LexerT>
class Parser {
 public:
  explicit Parser(Arena<Stmt>& arena, std::string_view buffer, LexerT lexer)
      : arena_(arena),
        buffer_(buffer),
        lexer_(std::move(lexer)),
        next_(lexer_.Next()) {}

  std::variant<StmtRef, ParserError> ParseFuncDef() {
    if (Peek().kind == Token::Kind::End) {
      return MakeError(ParserError::Kind::End, Peek());
    }

    if (auto r = ExpectIdent("let"); IsError(r)) return *r;

    const auto maybe_name = ParseName();
    if (IsError(maybe_name)) return std::get<ParserError>(maybe_name);

    if (auto r = ExpectToken(Token::Kind::Equal); IsError(r)) return *r;
    if (auto r = ExpectToken(Token::Kind::OpenParen); IsError(r)) return *r;
    if (auto r = ExpectToken(Token::Kind::CloseParen); IsError(r)) return *r;
    if (auto r = ExpectToken(Token::Kind::OpenBrace); IsError(r)) return *r;
    if (auto r = ExpectToken(Token::Kind::CloseBrace); IsError(r)) return *r;

    return arena_.add(FuncDefStmt{
        .name = std::get<std::string_view>(maybe_name),
    });
  }

 private:
  std::variant<std::string_view, ParserError> ParseName() {
    Token token = Read();
    if (token.kind != Token::Kind::Ident) {
      return MakeError(ParserError::Kind::ExpectedIdent, token);
    }
    return buffer_.substr(token.start_pos, token.end_pos - token.start_pos);
  }

  std::optional<ParserError> ExpectToken(Token::Kind kind) {
    Token token = Read();
    if (token.kind == kind) return std::nullopt;
    return MakeError(ParserError::Kind::UnexpectedToken, token);
  }

  std::optional<ParserError> ExpectIdent(std::string_view text) {
    Token token = Read();
    if (token.kind != Token::Kind::Ident || Materialize(token) != text) {
      return MakeError(ParserError::Kind::ExpectedLet, token);
    }
    return std::nullopt;
  }

  std::string_view Materialize(Token token) const {
    return buffer_.substr(token.start_pos, token.end_pos - token.start_pos);
  }

  Token Read() { return std::exchange(next_, lexer_.Next()); }

  const Token& Peek() const { return next_; }

  ParserError MakeError(ParserError::Kind kind, const Token& token) {
    return ParserError(kind, FindLine(buffer_, token),
                       FindColumn(buffer_, token));
  }

  template <typename T>
  bool IsError(const std::variant<T, ParserError>& r) {
    return std::holds_alternative<ParserError>(r);
  }

  bool IsError(const std::optional<ParserError>& r) { return r.has_value(); }

  Arena<Stmt>& arena_;
  std::string_view buffer_;
  LexerT lexer_;
  Token next_;
};

}  // namespace lucid
