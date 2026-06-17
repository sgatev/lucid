#pragma once

#include <array>
#include <charconv>
#include <cstddef>
#include <cstdint>
#include <expected>
#include <functional>
#include <optional>
#include <ostream>
#include <string_view>
#include <utility>
#include <variant>

#include "lucid/core/container/arena.h"
#include "lucid/core/container/fixed_map.h"
#include "lucid/core/container/successive_list.h"
#include "lucid/core/string/index.h"
#include "lucid/syntax/ast.h"
#include "lucid/syntax/context.h"
#include "lucid/syntax/token.h"

namespace lucid {

// An error that occurred while parsing Lucid code.
class ParserError {
 public:
  enum class Kind : std::uint8_t {
    ExpectedIdent,
    ExpectedNumber,
    ExpectedString,
    UnexpectedToken,
    ExpectedClosingParenOrParam,
    ExpectedClosingParenOrExpr,
    ExpectedValKeyword,
    ExpectedTypeKeyword,
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
      case Kind::ExpectedValKeyword:
        return "expected 'val' keyword";
      case Kind::ExpectedTypeKeyword:
        return "expected 'Type' keyword";
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
  explicit Parser(SyntaxContext& syn_ctx, std::string_view buffer, LexerT lexer)
      : syn_ctx_(syn_ctx),
        buffer_(buffer),
        lexer_(std::move(lexer)),
        next_(lexer_.next()) {}

  std::expected<std::optional<Def>, ParserError> ParseDef() {
    if (PeekIgnoringNonSemantic().kind == Token::Kind::End) return std::nullopt;

    bool is_comp = false;
    if (PeekIgnoringNonSemantic().kind == Token::Kind::Ident &&
        TokenString(PeekIgnoringNonSemantic()) == "comp") {
      ReadIgnoringNonSemantic();
      is_comp = true;
    }

    if (PeekIgnoringNonSemantic().kind != Token::Kind::Ident) {
      return std::unexpected(MakeError(ParserError::Kind::UnexpectedToken,
                                       PeekIgnoringNonSemantic()));
    }

    using namespace std::literals::string_view_literals;
    static constexpr FixedMap parselets(
        std::array{
            std::pair{"fun"sv, &Parser::ParseFuncDef},
            std::pair{"val"sv, &Parser::ParseValDef},
        },
        &Parser::UnknownDef);
    return std::invoke(parselets[TokenString(PeekIgnoringNonSemantic())], this,
                       is_comp)
        .and_then(
            [](Def def) -> std::expected<std::optional<Def>, ParserError> {
              return std::optional<Def>(std::move(def));
            });
  }

 private:
  std::expected<Def, ParserError> UnknownDef(bool is_comp) {
    return std::unexpected(MakeError(ParserError::Kind::UnexpectedToken,
                                     PeekIgnoringNonSemantic()));
  }

  std::expected<Def, ParserError> ParseFuncDef(bool is_comp) {
    ReadIgnoringNonSemantic();

    const auto maybe_name = ParseIdent();
    if (IsError(maybe_name)) {
      return std::unexpected(std::get<ParserError>(maybe_name));
    }

    FuncDefStmt stmt = {
        .name = std::get<StringIndex::Ref>(maybe_name),
        .is_comp = is_comp,
    };

    std::uint8_t params_size = 0;
    ParamRef first_param = Arena<FuncParam>::kNullRef;
    if (auto r = ExpectTokenIgnoringNonSemantic(Token::Kind::OpenParen);
        IsError(r)) {
      return std::unexpected(*r);
    }
    while (true) {
      if (PeekIgnoringNonSemantic().kind == Token::Kind::Comma) {
        ReadIgnoringNonSemantic();
      } else if (PeekIgnoringNonSemantic().kind == Token::Kind::CloseParen) {
        ReadIgnoringNonSemantic();
        break;
      }

      if (PeekIgnoringNonSemantic().kind != Token::Kind::Ident) {
        return std::unexpected(
            MakeError(ParserError::Kind::ExpectedClosingParenOrParam,
                      PeekIgnoringNonSemantic()));
      }

      auto maybe_param = ParseParam();
      if (IsError(maybe_param)) {
        return std::unexpected(std::get<ParserError>(maybe_param));
      }
      if (params_size == 0) first_param = std::get<ParamRef>(maybe_param);
      ++params_size;
    }
    stmt.params = SuccessiveList<ParamRef>(params_size, first_param);

    if (auto r = ExpectTokenIgnoringNonSemantic(Token::Kind::Colon);
        IsError(r)) {
      return std::unexpected(*r);
    }

    const auto maybe_result_type = ParseType();
    if (IsError(maybe_result_type)) {
      return std::unexpected(std::get<ParserError>(maybe_result_type));
    }
    stmt.result_type = std::get<TypeRef>(maybe_result_type);

    auto maybe_body = ParseCompoundStmt();
    if (IsError(maybe_body)) {
      return std::unexpected(std::get<ParserError>(maybe_body));
    }
    stmt.stmts = std::get<SuccessiveList<StmtRef>>(std::move(maybe_body));

    return stmt;
  }

