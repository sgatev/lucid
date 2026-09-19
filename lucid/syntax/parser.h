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

// Represents a location in a source code.
struct CodeLocation {
  std::size_t line;
  std::size_t col;
};

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

  explicit ParserError(Kind kind, CodeLocation loc) : kind_(kind), loc_(loc) {}

  Kind GetKind() const { return kind_; }

  friend std::ostream& operator<<(std::ostream& out, const ParserError error) {
    return out << error.KindString() << " at line " << error.loc_.line
               << ", column " << error.loc_.col;
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
  CodeLocation loc_;
};

// Converts a string of Lucid code into a stream of AST nodes.
template <typename LexerT>
class Parser {
 public:
  explicit Parser(SyntaxContext& syn_ctx, std::string_view buffer, LexerT lexer)
      : syn_ctx_(syn_ctx),
        buffer_(buffer),
        lexer_(std::forward<LexerT>(lexer)),
        next_(lexer_.next()),
        true_ident_(syn_ctx.AddIdent("true")),
        false_ident_(syn_ctx.AddIdent("false")) {}

  std::expected<std::optional<Def>, ParserError> Parse() {
    if (PeekIgnoringNonSemantic().kind == Token::Kind::End) [[unlikely]] {
      return std::nullopt;
    }

    return ParseDef().transform(
        [](Def def) { return std::make_optional(def); });
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

    std::uint32_t params_size = 0;
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

    std::uint32_t params_size = 0;
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

    const std::uint32_t args_size = pending_stmts_.size() - start_idx;
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

  // The precedence of the operators that bind least tightly.
  static constexpr std::uint8_t kLowestPrecedence = 1;

  // The binary operator a token opens, and how tightly that operator binds.
  // A precedence of zero is a token that does not continue an expression.
  struct BinaryOpInfo {
    BinaryOp op;
    std::uint8_t precedence;
    // Whether the operator is spelled with a second `=` right after the token.
    bool takes_equal;
  };

  static constexpr BinaryOpInfo BinaryOpOf(Token::Kind kind) {
    switch (kind) {
      case Token::Kind::Greater:
        return {.op = BinaryOp::Gt, .precedence = 1, .takes_equal = false};
      case Token::Kind::Less:
        return {.op = BinaryOp::Lt, .precedence = 1, .takes_equal = false};
      case Token::Kind::Equal:
        return {.op = BinaryOp::Eq, .precedence = 1, .takes_equal = true};
      case Token::Kind::Exclamation:
        return {.op = BinaryOp::NotEq, .precedence = 1, .takes_equal = true};
      case Token::Kind::Plus:
        return {.op = BinaryOp::Add, .precedence = 2, .takes_equal = false};
      case Token::Kind::Minus:
        return {.op = BinaryOp::Sub, .precedence = 2, .takes_equal = false};
      case Token::Kind::Star:
        return {.op = BinaryOp::Mul, .precedence = 3, .takes_equal = false};
      case Token::Kind::Slash:
        return {.op = BinaryOp::Div, .precedence = 3, .takes_equal = false};
      case Token::Kind::Percent:
        return {.op = BinaryOp::Mod, .precedence = 3, .takes_equal = false};
      default:
        return {.op = BinaryOp::Add, .precedence = 0, .takes_equal = false};
    }
  }

  std::expected<Expr, ParserError> ParseExpr() {
    return ParseExpr(kLowestPrecedence);
  }

  // Parses an expression made of operators that bind at least as tightly as
  // `min_precedence`, taking one operand and one operator at a time.
  std::expected<Expr, ParserError> ParseExpr(std::uint8_t min_precedence) {
    ASSIGN_OR_RETURN(Expr expr, ParseElement());

    while (true) {
      const BinaryOpInfo info = BinaryOpOf(PeekIgnoringNonSemantic().kind);
      if (info.precedence < min_precedence) return expr;

      ReadIgnoringNonSemantic();
      if (info.takes_equal) {
        RETURN_IF_ERROR(ExpectTokenImmediate(Token::Kind::Equal));
      }

      // The right side stops one level up, so that the next operator of this
      // level is left to this loop: `a - b - c` is `(a - b) - c`.
      ASSIGN_OR_RETURN(Expr rhs, ParseExpr(info.precedence + 1));
      expr = MakeBinaryOpExpr(info.op, syn_ctx_.Add(std::move(expr)),
                              syn_ctx_.Add(std::move(rhs)));
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

      const std::uint32_t args_size = pending_exprs_.size() - start_idx;
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

    if (ident == true_ident_) return BoolLitExpr{.value = true};
    if (ident == false_ident_) return BoolLitExpr{.value = false};
    return IdentExpr{.name = ident};
  }

  std::expected<IntLitExpr, ParserError> ParseIntLitExpr() {
    Token token = ReadIgnoringNonSemantic();
    if (token.kind != Token::Kind::Number) [[unlikely]] {
      return std::unexpected(
          MakeError(ParserError::Kind::ExpectedNumber, token));
    }

    std::int64_t value;
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
    return ParserError(kind, {
                                 .line = FindLine(buffer_, token),
                                 .col = FindColumn(buffer_, token),
                             });
  }

  SyntaxContext& syn_ctx_;
  std::string_view buffer_;
  LexerT lexer_;
  Token next_;
  StringIndex::Ref true_ident_;
  StringIndex::Ref false_ident_;
  std::vector<Stmt> pending_stmts_;
  std::vector<Expr> pending_exprs_;
};

// A lexer handed over as an lvalue is borrowed rather than copied: it holds
// the tokens of the whole source, and the caller keeps it alive anyway.
template <typename LexerT>
Parser(SyntaxContext&, std::string_view, LexerT&) -> Parser<LexerT&>;

}  // namespace lucid
