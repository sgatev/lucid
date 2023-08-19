#include "lucid/parser.h"

#include <cstddef>
#include <functional>
#include <string>
#include <string_view>
#include <utility>
#include <variant>
#include <vector>

#include "gmock/gmock.h"
#include "gtest/gtest.h"
#include "lucid/arena.h"
#include "lucid/ast.h"
#include "lucid/lexer.h"

MATCHER_P(HoldsFuncDef, match_stmt, "") {
  auto* stmt = std::get_if<lucid::FuncDefStmt>(&arg);
  if (stmt == nullptr) return false;
  return match_stmt(*stmt);
}

MATCHER_P(HoldsError, match_err, "") {
  auto* err = std::get_if<std::string>(&arg);
  if (err == nullptr) return false;
  return match_err == *err;
}

namespace lucid {
namespace {

template <typename T, typename P>
bool AllMatch(const std::vector<T>& real, const std::vector<P>& patterns) {
  if (real.size() != patterns.size()) return false;
  for (std::size_t i = 0; i < real.size(); ++i) {
    if (!patterns[i](real[i])) return false;
  }
  return true;
}

using StmtRefMatcher = std::function<bool(StmtRef)>;

using ExprRefMatcher = std::function<bool(ExprRef)>;

struct FuncParamPattern {
  std::string_view name;
  std::string_view type;

  bool operator()(const FuncParam& param) const {
    return name == param.name && type == param.type;
  }
};

struct CompoundStmtPattern {
  std::vector<StmtRefMatcher> statements;
};

struct FuncDefStmtPattern {
  std::string_view name;
  std::vector<FuncParamPattern> parameters;
  std::string_view result_type;
  CompoundStmtPattern body;

  bool operator()(const FuncDefStmt& stmt) const {
    return name == stmt.name && AllMatch(stmt.parameters, parameters) &&
           result_type == stmt.result_type &&
           AllMatch(stmt.body.statements, body.statements);
  }
};

struct ReturnStmtPattern {
  StmtRefMatcher value;

  bool operator()(const ReturnStmt& stmt) const { return value(stmt.value); }
};

struct IfStmtPattern {
  ExprRefMatcher cond;
  CompoundStmtPattern then_body;
  CompoundStmtPattern else_body;

  bool operator()(const IfStmt& stmt) const {
    return cond(stmt.cond) &&
           AllMatch(stmt.then_body.statements, then_body.statements) &&
           AllMatch(stmt.else_body.statements, else_body.statements);
  }
};

struct IntLitExprPattern {
  std::string_view value;

  bool operator()(const IntLitExpr& expr) const { return value == expr.value; }
};

struct BoolLitExprPattern {
  std::string_view value;

  bool operator()(const BoolLitExpr& expr) const { return value == expr.value; }
};

struct BinaryOpExprPattern {
  BinaryOp op;
  ExprRefMatcher lhs;
  ExprRefMatcher rhs;

  bool operator()(const BinaryOpExpr& expr) const {
    return op == expr.op && lhs(expr.lhs) && rhs(expr.rhs);
  }
};

struct IdentExprPattern {
  std::string_view name;

  bool operator()(const IdentExpr& expr) const { return name == expr.name; }
};

struct FuncCallExprPattern {
  std::string_view func_name;
  std::vector<ExprRefMatcher> arguments;

  bool operator()(const FuncCallExpr& expr) const {
    return func_name == expr.func_name && AllMatch(expr.arguments, arguments);
  }
};

class ParserTest : public testing::Test {
 protected:
  std::variant<FuncDefStmt, std::string> Parse(std::string_view src) {
    auto maybe_func_def_stmt = Parser(arena_, src, Lexer(src)).ParseFuncDef();
    if (auto* ref = std::get_if<FuncDefStmt>(&maybe_func_def_stmt)) return *ref;
    return std::get<ParserError>(maybe_func_def_stmt).ToString();
  }

  std::function<bool(FuncDefStmt)> MatchesFuncDefStmt(
      FuncDefStmtPattern pattern) {
    return [pattern](FuncDefStmt stmt) { return pattern(stmt); };
  }

  StmtRefMatcher MatchesReturnStmt(ReturnStmtPattern pattern) {
    return MatchesStmt<ReturnStmt>(std::move(pattern));
  }

  StmtRefMatcher MatchesIfStmt(IfStmtPattern pattern) {
    return MatchesStmt<IfStmt>(std::move(pattern));
  }

  ExprRefMatcher MatchesIntLitExpr(IntLitExprPattern pattern) {
    return MatchesExpr<IntLitExpr>(std::move(pattern));
  }

  ExprRefMatcher MatchesBoolLitExpr(BoolLitExprPattern pattern) {
    return MatchesExpr<BoolLitExpr>(std::move(pattern));
  }