  std::expected<Def, ParserError> ParseValDef(bool is_comp) {
    if (auto r = ExpectIdentIgnoringNonSemantic(
            "val", ParserError::Kind::ExpectedValKeyword);
        IsError(r)) {
      return std::unexpected(*r);
    }

    const auto maybe_name = ParseIdent();
    if (IsError(maybe_name)) {
      return std::unexpected(std::get<ParserError>(maybe_name));
    }

    if (auto r = ExpectTokenIgnoringNonSemantic(Token::Kind::Colon);
        IsError(r)) {
      return std::unexpected(*r);
    }

    if (auto r = ExpectIdentIgnoringNonSemantic(
            "Type", ParserError::Kind::ExpectedTypeKeyword);
        IsError(r)) {
      return std::unexpected(*r);
    }

    if (auto r = ExpectTokenIgnoringNonSemantic(Token::Kind::Equal);
        IsError(r)) {
      return std::unexpected(*r);
    }

    std::uint8_t params_size = 0;
    ParamRef first_param = Arena<FuncParam>::kNullRef;
    if (auto r = ExpectTokenIgnoringNonSemantic(Token::Kind::OpenParen);
        IsError(r)) {
      return std::unexpected(*r);
    }
    while (true) {
      if (PeekIgnoringNonSemantic().kind == Token::Kind::Comma) {
        ReadIgnoringNonSemantic();

      } else if (PeekIgnoringNonSemantic().kind == Token::Kind::CloseParen) {
        ReadIgnoringNonSemantic();
        break;
      }

      if (PeekIgnoringNonSemantic().kind != Token::Kind::Ident) {
        return std::unexpected(
            MakeError(ParserError::Kind::ExpectedClosingParenOrParam,
                      PeekIgnoringNonSemantic()));
      }

      auto maybe_param = ParseParam();
      if (IsError(maybe_param)) {
        return std::unexpected(std::get<ParserError>(maybe_param));
      }
      if (params_size == 0) first_param = std::get<ParamRef>(maybe_param);
      ++params_size;
    }

    TypeRef type = syn_ctx_.Add(TupleType{
        .fields = SuccessiveList<ParamRef>(params_size, first_param),
    });
    return TypeDefStmt{
        .name = std::get<StringIndex::Ref>(maybe_name),
        .type = type,
    };
  }

  std::variant<ParamRef, ParserError> ParseParam() {
    const auto maybe_name = ParseIdent();
    if (IsError(maybe_name)) return std::get<ParserError>(maybe_name);

    if (auto r = ExpectTokenIgnoringNonSemantic(Token::Kind::Colon);
        IsError(r)) {
      return *r;
    }

    const auto maybe_type = ParseType();
    if (IsError(maybe_type)) return std::get<ParserError>(maybe_type);

    return syn_ctx_.Add(FuncParam{
        .name = std::get<StringIndex::Ref>(maybe_name),
        .type_constraint = std::get<TypeRef>(maybe_type),
    });
  }

  std::variant<StringIndex::Ref, ParserError> ParseIdent() {
    Token token = ReadIgnoringNonSemantic();
    if (token.kind == Token::Kind::Ident) [[likely]] {
      return syn_ctx_.AddIdent(TokenString(token));
    }
    return MakeError(ParserError::Kind::ExpectedIdent, token);
  }

