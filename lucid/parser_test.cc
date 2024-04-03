#include "lucid/parser.h"

#include <optional>
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

using namespace std::string_literals;

class ParserTest : public testing::Test, public AstFixture {
 protected:
  std::variant<FuncDefStmt, std::string> Parse(std::string_view src) {
    std::string code_with_null(src);
    code_with_null.append("\0"s);

    auto maybe_func_def_stmt =
        Parser(ctx_, src, Lexer(code_with_null)).ParseFuncDef();
    if (auto* ref =
            std::get_if<std::optional<FuncDefStmt>>(&maybe_func_def_stmt)) {
      return ref->value();
    }
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

TEST_F(ParserTest, ReturnModBinaryOpExpr) {
  std::string_view src = R"(
    let main = () -> Int32 {
      return 3 % 2
    }
  )";
  EXPECT_THAT(Parse(src),
              HoldsFuncDef(MatchesFuncDefStmt({
                  .name = "main",
                  .result_type = MatchesBasicType({.name = "Int32"}),
                  .body = {{
                      MatchesReturnStmt({
                          .value = MatchesBinaryOpExpr({
                              .op = BinaryOp::Mod,
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
                  .params =
                      {
                          MatchesFuncParam({
                              .name = "x",
                              .type = MatchesBasicType({.name = "Int32"}),
                          }),
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
  EXPECT_THAT(Parse(src),
              HoldsFuncDef(MatchesFuncDefStmt({
                  .name = "foo",
                  .params =
                      {
                          MatchesFuncParam({
                              .name = "a",
                              .type = MatchesBasicType({.name = "Int32"}),
                          }),
                          MatchesFuncParam({
                              .name = "b",
                              .type = MatchesBasicType({.name = "Double"}),
                          }),
                          MatchesFuncParam({
                              .name = "c",
                              .type = MatchesBasicType({.name = "Bool"}),
                          }),
                      },
                  .result_type = MatchesBasicType({.name = "Void"}),
              })));
}

TEST_F(ParserTest, FuncCallExprIntLitArg) {
  std::string_view src = R"(
    let foo = () -> Void {
      do bar(3)
    }
  )";
  EXPECT_THAT(Parse(src),
              HoldsFuncDef(MatchesFuncDefStmt({
                  .name = "foo",
                  .result_type = MatchesBasicType({.name = "Void"}),
                  .body = {{
                      MatchesDoStmt({
                          .expr = MatchesFuncCallExpr({
                              .func_name = "bar",
                              .args =
                                  {
                                      MatchesIntLitExpr({.value = "3"}),
                                  },
                          }),
                      }),
                  }},
              })));
}

TEST_F(ParserTest, FuncCallExprStringLitArg) {
  std::string_view src = R"(
    let foo = () -> Void {
      do bar("foo")
    }
  )";
  EXPECT_THAT(
      Parse(src),
      HoldsFuncDef(MatchesFuncDefStmt({
          .name = "foo",
          .result_type = MatchesBasicType({.name = "Void"}),
          .body = {{
              MatchesDoStmt({
                  .expr = MatchesFuncCallExpr({
                      .func_name = "bar",
                      .args =
                          {
                              MatchesStringLitExpr({.value = R"("foo")"}),
                          },
                  }),
              }),
          }},
      })));
}

TEST_F(ParserTest, FuncCallExprNestedArg) {
  std::string_view src = R"(
    let foo = () -> Void {
      do bar(baz(1, 2), qux(3, 4))
    }
  )";
  EXPECT_THAT(
      Parse(src),
      HoldsFuncDef(MatchesFuncDefStmt({
          .name = "foo",
          .result_type = MatchesBasicType({.name = "Void"}),
          .body = {{
              MatchesDoStmt({
                  .expr = MatchesFuncCallExpr({
                      .func_name = "bar",
                      .args =
                          {
                              MatchesFuncCallExpr({
                                  .func_name = "baz",
                                  .args =
                                      {
                                          MatchesIntLitExpr({.value = "1"}),
                                          MatchesIntLitExpr({.value = "2"}),
                                      },
                              }),
                              MatchesFuncCallExpr({
                                  .func_name = "qux",
                                  .args =
                                      {
                                          MatchesIntLitExpr({.value = "3"}),
                                          MatchesIntLitExpr({.value = "4"}),
                                      },
                              }),
                          },
                  }),
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
                              .args =
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

TEST_F(ParserTest, IfElseIfElseStmt) {
  std::string_view src = R"(
    let foo = (x: Int32) -> Int32 {
      if x > 0 {
        return 1
      } else if x < 0 {
        return 2
      } else {
        return 3
      }
    }
  )";
  EXPECT_THAT(
      Parse(src),
      HoldsFuncDef(MatchesFuncDefStmt({
          .name = "foo",
          .params =
              {
                  MatchesFuncParam({
                      .name = "x",
                      .type = MatchesBasicType({.name = "Int32"}),
                  }),
              },
          .result_type = MatchesBasicType({.name = "Int32"}),
          .body = {{
              MatchesIfStmt({
                  .cond = MatchesBinaryOpExpr({
                      .op = BinaryOp::Gt,
                      .lhs = MatchesIdentExpr({.name = "x"}),
                      .rhs = MatchesIntLitExpr({.value = "0"}),
                  }),
                  .then_body = {{
                      MatchesReturnStmt({
                          .value = MatchesIntLitExpr({.value = "1"}),
                      }),
                  }},
                  .else_body = {{
                      MatchesIfStmt({
                          .cond = MatchesBinaryOpExpr({
                              .op = BinaryOp::Lt,
                              .lhs = MatchesIdentExpr({.name = "x"}),
                              .rhs = MatchesIntLitExpr({.value = "0"}),
                          }),
                          .then_body = {{
                              MatchesReturnStmt({
                                  .value = MatchesIntLitExpr({.value = "2"}),
                              }),
                          }},
                          .else_body = {{
                              MatchesReturnStmt({
                                  .value = MatchesIntLitExpr({.value = "3"}),
                              }),
                          }},
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
  EXPECT_THAT(Parse(src),
              HoldsFuncDef(MatchesFuncDefStmt({
                  .name = "gt",
                  .result_type = MatchesBasicType({.name = "Bool"}),
                  .params =
                      {
                          MatchesFuncParam({
                              .name = "x",
                              .type = MatchesBasicType({.name = "Int32"}),
                          }),
                          MatchesFuncParam({
                              .name = "y",
                              .type = MatchesBasicType({.name = "Int32"}),
                          }),
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
  EXPECT_THAT(Parse(src),
              HoldsFuncDef(MatchesFuncDefStmt({
                  .name = "lt",
                  .result_type = MatchesBasicType({.name = "Bool"}),
                  .params =
                      {
                          MatchesFuncParam({
                              .name = "x",
                              .type = MatchesBasicType({.name = "Int32"}),
                          }),
                          MatchesFuncParam({
                              .name = "y",
                              .type = MatchesBasicType({.name = "Int32"}),
                          }),
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
  EXPECT_THAT(Parse(src),
              HoldsFuncDef(MatchesFuncDefStmt({
                  .name = "eq",
                  .result_type = MatchesBasicType({.name = "Bool"}),
                  .params =
                      {
                          MatchesFuncParam({
                              .name = "x",
                              .type = MatchesBasicType({.name = "Int32"}),
                          }),
                          MatchesFuncParam({
                              .name = "y",
                              .type = MatchesBasicType({.name = "Int32"}),
                          }),
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
  EXPECT_THAT(Parse(src),
              HoldsFuncDef(MatchesFuncDefStmt({
                  .name = "neq",
                  .result_type = MatchesBasicType({.name = "Bool"}),
                  .params =
                      {
                          MatchesFuncParam({
                              .name = "x",
                              .type = MatchesBasicType({.name = "Int32"}),
                          }),
                          MatchesFuncParam({
                              .name = "y",
                              .type = MatchesBasicType({.name = "Int32"}),
                          }),
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
  EXPECT_THAT(Parse(src),
              HoldsFuncDef(MatchesFuncDefStmt({
                  .name = "inc",
                  .result_type = MatchesBasicType({.name = "Int32"}),
                  .params =
                      {
                          MatchesFuncParam({
                              .name = "n",
                              .type = MatchesBasicType({.name = "Int32"}),
                          }),
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
  EXPECT_THAT(Parse(src),
              HoldsFuncDef(MatchesFuncDefStmt({
                  .name = "foo",
                  .result_type = MatchesBasicType({.name = "Void"}),
                  .params =
                      {
                          MatchesFuncParam({
                              .name = "n",
                              .type = MatchesBasicType({.name = "Int32"}),
                          }),
                      },
                  .body = {{
                      MatchesVarAssignStmt({
                          .name = "n",
                          .expr = MatchesIntLitExpr({.value = "3"}),
                      }),
                  }},
              })));
}

TEST_F(ParserTest, LoopAndBreakStmt) {
  std::string_view src = R"(
    let foo = () -> Int32 {
      loop {
        break
      }
      return 2
    }
  )";
  EXPECT_THAT(Parse(src),
              HoldsFuncDef(MatchesFuncDefStmt({
                  .name = "foo",
                  .result_type = MatchesBasicType({.name = "Int32"}),
                  .body = {{
                      MatchesLoopStmt({
                          .body = {{
                              MatchesBreakStmt(),
                          }},
                      }),
                      MatchesReturnStmt({
                          .value = MatchesIntLitExpr({.value = "2"}),
                      }),
                  }},
              })));
}

TEST_F(ParserTest, EqOverMod) {
  std::string_view src = R"(
    let foo = (a: Int32, b: Int32) -> Int32 {
      if a % b == 10 {
        return 1
      }
      return 2
    }
  )";
  EXPECT_THAT(Parse(src),
              HoldsFuncDef(MatchesFuncDefStmt({
                  .name = "foo",
                  .params =
                      {
                          MatchesFuncParam({
                              .name = "a",
                              .type = MatchesBasicType({.name = "Int32"}),
                          }),
                          MatchesFuncParam({
                              .name = "b",
                              .type = MatchesBasicType({.name = "Int32"}),
                          }),
                      },
                  .result_type = MatchesBasicType({.name = "Int32"}),
                  .body = {{
                      MatchesIfStmt({
                          .cond = MatchesBinaryOpExpr({
                              .op = BinaryOp::Eq,
                              .lhs = MatchesBinaryOpExpr({
                                  .op = BinaryOp::Mod,
                                  .lhs = MatchesIdentExpr({
                                      .name = "a",
                                  }),
                                  .rhs = MatchesIdentExpr({
                                      .name = "b",
                                  }),
                              }),
                              .rhs = MatchesIntLitExpr({
                                  .value = "10",
                              }),
                          }),
                          .then_body = {{
                              MatchesReturnStmt({
                                  .value = MatchesIntLitExpr({.value = "1"}),
                              }),
                          }},
                      }),
                      MatchesReturnStmt({
                          .value = MatchesIntLitExpr({.value = "2"}),
                      }),
                  }},
              })));
}

TEST_F(ParserTest, NotEqOverAdd) {
  std::string_view src = R"(
    let foo = (a: Int32, b: Int32) -> Int32 {
      if a + b != 10 {
        return 1
      }
      return 2
    }
  )";
  EXPECT_THAT(Parse(src),
              HoldsFuncDef(MatchesFuncDefStmt({
                  .name = "foo",
                  .params =
                      {
                          MatchesFuncParam({
                              .name = "a",
                              .type = MatchesBasicType({.name = "Int32"}),
                          }),
                          MatchesFuncParam({
                              .name = "b",
                              .type = MatchesBasicType({.name = "Int32"}),
                          }),
                      },
                  .result_type = MatchesBasicType({.name = "Int32"}),
                  .body = {{
                      MatchesIfStmt({
                          .cond = MatchesBinaryOpExpr({
                              .op = BinaryOp::NotEq,
                              .lhs = MatchesBinaryOpExpr({
                                  .op = BinaryOp::Add,
                                  .lhs = MatchesIdentExpr({
                                      .name = "a",
                                  }),
                                  .rhs = MatchesIdentExpr({
                                      .name = "b",
                                  }),
                              }),
                              .rhs = MatchesIntLitExpr({
                                  .value = "10",
                              }),
                          }),
                          .then_body = {{
                              MatchesReturnStmt({
                                  .value = MatchesIntLitExpr({.value = "1"}),
                              }),
                          }},
                      }),
                      MatchesReturnStmt({
                          .value = MatchesIntLitExpr({.value = "2"}),
                      }),
                  }},
              })));
}

TEST_F(ParserTest, GtOverMul) {
  std::string_view src = R"(
    let foo = (a: Int32, b: Int32) -> Int32 {
      if a * b > 10 {
        return 1
      }
      return 2
    }
  )";
  EXPECT_THAT(Parse(src),
              HoldsFuncDef(MatchesFuncDefStmt({
                  .name = "foo",
                  .params =
                      {
                          MatchesFuncParam({
                              .name = "a",
                              .type = MatchesBasicType({.name = "Int32"}),
                          }),
                          MatchesFuncParam({
                              .name = "b",
                              .type = MatchesBasicType({.name = "Int32"}),
                          }),
                      },
                  .result_type = MatchesBasicType({.name = "Int32"}),
                  .body = {{
                      MatchesIfStmt({
                          .cond = MatchesBinaryOpExpr({
                              .op = BinaryOp::Gt,
                              .lhs = MatchesBinaryOpExpr({
                                  .op = BinaryOp::Mul,
                                  .lhs = MatchesIdentExpr({
                                      .name = "a",
                                  }),
                                  .rhs = MatchesIdentExpr({
                                      .name = "b",
                                  }),
                              }),
                              .rhs = MatchesIntLitExpr({
                                  .value = "10",
                              }),
                          }),
                          .then_body = {{
                              MatchesReturnStmt({
                                  .value = MatchesIntLitExpr({.value = "1"}),
                              }),
                          }},
                      }),
                      MatchesReturnStmt({
                          .value = MatchesIntLitExpr({.value = "2"}),
                      }),
                  }},
              })));
}

TEST_F(ParserTest, LtOverSub) {
  std::string_view src = R"(
    let foo = (a: Int32, b: Int32) -> Int32 {
      if a - b < 10 {
        return 1
      }
      return 2
    }
  )";
  EXPECT_THAT(Parse(src),
              HoldsFuncDef(MatchesFuncDefStmt({
                  .name = "foo",
                  .params =
                      {
                          MatchesFuncParam({
                              .name = "a",
                              .type = MatchesBasicType({.name = "Int32"}),
                          }),
                          MatchesFuncParam({
                              .name = "b",
                              .type = MatchesBasicType({.name = "Int32"}),
                          }),
                      },
                  .result_type = MatchesBasicType({.name = "Int32"}),
                  .body = {{
                      MatchesIfStmt({
                          .cond = MatchesBinaryOpExpr({
                              .op = BinaryOp::Lt,
                              .lhs = MatchesBinaryOpExpr({
                                  .op = BinaryOp::Sub,
                                  .lhs = MatchesIdentExpr({
                                      .name = "a",
                                  }),
                                  .rhs = MatchesIdentExpr({
                                      .name = "b",
                                  }),
                              }),
                              .rhs = MatchesIntLitExpr({
                                  .value = "10",
                              }),
                          }),
                          .then_body = {{
                              MatchesReturnStmt({
                                  .value = MatchesIntLitExpr({.value = "1"}),
                              }),
                          }},
                      }),
                      MatchesReturnStmt({
                          .value = MatchesIntLitExpr({.value = "2"}),
                      }),
                  }},
              })));
}

TEST_F(ParserTest, ArrayParam) {
  std::string_view src = R"(
    let len = (a: Int32[10]) -> Int32 {
      return a[2]
    }
  )";
  EXPECT_THAT(
      Parse(src),
      HoldsFuncDef(MatchesFuncDefStmt({
          .name = "len",
          .params =
              {
                  MatchesFuncParam({
                      .name = "a",
                      .type = MatchesArrayType({
                          .element_type = MatchesBasicType({.name = "Int32"}),
                          .size = {.value = "10"},
                      }),
                  }),
              },
          .result_type = MatchesBasicType({.name = "Int32"}),
          .body = {{
              MatchesReturnStmt({
                  .value = MatchesIndexExpr({
                      .base = MatchesIdentExpr({.name = "a"}),
                      .index = MatchesIntLitExpr({.value = "2"}),
                  }),
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
  EXPECT_THAT(Parse(src), HoldsError("unexpected token at line 2, column 16"));
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
                                     "parameter at line 2, column 17"));
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
  EXPECT_THAT(Parse(src), HoldsError("unexpected token at line 2, column 20"));
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
  EXPECT_THAT(Parse(src), HoldsError("unexpected token at line 3, column 12"));
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
  EXPECT_THAT(Parse(src), HoldsError("unexpected token at line 3, column 12"));
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
  EXPECT_THAT(Parse(src), HoldsError("unexpected token at line 3, column 13"));
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
  EXPECT_THAT(Parse(src), HoldsError("unexpected token at line 3, column 13"));
}

TEST_F(ParserTest, MissingStringClosingQuote) {
  std::string_view src = R"(
    let foo = () -> Void {
      do bar("foo)
    }
  )";
  EXPECT_THAT(Parse(src),
              HoldsError("incomplete string literal at line 3, column 14"));
}

TEST_F(ParserTest, MissingLoopOpenBrace) {
  std::string_view src = R"(
    let main = () -> Int32 {
      loop
        return 1
      } 
      return 2
    }
  )";
  EXPECT_THAT(Parse(src), HoldsError("unexpected token at line 4, column 9"));
}

TEST_F(ParserTest, MissingLoopCloseBrace) {
  std::string_view src = R"(
    let main = () -> Int32 {
      loop {
        return 1
      return 2
    }
  )";
  EXPECT_THAT(Parse(src), HoldsError("unexpected token at line 7, column 3"));
}

}  // namespace
}  // namespace lucid
