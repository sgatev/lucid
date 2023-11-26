#include "lucid/parser.h"

#include <sstream>
#include <string>
#include <string_view>
#include <variant>
#include <vector>

#include "gmock/gmock.h"
#include "gtest/gtest.h"
#include "lucid/arena.h"
#include "lucid/ast.h"
#include "lucid/ast_fixture.h"
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

class ParserTest : public testing::Test, public AstFixture {
 protected:
  std::variant<FuncDefStmt, std::string> Parse(std::string_view src) {
    auto maybe_func_def_stmt = Parser(arena_, src, Lexer(src)).ParseFuncDef();
    if (auto* ref = std::get_if<FuncDefStmt>(&maybe_func_def_stmt)) return *ref;
    std::stringstream out;
    out << std::get<ParserError>(maybe_func_def_stmt);
    return out.str();
  }
};

TEST_F(ParserTest, EmptyFuncDefStmt) {
  std::string_view src = R"(
    let main = () -> Void {
    }
  )";
  EXPECT_THAT(Parse(src), HoldsFuncDef(MatchesFuncDefStmt({
                              .name = "main",
                              .result_type = MatchesBasicType({.name = "Void"}),
                          })));
}

TEST_F(ParserTest, ReturnIntLitExpr) {
  std::string_view src = R"(
    let main = () -> Int32 {
      return 0
    }
  )";

  EXPECT_THAT(Parse(src),
              HoldsFuncDef(MatchesFuncDefStmt({
                  .name = "main",
                  .result_type = MatchesBasicType({.name = "Int32"}),
                  .body = {{
                      MatchesReturnStmt({
                          .value = MatchesIntLitExpr({
                              .value = "0",
                          }),
                      }),
                  }},
              })));
}

TEST_F(ParserTest, ReturnAddBinaryOpExpr) {
  std::string_view src = R"(
    let main = () -> Int32 {
      return 3 + 2
    }
  )";
  EXPECT_THAT(Parse(src),
              HoldsFuncDef(MatchesFuncDefStmt({
                  .name = "main",
                  .result_type = MatchesBasicType({.name = "Int32"}),
                  .body = {{
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
                  }},
              })));
}

TEST_F(ParserTest, ReturnSubBinaryOpExpr) {
  std::string_view src = R"(
    let main = () -> Int32 {
      return 3 - 2
    }
  )";
  EXPECT_THAT(Parse(src),
              HoldsFuncDef(MatchesFuncDefStmt({
                  .name = "main",
                  .result_type = MatchesBasicType({.name = "Int32"}),
                  .body = {{
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
                  }},
              })));
}

TEST_F(ParserTest, ReturnMulBinaryOpExpr) {
  std::string_view src = R"(
    let main = () -> Int32 {
      return 3 * 2
    }
  )";
  EXPECT_THAT(Parse(src),
              HoldsFuncDef(MatchesFuncDefStmt({
                  .name = "main",
                  .result_type = MatchesBasicType({.name = "Int32"}),
                  .body = {{
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
                  }},
              })));
}

TEST_F(ParserTest, ReturnDivBinaryOpExpr) {
  std::string_view src = R"(
    let main = () -> Int32 {
      return 3 / 2
    }
  )";
  EXPECT_THAT(Parse(src),
              HoldsFuncDef(MatchesFuncDefStmt({
                  .name = "main",
                  .result_type = MatchesBasicType({.name = "Int32"}),
                  .body = {{
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
                  }},
              })));
}

TEST_F(ParserTest, ReturnGtBinaryOpExpr) {
  std::string_view src = R"(
    let foo = () -> Bool {
      return 3 > 2
    }
  )";
  EXPECT_THAT(Parse(src), HoldsFuncDef(MatchesFuncDefStmt({
                              .name = "foo",
                              .result_type = MatchesBasicType({.name = "Bool"}),
                              .body = {{
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
                              }},
                          })));
}