  std::variant<SuccessiveList<StmtRef>, ParserError> ParseCompoundStmt() {
    if (auto r = ExpectTokenIgnoringNonSemantic(Token::Kind::OpenBrace);
        IsError(r)) {
      return *r;
    }

    const auto start_idx = pending_stmts_.size();
    while (true) {
      if (PeekIgnoringNonSemantic().kind == Token::Kind::CloseBrace) break;

      auto maybe_stmt = ParseStmt();
      if (IsError(maybe_stmt)) return std::get<ParserError>(maybe_stmt);
      pending_stmts_.push_back(std::get<Stmt>(std::move(maybe_stmt)));
    }

    ReadIgnoringNonSemantic();

    const std::uint8_t args_size = pending_stmts_.size() - start_idx;
    StmtRef args_first = Arena<Stmt>::kNullRef;
    if (args_size > 0) {
      args_first = syn_ctx_.Add(std::move(pending_stmts_[start_idx]));
    }
    for (std::size_t i = 1; i < args_size; ++i) {
      syn_ctx_.Add(std::move(pending_stmts_[start_idx + i]));
    }
    for (std::size_t i = 0; i < args_size; ++i) {
      pending_stmts_.pop_back();
    }

    return SuccessiveList<StmtRef>(args_size, args_first);
  }

  std::variant<Stmt, ParserError> ParseStmt() {
    if (PeekIgnoringNonSemantic().kind != Token::Kind::Ident) {
      return MakeError(ParserError::Kind::UnexpectedToken,
                       PeekIgnoringNonSemantic());
    }

    using namespace std::literals::string_view_literals;
    static constexpr FixedMap parselets(
        std::array{
            std::pair{"do"sv, &Parser::ParseDoStmt},
            std::pair{"return"sv, &Parser::ParseReturnStmt},
            std::pair{"loop"sv, &Parser::ParseLoopStmt},
            std::pair{"if"sv, &Parser::ParseIfStmt},
            std::pair{"val"sv, &Parser::ParseValStmt},
            std::pair{"comp"sv, &Parser::ParseCompStmt},
            std::pair{"break"sv, &Parser::ParseBreakStmt},
        },
        &Parser::ParseAssignStmt);
    return std::invoke(parselets[TokenString(PeekIgnoringNonSemantic())], this);
  }

  std::variant<Stmt, ParserError> ParseDoStmt() {
    ReadIgnoringNonSemantic();

    const auto maybe_value = ParseExpr();
    if (IsError(maybe_value)) return std::get<ParserError>(maybe_value);

    return DoStmt{
        .expr = syn_ctx_.Add(std::get<Expr>(maybe_value)),
    };
  }

  std::variant<Stmt, ParserError> ParseReturnStmt() {
    ReadIgnoringNonSemantic();

    const auto maybe_value = ParseExpr();
    if (IsError(maybe_value)) return std::get<ParserError>(maybe_value);

    return ReturnStmt{
        .value = syn_ctx_.Add(std::get<Expr>(maybe_value)),
    };
  }

  std::variant<Stmt, ParserError> ParseLoopStmt() {
    ReadIgnoringNonSemantic();

    LoopStmt loop_stmt;

    auto body = ParseCompoundStmt();
    if (IsError(body)) return std::get<ParserError>(body);

    return LoopStmt{
        .stmts = std::get<SuccessiveList<StmtRef>>(body),
    };
  }

  std::variant<Stmt, ParserError> ParseIfStmt() {
    ReadIgnoringNonSemantic();

    IfStmt if_stmt;

    const auto cond = ParseExpr();
    if (IsError(cond)) return std::get<ParserError>(cond);
    if_stmt.cond = syn_ctx_.Add(std::get<Expr>(cond));

    auto then_body = ParseCompoundStmt();
    if (IsError(then_body)) return std::get<ParserError>(then_body);
    if_stmt.then_stmts = std::get<SuccessiveList<StmtRef>>(then_body);

    if (PeekIgnoringNonSemantic().kind == Token::Kind::Ident &&
        TokenString(PeekIgnoringNonSemantic()) == "else") {
      ReadIgnoringNonSemantic();

      if (PeekIgnoringNonSemantic().kind == Token::Kind::Ident &&
          TokenString(PeekIgnoringNonSemantic()) == "if") {
        const auto stmt = ParseStmt();
        if (IsError(stmt)) return std::get<ParserError>(stmt);
        if_stmt.else_stmts =
            SuccessiveList<StmtRef>(1, syn_ctx_.Add(std::get<Stmt>(stmt)));
      } else {
        auto else_body = ParseCompoundStmt();
        if (IsError(else_body)) return std::get<ParserError>(else_body);
        if_stmt.else_stmts = std::get<SuccessiveList<StmtRef>>(else_body);
      }
    }

    return std::move(if_stmt);
  }