  ExprRefMatcher MatchesBinaryOpExpr(BinaryOpExprPattern pattern) {
    return MatchesExpr<BinaryOpExpr>(std::move(pattern));
  }

  ExprRefMatcher MatchesIdentExpr(IdentExprPattern pattern) {
    return MatchesExpr<IdentExpr>(std::move(pattern));
  }

  ExprRefMatcher MatchesFuncCallExpr(FuncCallExprPattern pattern) {
    return MatchesExpr<FuncCallExpr>(std::move(pattern));
  }

 private:
  template <typename S, typename P>
  ExprRefMatcher MatchesStmt(P pattern) {
    return [this, pattern](ExprRef ref) {
      if (auto* stmt = std::get_if<S>(&arena_.get(ref))) return pattern(*stmt);
      return false;
    };
  }

  template <typename E, typename P>
  ExprRefMatcher MatchesExpr(P pattern) {
    return MatchesStmt<Expr>([pattern](const Expr& stmt) {
      if (auto* expr = std::get_if<E>(&stmt)) return pattern(*expr);
      return false;
    });
  }

  Arena<Stmt> arena_;
};

TEST_F(ParserTest, EmptyFuncDefStmt) {
  std::string_view src = R"(
    let main = () -> Void {
    }
  )";
  EXPECT_THAT(Parse(src),  //
              HoldsFuncDef(MatchesFuncDefStmt({
                  .name = "main",
                  .result_type = "Void",
              })));
}

TEST_F(ParserTest, ReturnIntLitExpr) {
  std::string_view src = R"(
    let main = () -> Int {
      return 0
    }
  )";
  EXPECT_THAT(Parse(src),  //
              HoldsFuncDef(MatchesFuncDefStmt(
                  {.name = "main",
                   .result_type = "Int",
                   .body = {
                       .statements =
                           {
                               MatchesReturnStmt({
                                   .value = MatchesIntLitExpr({
                                       .value = "0",
                                   }),
                               }),
                           },
                   }})));
}

TEST_F(ParserTest, ReturnAddBinaryOpExpr) {
  std::string_view src = R"(
    let main = () -> Int {
      return 3 + 2
    }
  )";
  EXPECT_THAT(Parse(src),  //
              HoldsFuncDef(MatchesFuncDefStmt(
                  {.name = "main",
                   .result_type = "Int",
                   .body = {
                       .statements =
                           {
                               MatchesReturnStmt({
                                   .value = MatchesBinaryOpExpr({
                                       .op = BinaryOp::Add,
                                       .lhs = MatchesIntLitExpr({
                                           .value = "3",
                                       }),
                                       .rhs = MatchesIntLitExpr({
                                           .value = "2",
                                       }),
                                   }),
                               }),
                           },
                   }})));
}

TEST_F(ParserTest, ReturnSubBinaryOpExpr) {
  std::string_view src = R"(
    let main = () -> Int {
      return 3 - 2
    }
  )";
  EXPECT_THAT(Parse(src),  //
              HoldsFuncDef(MatchesFuncDefStmt(
                  {.name = "main",
                   .result_type = "Int",
                   .body = {
                       .statements =
                           {
                               MatchesReturnStmt({
                                   .value = MatchesBinaryOpExpr({
                                       .op = BinaryOp::Sub,
                                       .lhs = MatchesIntLitExpr({
                                           .value = "3",
                                       }),
                                       .rhs = MatchesIntLitExpr({
                                           .value = "2",
                                       }),
                                   }),
                               }),
                           },
                   }})));
}

TEST_F(ParserTest, ReturnMulBinaryOpExpr) {
  std::string_view src = R"(
    let main = () -> Int {
      return 3 * 2
    }
  )";
  EXPECT_THAT(Parse(src),  //
              HoldsFuncDef(MatchesFuncDefStmt(
                  {.name = "main",
                   .result_type = "Int",
                   .body = {
                       .statements =
                           {
                               MatchesReturnStmt({
                                   .value = MatchesBinaryOpExpr({
                                       .op = BinaryOp::Mul,
                                       .lhs = MatchesIntLitExpr({
                                           .value = "3",
                                       }),
                                       .rhs = MatchesIntLitExpr({
                                           .value = "2",
                                       }),
                                   }),
                               }),
                           },
                   }})));
}

TEST_F(ParserTest, ReturnDivBinaryOpExpr) {
  std::string_view src = R"(
    let main = () -> Int {
      return 3 / 2
    }
  )";
  EXPECT_THAT(Parse(src),  //
              HoldsFuncDef(MatchesFuncDefStmt(
                  {.name = "main",
                   .result_type = "Int",
                   .body = {
                       .statements =
                           {
                               MatchesReturnStmt({
                                   .value = MatchesBinaryOpExpr({
                                       .op = BinaryOp::Div,
                                       .lhs = MatchesIntLitExpr({
                                           .value = "3",
                                       }),
                                       .rhs = MatchesIntLitExpr({
                                           .value = "2",
                                       }),
                                   }),
                               }),
                           },
                   }})));
}