TEST_F(ParserTest, SingleFuncParam) {
  std::string_view src = R"(
    let id = (x: Int32) -> Int32 {
      return x
    }
  )";
  EXPECT_THAT(Parse(src),
              HoldsFuncDef(MatchesFuncDefStmt({
                  .name = "id",
                  .parameters =
                      {
                          {
                              .name = "x",
                              .type = MatchesBasicType({.name = "Int32"}),
                          },
                      },
                  .result_type = MatchesBasicType({.name = "Int32"}),
                  .body = {{
                      MatchesReturnStmt({
                          .value = MatchesIdentExpr({
                              .name = "x",
                          }),
                      }),
                  }},
              })));
}

TEST_F(ParserTest, MultipleFuncParams) {
  std::string_view src = R"(
    let foo = (a: Int32, b: Double, c: Bool) -> Void {
    }
  )";
  EXPECT_THAT(
      Parse(src),
      HoldsFuncDef(MatchesFuncDefStmt({
          .name = "foo",
          .parameters =
              {
                  {.name = "a", .type = MatchesBasicType({.name = "Int32"})},
                  {.name = "b", .type = MatchesBasicType({.name = "Double"})},
                  {.name = "c", .type = MatchesBasicType({.name = "Bool"})},
              },
          .result_type = MatchesBasicType({.name = "Void"}),
      })));
}

TEST_F(ParserTest, FuncCallExprIntLitArg) {
  std::string_view src = R"(
    let foo = () -> Void {
      bar(3)
    }
  )";
  EXPECT_THAT(Parse(src), HoldsFuncDef(MatchesFuncDefStmt({
                              .name = "foo",
                              .result_type = MatchesBasicType({.name = "Void"}),
                              .body = {{
                                  MatchesFuncCallExpr({
                                      .func_name = "bar",
                                      .arguments =
                                          {
                                              MatchesIntLitExpr({.value = "3"}),
                                          },
                                  }),
                              }},
                          })));
}

TEST_F(ParserTest, FuncCallExprStringLitArg) {
  std::string_view src = R"(
    let foo = () -> Void {
      bar("foo")
    }
  )";
  EXPECT_THAT(Parse(src),
              HoldsFuncDef(MatchesFuncDefStmt({
                  .name = "foo",
                  .result_type = MatchesBasicType({.name = "Void"}),
                  .body = {{
                      MatchesFuncCallExpr({
                          .func_name = "bar",
                          .arguments =
                              {
                                  MatchesStringLitExpr({.value = R"("foo")"}),
                              },
                      }),
                  }},
              })));
}

TEST_F(ParserTest, ReturnFuncCallExpr) {
  std::string_view src = R"(
    let main = () -> Int32 {
      return id(21)
    }
  )";
  EXPECT_THAT(Parse(src),
              HoldsFuncDef(MatchesFuncDefStmt({
                  .name = "main",
                  .result_type = MatchesBasicType({.name = "Int32"}),
                  .body = {{
                      MatchesReturnStmt({
                          .value = MatchesFuncCallExpr({
                              .func_name = "id",
                              .arguments =
                                  {
                                      MatchesIntLitExpr({.value = "21"}),
                                  },
                          }),
                      }),
                  }},
              })));
}

TEST_F(ParserTest, ReturnTrueBoolLit) {
  std::string_view src = R"(
    let truth = () -> Bool {
      return true
    }
  )";
  EXPECT_THAT(Parse(src), HoldsFuncDef(MatchesFuncDefStmt({
                              .name = "truth",
                              .result_type = MatchesBasicType({.name = "Bool"}),
                              .body = {{
                                  MatchesReturnStmt({
                                      .value = MatchesBoolLitExpr({
                                          .value = "true",
                                      }),
                                  }),
                              }},
                          })));
}

TEST_F(ParserTest, ReturnFalseBoolLit) {
  std::string_view src = R"(
    let falsity = () -> Bool {
      return false
    }
  )";
  EXPECT_THAT(Parse(src), HoldsFuncDef(MatchesFuncDefStmt({
                              .name = "falsity",
                              .result_type = MatchesBasicType({.name = "Bool"}),
                              .body = {{
                                  MatchesReturnStmt({
                                      .value = MatchesBoolLitExpr({
                                          .value = "false",
                                      }),
                                  }),
                              }},
                          })));
}