  std::variant<Stmt, ParserError> ParseCompStmt() {
    ReadIgnoringNonSemantic();
    return ParseVal(/*is_comp=*/true);
  }

  std::variant<Stmt, ParserError> ParseValStmt() {
    return ParseVal(/*is_comp=*/false);
  }

  std::variant<Stmt, ParserError> ParseVal(bool is_comp) {
    ReadIgnoringNonSemantic();

    const auto maybe_name = ParseIdent();
    if (IsError(maybe_name)) return std::get<ParserError>(maybe_name);

    if (auto r = ExpectTokenIgnoringNonSemantic(Token::Kind::Colon);
        IsError(r)) {
      return *r;
    }

    const auto maybe_type = ParseType();
    if (IsError(maybe_type)) return std::get<ParserError>(maybe_type);

    std::optional<ExprRef> init;
    if (PeekIgnoringNonSemantic().kind == Token::Kind::Equal) {
      ReadIgnoringNonSemantic();

      auto maybe_init = ParseExpr();
      if (IsError(maybe_init)) return std::get<ParserError>(maybe_init);
      init = syn_ctx_.Add(std::get<Expr>(maybe_init));
    }

    return VarDeclStmt{
        .name = std::get<StringIndex::Ref>(maybe_name),
        .type_constraint = std::get<TypeRef>(maybe_type),
        .init = init,
        .is_comp = is_comp,
    };
  }

  std::variant<Stmt, ParserError> ParseBreakStmt() {
    ReadIgnoringNonSemantic();

    return BreakStmt{};
  }

  std::variant<Stmt, ParserError> ParseAssignStmt() {
    auto ident = std::get<StringIndex::Ref>(ParseIdent());

    if (PeekIgnoringNonSemantic().kind == Token::Kind::OpenBracket) {
      ReadIgnoringNonSemantic();

      const auto maybe_size = ParseExpr();
      if (IsError(maybe_size)) return std::get<ParserError>(maybe_size);

      if (auto r = ExpectTokenIgnoringNonSemantic(Token::Kind::CloseBracket);
          IsError(r)) {
        return *r;
      }

      if (PeekIgnoringNonSemantic().kind == Token::Kind::Equal) {
        ReadIgnoringNonSemantic();

        const auto expr = ParseExpr();
        if (IsError(expr)) return std::get<ParserError>(expr);

        return ArrayAssignStmt{
            .name = ident,
            .index = syn_ctx_.Add(std::get<Expr>(maybe_size)),
            .expr = syn_ctx_.Add(std::get<Expr>(expr)),
        };
      }

      return MakeError(ParserError::Kind::UnexpectedToken,
                       PeekIgnoringNonSemantic());
    }

    if (PeekIgnoringNonSemantic().kind == Token::Kind::Dot) {
      ReadIgnoringNonSemantic();

      const auto maybe_field_name = ParseIdent();
      if (IsError(maybe_field_name)) {
        return std::get<ParserError>(maybe_field_name);
      }

      if (PeekIgnoringNonSemantic().kind == Token::Kind::Equal) {
        ReadIgnoringNonSemantic();

        const auto expr = ParseExpr();
        if (IsError(expr)) return std::get<ParserError>(expr);

        return FieldAssignStmt{
            .base = syn_ctx_.Add(IdentExpr{.name = ident}),
            .field_name = std::get<StringIndex::Ref>(maybe_field_name),
            .expr = syn_ctx_.Add(std::get<Expr>(expr)),
        };
      }

      return MakeError(ParserError::Kind::UnexpectedToken,
                       PeekIgnoringNonSemantic());
    }

    if (PeekIgnoringNonSemantic().kind == Token::Kind::Equal) {
      ReadIgnoringNonSemantic();

      const auto expr = ParseExpr();
      if (IsError(expr)) return std::get<ParserError>(expr);

      return VarAssignStmt{
          .name = ident,
          .expr = syn_ctx_.Add(std::get<Expr>(expr)),
      };
    }

    return MakeError(ParserError::Kind::UnexpectedToken,
                     PeekIgnoringNonSemantic());
  }

