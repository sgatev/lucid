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

#include "lucid/core/container/arena.h"
#include "lucid/core/container/fixed_map.h"
#include "lucid/core/container/successive_list.h"
#include "lucid/core/functional/expected.h"
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

  std::expected<std::optional<Def>, ParserError> Parse() {
    if (PeekIgnoringNonSemantic().kind == Token::Kind::End) [[unlikely]] {
      return std::nullopt;
    }

    return ParseDef().transform(
        [](Def def) { return std::make_optional(std::move(def)); });
  }

 private:
  std::expected<Def, ParserError> ParseDef() {
    // Ok to read immediate as non-semantic characters were ignored in `Parse`.
    Token token = ReadImmediate();

    if (token.kind != Token::Kind::Ident) [[unlikely]] {
      return std::unexpected(
          MakeError(ParserError::Kind::UnexpectedToken, token));
    }

    using namespace std::literals::string_view_literals;
    static constexpr FixedMap parselets(
        std::array{
            std::pair{"comp"sv, &Parser::ParseCompDef},
            std::pair{"fun"sv, &Parser::ParseFuncDef},
            std::pair{"val"sv, &Parser::ParseValDef},
        },
        &Parser::UnknownDef);
    return std::invoke(parselets[TokenString(token)], this,
                       /*is_comp=*/false);
  }

  std::expected<Def, ParserError> ParseCompDef(bool /*is_comp*/) {
    Token token = ReadIgnoringNonSemantic();

    if (token.kind != Token::Kind::Ident) [[unlikely]] {
      return std::unexpected(
          MakeError(ParserError::Kind::UnexpectedToken, token));
    }

    using namespace std::literals::string_view_literals;
    static constexpr FixedMap parselets(
        std::array{
            std::pair{"fun"sv, &Parser::ParseFuncDef},
            std::pair{"val"sv, &Parser::ParseValDef},
        },
        &Parser::UnknownDef);
    return std::invoke(parselets[TokenString(token)], this,
                       /*is_comp=*/true);
  }

  std::expected<Def, ParserError> ParseFuncDef(bool is_comp) {
    ASSIGN_OR_RETURN(StringIndex::Ref name, ParseIdent());

    std::uint8_t params_size = 0;
    ParamRef first_param = Arena<FuncParam>::kNullRef;
    RETURN_IF_ERROR(ExpectTokenIgnoringNonSemantic(Token::Kind::OpenParen));
    while (true) {
      Token next_token = PeekIgnoringNonSemantic();

      if (next_token.kind == Token::Kind::Comma) {
        ReadIgnoringNonSemantic();
      } else if (next_token.kind == Token::Kind::CloseParen) {
        ReadIgnoringNonSemantic();
        break;
      }

      if (PeekIgnoringNonSemantic().kind != Token::Kind::Ident) [[unlikely]] {
        return std::unexpected(
            MakeError(ParserError::Kind::ExpectedClosingParenOrParam,
                      PeekIgnoringNonSemantic()));
      }

      ASSIGN_OR_RETURN(ParamRef param, ParseParam());
      if (params_size == 0) first_param = param;
      ++params_size;
    }

    RETURN_IF_ERROR(ExpectTokenIgnoringNonSemantic(Token::Kind::Colon));
    ASSIGN_OR_RETURN(TypeRef result_type, ParseType());
    ASSIGN_OR_RETURN(SuccessiveList<StmtRef> stmts, ParseCompoundStmt());

    return FuncDefStmt{
        .name = name,
        .params = SuccessiveList<ParamRef>(params_size, first_param),
        .result_type = result_type,
        .stmts = stmts,
        .is_comp = is_comp,
    };
  }

  std::expected<Def, ParserError> ParseValDef(bool is_comp) {
    ASSIGN_OR_RETURN(StringIndex::Ref name, ParseIdent());
    RETURN_IF_ERROR(ExpectTokenIgnoringNonSemantic(Token::Kind::Colon));
    RETURN_IF_ERROR(ExpectIdentIgnoringNonSemantic(
        "Type", ParserError::Kind::ExpectedTypeKeyword));
    RETURN_IF_ERROR(ExpectTokenIgnoringNonSemantic(Token::Kind::Equal));

    std::uint8_t params_size = 0;
    ParamRef first_param = Arena<FuncParam>::kNullRef;
    RETURN_IF_ERROR(ExpectTokenIgnoringNonSemantic(Token::Kind::OpenParen));
    while (true) {
      Token next_token = PeekIgnoringNonSemantic();

      if (next_token.kind == Token::Kind::Comma) {
        ReadIgnoringNonSemantic();

      } else if (next_token.kind == Token::Kind::CloseParen) {
        ReadIgnoringNonSemantic();
        break;
      }

      if (PeekIgnoringNonSemantic().kind != Token::Kind::Ident) [[unlikely]] {
        return std::unexpected(
            MakeError(ParserError::Kind::ExpectedClosingParenOrParam,
                      PeekIgnoringNonSemantic()));
      }

      ASSIGN_OR_RETURN(ParamRef param, ParseParam());
      if (params_size == 0) first_param = param;
      ++params_size;
    }

    return TypeDefStmt{
        .name = name,
        .type = syn_ctx_.Add(TupleType{
            .fields = SuccessiveList<ParamRef>(params_size, first_param),
        }),
    };
  }

  std::expected<Def, ParserError> UnknownDef(bool is_comp) {
    return std::unexpected(MakeError(ParserError::Kind::UnexpectedToken,
                                     PeekIgnoringNonSemantic()));
  }

  std::expected<ParamRef, ParserError> ParseParam() {
    ASSIGN_OR_RETURN(StringIndex::Ref name, ParseIdent());
    RETURN_IF_ERROR(ExpectTokenIgnoringNonSemantic(Token::Kind::Colon));
    ASSIGN_OR_RETURN(TypeRef type, ParseType());
    return syn_ctx_.Add(FuncParam{.name = name, .type_constraint = type});
  }

  std::expected<StringIndex::Ref, ParserError> ParseIdent() {
    Token token = ReadIgnoringNonSemantic();
    if (token.kind != Token::Kind::Ident) [[unlikely]] {
      return std::unexpected(
          MakeError(ParserError::Kind::ExpectedIdent, token));
    }
    return syn_ctx_.AddIdent(TokenString(token));
  }

  std::expected<SuccessiveList<StmtRef>, ParserError> ParseCompoundStmt() {
    RETURN_IF_ERROR(ExpectTokenIgnoringNonSemantic(Token::Kind::OpenBrace));

    const auto start_idx = pending_stmts_.size();
    while (true) {
      if (PeekIgnoringNonSemantic().kind == Token::Kind::CloseBrace) break;

      ASSIGN_OR_RETURN(Stmt stmt, ParseStmt());
      pending_stmts_.push_back(std::move(stmt));
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

  std::expected<Stmt, ParserError> ParseStmt() {
    Token token = ReadIgnoringNonSemantic();
    if (token.kind == Token::Kind::Ampersand) [[unlikely]] {
      return ParseAssignStmt();
    } else if (token.kind != Token::Kind::Ident) [[unlikely]] {
      return std::unexpected(
          MakeError(ParserError::Kind::UnexpectedToken, token));
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
        &Parser::UnknownStmt);
    return std::invoke(parselets[TokenString(token)], this);
  }

  std::expected<Stmt, ParserError> ParseDoStmt() {
    ASSIGN_OR_RETURN(Expr value, ParseExpr());
    return DoStmt{.expr = syn_ctx_.Add(std::move(value))};
  }

  std::expected<Stmt, ParserError> ParseReturnStmt() {
    ASSIGN_OR_RETURN(Expr value, ParseExpr());
    return ReturnStmt{.value = syn_ctx_.Add(std::move(value))};
  }

  std::expected<Stmt, ParserError> ParseLoopStmt() {
    ASSIGN_OR_RETURN(SuccessiveList<StmtRef> body, ParseCompoundStmt());
    return LoopStmt{.stmts = body};
  }

  std::expected<Stmt, ParserError> ParseIfStmt() {
    IfStmt if_stmt;

    ASSIGN_OR_RETURN(Expr cond, ParseExpr());
    if_stmt.cond = syn_ctx_.Add(std::move(cond));

    ASSIGN_OR_RETURN(SuccessiveList<StmtRef> then_stmts, ParseCompoundStmt());
    if_stmt.then_stmts = then_stmts;

    if (PeekIgnoringNonSemantic().kind == Token::Kind::Ident &&
        TokenString(PeekIgnoringNonSemantic()) == "else") {
      ReadIgnoringNonSemantic();

      if (PeekIgnoringNonSemantic().kind == Token::Kind::Ident &&
          TokenString(PeekIgnoringNonSemantic()) == "if") {
        ASSIGN_OR_RETURN(Stmt stmt, ParseStmt());
        if_stmt.else_stmts =
            SuccessiveList<StmtRef>(1, syn_ctx_.Add(std::move(stmt)));
      } else {
        ASSIGN_OR_RETURN(SuccessiveList<StmtRef> else_stmts,
                         ParseCompoundStmt());
        if_stmt.else_stmts = else_stmts;
      }
    }

    return if_stmt;
  }

  std::expected<Stmt, ParserError> ParseCompStmt() {
    ReadIgnoringNonSemantic();
    return ParseVal(/*is_comp=*/true);
  }

  std::expected<Stmt, ParserError> ParseValStmt() {
    return ParseVal(/*is_comp=*/false);
  }

  std::expected<Stmt, ParserError> ParseVal(bool is_comp) {
    ASSIGN_OR_RETURN(StringIndex::Ref name, ParseIdent());
    RETURN_IF_ERROR(ExpectTokenIgnoringNonSemantic(Token::Kind::Colon));
    ASSIGN_OR_RETURN(TypeRef type, ParseType());

    std::optional<ExprRef> init;
    if (PeekIgnoringNonSemantic().kind == Token::Kind::Equal) {
      ReadIgnoringNonSemantic();
      ASSIGN_OR_RETURN(Expr init_expr, ParseExpr());
      init = syn_ctx_.Add(std::move(init_expr));
    }

    return VarDeclStmt{
        .name = name,
        .type_constraint = type,
        .init = init,
        .is_comp = is_comp,
    };
  }

  std::expected<Stmt, ParserError> ParseBreakStmt() { return BreakStmt{}; }

  std::expected<Stmt, ParserError> ParseAssignStmt() {
    ASSIGN_OR_RETURN(StringIndex::Ref ident, ParseIdent());

    if (PeekIgnoringNonSemantic().kind == Token::Kind::OpenBracket) {
      ReadIgnoringNonSemantic();

      ASSIGN_OR_RETURN(Expr size, ParseExpr());
      RETURN_IF_ERROR(
          ExpectTokenIgnoringNonSemantic(Token::Kind::CloseBracket));

      if (PeekIgnoringNonSemantic().kind == Token::Kind::Equal) [[likely]] {
        ReadIgnoringNonSemantic();
        ASSIGN_OR_RETURN(Expr expr, ParseExpr());
        return ArrayAssignStmt{
            .name = ident,
            .index = syn_ctx_.Add(std::move(size)),
            .expr = syn_ctx_.Add(std::move(expr)),
        };
      }

      return std::unexpected(MakeError(ParserError::Kind::UnexpectedToken,
                                       PeekIgnoringNonSemantic()));
    }

    if (PeekIgnoringNonSemantic().kind == Token::Kind::Dot) {
      ReadIgnoringNonSemantic();

      ASSIGN_OR_RETURN(StringIndex::Ref field_name, ParseIdent());

      if (PeekIgnoringNonSemantic().kind == Token::Kind::Equal) [[likely]] {
        ReadIgnoringNonSemantic();
        ASSIGN_OR_RETURN(Expr expr, ParseExpr());
        return FieldAssignStmt{
            .base = syn_ctx_.Add(IdentExpr{.name = ident}),
            .field_name = field_name,
            .expr = syn_ctx_.Add(std::move(expr)),
        };
      }

      return std::unexpected(MakeError(ParserError::Kind::UnexpectedToken,
                                       PeekIgnoringNonSemantic()));
    }

    if (PeekIgnoringNonSemantic().kind == Token::Kind::Equal) [[likely]] {
      ReadIgnoringNonSemantic();
      ASSIGN_OR_RETURN(Expr expr, ParseExpr());
      return VarAssignStmt{
          .name = ident,
          .expr = syn_ctx_.Add(std::move(expr)),
      };
    }

    return std::unexpected(MakeError(ParserError::Kind::UnexpectedToken,
                                     PeekIgnoringNonSemantic()));
  }

  std::expected<Stmt, ParserError> UnknownStmt() {
    return std::unexpected(MakeError(ParserError::Kind::UnexpectedToken,
                                     PeekIgnoringNonSemantic()));
  }

  std::expected<Expr, ParserError> ParseExpr() {
    ASSIGN_OR_RETURN(Expr expr, ParseTerm());
    switch (PeekIgnoringNonSemantic().kind) {
      case Token::Kind::Greater: {
        ReadIgnoringNonSemantic();
        ASSIGN_OR_RETURN(Expr rhs, ParseExpr());
        return MakeBinaryOpExpr(BinaryOp::Gt, syn_ctx_.Add(std::move(expr)),
                                syn_ctx_.Add(std::move(rhs)));
      }
      case Token::Kind::Less: {
        ReadIgnoringNonSemantic();
        ASSIGN_OR_RETURN(Expr rhs, ParseExpr());
        return MakeBinaryOpExpr(BinaryOp::Lt, syn_ctx_.Add(std::move(expr)),
                                syn_ctx_.Add(std::move(rhs)));
      }
      case Token::Kind::Equal: {
        ReadIgnoringNonSemantic();
        RETURN_IF_ERROR(ExpectTokenImmediate(Token::Kind::Equal));
        ASSIGN_OR_RETURN(Expr rhs, ParseExpr());
        return MakeBinaryOpExpr(BinaryOp::Eq, syn_ctx_.Add(std::move(expr)),
                                syn_ctx_.Add(std::move(rhs)));
      }
      case Token::Kind::Exclamation: {
        ReadIgnoringNonSemantic();
        RETURN_IF_ERROR(ExpectTokenImmediate(Token::Kind::Equal));
        ASSIGN_OR_RETURN(Expr rhs, ParseExpr());
        return MakeBinaryOpExpr(BinaryOp::NotEq, syn_ctx_.Add(std::move(expr)),
                                syn_ctx_.Add(std::move(rhs)));
      }
      default:
        return expr;
    }
  }

  std::expected<Expr, ParserError> ParseTerm() {
    ASSIGN_OR_RETURN(Expr expr, ParseFactor());
    switch (PeekIgnoringNonSemantic().kind) {
      case Token::Kind::Plus: {
        ReadIgnoringNonSemantic();
        ASSIGN_OR_RETURN(Expr rhs, ParseTerm());
        return MakeBinaryOpExpr(BinaryOp::Add, syn_ctx_.Add(std::move(expr)),
                                syn_ctx_.Add(std::move(rhs)));
      }
      case Token::Kind::Minus: {
        ReadIgnoringNonSemantic();
        ASSIGN_OR_RETURN(Expr rhs, ParseTerm());
        return MakeBinaryOpExpr(BinaryOp::Sub, syn_ctx_.Add(std::move(expr)),
                                syn_ctx_.Add(std::move(rhs)));
      }
      default:
        return expr;
    }
  }

  std::expected<Expr, ParserError> ParseFactor() {
    ASSIGN_OR_RETURN(Expr expr, ParseElement());
    switch (PeekIgnoringNonSemantic().kind) {
      case Token::Kind::Star: {
        ReadIgnoringNonSemantic();
        ASSIGN_OR_RETURN(Expr rhs, ParseFactor());
        return MakeBinaryOpExpr(BinaryOp::Mul, syn_ctx_.Add(std::move(expr)),
                                syn_ctx_.Add(std::move(rhs)));
      }
      case Token::Kind::Slash: {
        ReadIgnoringNonSemantic();
        ASSIGN_OR_RETURN(Expr rhs, ParseFactor());
        return MakeBinaryOpExpr(BinaryOp::Div, syn_ctx_.Add(std::move(expr)),
                                syn_ctx_.Add(std::move(rhs)));
      }
      case Token::Kind::Percent: {
        ReadIgnoringNonSemantic();
        ASSIGN_OR_RETURN(Expr rhs, ParseFactor());
        return MakeBinaryOpExpr(BinaryOp::Mod, syn_ctx_.Add(std::move(expr)),
                                syn_ctx_.Add(std::move(rhs)));
      }
      default:
        return expr;
    }
  }

  std::expected<Expr, ParserError> ParseElement() {
    switch (Token next_token = PeekIgnoringNonSemantic(); next_token.kind) {
      case Token::Kind::Ident:
        return ParseExprStartingWithIdent(ParseIdent().value());
      case Token::Kind::Number:
        return ParseIntLitExpr();
      case Token::Kind::String:
        [[fallthrough]];
      case Token::Kind::Error:
        return ParseString();
      [[unlikely]] default:
        return std::unexpected(
            MakeError(ParserError::Kind::UnexpectedToken, next_token));
    }
  }

  std::expected<Expr, ParserError> ParseExprStartingWithIdent(
      StringIndex::Ref ident) {
    ASSIGN_OR_RETURN(Expr expr, ParseIdentOrFuncCall(ident));

    if (PeekIgnoringNonSemantic().kind == Token::Kind::OpenBracket) {
      ReadIgnoringNonSemantic();

      ASSIGN_OR_RETURN(Expr size, ParseExpr());
      RETURN_IF_ERROR(
          ExpectTokenIgnoringNonSemantic(Token::Kind::CloseBracket));
      return IndexExpr{
          .base = syn_ctx_.Add(std::move(expr)),
          .index = syn_ctx_.Add(std::move(size)),
      };
    }

    if (PeekIgnoringNonSemantic().kind == Token::Kind::Dot) {
      ReadIgnoringNonSemantic();
      ASSIGN_OR_RETURN(StringIndex::Ref field_name, ParseIdent());
      return FieldAccessExpr{
          .base = syn_ctx_.Add(std::move(expr)),
          .field_name = field_name,
      };
    }

    return expr;
  }

  std::expected<Expr, ParserError> ParseIdentOrFuncCall(
      StringIndex::Ref ident) {
    if (PeekIgnoringNonSemantic().kind == Token::Kind::OpenParen) {
      ReadIgnoringNonSemantic();

      const auto start_idx = pending_exprs_.size();
      while (PeekIgnoringNonSemantic().kind != Token::Kind::CloseParen) {
        ASSIGN_OR_RETURN(Expr arg, ParseExpr());
        pending_exprs_.push_back(std::move(arg));

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

  std::expected<IntLitExpr, ParserError> ParseIntLitExpr() {
    Token token = ReadIgnoringNonSemantic();
    if (token.kind != Token::Kind::Number) [[unlikely]] {
      return std::unexpected(
          MakeError(ParserError::Kind::ExpectedNumber, token));
    }

    int value;
    std::string_view value_string = TokenString(token);
    auto res = std::from_chars(value_string.begin(), value_string.end(), value);
    if (res.ec != std::errc()) [[unlikely]] {
      return std::unexpected(
          MakeError(ParserError::Kind::ExpectedNumber, token));
    }

    return IntLitExpr{.value = value};
  }

  std::expected<Expr, ParserError> ParseString() {
    Token token = ReadIgnoringNonSemantic();
    if (token.kind == Token::Kind::Error) [[unlikely]] {
      return std::unexpected(
          MakeError(ParserError::Kind::IncompleteStringLiteral, token));
    }
    if (token.kind != Token::Kind::String) [[unlikely]] {
      return std::unexpected(
          MakeError(ParserError::Kind::ExpectedString, token));
    }
    return StringLitExpr{.value = syn_ctx_.AddIdent(TokenString(token))};
  }

  std::expected<TypeRef, ParserError> ParseType() {
    ASSIGN_OR_RETURN(StringIndex::Ref type_name, ParseIdent());
    TypeRef type = syn_ctx_.ResolveType(type_name);

    if (PeekIgnoringNonSemantic().kind == Token::Kind::OpenBracket) {
      ReadIgnoringNonSemantic();
      ArrayType array_type{.element_type_constraint = type};
      ASSIGN_OR_RETURN(array_type.size, ParseIntLitExpr());
      RETURN_IF_ERROR(
          ExpectTokenIgnoringNonSemantic(Token::Kind::CloseBracket));
      return syn_ctx_.Add(std::move(array_type));
    }

    return type;
  }

  Expr MakeBinaryOpExpr(BinaryOp op, ExprRef lhs, ExprRef rhs) {
    return BinaryOpExpr{.op = op, .lhs = lhs, .rhs = rhs};
  }

  std::expected<void, ParserError> ExpectTokenIgnoringNonSemantic(
      Token::Kind kind) {
    Token token = ReadIgnoringNonSemantic();
    if (token.kind != kind) [[unlikely]] {
      return std::unexpected(
          MakeError(ParserError::Kind::UnexpectedToken, token));
    }
    return {};
  }

  std::expected<void, ParserError> ExpectTokenImmediate(Token::Kind kind) {
    Token token = ReadImmediate();
    if (token.kind != kind) [[unlikely]] {
      return std::unexpected(
          MakeError(ParserError::Kind::UnexpectedToken, token));
    }
    return {};
  }

  std::expected<void, ParserError> ExpectIdentIgnoringNonSemantic(
      std::string_view text, ParserError::Kind error_kind) {
    Token token = ReadIgnoringNonSemantic();
    if (token.kind != Token::Kind::Ident || TokenString(token) != text)
        [[unlikely]] {
      return std::unexpected(MakeError(error_kind, token));
    }
    return {};
  }

  std::string_view TokenString(Token token) const {
    return buffer_.substr(token.start_pos, token.end_pos - token.start_pos);
  }

  Token ReadImmediate() { return std::exchange(next_, lexer_.next()); }

  Token ReadIgnoringNonSemantic() {
    SkipNonSemantic();
    return ReadImmediate();
  }

  const Token& PeekIgnoringNonSemantic() {
    SkipNonSemantic();
    return next_;
  }

  void SkipNonSemantic() {
    while (next_.kind < Token::kFirstSemanticKind) next_ = lexer_.next();
  }

  ParserError MakeError(ParserError::Kind kind, const Token& token) const {
    return ParserError(kind, FindLine(buffer_, token),
                       FindColumn(buffer_, token));
  }

  SyntaxContext& syn_ctx_;
  std::string_view buffer_;
  LexerT lexer_;
  Token next_;
  std::vector<Stmt> pending_stmts_;
  std::vector<Expr> pending_exprs_;
};

}  // namespace lucid