TEST_F(ParserTest, IfStmt) {
  std::string_view src = R"(
    let foo = () -> Int32 {
      if true {
        return 2 + 3
      }
      return 4 * 5
    }
  )";
  EXPECT_THAT(Parse(src),
              HoldsFuncDef(MatchesFuncDefStmt({
                  .name = "foo",
                  .result_type = MatchesBasicType({.name = "Int32"}),
                  .body = {{
                      MatchesIfStmt({
                          .cond = MatchesBoolLitExpr({
                              .value = "true",
                          }),
                          .then_body = {{
                              MatchesReturnStmt({
                                  .value = MatchesBinaryOpExpr({
                                      .op = BinaryOp::Add,
                                      .lhs = MatchesIntLitExpr({.value = "2"}),
                                      .rhs = MatchesIntLitExpr({.value = "3"}),
                                  }),
                              }),
                          }},
                      }),
                      MatchesReturnStmt({
                          .value = MatchesBinaryOpExpr({
                              .op = BinaryOp::Mul,
                              .lhs = MatchesIntLitExpr({.value = "4"}),
                              .rhs = MatchesIntLitExpr({.value = "5"}),
                          }),
                      }),
                  }},
              })));
}

TEST_F(ParserTest, IfElseStmt) {
  std::string_view src = R"(
    let foo = () -> Int32 {
      if true {
        return 2 + 3
      } else {
        return 4 * 5
      }
    }
  )";
  EXPECT_THAT(Parse(src),
              HoldsFuncDef(MatchesFuncDefStmt({
                  .name = "foo",
                  .result_type = MatchesBasicType({.name = "Int32"}),
                  .body = {{
                      MatchesIfStmt({
                          .cond = MatchesBoolLitExpr({
                              .value = "true",
                          }),
                          .then_body = {{
                              MatchesReturnStmt({
                                  .value = MatchesBinaryOpExpr({
                                      .op = BinaryOp::Add,
                                      .lhs = MatchesIntLitExpr({.value = "2"}),
                                      .rhs = MatchesIntLitExpr({.value = "3"}),
                                  }),
                              }),
                          }},
                          .else_body = {{
                              MatchesReturnStmt({
                                  .value = MatchesBinaryOpExpr({
                                      .op = BinaryOp::Mul,
                                      .lhs = MatchesIntLitExpr({.value = "4"}),
                                      .rhs = MatchesIntLitExpr({.value = "5"}),
                                  }),
                              }),
                          }},
                      }),
                  }},
              })));
}

TEST_F(ParserTest, GtInts) {
  std::string_view src = R"(
    let gt = (x: Int32, y: Int32) -> Bool {
      return x > y
    }
  )";
  EXPECT_THAT(
      Parse(src),
      HoldsFuncDef(MatchesFuncDefStmt({
          .name = "gt",
          .result_type = MatchesBasicType({.name = "Bool"}),
          .parameters =
              {
                  {.name = "x", .type = MatchesBasicType({.name = "Int32"})},
                  {.name = "y", .type = MatchesBasicType({.name = "Int32"})},
              },
          .body = {{
              MatchesReturnStmt({
                  .value = MatchesBinaryOpExpr({
                      .op = BinaryOp::Gt,
                      .lhs = MatchesIdentExpr({.name = "x"}),
                      .rhs = MatchesIdentExpr({.name = "y"}),
                  }),
              }),
          }},
      })));
}

TEST_F(ParserTest, LtInts) {
  std::string_view src = R"(
    let lt = (x: Int32, y: Int32) -> Bool {
      return x < y
    }
  )";
  EXPECT_THAT(
      Parse(src),
      HoldsFuncDef(MatchesFuncDefStmt({
          .name = "lt",
          .result_type = MatchesBasicType({.name = "Bool"}),
          .parameters =
              {
                  {.name = "x", .type = MatchesBasicType({.name = "Int32"})},
                  {.name = "y", .type = MatchesBasicType({.name = "Int32"})},
              },
          .body = {{
              MatchesReturnStmt({
                  .value = MatchesBinaryOpExpr({
                      .op = BinaryOp::Lt,
                      .lhs = MatchesIdentExpr({.name = "x"}),
                      .rhs = MatchesIdentExpr({.name = "y"}),
                  }),
              }),
          }},
      })));
}

