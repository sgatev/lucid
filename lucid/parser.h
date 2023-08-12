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
    ExpectedNumber,
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
      case Kind::ExpectedNumber:
        return "expected number";
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

    if (auto r = ExpectToken(Token::Kind::Minus); IsError(r)) return *r;
    if (auto r = ExpectToken(Token::Kind::Greater); IsError(r)) return *r;

    const auto maybe_result_type = ParseName();
    if (IsError(maybe_result_type)) {
      return std::get<ParserError>(maybe_result_type);
    }

    const auto maybe_body = ParseCompoundStmt();
    if (IsError(maybe_body)) return std::get<ParserError>(maybe_body);

    return arena_.add(FuncDefStmt{
        .name = std::get<std::string_view>(maybe_name),
        .result_type = std::get<std::string_view>(maybe_result_type),
        .body = std::get<CompoundStmt>(maybe_body),
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

  std::variant<std::string_view, ParserError> ParseNumber() {
    Token token = Read();
    if (token.kind != Token::Kind::Number) {
      return MakeError(ParserError::Kind::ExpectedNumber, token);
    }
    return buffer_.substr(token.start_pos, token.end_pos - token.start_pos);
  }

  std::variant<CompoundStmt, ParserError> ParseCompoundStmt() {
    CompoundStmt stmt;

    if (auto r = ExpectToken(Token::Kind::OpenBrace); IsError(r)) return *r;

    if (Peek().kind != Token::Kind::CloseBrace &&
        Peek().kind != Token::Kind::End) {
      if (auto r = ExpectIdent("return"); IsError(r)) return *r;

      const auto maybe_value = ParseExpr();
      if (IsError(maybe_value)) return std::get<ParserError>(maybe_value);

      stmt.statements.push_back(arena_.add(ReturnStmt{
          .value = std::get<ExprRef>(maybe_value),
      }));
    }

    if (auto r = ExpectToken(Token::Kind::CloseBrace); IsError(r)) return *r;

    return stmt;
  }

  std::variant<ExprRef, ParserError> ParseExpr() {
    const auto maybe_number = ParseNumber();
    if (IsError(maybe_number)) return std::get<ParserError>(maybe_number);
    auto number_expr = arena_.add(IntLitExpr{
        .value = std::get<std::string_view>(maybe_number),
    });

    if (Peek().kind == Token::Kind::Plus) {
      if (auto r = ExpectToken(Token::Kind::Plus); IsError(r)) return *r;

      const auto maybe_rhs = ParseExpr();
      if (IsError(maybe_rhs)) return std::get<ParserError>(maybe_rhs);

      return arena_.add(BinaryOpExpr{
          .op = BinaryOp::Add,
          .lhs = number_expr,
          .rhs = std::get<ExprRef>(maybe_rhs),
      });
    } else if (Peek().kind == Token::Kind::Minus) {
      if (auto r = ExpectToken(Token::Kind::Minus); IsError(r)) return *r;

      const auto maybe_rhs = ParseExpr();
      if (IsError(maybe_rhs)) return std::get<ParserError>(maybe_rhs);

      return arena_.add(BinaryOpExpr{
          .op = BinaryOp::Sub,
          .lhs = number_expr,
          .rhs = std::get<ExprRef>(maybe_rhs),
      });
    } else if (Peek().kind == Token::Kind::Star) {
      if (auto r = ExpectToken(Token::Kind::Star); IsError(r)) return *r;

      const auto maybe_rhs = ParseExpr();
      if (IsError(maybe_rhs)) return std::get<ParserError>(maybe_rhs);

      return arena_.add(BinaryOpExpr{
          .op = BinaryOp::Mul,
          .lhs = number_expr,
          .rhs = std::get<ExprRef>(maybe_rhs),
      });
    } else if (Peek().kind == Token::Kind::Slash) {
      if (auto r = ExpectToken(Token::Kind::Slash); IsError(r)) return *r;

      const auto maybe_rhs = ParseExpr();
      if (IsError(maybe_rhs)) return std::get<ParserError>(maybe_rhs);

      return arena_.add(BinaryOpExpr{
          .op = BinaryOp::Div,
          .lhs = number_expr,
          .rhs = std::get<ExprRef>(maybe_rhs),
      });
    }

    return number_expr;
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