  std::variant<Expr, ParserError> ParseExpr() {
    const auto maybe_expr = ParseTerm();
    if (IsError(maybe_expr)) return std::get<ParserError>(maybe_expr);

    if (PeekIgnoringNonSemantic().kind == Token::Kind::Greater) {
      ReadIgnoringNonSemantic();

      const auto maybe_rhs = ParseExpr();
      if (IsError(maybe_rhs)) return std::get<ParserError>(maybe_rhs);

      return MakeBinaryOpExpr(BinaryOp::Gt,
                              syn_ctx_.Add(std::get<Expr>(maybe_expr)),
                              syn_ctx_.Add(std::get<Expr>(maybe_rhs)));
    } else if (PeekIgnoringNonSemantic().kind == Token::Kind::Less) {
      ReadIgnoringNonSemantic();

      const auto maybe_rhs = ParseExpr();
      if (IsError(maybe_rhs)) return std::get<ParserError>(maybe_rhs);

      return MakeBinaryOpExpr(BinaryOp::Lt,
                              syn_ctx_.Add(std::get<Expr>(maybe_expr)),
                              syn_ctx_.Add(std::get<Expr>(maybe_rhs)));
    } else if (PeekIgnoringNonSemantic().kind == Token::Kind::Equal) {
      ReadImmediate();

      if (auto r = ExpectTokenImmediate(Token::Kind::Equal); IsError(r)) {
        return *r;
      }

      const auto maybe_rhs = ParseExpr();
      if (IsError(maybe_rhs)) return std::get<ParserError>(maybe_rhs);

      return MakeBinaryOpExpr(BinaryOp::Eq,
                              syn_ctx_.Add(std::get<Expr>(maybe_expr)),
                              syn_ctx_.Add(std::get<Expr>(maybe_rhs)));
    } else if (PeekIgnoringNonSemantic().kind == Token::Kind::Exclamation) {
      ReadImmediate();

      if (auto r = ExpectTokenImmediate(Token::Kind::Equal); IsError(r)) {
        return *r;
      }

      const auto maybe_rhs = ParseExpr();
      if (IsError(maybe_rhs)) return std::get<ParserError>(maybe_rhs);

      return MakeBinaryOpExpr(BinaryOp::NotEq,
                              syn_ctx_.Add(std::get<Expr>(maybe_expr)),
                              syn_ctx_.Add(std::get<Expr>(maybe_rhs)));
    }
    return maybe_expr;
  }

  std::variant<Expr, ParserError> ParseTerm() {
    auto maybe_expr = ParseFactor();
    if (IsError(maybe_expr)) return std::get<ParserError>(maybe_expr);

    if (PeekIgnoringNonSemantic().kind == Token::Kind::Plus) {
      ReadIgnoringNonSemantic();

      const auto maybe_rhs = ParseTerm();
      if (IsError(maybe_rhs)) return std::get<ParserError>(maybe_rhs);

      return MakeBinaryOpExpr(BinaryOp::Add,
                              syn_ctx_.Add(std::get<Expr>(maybe_expr)),
                              syn_ctx_.Add(std::get<Expr>(maybe_rhs)));
    } else if (PeekIgnoringNonSemantic().kind == Token::Kind::Minus) {
      ReadIgnoringNonSemantic();

      const auto maybe_rhs = ParseTerm();
      if (IsError(maybe_rhs)) return std::get<ParserError>(maybe_rhs);

      return MakeBinaryOpExpr(BinaryOp::Sub,
                              syn_ctx_.Add(std::get<Expr>(maybe_expr)),
                              syn_ctx_.Add(std::get<Expr>(maybe_rhs)));
    }
    return maybe_expr;
  }

