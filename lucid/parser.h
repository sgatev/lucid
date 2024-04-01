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
    ExpectedString,
    UnexpectedToken,
    ExpectedClosingParenOrParam,
    ExpectedClosingParenOrExpr,
    ExpectedLetKeyword,
    IncompleteStringLiteral,
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
      case Kind::ExpectedString:
        return "expected string";
      case Kind::UnexpectedToken:
        return "unexpected token";
      case Kind::ExpectedClosingParenOrParam:
        return "expected closing parenthesis or parameter";
      case Kind::ExpectedClosingParenOrExpr:
        return "expected closing parenthesis or expression";
      case Kind::ExpectedLetKeyword:
        return "expected 'let' keyword";
      case Kind::IncompleteStringLiteral:
        return "incomplete string literal";
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
  explicit Parser(SyntaxContext& ctx, std::string_view buffer, LexerT lexer)
      : ctx_(ctx),
        buffer_(buffer),
        lexer_(std::move(lexer)),
        next_(lexer_.next()) {}

  std::variant<FuncDefStmt, ParserError> ParseFuncDef() {
    SkipSpace();

    if (Peek().kind == Token::Kind::End) {
      return MakeError(ParserError::Kind::End, Peek());
    }

    FuncDefStmt stmt;

    if (auto r = ExpectIdent("let", ParserError::Kind::ExpectedLetKeyword);
        IsError(r)) {
      return *r;
    }

    SkipSpace();

    const auto maybe_name = ParseIdent();
    if (IsError(maybe_name)) return std::get<ParserError>(maybe_name);
    stmt.name = std::get<std::string_view>(maybe_name);

    SkipSpace();

    if (auto r = ExpectToken(Token::Kind::Equal); IsError(r)) return *r;

    SkipSpace();

    std::uint8_t params_size = 0;
    ParamRef first_param = Arena<FuncParam>::kNullRef;
    if (auto r = ExpectToken(Token::Kind::OpenParen); IsError(r)) return *r;
    while (true) {
      if (Peek().kind == Token::Kind::Comma) {
        Read();

        SkipSpace();
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
      auto param = ctx_.Add(std::move(std::get<FuncParam>(maybe_param)));
      if (params_size == 0) first_param = param;
      ++params_size;
    }
    stmt.params = List<ParamRef>(params_size, first_param);

    SkipSpace();

    if (auto r = ExpectToken(Token::Kind::Minus); IsError(r)) return *r;
    if (auto r = ExpectToken(Token::Kind::Greater); IsError(r)) return *r;

    SkipSpace();

    const auto maybe_result_type = ParseType();
    if (IsError(maybe_result_type)) {
      return std::get<ParserError>(maybe_result_type);
    }
    stmt.result_type = std::get<TypeRef>(maybe_result_type);

    auto maybe_body = ParseCompoundStmt();
    if (IsError(maybe_body)) return std::get<ParserError>(maybe_body);
    stmt.stmts = std::get<List<StmtRef>>(std::move(maybe_body));

    return stmt;
  }

 private:
  std::variant<FuncParam, ParserError> ParseParam() {
    FuncParam param;

    const auto maybe_name = ParseIdent();
    if (IsError(maybe_name)) return std::get<ParserError>(maybe_name);
    param.name = std::get<std::string_view>(maybe_name);

    if (auto r = ExpectToken(Token::Kind::Colon); IsError(r)) return *r;

    SkipSpace();

    const auto maybe_type = ParseType();
    if (IsError(maybe_type)) return std::get<ParserError>(maybe_type);
    param.type = std::get<TypeRef>(maybe_type);

    return std::move(param);
  }

  std::variant<std::string_view, ParserError> ParseIdent() {
    Token token = Read();
    if (token.kind == Token::Kind::Ident) return TokenString(token);
    return MakeError(ParserError::Kind::ExpectedIdent, token);
  }

  std::variant<List<StmtRef>, ParserError> ParseCompoundStmt() {
    SkipSpace();

    if (auto r = ExpectToken(Token::Kind::OpenBrace); IsError(r)) return *r;

    const auto start_idx = pending_stmts_.size();
    while (true) {
      SkipSpace();

      if (Peek().kind == Token::Kind::CloseBrace) break;

      const auto maybe_stmt = ParseStmt();
      if (IsError(maybe_stmt)) return std::get<ParserError>(maybe_stmt);
      pending_stmts_.push_back(std::get<StmtRef>(maybe_stmt));
    }

    Read();

    const std::uint8_t args_size = pending_stmts_.size() - start_idx;
    StmtRef args_first = Arena<Stmt>::kNullRef;
    if (args_size > 0) {
      args_first = ctx_.AliasStmt(pending_stmts_[start_idx]);
    }
    for (std::size_t i = 1; i < args_size; ++i) {
      ctx_.AliasStmt(pending_stmts_[start_idx + i]);
    }
    for (std::size_t i = 0; i < args_size; ++i) {
      pending_stmts_.pop_back();
    }

    return List<StmtRef>(args_size, args_first);
  }

  std::variant<StmtRef, ParserError> ParseStmt() {
    if (Peek().kind == Token::Kind::Ident && TokenString(Peek()) == "do") {
      Read();

      const auto maybe_value = ParseExpr();
      if (IsError(maybe_value)) return std::get<ParserError>(maybe_value);

      return ctx_.Add(DoStmt{
          .expr = std::get<ExprRef>(maybe_value),
      });
    }
    if (Peek().kind == Token::Kind::Ident && TokenString(Peek()) == "return") {
      Read();

      const auto maybe_value = ParseExpr();
      if (IsError(maybe_value)) return std::get<ParserError>(maybe_value);

      return ctx_.Add(ReturnStmt{
          .value = std::get<ExprRef>(maybe_value),
      });
    }
    if (Peek().kind == Token::Kind::Ident && TokenString(Peek()) == "loop") {
      Read();

      LoopStmt loop_stmt;

      auto body = ParseCompoundStmt();
      if (IsError(body)) return std::get<ParserError>(body);
      loop_stmt.stmts = std::get<List<StmtRef>>(body);

      return ctx_.Add(std::move(loop_stmt));
    }
    if (Peek().kind == Token::Kind::Ident && TokenString(Peek()) == "if") {
      Read();

      SkipSpace();

      IfStmt if_stmt;

      const auto cond = ParseExpr();
      if (IsError(cond)) return std::get<ParserError>(cond);
      if_stmt.cond = std::get<ExprRef>(cond);

      auto then_body = ParseCompoundStmt();
      if (IsError(then_body)) return std::get<ParserError>(then_body);
      if_stmt.then_stmts = std::get<List<StmtRef>>(then_body);

      SkipSpace();

      if (Peek().kind == Token::Kind::Ident && TokenString(Peek()) == "else") {
        Read();

        SkipSpace();

        if (Peek().kind == Token::Kind::Ident && TokenString(Peek()) == "if") {
          const auto stmt = ParseStmt();
          if (IsError(stmt)) return std::get<ParserError>(stmt);
          if_stmt.else_stmts = List<StmtRef>(1, std::get<StmtRef>(stmt));
        } else {
          auto else_body = ParseCompoundStmt();
          if (IsError(else_body)) return std::get<ParserError>(else_body);
          if_stmt.else_stmts = std::get<List<StmtRef>>(else_body);
        }
      }

      return ctx_.Add(std::move(if_stmt));
    }
    if (Peek().kind == Token::Kind::Ident && TokenString(Peek()) == "let") {
      Read();

      SkipSpace();

      const auto maybe_name = ParseIdent();
      if (IsError(maybe_name)) return std::get<ParserError>(maybe_name);

      if (auto r = ExpectToken(Token::Kind::Colon); IsError(r)) return *r;

      SkipSpace();

      const auto maybe_type = ParseType();
      if (IsError(maybe_type)) return std::get<ParserError>(maybe_type);

      SkipSpace();

      if (auto r = ExpectToken(Token::Kind::Equal); IsError(r)) return *r;

      SkipSpace();

      const auto init = ParseExpr();
      if (IsError(init)) return std::get<ParserError>(init);

      return ctx_.Add(VarDeclStmt{
          .type = std::get<TypeRef>(maybe_type),
          .name = std::get<std::string_view>(maybe_name),
          .init = std::get<ExprRef>(init),
      });
    }
    if (Peek().kind == Token::Kind::Ident && TokenString(Peek()) == "break") {
      Read();

      return ctx_.Add(BreakStmt{});
    }
    if (Peek().kind == Token::Kind::Ident) {
      const auto maybe_ident = ParseIdent();
      if (IsError(maybe_ident)) return std::get<ParserError>(maybe_ident);
      auto ident = std::get<std::string_view>(maybe_ident);

      if (Peek().kind == Token::Kind::OpenBracket) {
        Read();

        const auto maybe_size = ParseExpr();
        if (IsError(maybe_size)) return std::get<ParserError>(maybe_size);

        if (auto r = ExpectToken(Token::Kind::CloseBracket); IsError(r)) {
          return *r;
        }

        SkipSpace();

        if (Peek().kind == Token::Kind::Equal) {
          Read();

          SkipSpace();

          ArrayAssignStmt stmt;
          stmt.name = ident;
          stmt.index = std::get<ExprRef>(maybe_size);

          const auto expr = ParseExpr();
          if (IsError(expr)) return std::get<ParserError>(expr);
          stmt.expr = std::get<ExprRef>(expr);

          return ctx_.Add(std::move(stmt));
        }

        return MakeError(ParserError::Kind::UnexpectedToken, Peek());
      }

      SkipSpace();

      if (Peek().kind == Token::Kind::Equal) {
        Read();

        SkipSpace();

        VarAssignStmt stmt;
        stmt.name = ident;

        const auto expr = ParseExpr();
        if (IsError(expr)) return std::get<ParserError>(expr);
        stmt.expr = std::get<ExprRef>(expr);

        return ctx_.Add(std::move(stmt));
      }

      return ParseExprStartingWithIdent(ident);
    }
    return MakeError(ParserError::Kind::UnexpectedToken, Peek());
  }

  std::variant<ExprRef, ParserError> ParseExpr() {
    SkipSpace();

    const auto maybe_expr = ParseExprInternal();
    if (IsError(maybe_expr)) return std::get<ParserError>(maybe_expr);

    SkipSpace();

    if (Peek().kind == Token::Kind::Greater) {
      Read();

      SkipSpace();

      const auto maybe_rhs = ParseExprInternal();
      if (IsError(maybe_rhs)) return std::get<ParserError>(maybe_rhs);

      return MakeBinaryOpExpr(BinaryOp::Gt, std::get<ExprRef>(maybe_expr),
                              std::get<ExprRef>(maybe_rhs));
    } else if (Peek().kind == Token::Kind::Less) {
      Read();

      SkipSpace();

      const auto maybe_rhs = ParseExprInternal();
      if (IsError(maybe_rhs)) return std::get<ParserError>(maybe_rhs);

      return MakeBinaryOpExpr(BinaryOp::Lt, std::get<ExprRef>(maybe_expr),
                              std::get<ExprRef>(maybe_rhs));
    } else if (Peek().kind == Token::Kind::Equal) {
      Read();

      if (auto r = ExpectToken(Token::Kind::Equal); IsError(r)) {
        return *r;
      }

      SkipSpace();

      const auto maybe_rhs = ParseExprInternal();
      if (IsError(maybe_rhs)) return std::get<ParserError>(maybe_rhs);

      return MakeBinaryOpExpr(BinaryOp::Eq, std::get<ExprRef>(maybe_expr),
                              std::get<ExprRef>(maybe_rhs));
    } else if (Peek().kind == Token::Kind::Exclamation) {
      Read();

      if (auto r = ExpectToken(Token::Kind::Equal); IsError(r)) {
        return *r;
      }

      SkipSpace();

      const auto maybe_rhs = ParseExprInternal();
      if (IsError(maybe_rhs)) return std::get<ParserError>(maybe_rhs);

      return MakeBinaryOpExpr(BinaryOp::NotEq, std::get<ExprRef>(maybe_expr),
                              std::get<ExprRef>(maybe_rhs));
    }

    return maybe_expr;
  }

  std::variant<ExprRef, ParserError> ParseExprInternal() {
    std::variant<ExprRef, ParserError> maybe_expr;
    if (Peek().kind == Token::Kind::Ident) {
      auto ident = std::get<std::string_view>(ParseIdent());
      maybe_expr = ParseExprStartingWithIdent(ident);
    } else if (Peek().kind == Token::Kind::Number) {
      maybe_expr = ParseNumber();
    } else if (Peek().kind == Token::Kind::String ||
               Peek().kind == Token::Kind::Error) {
      maybe_expr = ParseString();
    } else {
      maybe_expr = MakeError(ParserError::Kind::UnexpectedToken, Peek());
    }
    if (IsError(maybe_expr)) return std::get<ParserError>(maybe_expr);

    SkipSpace();

    if (Peek().kind == Token::Kind::OpenBracket) {
      Read();

      const auto maybe_size = ParseExpr();
      if (IsError(maybe_size)) return std::get<ParserError>(maybe_size);

      if (auto r = ExpectToken(Token::Kind::CloseBracket); IsError(r)) {
        return *r;
      }

      maybe_expr = ctx_.Add(IndexExpr{
          .base = std::get<ExprRef>(maybe_expr),
          .index = std::get<ExprRef>(maybe_size),
      });
    }

    if (Peek().kind == Token::Kind::Plus) {
      Read();

      SkipSpace();

      const auto maybe_rhs = ParseExprInternal();
      if (IsError(maybe_rhs)) return std::get<ParserError>(maybe_rhs);

      return MakeBinaryOpExpr(BinaryOp::Add, std::get<ExprRef>(maybe_expr),
                              std::get<ExprRef>(maybe_rhs));
    } else if (Peek().kind == Token::Kind::Minus) {
      Read();

      SkipSpace();

      const auto maybe_rhs = ParseExprInternal();
      if (IsError(maybe_rhs)) return std::get<ParserError>(maybe_rhs);

      return MakeBinaryOpExpr(BinaryOp::Sub, std::get<ExprRef>(maybe_expr),
                              std::get<ExprRef>(maybe_rhs));
    } else if (Peek().kind == Token::Kind::Star) {
      Read();

      SkipSpace();

      const auto maybe_rhs = ParseExprInternal();
      if (IsError(maybe_rhs)) return std::get<ParserError>(maybe_rhs);

      return MakeBinaryOpExpr(BinaryOp::Mul, std::get<ExprRef>(maybe_expr),
                              std::get<ExprRef>(maybe_rhs));
    } else if (Peek().kind == Token::Kind::Slash) {
      Read();

      SkipSpace();

      const auto maybe_rhs = ParseExprInternal();
      if (IsError(maybe_rhs)) return std::get<ParserError>(maybe_rhs);

      return MakeBinaryOpExpr(BinaryOp::Div, std::get<ExprRef>(maybe_expr),
                              std::get<ExprRef>(maybe_rhs));
    } else if (Peek().kind == Token::Kind::Percent) {
      Read();

      SkipSpace();

      const auto maybe_rhs = ParseExprInternal();
      if (IsError(maybe_rhs)) return std::get<ParserError>(maybe_rhs);

      return MakeBinaryOpExpr(BinaryOp::Mod, std::get<ExprRef>(maybe_expr),
                              std::get<ExprRef>(maybe_rhs));
    }

    return maybe_expr;
  }

  std::variant<ExprRef, ParserError> ParseExprStartingWithIdent(
      std::string_view ident) {
    if (Peek().kind == Token::Kind::OpenParen) {
      Read();

      const auto start_idx = pending_exprs_.size();
      while (Peek().kind != Token::Kind::CloseParen) {
        auto maybe_arg = ParseExpr();
        if (IsError(maybe_arg)) return std::get<ParserError>(maybe_arg);
        pending_exprs_.push_back(std::get<ExprRef>(maybe_arg));

        if (Peek().kind == Token::Kind::Comma) Read();
      }
      Read();

      const std::uint8_t args_size = pending_exprs_.size() - start_idx;
      ExprRef args_first = Arena<Expr>::kNullRef;
      if (args_size > 0) {
        args_first = ctx_.AliasExpr(pending_exprs_[start_idx]);
      }
      for (std::size_t i = 1; i < args_size; ++i) {
        ctx_.AliasExpr(pending_exprs_[start_idx + i]);
      }
      for (std::size_t i = 0; i < args_size; ++i) {
        pending_exprs_.pop_back();
      }

      return ctx_.Add(FuncCallExpr{
          .func_name = ident,
          .args = List<ExprRef>(args_size, args_first),
      });
    }

    if (ident == "true" || ident == "false") {
      return ctx_.Add(BoolLitExpr{
          .value = ident,
      });
    }

    return ctx_.Add(IdentExpr{
        .name = ident,
    });
  }

  std::variant<IntLitExpr, ParserError> ParseIntLitExpr() {
    Token token = Read();
    if (token.kind != Token::Kind::Number) {
      return MakeError(ParserError::Kind::ExpectedNumber, token);
    }
    return IntLitExpr{
        .value = TokenString(token),
    };
  }

  std::variant<ExprRef, ParserError> ParseNumber() {
    const auto maybe_int_lit = ParseIntLitExpr();
    if (IsError(maybe_int_lit)) return std::get<ParserError>(maybe_int_lit);
    return ctx_.Add(std::get<IntLitExpr>(maybe_int_lit));
  }

  std::variant<ExprRef, ParserError> ParseString() {
    Token token = Read();
    if (token.kind == Token::Kind::Error) {
      return MakeError(ParserError::Kind::IncompleteStringLiteral, token);
    }
    if (token.kind != Token::Kind::String) {
      return MakeError(ParserError::Kind::ExpectedString, token);
    }
    return ctx_.Add(StringLitExpr{
        .value = TokenString(token),
    });
  }

  std::variant<TypeRef, ParserError> ParseType() {
    const auto maybe_type = ParseIdent();
    if (IsError(maybe_type)) return std::get<ParserError>(maybe_type);

    if (Peek().kind == Token::Kind::OpenBracket) {
      Read();

      const auto maybe_size = ParseIntLitExpr();
      if (IsError(maybe_size)) return std::get<ParserError>(maybe_size);

      if (auto r = ExpectToken(Token::Kind::CloseBracket); IsError(r)) {
        return *r;
      }

      return ctx_.Add(ArrayType{
          .element_type = ctx_.Add(BasicType{
              .name = std::get<std::string_view>(maybe_type),
          }),
          .size = std::get<IntLitExpr>(maybe_size),
      });
    }

    return ctx_.Add(BasicType{
        .name = std::get<std::string_view>(maybe_type),
    });
  }

  ExprRef MakeBinaryOpExpr(BinaryOp op, ExprRef lhs, ExprRef rhs) {
    return ctx_.Add(BinaryOpExpr{.op = op, .lhs = lhs, .rhs = rhs});
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

  void SkipSpace() {
    if (Peek().kind == Token::Kind::Space) Read();
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

  SyntaxContext& ctx_;
  std::string_view buffer_;
  LexerT lexer_;
  Token next_;
  std::vector<StmtRef> pending_stmts_;
  std::vector<ExprRef> pending_exprs_;
};

}  // namespace lucid