TEST_F(ParserTest, ReturnGtBinaryOpExpr) {
  std::string_view src = R"(
    let foo = () -> Bool {
      return 3 > 2
    }
  )";
  EXPECT_THAT(Parse(src),  //
              HoldsFuncDef(MatchesFuncDefStmt(
                  {.name = "foo",
                   .result_type = "Bool",
                   .body = {
                       .statements =
                           {
                               MatchesReturnStmt({
                                   .value = MatchesBinaryOpExpr({
                                       .op = BinaryOp::Gt,
                                       .lhs = MatchesIntLitExpr({
                                           .value = "3",
                                       }),
                                       .rhs = MatchesIntLitExpr({
                                           .value = "2",
                                       }),
                                   }),
                               }),
                           },
                   }})));
}

TEST_F(ParserTest, SingleFuncParam) {
  std::string_view src = R"(
    let id = (x: Int) -> Int {
      return x
    }
  )";
  EXPECT_THAT(Parse(src),  //
              HoldsFuncDef(MatchesFuncDefStmt(
                  {.name = "id",
                   .parameters =
                       {
                           {.name = "x", .type = "Int"},
                       },
                   .result_type = "Int",
                   .body = {
                       .statements =
                           {
                               MatchesReturnStmt({
                                   .value = MatchesIdentExpr({
                                       .name = "x",
                                   }),
                               }),
                           },
                   }})));
}

TEST_F(ParserTest, MultipleFuncParams) {
  std::string_view src = R"(
    let foo = (a: Int, b: Double, c: Bool) -> Void {
    }
  )";
  EXPECT_THAT(Parse(src),  //
              HoldsFuncDef(MatchesFuncDefStmt({
                  .name = "foo",
                  .parameters =
                      {
                          {.name = "a", .type = "Int"},
                          {.name = "b", .type = "Double"},
                          {.name = "c", .type = "Bool"},
                      },
                  .result_type = "Void",
              })));
}

TEST_F(ParserTest, FuncCallExpr) {
  std::string_view src = R"(
    let main = () -> Int {
      return id(21)
    }
  )";
  EXPECT_THAT(
      Parse(src),  //
      HoldsFuncDef(MatchesFuncDefStmt(
          {.name = "main",
           .result_type = "Int",
           .body = {
               .statements =
                   {
                       MatchesReturnStmt({
                           .value = MatchesFuncCallExpr({
                               .func_name = "id",
                               .arguments =
                                   {
                                       MatchesIntLitExpr({.value = "21"}),
                                   },
                           }),
                       }),
                   },
           }})));
}

TEST_F(ParserTest, ReturnTrueBoolLit) {
  std::string_view src = R"(
    let truth = () -> Bool {
      return true
    }
  )";
  EXPECT_THAT(Parse(src),  //
              HoldsFuncDef(MatchesFuncDefStmt(
                  {.name = "truth",
                   .result_type = "Bool",
                   .body = {
                       .statements =
                           {
                               MatchesReturnStmt({
                                   .value = MatchesBoolLitExpr({
                                       .value = "true",
                                   }),
                               }),
                           },
                   }})));
}

TEST_F(ParserTest, ReturnFalseBoolLit) {
  std::string_view src = R"(
    let falsity = () -> Bool {
      return false
    }
  )";
  EXPECT_THAT(Parse(src),  //
              HoldsFuncDef(MatchesFuncDefStmt(
                  {.name = "falsity",
                   .result_type = "Bool",
                   .body = {
                       .statements =
                           {
                               MatchesReturnStmt({
                                   .value = MatchesBoolLitExpr({
                                       .value = "false",
                                   }),
                               }),
                           },
                   }})));
}

TEST_F(ParserTest, IfStmt) {
  std::string_view src = R"(
    let foo = () -> Int {
      if true {
        return 2 + 3
      } else {
        return 4 * 5
      }
    }
  )";
  EXPECT_THAT(
      Parse(src),  //
      HoldsFuncDef(MatchesFuncDefStmt(
          {.name = "foo",
           .result_type = "Int",
           .body = {
               .statements =
                   {
                       MatchesIfStmt(
                           {.cond = MatchesBoolLitExpr({
                                .value = "true",
                            }),
                            .then_body =
                                {
                                    .statements =
                                        {
                                            MatchesReturnStmt({
                                                .value = MatchesBinaryOpExpr({
                                                    .op = BinaryOp::Add,
                                                    .lhs = MatchesIntLitExpr(
                                                        {.value = "2"}),
                                                    .rhs = MatchesIntLitExpr(
                                                        {.value = "3"}),
                                                }),
                                            }),
                                        },
                                },
                            .else_body =
                                {
                                    .statements =
                                        {
                                            MatchesReturnStmt({
                                                .value = MatchesBinaryOpExpr({
                                                    .op = BinaryOp::Mul,
                                                    .lhs = MatchesIntLitExpr(
                                                        {.value = "4"}),
                                                    .rhs = MatchesIntLitExpr(
                                                        {.value = "5"}),
                                                }),
                                            }),
                                        },
                                }}),
                   },
           }})));
}

