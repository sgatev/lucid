#pragma once

#include <cstddef>
#include <cstdint>
#include <optional>
#include <ostream>
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
  enum class Kind : std::uint8_t {
    End,
    ExpectedIdent,
    ExpectedNumber,
    UnexpectedToken,
    ExpectedClosingParenOrParam,
    ExpectedClosingParenOrExpr,
    ExpectedLetKeyword,
  };

  explicit ParserError(Kind kind, std::size_t line, std::size_t col)
      : kind_(kind), line_(line), col_(col) {}

  Kind GetKind() const { return kind_; }

  friend std::ostream& operator<<(std::ostream& out, const ParserError error) {
    return out << error.KindString() << " at line " << error.line_
               << ", column " << error.col_;
  }

 private:
  std::string_view KindString() const {
    switch (kind_) {
      case Kind::End:
        return "end";
      case Kind::ExpectedIdent:
        return "expected identifier";
      case Kind::ExpectedNumber:
        return "expected number";
      case Kind::UnexpectedToken:
        return "unexpected token";
      case Kind::ExpectedClosingParenOrParam:
        return "expected closing parenthesis or parameter";
      case Kind::ExpectedClosingParenOrExpr:
        return "expected closing parenthesis or expression";
      case Kind::ExpectedLetKeyword:
        return "expected 'let' keyword";
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
        next_(lexer_.next()) {}

  std::variant<FuncDefStmt, ParserError> ParseFuncDef() {
    if (Peek().kind == Token::Kind::End) {
      return MakeError(ParserError::Kind::End, Peek());
    }

    FuncDefStmt stmt;

    if (auto r = ExpectIdent("let", ParserError::Kind::ExpectedLetKeyword);
        IsError(r)) {
      return *r;
    }

    const auto maybe_name = ParseIdent();
    if (IsError(maybe_name)) return std::get<ParserError>(maybe_name);
    stmt.name = std::get<std::string_view>(maybe_name);

    if (auto r = ExpectToken(Token::Kind::Equal); IsError(r)) return *r;

    if (auto r = ExpectToken(Token::Kind::OpenParen); IsError(r)) return *r;
    while (true) {
      if (Peek().kind == Token::Kind::Comma) {
        Read();
      } else if (Peek().kind == Token::Kind::CloseParen) {
        Read();
        break;
      }

      if (Peek().kind != Token::Kind::Ident) {
        return MakeError(ParserError::Kind::ExpectedClosingParenOrParam,
                         Peek());
      }

      auto maybe_param = ParseParam();
      if (IsError(maybe_param)) return std::get<ParserError>(maybe_param);
      stmt.parameters.push_back(std::move(std::get<FuncParam>(maybe_param)));
    }

    if (auto r = ExpectToken(Token::Kind::Minus); IsError(r)) return *r;
    if (auto r = ExpectToken(Token::Kind::Greater); IsError(r)) return *r;

    const auto maybe_result_type = ParseIdent();
    if (IsError(maybe_result_type)) {
      return std::get<ParserError>(maybe_result_type);
    }
    stmt.result_type = std::get<std::string_view>(maybe_result_type);

    auto maybe_body = ParseCompoundStmt();
    if (IsError(maybe_body)) return std::get<ParserError>(maybe_body);
    stmt.body = std::get<CompoundStmt>(std::move(maybe_body));

    return stmt;
  }

 private:
  std::variant<FuncParam, ParserError> ParseParam() {
    FuncParam param;

    const auto maybe_name = ParseIdent();
    if (IsError(maybe_name)) return std::get<ParserError>(maybe_name);
    param.name = std::get<std::string_view>(maybe_name);

    if (auto r = ExpectToken(Token::Kind::Colon); IsError(r)) return *r;

    const auto maybe_type = ParseIdent();
    if (IsError(maybe_type)) return std::get<ParserError>(maybe_type);
    param.type = std::get<std::string_view>(maybe_type);

    return std::move(param);
  }

  std::variant<std::string_view, ParserError> ParseIdent() {
    Token token = Read();
    if (token.kind == Token::Kind::Ident) return TokenString(token);
    return MakeError(ParserError::Kind::ExpectedIdent, token);
  }

  std::variant<CompoundStmt, ParserError> ParseCompoundStmt() {
    CompoundStmt stmt;

    if (auto r = ExpectToken(Token::Kind::OpenBrace); IsError(r)) return *r;

    while (Peek().kind != Token::Kind::CloseBrace) {
      const auto maybe_stmt = ParseStmt();
      if (IsError(maybe_stmt)) return std::get<ParserError>(maybe_stmt);
      stmt.statements.push_back(std::get<StmtRef>(maybe_stmt));
    }

    Read();

    return stmt;
  }

  std::variant<StmtRef, ParserError> ParseStmt() {
    if (Peek().kind == Token::Kind::Ident && TokenString(Peek()) == "return") {
      Read();

      const auto maybe_value = ParseExpr();
      if (IsError(maybe_value)) return std::get<ParserError>(maybe_value);

      return arena_.add(ReturnStmt{
          .value = std::get<ExprRef>(maybe_value),
      });
    }
    if (Peek().kind == Token::Kind::Ident && TokenString(Peek()) == "if") {
      Read();

      const auto cond = ParseExpr();
      if (IsError(cond)) return std::get<ParserError>(cond);

      auto then_body = ParseCompoundStmt();
      if (IsError(then_body)) return std::get<ParserError>(then_body);

      if (auto r = ExpectIdent("else", ParserError::Kind::ExpectedLetKeyword);
          IsError(r)) {
        return *r;
      }

      auto else_body = ParseCompoundStmt();
      if (IsError(else_body)) return std::get<ParserError>(else_body);

      return arena_.add(IfStmt{
          .cond = std::get<ExprRef>(cond),
          .then_body = std::get<CompoundStmt>(then_body),
          .else_body = std::get<CompoundStmt>(else_body),
      });
    }
    if (Peek().kind == Token::Kind::Ident && TokenString(Peek()) == "let") {
      Read();

      const auto maybe_name = ParseIdent();
      if (IsError(maybe_name)) return std::get<ParserError>(maybe_name);

      if (auto r = ExpectToken(Token::Kind::Colon); IsError(r)) return *r;

      const auto maybe_type = ParseIdent();
      if (IsError(maybe_type)) return std::get<ParserError>(maybe_type);

      if (auto r = ExpectToken(Token::Kind::Equal); IsError(r)) return *r;

      const auto init = ParseExpr();
      if (IsError(init)) return std::get<ParserError>(init);

      return arena_.add(VarDeclStmt{
          .name = std::get<std::string_view>(maybe_name),
          .type = std::get<std::string_view>(maybe_type),
          .init = std::get<ExprRef>(init),
      });
    }
    return MakeError(ParserError::Kind::UnexpectedToken, Peek());
  }

  std::variant<ExprRef, ParserError> ParseExpr() {
    std::variant<ExprRef, ParserError> maybe_expr;
    if (Peek().kind == Token::Kind::Ident) {
      maybe_expr = ParseExprStartingWithIdent();
    } else if (Peek().kind == Token::Kind::Number) {
      maybe_expr = ParseNumber();
    } else {
      maybe_expr = MakeError(ParserError::Kind::UnexpectedToken, Peek());
    }
    if (IsError(maybe_expr)) return std::get<ParserError>(maybe_expr);

    if (Peek().kind == Token::Kind::Plus) {
      Read();

      const auto maybe_rhs = ParseExpr();
      if (IsError(maybe_rhs)) return std::get<ParserError>(maybe_rhs);

      return MakeBinaryOpExpr(BinaryOp::Add, std::get<ExprRef>(maybe_expr),
                              std::get<ExprRef>(maybe_rhs));
    } else if (Peek().kind == Token::Kind::Minus) {
      Read();

      const auto maybe_rhs = ParseExpr();
      if (IsError(maybe_rhs)) return std::get<ParserError>(maybe_rhs);

      return MakeBinaryOpExpr(BinaryOp::Sub, std::get<ExprRef>(maybe_expr),
                              std::get<ExprRef>(maybe_rhs));
    } else if (Peek().kind == Token::Kind::Star) {
      Read();

      const auto maybe_rhs = ParseExpr();
      if (IsError(maybe_rhs)) return std::get<ParserError>(maybe_rhs);

      return MakeBinaryOpExpr(BinaryOp::Mul, std::get<ExprRef>(maybe_expr),
                              std::get<ExprRef>(maybe_rhs));
    } else if (Peek().kind == Token::Kind::Slash) {
      Read();

      const auto maybe_rhs = ParseExpr();
      if (IsError(maybe_rhs)) return std::get<ParserError>(maybe_rhs);

      return MakeBinaryOpExpr(BinaryOp::Div, std::get<ExprRef>(maybe_expr),
                              std::get<ExprRef>(maybe_rhs));
    } else if (Peek().kind == Token::Kind::Greater) {
      Read();

      const auto maybe_rhs = ParseExpr();
      if (IsError(maybe_rhs)) return std::get<ParserError>(maybe_rhs);

      return MakeBinaryOpExpr(BinaryOp::Gt, std::get<ExprRef>(maybe_expr),
                              std::get<ExprRef>(maybe_rhs));
    } else if (Peek().kind == Token::Kind::Less) {
      Read();

      const auto maybe_rhs = ParseExpr();
      if (IsError(maybe_rhs)) return std::get<ParserError>(maybe_rhs);

      return MakeBinaryOpExpr(BinaryOp::Lt, std::get<ExprRef>(maybe_expr),
                              std::get<ExprRef>(maybe_rhs));
    } else if (Peek().kind == Token::Kind::DoubleEqual) {
      Read();

      const auto maybe_rhs = ParseExpr();
      if (IsError(maybe_rhs)) return std::get<ParserError>(maybe_rhs);

      return MakeBinaryOpExpr(BinaryOp::Eq, std::get<ExprRef>(maybe_expr),
                              std::get<ExprRef>(maybe_rhs));
    }

    return maybe_expr;
  }

  std::variant<ExprRef, ParserError> ParseExprStartingWithIdent() {
    const auto ident = std::get<std::string_view>(ParseIdent());

    if (Peek().kind == Token::Kind::OpenParen) {
      Read();

      FuncCallExpr expr;
      expr.func_name = ident;

      while (Peek().kind != Token::Kind::CloseParen) {
        auto maybe_arg = ParseExpr();
        if (IsError(maybe_arg)) return std::get<ParserError>(maybe_arg);
        expr.arguments.push_back(std::get<ExprRef>(maybe_arg));

        if (Peek().kind == Token::Kind::Comma) Read();
      }
      Read();

      return arena_.add(std::move(expr));
    }

    if (ident == "true" || ident == "false") {
      return arena_.add(BoolLitExpr{
          .value = ident,
      });
    }

    return arena_.add(IdentExpr{
        .name = ident,
    });
  }

  std::variant<ExprRef, ParserError> ParseNumber() {
    Token token = Read();
    if (token.kind != Token::Kind::Number) {
      return MakeError(ParserError::Kind::ExpectedNumber, token);
    }
    return arena_.add(IntLitExpr{
        .value = TokenString(token),
    });
  }

  ExprRef MakeBinaryOpExpr(BinaryOp op, ExprRef lhs, ExprRef rhs) {
    return arena_.add(BinaryOpExpr{.op = op, .lhs = lhs, .rhs = rhs});
  }

  std::optional<ParserError> ExpectToken(Token::Kind kind) {
    Token token = Read();
    if (token.kind == kind) return std::nullopt;
    return MakeError(ParserError::Kind::UnexpectedToken, token);
  }

  std::optional<ParserError> ExpectIdent(std::string_view text,
                                         ParserError::Kind error_kind) {
    Token token = Read();
    if (token.kind != Token::Kind::Ident || TokenString(token) != text) {
      return MakeError(error_kind, token);
    }
    return std::nullopt;
  }

  std::string_view TokenString(Token token) const {
    return buffer_.substr(token.start_pos, token.end_pos - token.start_pos);
  }

  Token Read() { return std::exchange(next_, lexer_.next()); }

  const Token& Peek() const { return next_; }

  ParserError MakeError(ParserError::Kind kind, const Token& token) const {
    return ParserError(kind, FindLine(buffer_, token),
                       FindColumn(buffer_, token));
  }

  template <typename T>
  bool IsError(const std::variant<T, ParserError>& r) const {
    return std::holds_alternative<ParserError>(r);
  }

  bool IsError(const std::optional<ParserError>& r) const {
    return r.has_value();
  }

  Arena<Stmt>& arena_;
  std::string_view buffer_;
  LexerT lexer_;
  Token next_;
};

}  // namespace lucid