  std::variant<Expr, ParserError> ParseFactor() {
    auto maybe_expr = [&]() -> std::variant<Expr, ParserError> {
      if (PeekIgnoringNonSemantic().kind == Token::Kind::Ident) {
        auto ident = std::get<StringIndex::Ref>(ParseIdent());
        return ParseExprStartingWithIdent(ident);
      } else if (PeekIgnoringNonSemantic().kind == Token::Kind::Number) {
        return ParseNumber();
      } else if (PeekIgnoringNonSemantic().kind == Token::Kind::String ||
                 PeekIgnoringNonSemantic().kind == Token::Kind::Error) {
        return ParseString();
      }
      return MakeError(ParserError::Kind::UnexpectedToken,
                       PeekIgnoringNonSemantic());
    }();
    if (IsError(maybe_expr)) return std::get<ParserError>(maybe_expr);

    if (PeekIgnoringNonSemantic().kind == Token::Kind::Star) {
      ReadIgnoringNonSemantic();

      const auto maybe_rhs = ParseFactor();
      if (IsError(maybe_rhs)) return std::get<ParserError>(maybe_rhs);

      return MakeBinaryOpExpr(BinaryOp::Mul,
                              syn_ctx_.Add(std::get<Expr>(maybe_expr)),
                              syn_ctx_.Add(std::get<Expr>(maybe_rhs)));
    } else if (PeekIgnoringNonSemantic().kind == Token::Kind::Slash) {
      ReadIgnoringNonSemantic();

      const auto maybe_rhs = ParseFactor();
      if (IsError(maybe_rhs)) return std::get<ParserError>(maybe_rhs);

      return MakeBinaryOpExpr(BinaryOp::Div,
                              syn_ctx_.Add(std::get<Expr>(maybe_expr)),
                              syn_ctx_.Add(std::get<Expr>(maybe_rhs)));
    } else if (PeekIgnoringNonSemantic().kind == Token::Kind::Percent) {
      ReadIgnoringNonSemantic();

      const auto maybe_rhs = ParseFactor();
      if (IsError(maybe_rhs)) return std::get<ParserError>(maybe_rhs);

      return MakeBinaryOpExpr(BinaryOp::Mod,
                              syn_ctx_.Add(std::get<Expr>(maybe_expr)),
                              syn_ctx_.Add(std::get<Expr>(maybe_rhs)));
    }

    return maybe_expr;
  }

  std::variant<Expr, ParserError> ParseExprStartingWithIdent(
      StringIndex::Ref ident) {
    auto maybe_expr = ParseIdentOrFuncCall(ident);
    if (IsError(maybe_expr)) return std::get<ParserError>(maybe_expr);

    if (PeekIgnoringNonSemantic().kind == Token::Kind::OpenBracket) {
      ReadIgnoringNonSemantic();

      const auto maybe_size = ParseExpr();
      if (IsError(maybe_size)) return std::get<ParserError>(maybe_size);

      if (auto r = ExpectTokenIgnoringNonSemantic(Token::Kind::CloseBracket);
          IsError(r)) {
        return *r;
      }

      return IndexExpr{
          .base = syn_ctx_.Add(std::get<Expr>(maybe_expr)),
          .index = syn_ctx_.Add(std::get<Expr>(maybe_size)),
      };
    }

    if (PeekIgnoringNonSemantic().kind == Token::Kind::Dot) {
      ReadIgnoringNonSemantic();

      const auto maybe_field_name = ParseIdent();
      if (IsError(maybe_field_name)) {
        return std::get<ParserError>(maybe_field_name);
      }

      return FieldAccessExpr{
          .base = syn_ctx_.Add(std::get<Expr>(maybe_expr)),
          .field_name = std::get<StringIndex::Ref>(maybe_field_name),
      };
    }

    return maybe_expr;
  }

  std::variant<Expr, ParserError> ParseIdentOrFuncCall(StringIndex::Ref ident) {
    if (PeekIgnoringNonSemantic().kind == Token::Kind::OpenParen) {
      ReadIgnoringNonSemantic();

      const auto start_idx = pending_exprs_.size();
      while (PeekIgnoringNonSemantic().kind != Token::Kind::CloseParen) {
        auto maybe_arg = ParseExpr();
        if (IsError(maybe_arg)) return std::get<ParserError>(maybe_arg);
        pending_exprs_.push_back(std::get<Expr>(std::move(maybe_arg)));

        if (PeekIgnoringNonSemantic().kind == Token::Kind::Comma) {
          ReadIgnoringNonSemantic();
        }
      }
      ReadIgnoringNonSemantic();

      const std::uint8_t args_size = pending_exprs_.size() - start_idx;
      ExprRef args_first = Arena<Expr>::kNullRef;
      if (args_size > 0) {
        args_first = syn_ctx_.Add(std::move(pending_exprs_[start_idx]));
      }
      for (std::size_t i = 1; i < args_size; ++i) {
        syn_ctx_.Add(std::move(pending_exprs_[start_idx + i]));
      }
      for (std::size_t i = 0; i < args_size; ++i) {
        pending_exprs_.pop_back();
      }

      return FuncCallExpr{
          .func_name = ident,
          .args = SuccessiveList<ExprRef>(args_size, args_first),
      };
    }

    bool is_true = ident == syn_ctx_.AddIdent("true");
    bool is_false = ident == syn_ctx_.AddIdent("false");
    if (is_true || is_false) return BoolLitExpr{.value = is_true};

    return IdentExpr{.name = ident};
  }