TEST_F(ParserTest, FuncDefMissingLet) {
  std::string_view src = R"(
    = () -> Void {
    }
  )";
  EXPECT_THAT(Parse(src),
              HoldsError("expected 'let' keyword at line 2, column 5"));
}

TEST_F(ParserTest, FuncDefMissingName) {
  std::string_view src = R"(
    let = () -> Void {
    }
  )";
  EXPECT_THAT(Parse(src),
              HoldsError("expected identifier at line 2, column 9"));
}

TEST_F(ParserTest, FuncDefMissingEqual) {
  std::string_view src = R"(
    let main () -> Void {
    }
  )";
  EXPECT_THAT(Parse(src), HoldsError("unexpected token at line 2, column 14"));
}

TEST_F(ParserTest, FuncDefMissingOpeningParen) {
  std::string_view src = R"(
    let main = ) -> Void {
    }
  )";
  EXPECT_THAT(Parse(src), HoldsError("unexpected token at line 2, column 16"));
}

TEST_F(ParserTest, FuncDefMissingParamName) {
  std::string_view src = R"(
    let id = (: Int) -> Int {
      return x
    }
  )";
  EXPECT_THAT(Parse(src), HoldsError("expected closing parenthesis or "
                                     "parameter at line 2, column 15"));
}

TEST_F(ParserTest, FuncDefMissingParamColon) {
  std::string_view src = R"(
    let id = (x Int) -> Int {
      return x
    }
  )";
  EXPECT_THAT(Parse(src), HoldsError("unexpected token at line 2, column 17"));
}

TEST_F(ParserTest, FuncDefMissingParamType) {
  std::string_view src = R"(
    let id = (x:) -> Int {
      return x
    }
  )";
  EXPECT_THAT(Parse(src),
              HoldsError("expected identifier at line 2, column 17"));
}

TEST_F(ParserTest, FuncDefMissingParamColonAndType) {
  std::string_view src = R"(
    let id = (x) -> Int {
      return x
    }
  )";
  EXPECT_THAT(Parse(src), HoldsError("unexpected token at line 2, column 16"));
}

TEST_F(ParserTest, FuncDefMissingNextParam) {
  std::string_view src = R"(
    let id = (x: Int,) -> Int {
      return x
    }
  )";
  EXPECT_THAT(Parse(src), HoldsError("expected closing parenthesis or "
                                     "parameter at line 2, column 22"));
}

TEST_F(ParserTest, FuncDefMissingClosingParen) {
  std::string_view src = R"(
    let main = ( -> Void {
    }
  )";
  EXPECT_THAT(Parse(src), HoldsError("expected closing parenthesis or "
                                     "parameter at line 2, column 18"));
}

TEST_F(ParserTest, FuncDefMissingResultArrowDash) {
  std::string_view src = R"(
    let main = () > Void {
    }
  )";
  EXPECT_THAT(Parse(src), HoldsError("unexpected token at line 2, column 19"));
}

TEST_F(ParserTest, FuncDefMissingResultArrowHead) {
  std::string_view src = R"(
    let main = () - Void {
    }
  )";
  EXPECT_THAT(Parse(src), HoldsError("unexpected token at line 2, column 21"));
}

TEST_F(ParserTest, FuncDefMissingResultType) {
  std::string_view src = R"(
    let main = () -> {
    }
  )";
  EXPECT_THAT(Parse(src),
              HoldsError("expected identifier at line 2, column 22"));
}

TEST_F(ParserTest, FuncDefMissingOpenBrace) {
  std::string_view src = R"(
    let main = () -> Void
    }
  )";
  EXPECT_THAT(Parse(src), HoldsError("unexpected token at line 3, column 5"));
}

TEST_F(ParserTest, FuncDefMissingClosingBrace) {
  std::string_view src = R"(
    let main = () -> Void {
  )";
  EXPECT_THAT(Parse(src), HoldsError("unexpected token at line 3, column 3"));
}

TEST_F(ParserTest, ReturnMissingValue) {
  std::string_view src = R"(
    let id = (x: Int) -> Int {
      return
    }
  )";
  EXPECT_THAT(Parse(src), HoldsError("unexpected token at line 4, column 5"));
}

}  // namespace
}  // namespace lucid