TEST_F(ParserTest, EqInts) {
  std::string_view src = R"(
    let eq = (x: Int32, y: Int32) -> Bool {
      return x == y
    }
  )";
  EXPECT_THAT(
      Parse(src),
      HoldsFuncDef(MatchesFuncDefStmt({
          .name = "eq",
          .result_type = MatchesBasicType({.name = "Bool"}),
          .parameters =
              {
                  {.name = "x", .type = MatchesBasicType({.name = "Int32"})},
                  {.name = "y", .type = MatchesBasicType({.name = "Int32"})},
              },
          .body = {{
              MatchesReturnStmt({
                  .value = MatchesBinaryOpExpr({
                      .op = BinaryOp::Eq,
                      .lhs = MatchesIdentExpr({.name = "x"}),
                      .rhs = MatchesIdentExpr({.name = "y"}),
                  }),
              }),
          }},
      })));
}

TEST_F(ParserTest, NotEqInts) {
  std::string_view src = R"(
    let neq = (x: Int32, y: Int32) -> Bool {
      return x != y
    }
  )";
  EXPECT_THAT(
      Parse(src),
      HoldsFuncDef(MatchesFuncDefStmt({
          .name = "neq",
          .result_type = MatchesBasicType({.name = "Bool"}),
          .parameters =
              {
                  {.name = "x", .type = MatchesBasicType({.name = "Int32"})},
                  {.name = "y", .type = MatchesBasicType({.name = "Int32"})},
              },
          .body = {{
              MatchesReturnStmt({
                  .value = MatchesBinaryOpExpr({
                      .op = BinaryOp::NotEq,
                      .lhs = MatchesIdentExpr({.name = "x"}),
                      .rhs = MatchesIdentExpr({.name = "y"}),
                  }),
              }),
          }},
      })));
}

TEST_F(ParserTest, VarDecl) {
  std::string_view src = R"(
    let inc = (n: Int32) -> Int32 {
      let m: Int32 = 1
      return n + m 
    }
  )";
  EXPECT_THAT(
      Parse(src),
      HoldsFuncDef(MatchesFuncDefStmt({
          .name = "inc",
          .result_type = MatchesBasicType({.name = "Int32"}),
          .parameters =
              {
                  {.name = "n", .type = MatchesBasicType({.name = "Int32"})},
              },
          .body = {{
              MatchesVarDeclStmt({
                  .name = "m",
                  .type = MatchesBasicType({.name = "Int32"}),
                  .init = MatchesIntLitExpr({.value = "1"}),
              }),
              MatchesReturnStmt({
                  .value = MatchesBinaryOpExpr({
                      .op = BinaryOp::Add,
                      .lhs = MatchesIdentExpr({.name = "n"}),
                      .rhs = MatchesIdentExpr({.name = "m"}),
                  }),
              }),
          }},
      })));
}

TEST_F(ParserTest, VarAssignment) {
  std::string_view src = R"(
    let foo = (n: Int32) -> Void {
      n = 3
    }
  )";
  EXPECT_THAT(
      Parse(src),
      HoldsFuncDef(MatchesFuncDefStmt({
          .name = "foo",
          .result_type = MatchesBasicType({.name = "Void"}),
          .parameters =
              {
                  {.name = "n", .type = MatchesBasicType({.name = "Int32"})},
              },
          .body = {{
              MatchesVarAssignStmt({
                  .name = "n",
                  .expr = MatchesIntLitExpr({.value = "3"}),
              }),
          }},
      })));
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
    let id = (: Int32) -> Int32 {
      return x
    }
  )";
  EXPECT_THAT(Parse(src), HoldsError("expected closing parenthesis or "
                                     "parameter at line 2, column 15"));
}

TEST_F(ParserTest, FuncDefMissingParamColon) {
  std::string_view src = R"(
    let id = (x Int32) -> Int32 {
      return x
    }
  )";
  EXPECT_THAT(Parse(src), HoldsError("unexpected token at line 2, column 17"));
}