  std::variant<IntLitExpr, ParserError> ParseIntLitExpr() {
    Token token = ReadIgnoringNonSemantic();
    if (token.kind != Token::Kind::Number) [[unlikely]] {
      return MakeError(ParserError::Kind::ExpectedNumber, token);
    }

    std::string_view value_string = TokenString(token);
    int value;
    auto res = std::from_chars(value_string.begin(), value_string.end(), value);
    if (res.ec != std::errc()) {
      return MakeError(ParserError::Kind::ExpectedNumber, token);
    }

    return IntLitExpr{
        .value = value,
    };
  }

  std::variant<Expr, ParserError> ParseNumber() {
    const auto maybe_int_lit = ParseIntLitExpr();
    if (IsError(maybe_int_lit)) [[unlikely]] {
      return std::get<ParserError>(maybe_int_lit);
    }
    return std::get<IntLitExpr>(maybe_int_lit);
  }

  std::variant<Expr, ParserError> ParseString() {
    Token token = ReadIgnoringNonSemantic();
    if (token.kind == Token::Kind::Error) [[unlikely]] {
      return MakeError(ParserError::Kind::IncompleteStringLiteral, token);
    }
    if (token.kind != Token::Kind::String) [[unlikely]] {
      return MakeError(ParserError::Kind::ExpectedString, token);
    }
    return StringLitExpr{
        .value = syn_ctx_.AddIdent(TokenString(token)),
    };
  }

  std::variant<TypeRef, ParserError> ParseType() {
    const auto maybe_type = ParseIdent();
    if (IsError(maybe_type)) return std::get<ParserError>(maybe_type);

    if (PeekIgnoringNonSemantic().kind == Token::Kind::OpenBracket) {
      ReadIgnoringNonSemantic();

      const auto maybe_size = ParseIntLitExpr();
      if (IsError(maybe_size)) return std::get<ParserError>(maybe_size);

      if (auto r = ExpectTokenIgnoringNonSemantic(Token::Kind::CloseBracket);
          IsError(r)) {
        return *r;
      }

      return syn_ctx_.Add(ArrayType{
          .element_type_constraint =
              syn_ctx_.ResolveType(std::get<StringIndex::Ref>(maybe_type)),
          .size = std::get<IntLitExpr>(maybe_size),
      });
    }

    return syn_ctx_.ResolveType(std::get<StringIndex::Ref>(maybe_type));
  }

  Expr MakeBinaryOpExpr(BinaryOp op, ExprRef lhs, ExprRef rhs) {
    return BinaryOpExpr{.op = op, .lhs = lhs, .rhs = rhs};
  }

  std::optional<ParserError> ExpectTokenIgnoringNonSemantic(Token::Kind kind) {
    Token token = ReadIgnoringNonSemantic();
    if (token.kind == kind) [[likely]] {
      return std::nullopt;
    }
    return MakeError(ParserError::Kind::UnexpectedToken, token);
  }

  std::optional<ParserError> ExpectTokenImmediate(Token::Kind kind) {
    Token token = ReadImmediate();
    if (token.kind == kind) return std::nullopt;
    return MakeError(ParserError::Kind::UnexpectedToken, token);
  }

  std::optional<ParserError> ExpectIdentIgnoringNonSemantic(
      std::string_view text, ParserError::Kind error_kind) {
    Token token = ReadIgnoringNonSemantic();
    if (token.kind != Token::Kind::Ident || TokenString(token) != text) {
      return MakeError(error_kind, token);
    }
    return std::nullopt;
  }

  std::string_view TokenString(Token token) const {
    return buffer_.substr(token.start_pos, token.end_pos - token.start_pos);
  }

  Token ReadImmediate() { return std::exchange(next_, lexer_.next()); }

  Token ReadIgnoringNonSemantic() {
    SkipComment();
    return ReadImmediate();
  }

  const Token& PeekIgnoringNonSemantic() {
    SkipComment();
    return next_;
  }

  void SkipComment() {
    while (next_.kind < Token::kFirstSemanticKind) next_ = lexer_.next();
  }

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

  SyntaxContext& syn_ctx_;
  std::string_view buffer_;
  LexerT lexer_;
  Token next_;
  std::vector<Stmt> pending_stmts_;
  std::vector<Expr> pending_exprs_;
};

}  // namespace lucid