TEST_F(ParserTest, FuncDefMissingParamType) {
  std::string_view src = R"(
    let id = (x:) -> Int32 {
      return x
    }
  )";
  EXPECT_THAT(Parse(src),
              HoldsError("expected identifier at line 2, column 17"));
}

TEST_F(ParserTest, FuncDefMissingParamColonAndType) {
  std::string_view src = R"(
    let id = (x) -> Int32 {
      return x
    }
  )";
  EXPECT_THAT(Parse(src), HoldsError("unexpected token at line 2, column 16"));
}

TEST_F(ParserTest, FuncDefMissingNextParam) {
  std::string_view src = R"(
    let id = (x: Int32,) -> Int32 {
      return x
    }
  )";
  EXPECT_THAT(Parse(src), HoldsError("expected closing parenthesis or "
                                     "parameter at line 2, column 24"));
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
    let id = (x: Int32) -> Int32 {
      return
    }
  )";
  EXPECT_THAT(Parse(src), HoldsError("unexpected token at line 4, column 5"));
}

TEST_F(ParserTest, VarDeclMissingLet) {
  std::string_view src = R"(
    let main = () -> Void {
      m: Int32 = 1
    }
  )";
  EXPECT_THAT(Parse(src), HoldsError("unexpected token at line 3, column 8"));
}

TEST_F(ParserTest, VarDeclMissingName) {
  std::string_view src = R"(
    let main = () -> Void {
      let : Int32 = 1
    }
  )";
  EXPECT_THAT(Parse(src),
              HoldsError("expected identifier at line 3, column 11"));
}

TEST_F(ParserTest, VarDeclMissingColon) {
  std::string_view src = R"(
    let main = () -> Void {
      let m Int32 = 1
    }
  )";
  EXPECT_THAT(Parse(src), HoldsError("unexpected token at line 3, column 13"));
}

TEST_F(ParserTest, VarDeclMissingType) {
  std::string_view src = R"(
    let main = () -> Void {
      let m: = 1
    }
  )";
  EXPECT_THAT(Parse(src),
              HoldsError("expected identifier at line 3, column 14"));
}

TEST_F(ParserTest, VarDeclMissingColonAndType) {
  std::string_view src = R"(
    let main = () -> Void {
      let m = 1
    }
  )";
  EXPECT_THAT(Parse(src), HoldsError("unexpected token at line 3, column 13"));
}

TEST_F(ParserTest, VarDeclMissingEqual) {
  std::string_view src = R"(
    let main = () -> Void {
      let m: Int32 1
    }
  )";
  EXPECT_THAT(Parse(src), HoldsError("unexpected token at line 3, column 20"));
}

TEST_F(ParserTest, VarDeclMissingInit) {
  std::string_view src = R"(
    let main = () -> Void {
      let m: Int32 =
    }
  )";
  EXPECT_THAT(Parse(src), HoldsError("unexpected token at line 4, column 5"));
}

TEST_F(ParserTest, VarAssignMissingValue) {
  std::string_view src = R"(
    let foo = (n: Int32) -> Void {
      n =
    }
  )";
  EXPECT_THAT(Parse(src), HoldsError("unexpected token at line 4, column 5"));
}

TEST_F(ParserTest, MissingEqualSign) {
  std::string_view src = R"(
    let main = () -> Int32 {
      if 1 = 1 {
        return 2
      } else {
        return 3
      }
    }
  )";
  EXPECT_THAT(Parse(src), HoldsError("unexpected token at line 3, column 12"));
}

TEST_F(ParserTest, SpaceBetweenEqualSigns) {
  std::string_view src = R"(
    let main = () -> Int32 {
      if 1 = = 1 {
        return 2
      } else {
        return 3
      }
    }
  )";
  EXPECT_THAT(Parse(src), HoldsError("unexpected token at line 3, column 12"));
}

TEST_F(ParserTest, MissingStringClosingQuote) {
  std::string_view src = R"(
    let foo = () -> Void {
      bar("foo)
    }
  )";
  EXPECT_THAT(Parse(src),
              HoldsError("incomplete string literal at line 3, column 11"));
}

}  // namespace
}  // namespace lucid
