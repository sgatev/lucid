#include "lucid/syntax/parser.h"

#include <expected>
#include <optional>
#include <sstream>
#include <string>
#include <string_view>
#include <variant>

#include "gmock/gmock.h"
#include "gtest/gtest.h"
#include "lucid/syntax/ast.h"
#include "lucid/syntax/ast_fixture.h"
#include "lucid/syntax/lexer.h"

MATCHER_P(HoldsFuncDef, match_stmt, "") {
  auto* def = std::get_if<lucid::Def>(&arg);
  if (def == nullptr) return false;

  auto* func_def = std::get_if<lucid::FuncDefStmt>(def);
  if (func_def == nullptr) return false;

  return match_stmt(*func_def);
}

MATCHER_P(HoldsTypeDef, match_stmt, "") {
  auto* def = std::get_if<lucid::Def>(&arg);
  if (def == nullptr) return false;

  auto* type_def = std::get_if<lucid::TypeDefStmt>(def);
  if (type_def == nullptr) return false;

  return match_stmt(*type_def);
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
  std::variant<Def, std::string> Parse(std::string_view src) {
    std::string code_with_null(src);
    code_with_null.append("\0"s);

    std::expected<std::optional<Def>, ParserError> def_or_error =
        Parser(syn_ctx_, src, Lexer(code_with_null)).ParseDef();
    if (!def_or_error.has_value()) {
      std::stringstream out;
      out << def_or_error.error();
      return out.str();
    }

    std::optional<Def> maybe_def = std::move(def_or_error).value();
    if (!maybe_def.has_value()) return "no definition";

    return maybe_def.value();
  }
};

TEST_F(ParserTest, EmptyFuncDefStmt) {
  std::string_view src = R"(
    fun main(): Void {
    }
  )";
  EXPECT_THAT(Parse(src),
              HoldsFuncDef(MatchesFuncDefStmt({
                  .name = I("main"),
                  .result_type = MatchesBasicType({.name = I("Void")}),
              })));
}

TEST_F(ParserTest, ReturnIntLitExpr) {
  std::string_view src = R"(
    fun main(): Int32 {
      return 0
    }
  )";

  EXPECT_THAT(Parse(src),
              HoldsFuncDef(MatchesFuncDefStmt({
                  .name = I("main"),
                  .result_type = MatchesBasicType({.name = I("Int32")}),
                  .body = {{
                      MatchesReturnStmt({
                          .value = MatchesIntLitExpr({
                              .value = 0,
                          }),
                      }),
                  }},
              })));
}

TEST_F(ParserTest, CompFuncDef) {
  std::string_view src = R"(
    comp fun main(): Int32 {
      return 0
    }
  )";

  EXPECT_THAT(Parse(src),
              HoldsFuncDef(MatchesFuncDefStmt({
                  .name = I("main"),
                  .result_type = MatchesBasicType({.name = I("Int32")}),
                  .body = {{
                      MatchesReturnStmt({
                          .value = MatchesIntLitExpr({
                              .value = 0,
                          }),
                      }),
                  }},
                  .is_comp = true,
              })));
}

TEST_F(ParserTest, Comment) {
  std::string_view src = R"(
    # comment
    fun main(): Int32 { # comment
      return 0 # comment
    } # comment
    # comment
  )";

  EXPECT_THAT(Parse(src),
              HoldsFuncDef(MatchesFuncDefStmt({
                  .name = I("main"),
                  .result_type = MatchesBasicType({.name = I("Int32")}),
                  .body = {{
                      MatchesReturnStmt({
                          .value = MatchesIntLitExpr({
                              .value = 0,
                          }),
                      }),
                  }},
              })));
}

TEST_F(ParserTest, ReturnAddExpr) {
  std::string_view src = R"(
    fun main(): Int32 {
      return 3 + 2
    }
  )";
  EXPECT_THAT(Parse(src),
              HoldsFuncDef(MatchesFuncDefStmt({
                  .name = I("main"),
                  .result_type = MatchesBasicType({.name = I("Int32")}),
                  .body = {{
                      MatchesReturnStmt({
                          .value = MatchesBinaryOpExpr({
                              .op = BinaryOp::Add,
                              .lhs = MatchesIntLitExpr({
                                  .value = 3,
                              }),
                              .rhs = MatchesIntLitExpr({
                                  .value = 2,
                              }),
                          }),
                      }),
                  }},
              })));
}

TEST_F(ParserTest, ReturnSubExpr) {
  std::string_view src = R"(
    fun main(): Int32 {
      return 3 - 2
    }
  )";
  EXPECT_THAT(Parse(src),
              HoldsFuncDef(MatchesFuncDefStmt({
                  .name = I("main"),
                  .result_type = MatchesBasicType({.name = I("Int32")}),
                  .body = {{
                      MatchesReturnStmt({
                          .value = MatchesBinaryOpExpr({
                              .op = BinaryOp::Sub,
                              .lhs = MatchesIntLitExpr({
                                  .value = 3,
                              }),
                              .rhs = MatchesIntLitExpr({
                                  .value = 2,
                              }),
                          }),
                      }),
                  }},
              })));
}

TEST_F(ParserTest, ReturnMulExpr) {
  std::string_view src = R"(
    fun main(): Int32 {
      return 3 * 2
    }
  )";
  EXPECT_THAT(Parse(src),
              HoldsFuncDef(MatchesFuncDefStmt({
                  .name = I("main"),
                  .result_type = MatchesBasicType({.name = I("Int32")}),
                  .body = {{
                      MatchesReturnStmt({
                          .value = MatchesBinaryOpExpr({
                              .op = BinaryOp::Mul,
                              .lhs = MatchesIntLitExpr({
                                  .value = 3,
                              }),
                              .rhs = MatchesIntLitExpr({
                                  .value = 2,
                              }),
                          }),
                      }),
                  }},
              })));
}

TEST_F(ParserTest, ReturnDivExpr) {
  std::string_view src = R"(
    fun main(): Int32 {
      return 3 / 2
    }
  )";
  EXPECT_THAT(Parse(src),
              HoldsFuncDef(MatchesFuncDefStmt({
                  .name = I("main"),
                  .result_type = MatchesBasicType({.name = I("Int32")}),
                  .body = {{
                      MatchesReturnStmt({
                          .value = MatchesBinaryOpExpr({
                              .op = BinaryOp::Div,
                              .lhs = MatchesIntLitExpr({
                                  .value = 3,
                              }),
                              .rhs = MatchesIntLitExpr({
                                  .value = 2,
                              }),
                          }),
                      }),
                  }},
              })));
}

TEST_F(ParserTest, ReturnModExpr) {
  std::string_view src = R"(
    fun main(): Int32 {
      return 3 % 2
    }
  )";
  EXPECT_THAT(Parse(src),
              HoldsFuncDef(MatchesFuncDefStmt({
                  .name = I("main"),
                  .result_type = MatchesBasicType({.name = I("Int32")}),
                  .body = {{
                      MatchesReturnStmt({
                          .value = MatchesBinaryOpExpr({
                              .op = BinaryOp::Mod,
                              .lhs = MatchesIntLitExpr({
                                  .value = 3,
                              }),
                              .rhs = MatchesIntLitExpr({
                                  .value = 2,
                              }),
                          }),
                      }),
                  }},
              })));
}

TEST_F(ParserTest, ReturnGtExpr) {
  std::string_view src = R"(
    fun foo(): Bool {
      return 3 > 2
    }
  )";
  EXPECT_THAT(Parse(src),
              HoldsFuncDef(MatchesFuncDefStmt({
                  .name = I("foo"),
                  .result_type = MatchesBasicType({.name = I("Bool")}),
                  .body = {{
                      MatchesReturnStmt({
                          .value = MatchesBinaryOpExpr({
                              .op = BinaryOp::Gt,
                              .lhs = MatchesIntLitExpr({
                                  .value = 3,
                              }),
                              .rhs = MatchesIntLitExpr({
                                  .value = 2,
                              }),
                          }),
                      }),
                  }},
              })));
}

TEST_F(ParserTest, Precedence) {
  std::string_view src = R"(
    fun foo(x: Int32): Bool {
      return 2 * x + 4 > 5 - 6 / 7
    }
  )";
  EXPECT_THAT(
      Parse(src),
      HoldsFuncDef(MatchesFuncDefStmt({
          .name = I("foo"),
          .params =
              {
                  MatchesFuncParam({
                      .name = I("x"),
                      .type_constraint = MatchesBasicType({.name = I("Int32")}),
                  }),
              },
          .result_type = MatchesBasicType({.name = I("Bool")}),
          .body = {{
              MatchesReturnStmt({
                  .value = MatchesBinaryOpExpr({
                      .op = BinaryOp::Gt,
                      .lhs = MatchesBinaryOpExpr({
                          .op = BinaryOp::Add,
                          .lhs = MatchesBinaryOpExpr({
                              .op = BinaryOp::Mul,
                              .lhs = MatchesIntLitExpr({
                                  .value = 2,
                              }),
                              .rhs = MatchesIdentExpr({
                                  .name = I("x"),
                              }),
                          }),
                          .rhs = MatchesIntLitExpr({
                              .value = 4,
                          }),
                      }),
                      .rhs = MatchesBinaryOpExpr({
                          .op = BinaryOp::Sub,
                          .lhs = MatchesIntLitExpr({
                              .value = 5,
                          }),
                          .rhs = MatchesBinaryOpExpr({
                              .op = BinaryOp::Div,
                              .lhs = MatchesIntLitExpr({
                                  .value = 6,
                              }),
                              .rhs = MatchesIntLitExpr({
                                  .value = 7,
                              }),
                          }),
                      }),
                  }),
              }),
          }},
      })));
}

TEST_F(ParserTest, SingleFuncParam) {
  std::string_view src = R"(
    fun id(x: Int32): Int32 {
      return x
    }
  )";
  EXPECT_THAT(
      Parse(src),
      HoldsFuncDef(MatchesFuncDefStmt({
          .name = I("id"),
          .params =
              {
                  MatchesFuncParam({
                      .name = I("x"),
                      .type_constraint = MatchesBasicType({.name = I("Int32")}),
                  }),
              },
          .result_type = MatchesBasicType({.name = I("Int32")}),
          .body = {{
              MatchesReturnStmt({
                  .value = MatchesIdentExpr({
                      .name = I("x"),
                  }),
              }),
          }},
      })));
}

TEST_F(ParserTest, MultipleFuncParams) {
  std::string_view src = R"(
    fun foo(a: Int32, b: Double, c: Bool): Void {
    }
  )";
  EXPECT_THAT(
      Parse(src),
      HoldsFuncDef(MatchesFuncDefStmt({
          .name = I("foo"),
          .params =
              {
                  MatchesFuncParam({
                      .name = I("a"),
                      .type_constraint = MatchesBasicType({.name = I("Int32")}),
                  }),
                  MatchesFuncParam({
                      .name = I("b"),
                      .type_constraint =
                          MatchesBasicType({.name = I("Double")}),
                  }),
                  MatchesFuncParam({
                      .name = I("c"),
                      .type_constraint = MatchesBasicType({.name = I("Bool")}),
                  }),
              },
          .result_type = MatchesBasicType({.name = I("Void")}),
      })));
}

TEST_F(ParserTest, FuncCallExprIntLitArg) {
  std::string_view src = R"(
    fun foo(): Void {
      do bar(3)
    }
  )";
  EXPECT_THAT(Parse(src),
              HoldsFuncDef(MatchesFuncDefStmt({
                  .name = I("foo"),
                  .result_type = MatchesBasicType({.name = I("Void")}),
                  .body = {{
                      MatchesDoStmt({
                          .expr = MatchesFuncCallExpr({
                              .func_name = I("bar"),
                              .args =
                                  {
                                      MatchesIntLitExpr({.value = 3}),
                                  },
                          }),
                      }),
                  }},
              })));
}

TEST_F(ParserTest, FuncCallExprStringLitArg) {
  std::string_view src = R"(
    fun foo(): Void {
      do bar("foo")
    }
  )";
  EXPECT_THAT(
      Parse(src),
      HoldsFuncDef(MatchesFuncDefStmt({
          .name = I("foo"),
          .result_type = MatchesBasicType({.name = I("Void")}),
          .body = {{
              MatchesDoStmt({
                  .expr = MatchesFuncCallExpr({
                      .func_name = I("bar"),
                      .args =
                          {
                              MatchesStringLitExpr({.value = I(R"("foo")")}),
                          },
                  }),
              }),
          }},
      })));
}

TEST_F(ParserTest, FuncCallExprNestedArg) {
  std::string_view src = R"(
    fun foo(): Void {
      do bar(baz(1, 2), qux(3, 4))
    }
  )";
  EXPECT_THAT(
      Parse(src),
      HoldsFuncDef(MatchesFuncDefStmt({
          .name = I("foo"),
          .result_type = MatchesBasicType({.name = I("Void")}),
          .body = {{
              MatchesDoStmt({
                  .expr = MatchesFuncCallExpr({
                      .func_name = I("bar"),
                      .args =
                          {
                              MatchesFuncCallExpr({
                                  .func_name = I("baz"),
                                  .args =
                                      {
                                          MatchesIntLitExpr({.value = 1}),
                                          MatchesIntLitExpr({.value = 2}),
                                      },
                              }),
                              MatchesFuncCallExpr({
                                  .func_name = I("qux"),
                                  .args =
                                      {
                                          MatchesIntLitExpr({.value = 3}),
                                          MatchesIntLitExpr({.value = 4}),
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
    fun main(): Int32 {
      return id(21)
    }
  )";
  EXPECT_THAT(Parse(src),
              HoldsFuncDef(MatchesFuncDefStmt({
                  .name = I("main"),
                  .result_type = MatchesBasicType({.name = I("Int32")}),
                  .body = {{
                      MatchesReturnStmt({
                          .value = MatchesFuncCallExpr({
                              .func_name = I("id"),
                              .args =
                                  {
                                      MatchesIntLitExpr({.value = 21}),
                                  },
                          }),
                      }),
                  }},
              })));
}

TEST_F(ParserTest, ReturnTrueBoolLit) {
  std::string_view src = R"(
    fun truth(): Bool {
      return true
    }
  )";
  EXPECT_THAT(Parse(src),
              HoldsFuncDef(MatchesFuncDefStmt({
                  .name = I("truth"),
                  .result_type = MatchesBasicType({.name = I("Bool")}),
                  .body = {{
                      MatchesReturnStmt({
                          .value = MatchesBoolLitExpr({
                              .value = true,
                          }),
                      }),
                  }},
              })));
}

TEST_F(ParserTest, ReturnFalseBoolLit) {
  std::string_view src = R"(
    fun falsity(): Bool {
      return false
    }
  )";
  EXPECT_THAT(Parse(src),
              HoldsFuncDef(MatchesFuncDefStmt({
                  .name = I("falsity"),
                  .result_type = MatchesBasicType({.name = I("Bool")}),
                  .body = {{
                      MatchesReturnStmt({
                          .value = MatchesBoolLitExpr({
                              .value = false,
                          }),
                      }),
                  }},
              })));
}

TEST_F(ParserTest, IfStmt) {
  std::string_view src = R"(
    fun foo(): Int32 {
      if true {
        return 2 + 3
      }
      return 4 * 5
    }
  )";
  EXPECT_THAT(Parse(src),
              HoldsFuncDef(MatchesFuncDefStmt({
                  .name = I("foo"),
                  .result_type = MatchesBasicType({.name = I("Int32")}),
                  .body = {{
                      MatchesIfStmt({
                          .cond = MatchesBoolLitExpr({
                              .value = true,
                          }),
                          .then_body = {{
                              MatchesReturnStmt({
                                  .value = MatchesBinaryOpExpr({
                                      .op = BinaryOp::Add,
                                      .lhs = MatchesIntLitExpr({.value = 2}),
                                      .rhs = MatchesIntLitExpr({.value = 3}),
                                  }),
                              }),
                          }},
                      }),
                      MatchesReturnStmt({
                          .value = MatchesBinaryOpExpr({
                              .op = BinaryOp::Mul,
                              .lhs = MatchesIntLitExpr({.value = 4}),
                              .rhs = MatchesIntLitExpr({.value = 5}),
                          }),
                      }),
                  }},
              })));
}

TEST_F(ParserTest, IfElseStmt) {
  std::string_view src = R"(
    fun foo(): Int32 {
      if true {
        return 2 + 3
      } else {
        return 4 * 5
      }
    }
  )";
  EXPECT_THAT(Parse(src),
              HoldsFuncDef(MatchesFuncDefStmt({
                  .name = I("foo"),
                  .result_type = MatchesBasicType({.name = I("Int32")}),
                  .body = {{
                      MatchesIfStmt({
                          .cond = MatchesBoolLitExpr({
                              .value = true,
                          }),
                          .then_body = {{
                              MatchesReturnStmt({
                                  .value = MatchesBinaryOpExpr({
                                      .op = BinaryOp::Add,
                                      .lhs = MatchesIntLitExpr({.value = 2}),
                                      .rhs = MatchesIntLitExpr({.value = 3}),
                                  }),
                              }),
                          }},
                          .else_body = {{
                              MatchesReturnStmt({
                                  .value = MatchesBinaryOpExpr({
                                      .op = BinaryOp::Mul,
                                      .lhs = MatchesIntLitExpr({.value = 4}),
                                      .rhs = MatchesIntLitExpr({.value = 5}),
                                  }),
                              }),
                          }},
                      }),
                  }},
              })));
}

TEST_F(ParserTest, IfElseIfElseStmt) {
  std::string_view src = R"(
    fun foo(x: Int32): Int32 {
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
          .name = I("foo"),
          .params =
              {
                  MatchesFuncParam({
                      .name = I("x"),
                      .type_constraint = MatchesBasicType({.name = I("Int32")}),
                  }),
              },
          .result_type = MatchesBasicType({.name = I("Int32")}),
          .body = {{
              MatchesIfStmt({
                  .cond = MatchesBinaryOpExpr({
                      .op = BinaryOp::Gt,
                      .lhs = MatchesIdentExpr({.name = I("x")}),
                      .rhs = MatchesIntLitExpr({.value = 0}),
                  }),
                  .then_body = {{
                      MatchesReturnStmt({
                          .value = MatchesIntLitExpr({.value = 1}),
                      }),
                  }},
                  .else_body = {{
                      MatchesIfStmt({
                          .cond = MatchesBinaryOpExpr({
                              .op = BinaryOp::Lt,
                              .lhs = MatchesIdentExpr({.name = I("x")}),
                              .rhs = MatchesIntLitExpr({.value = 0}),
                          }),
                          .then_body = {{
                              MatchesReturnStmt({
                                  .value = MatchesIntLitExpr({.value = 2}),
                              }),
                          }},
                          .else_body = {{
                              MatchesReturnStmt({
                                  .value = MatchesIntLitExpr({.value = 3}),
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
    fun gt(x: Int32, y: Int32): Bool {
      return x > y
    }
  )";
  EXPECT_THAT(
      Parse(src),
      HoldsFuncDef(MatchesFuncDefStmt({
          .name = I("gt"),
          .params =
              {
                  MatchesFuncParam({
                      .name = I("x"),
                      .type_constraint = MatchesBasicType({.name = I("Int32")}),
                  }),
                  MatchesFuncParam({
                      .name = I("y"),
                      .type_constraint = MatchesBasicType({.name = I("Int32")}),
                  }),
              },
          .result_type = MatchesBasicType({.name = I("Bool")}),
          .body = {{
              MatchesReturnStmt({
                  .value = MatchesBinaryOpExpr({
                      .op = BinaryOp::Gt,
                      .lhs = MatchesIdentExpr({.name = I("x")}),
                      .rhs = MatchesIdentExpr({.name = I("y")}),
                  }),
              }),
          }},
      })));
}

TEST_F(ParserTest, LtInts) {
  std::string_view src = R"(
    fun lt(x: Int32, y: Int32): Bool {
      return x < y
    }
  )";
  EXPECT_THAT(
      Parse(src),
      HoldsFuncDef(MatchesFuncDefStmt({
          .name = I("lt"),
          .params =
              {
                  MatchesFuncParam({
                      .name = I("x"),
                      .type_constraint = MatchesBasicType({.name = I("Int32")}),
                  }),
                  MatchesFuncParam({
                      .name = I("y"),
                      .type_constraint = MatchesBasicType({.name = I("Int32")}),
                  }),
              },
          .result_type = MatchesBasicType({.name = I("Bool")}),
          .body = {{
              MatchesReturnStmt({
                  .value = MatchesBinaryOpExpr({
                      .op = BinaryOp::Lt,
                      .lhs = MatchesIdentExpr({.name = I("x")}),
                      .rhs = MatchesIdentExpr({.name = I("y")}),
                  }),
              }),
          }},
      })));
}

TEST_F(ParserTest, EqInts) {
  std::string_view src = R"(
    fun eq(x: Int32, y: Int32): Bool {
      return x == y
    }
  )";
  EXPECT_THAT(
      Parse(src),
      HoldsFuncDef(MatchesFuncDefStmt({
          .name = I("eq"),
          .params =
              {
                  MatchesFuncParam({
                      .name = I("x"),
                      .type_constraint = MatchesBasicType({.name = I("Int32")}),
                  }),
                  MatchesFuncParam({
                      .name = I("y"),
                      .type_constraint = MatchesBasicType({.name = I("Int32")}),
                  }),
              },
          .result_type = MatchesBasicType({.name = I("Bool")}),
          .body = {{
              MatchesReturnStmt({
                  .value = MatchesBinaryOpExpr({
                      .op = BinaryOp::Eq,
                      .lhs = MatchesIdentExpr({.name = I("x")}),
                      .rhs = MatchesIdentExpr({.name = I("y")}),
                  }),
              }),
          }},
      })));
}

TEST_F(ParserTest, NotEqInts) {
  std::string_view src = R"(
    fun neq(x: Int32, y: Int32): Bool {
      return x != y
    }
  )";
  EXPECT_THAT(
      Parse(src),
      HoldsFuncDef(MatchesFuncDefStmt({
          .name = I("neq"),
          .params =
              {
                  MatchesFuncParam({
                      .name = I("x"),
                      .type_constraint = MatchesBasicType({.name = I("Int32")}),
                  }),
                  MatchesFuncParam({
                      .name = I("y"),
                      .type_constraint = MatchesBasicType({.name = I("Int32")}),
                  }),
              },
          .result_type = MatchesBasicType({.name = I("Bool")}),
          .body = {{
              MatchesReturnStmt({
                  .value = MatchesBinaryOpExpr({
                      .op = BinaryOp::NotEq,
                      .lhs = MatchesIdentExpr({.name = I("x")}),
                      .rhs = MatchesIdentExpr({.name = I("y")}),
                  }),
              }),
          }},
      })));
}

TEST_F(ParserTest, VarDecl) {
  std::string_view src = R"(
    fun inc(n: Int32): Int32 {
      val m: Int32 = 1
      return n + m 
    }
  )";
  EXPECT_THAT(
      Parse(src),
      HoldsFuncDef(MatchesFuncDefStmt({
          .name = I("inc"),
          .params =
              {
                  MatchesFuncParam({
                      .name = I("n"),
                      .type_constraint = MatchesBasicType({.name = I("Int32")}),
                  }),
              },
          .result_type = MatchesBasicType({.name = I("Int32")}),
          .body = {{
              MatchesVarDeclStmt({
                  .name = I("m"),
                  .type_constraint = MatchesBasicType({.name = I("Int32")}),
                  .init = MatchesIntLitExpr({.value = 1}),
              }),
              MatchesReturnStmt({
                  .value = MatchesBinaryOpExpr({
                      .op = BinaryOp::Add,
                      .lhs = MatchesIdentExpr({.name = I("n")}),
                      .rhs = MatchesIdentExpr({.name = I("m")}),
                  }),
              }),
          }},
      })));
}

TEST_F(ParserTest, CompVarDecl) {
  std::string_view src = R"(
    fun inc(n: Int32): Int32 {
      comp val m: Int32 = 1
      return n + m
    }
  )";
  EXPECT_THAT(
      Parse(src),
      HoldsFuncDef(MatchesFuncDefStmt({
          .name = I("inc"),
          .params =
              {
                  MatchesFuncParam({
                      .name = I("n"),
                      .type_constraint = MatchesBasicType({.name = I("Int32")}),
                  }),
              },
          .result_type = MatchesBasicType({.name = I("Int32")}),
          .body = {{
              MatchesVarDeclStmt({
                  .name = I("m"),
                  .type_constraint = MatchesBasicType({.name = I("Int32")}),
                  .init = MatchesIntLitExpr({.value = 1}),
                  .is_comp = true,
              }),
              MatchesReturnStmt({
                  .value = MatchesBinaryOpExpr({
                      .op = BinaryOp::Add,
                      .lhs = MatchesIdentExpr({.name = I("n")}),
                      .rhs = MatchesIdentExpr({.name = I("m")}),
                  }),
              }),
          }},
      })));
}

TEST_F(ParserTest, VarAssignment) {
  std::string_view src = R"(
    fun foo(n: Int32): Void {
      n = 3
    }
  )";
  EXPECT_THAT(
      Parse(src),
      HoldsFuncDef(MatchesFuncDefStmt({
          .name = I("foo"),
          .params =
              {
                  MatchesFuncParam({
                      .name = I("n"),
                      .type_constraint = MatchesBasicType({.name = I("Int32")}),
                  }),
              },
          .result_type = MatchesBasicType({.name = I("Void")}),
          .body = {{
              MatchesVarAssignStmt({
                  .name = I("n"),
                  .expr = MatchesIntLitExpr({.value = 3}),
              }),
          }},
      })));
}

TEST_F(ParserTest, LoopAndBreakStmt) {
  std::string_view src = R"(
    fun foo(): Int32 {
      loop {
        break
      }
      return 2
    }
  )";
  EXPECT_THAT(Parse(src),
              HoldsFuncDef(MatchesFuncDefStmt({
                  .name = I("foo"),
                  .result_type = MatchesBasicType({.name = I("Int32")}),
                  .body = {{
                      MatchesLoopStmt({
                          .body = {{
                              MatchesBreakStmt(),
                          }},
                      }),
                      MatchesReturnStmt({
                          .value = MatchesIntLitExpr({.value = 2}),
                      }),
                  }},
              })));
}

TEST_F(ParserTest, EqOverMod) {
  std::string_view src = R"(
    fun foo(a: Int32, b: Int32): Int32 {
      if a % b == 10 {
        return 1
      }
      return 2
    }
  )";
  EXPECT_THAT(
      Parse(src),
      HoldsFuncDef(MatchesFuncDefStmt({
          .name = I("foo"),
          .params =
              {
                  MatchesFuncParam({
                      .name = I("a"),
                      .type_constraint = MatchesBasicType({.name = I("Int32")}),
                  }),
                  MatchesFuncParam({
                      .name = I("b"),
                      .type_constraint = MatchesBasicType({.name = I("Int32")}),
                  }),
              },
          .result_type = MatchesBasicType({.name = I("Int32")}),
          .body = {{
              MatchesIfStmt({
                  .cond = MatchesBinaryOpExpr({
                      .op = BinaryOp::Eq,
                      .lhs = MatchesBinaryOpExpr({
                          .op = BinaryOp::Mod,
                          .lhs = MatchesIdentExpr({
                              .name = I("a"),
                          }),
                          .rhs = MatchesIdentExpr({
                              .name = I("b"),
                          }),
                      }),
                      .rhs = MatchesIntLitExpr({
                          .value = 10,
                      }),
                  }),
                  .then_body = {{
                      MatchesReturnStmt({
                          .value = MatchesIntLitExpr({.value = 1}),
                      }),
                  }},
              }),
              MatchesReturnStmt({
                  .value = MatchesIntLitExpr({.value = 2}),
              }),
          }},
      })));
}

TEST_F(ParserTest, NotEqOverAdd) {
  std::string_view src = R"(
    fun foo(a: Int32, b: Int32): Int32 {
      if a + b != 10 {
        return 1
      }
      return 2
    }
  )";
  EXPECT_THAT(
      Parse(src),
      HoldsFuncDef(MatchesFuncDefStmt({
          .name = I("foo"),
          .params =
              {
                  MatchesFuncParam({
                      .name = I("a"),
                      .type_constraint = MatchesBasicType({.name = I("Int32")}),
                  }),
                  MatchesFuncParam({
                      .name = I("b"),
                      .type_constraint = MatchesBasicType({.name = I("Int32")}),
                  }),
              },
          .result_type = MatchesBasicType({.name = I("Int32")}),
          .body = {{
              MatchesIfStmt({
                  .cond = MatchesBinaryOpExpr({
                      .op = BinaryOp::NotEq,
                      .lhs = MatchesBinaryOpExpr({
                          .op = BinaryOp::Add,
                          .lhs = MatchesIdentExpr({
                              .name = I("a"),
                          }),
                          .rhs = MatchesIdentExpr({
                              .name = I("b"),
                          }),
                      }),
                      .rhs = MatchesIntLitExpr({
                          .value = 10,
                      }),
                  }),
                  .then_body = {{
                      MatchesReturnStmt({
                          .value = MatchesIntLitExpr({.value = 1}),
                      }),
                  }},
              }),
              MatchesReturnStmt({
                  .value = MatchesIntLitExpr({.value = 2}),
              }),
          }},
      })));
}

TEST_F(ParserTest, GtOverMul) {
  std::string_view src = R"(
    fun foo(a: Int32, b: Int32): Int32 {
      if a * b > 10 {
        return 1
      }
      return 2
    }
  )";
  EXPECT_THAT(
      Parse(src),
      HoldsFuncDef(MatchesFuncDefStmt({
          .name = I("foo"),
          .params =
              {
                  MatchesFuncParam({
                      .name = I("a"),
                      .type_constraint = MatchesBasicType({.name = I("Int32")}),
                  }),
                  MatchesFuncParam({
                      .name = I("b"),
                      .type_constraint = MatchesBasicType({.name = I("Int32")}),
                  }),
              },
          .result_type = MatchesBasicType({.name = I("Int32")}),
          .body = {{
              MatchesIfStmt({
                  .cond = MatchesBinaryOpExpr({
                      .op = BinaryOp::Gt,
                      .lhs = MatchesBinaryOpExpr({
                          .op = BinaryOp::Mul,
                          .lhs = MatchesIdentExpr({
                              .name = I("a"),
                          }),
                          .rhs = MatchesIdentExpr({
                              .name = I("b"),
                          }),
                      }),
                      .rhs = MatchesIntLitExpr({
                          .value = 10,
                      }),
                  }),
                  .then_body = {{
                      MatchesReturnStmt({
                          .value = MatchesIntLitExpr({.value = 1}),
                      }),
                  }},
              }),
              MatchesReturnStmt({
                  .value = MatchesIntLitExpr({.value = 2}),
              }),
          }},
      })));
}

TEST_F(ParserTest, LtOverSub) {
  std::string_view src = R"(
    fun foo(a: Int32, b: Int32): Int32 {
      if a - b < 10 {
        return 1
      }
      return 2
    }
  )";
  EXPECT_THAT(
      Parse(src),
      HoldsFuncDef(MatchesFuncDefStmt({
          .name = I("foo"),
          .params =
              {
                  MatchesFuncParam({
                      .name = I("a"),
                      .type_constraint = MatchesBasicType({.name = I("Int32")}),
                  }),
                  MatchesFuncParam({
                      .name = I("b"),
                      .type_constraint = MatchesBasicType({.name = I("Int32")}),
                  }),
              },
          .result_type = MatchesBasicType({.name = I("Int32")}),
          .body = {{
              MatchesIfStmt({
                  .cond = MatchesBinaryOpExpr({
                      .op = BinaryOp::Lt,
                      .lhs = MatchesBinaryOpExpr({
                          .op = BinaryOp::Sub,
                          .lhs = MatchesIdentExpr({
                              .name = I("a"),
                          }),
                          .rhs = MatchesIdentExpr({
                              .name = I("b"),
                          }),
                      }),
                      .rhs = MatchesIntLitExpr({
                          .value = 10,
                      }),
                  }),
                  .then_body = {{
                      MatchesReturnStmt({
                          .value = MatchesIntLitExpr({.value = 1}),
                      }),
                  }},
              }),
              MatchesReturnStmt({
                  .value = MatchesIntLitExpr({.value = 2}),
              }),
          }},
      })));
}

TEST_F(ParserTest, ArrayParam) {
  std::string_view src = R"(
    fun len(a: Int32[10]): Int32 {
      return a[2]
    }
  )";
  EXPECT_THAT(Parse(src),
              HoldsFuncDef(MatchesFuncDefStmt({
                  .name = I("len"),
                  .params =
                      {
                          MatchesFuncParam({
                              .name = I("a"),
                              .type_constraint = MatchesArrayType({
                                  .element_type_constraint =
                                      MatchesBasicType({.name = I("Int32")}),
                                  .size = {.value = 10},
                              }),
                          }),
                      },
                  .result_type = MatchesBasicType({.name = I("Int32")}),
                  .body = {{
                      MatchesReturnStmt({
                          .value = MatchesIndexExpr({
                              .base = MatchesIdentExpr({.name = I("a")}),
                              .index = MatchesIntLitExpr({.value = 2}),
                          }),
                      }),
                  }},
              })));
}

TEST_F(ParserTest, EmptyTuple) {
  std::string_view src = R"(
    comp val Empty: Type = ()
  )";
  EXPECT_THAT(Parse(src), HoldsTypeDef(MatchesTypeDefStmt({
                              .name = I("Empty"),
                              .type = MatchesTupleType({
                                  .fields = {},
                              }),
                          })));
}

TEST_F(ParserTest, Tuple) {
  std::string_view src = R"(
    comp val Point: Type = (x: Int32, y: Int32)
  )";
  EXPECT_THAT(Parse(src),
              HoldsTypeDef(MatchesTypeDefStmt({
                  .name = I("Point"),
                  .type = MatchesTupleType({
                      .fields =
                          {
                              MatchesFuncParam({
                                  .name = I("x"),
                                  .type_constraint =
                                      MatchesBasicType({.name = I("Int32")}),
                              }),
                              MatchesFuncParam({
                                  .name = I("y"),
                                  .type_constraint =
                                      MatchesBasicType({.name = I("Int32")}),
                              }),
                          },

                  }),
              })));
}

TEST_F(ParserTest, FuncDefMissingLet) {
  std::string_view src = R"(
    = () Void {
    }
  )";
  EXPECT_THAT(Parse(src), HoldsError("unexpected token at line 2, column 5"));
}

TEST_F(ParserTest, FuncDefMissingName) {
  std::string_view src = R"(
    fun = () Void {
    }
  )";
  EXPECT_THAT(Parse(src),
              HoldsError("expected identifier at line 2, column 9"));
}

TEST_F(ParserTest, FuncDefMissingOpeningParen) {
  std::string_view src = R"(
    fun main ) Void {
    }
  )";
  EXPECT_THAT(Parse(src), HoldsError("unexpected token at line 2, column 14"));
}

TEST_F(ParserTest, FuncDefMissingParamName) {
  std::string_view src = R"(
    fun id(: Int32) Int32 {
      return x
    }
  )";
  EXPECT_THAT(Parse(src), HoldsError("expected closing parenthesis or "
                                     "parameter at line 2, column 12"));
}

TEST_F(ParserTest, FuncDefMissingParamColon) {
  std::string_view src = R"(
    fun id(x Int32) Int32 {
      return x
    }
  )";
  EXPECT_THAT(Parse(src), HoldsError("unexpected token at line 2, column 13"));
}

TEST_F(ParserTest, FuncDefMissingParamType) {
  std::string_view src = R"(
    fun id(x:) Int32 {
      return x
    }
  )";
  EXPECT_THAT(Parse(src),
              HoldsError("expected identifier at line 2, column 14"));
}

TEST_F(ParserTest, FuncDefMissingParamColonAndType) {
  std::string_view src = R"(
    fun id(x) Int32 {
      return x
    }
  )";
  EXPECT_THAT(Parse(src), HoldsError("unexpected token at line 2, column 13"));
}

TEST_F(ParserTest, FuncDefMissingNextParam) {
  std::string_view src = R"(
    fun id(x: Int32,) Int32 {
      return x
    }
  )";
  EXPECT_THAT(Parse(src), HoldsError("expected closing parenthesis or "
                                     "parameter at line 2, column 21"));
}

TEST_F(ParserTest, FuncDefMissingClosingParen) {
  std::string_view src = R"(
    fun main( Void {
    }
  )";
  EXPECT_THAT(Parse(src), HoldsError("expected closing parenthesis or "
                                     "parameter at line 2, column 14"));
}

TEST_F(ParserTest, FuncDefMissingResultColon) {
  std::string_view src = R"(
    fun main() Void {
    }
  )";
  EXPECT_THAT(Parse(src), HoldsError("unexpected token at line 2, column 16"));
}

TEST_F(ParserTest, FuncDefMissingResultType) {
  std::string_view src = R"(
    fun main(): {
    }
  )";
  EXPECT_THAT(Parse(src),
              HoldsError("expected identifier at line 2, column 17"));
}

TEST_F(ParserTest, FuncDefMissingOpenBrace) {
  std::string_view src = R"(
    fun main(): Void
    }
  )";
  EXPECT_THAT(Parse(src), HoldsError("unexpected token at line 3, column 5"));
}

TEST_F(ParserTest, FuncDefMissingClosingBrace) {
  std::string_view src = R"(
    fun main(): Void {
  )";
  EXPECT_THAT(Parse(src), HoldsError("unexpected token at line 3, column 3"));
}

TEST_F(ParserTest, ReturnMissingValue) {
  std::string_view src = R"(
    fun id(x: Int32): Int32 {
      return
    }
  )";
  EXPECT_THAT(Parse(src), HoldsError("unexpected token at line 4, column 5"));
}

TEST_F(ParserTest, VarDeclMissingLet) {
  std::string_view src = R"(
    fun main(): Void {
      m: Int32 = 1
    }
  )";
  EXPECT_THAT(Parse(src), HoldsError("unexpected token at line 3, column 8"));
}

TEST_F(ParserTest, VarDeclMissingName) {
  std::string_view src = R"(
    fun main(): Void {
      val : Int32 = 1
    }
  )";
  EXPECT_THAT(Parse(src),
              HoldsError("expected identifier at line 3, column 11"));
}

TEST_F(ParserTest, VarDeclMissingColon) {
  std::string_view src = R"(
    fun main(): Void {
      val m Int32 = 1
    }
  )";
  EXPECT_THAT(Parse(src), HoldsError("unexpected token at line 3, column 12"));
}

TEST_F(ParserTest, VarDeclMissingType) {
  std::string_view src = R"(
    fun main(): Void {
      val m: = 1
    }
  )";
  EXPECT_THAT(Parse(src),
              HoldsError("expected identifier at line 3, column 14"));
}

TEST_F(ParserTest, VarDeclMissingColonAndType) {
  std::string_view src = R"(
    fun main(): Void {
      val m = 1
    }
  )";
  EXPECT_THAT(Parse(src), HoldsError("unexpected token at line 3, column 12"));
}

TEST_F(ParserTest, VarDeclMissingEqual) {
  std::string_view src = R"(
    fun main(): Void {
      val m: Int32 1
    }
  )";
  EXPECT_THAT(Parse(src), HoldsError("unexpected token at line 3, column 20"));
}

TEST_F(ParserTest, VarDeclMissingInit) {
  std::string_view src = R"(
    fun main(): Void {
      val m: Int32 =
    }
  )";
  EXPECT_THAT(Parse(src), HoldsError("unexpected token at line 4, column 5"));
}

TEST_F(ParserTest, VarAssignMissingValue) {
  std::string_view src = R"(
    fun foo(n: Int32): Void {
      n =
    }
  )";
  EXPECT_THAT(Parse(src), HoldsError("unexpected token at line 4, column 5"));
}

TEST_F(ParserTest, MissingEqualSign) {
  std::string_view src = R"(
    fun main(): Int32 {
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
    fun main(): Int32 {
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
    fun foo(): Void {
      do bar("foo)
    }
  )";
  EXPECT_THAT(Parse(src),
              HoldsError("incomplete string literal at line 3, column 14"));
}

TEST_F(ParserTest, MissingLoopOpenBrace) {
  std::string_view src = R"(
    fun main(): Int32 {
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
    fun main(): Int32 {
      loop {
        return 1
      return 2
    }
  )";
  EXPECT_THAT(Parse(src), HoldsError("unexpected token at line 7, column 3"));
}

TEST_F(ParserTest, TupleDefMissingCompVal) {
  std::string_view src = R"(
    : Type = (x: Int32, y: Int32)
  )";
  EXPECT_THAT(Parse(src), HoldsError("unexpected token at line 2, column 5"));
}

TEST_F(ParserTest, TupleDefMissingName) {
  std::string_view src = R"(
    comp val : Type = ()
  )";
  EXPECT_THAT(Parse(src),
              HoldsError("expected identifier at line 2, column 14"));
}

TEST_F(ParserTest, TupleDefMissingColon) {
  std::string_view src = R"(
    comp val Foo Type = ()
  )";
  EXPECT_THAT(Parse(src), HoldsError("unexpected token at line 2, column 18"));
}

TEST_F(ParserTest, TupleDefMissingType) {
  std::string_view src = R"(
    comp val Foo: = ()
  )";
  EXPECT_THAT(Parse(src),
              HoldsError("expected 'Type' keyword at line 2, column 19"));
}

TEST_F(ParserTest, TupleDefMissingEqual) {
  std::string_view src = R"(
    comp val Foo: Type ()
  )";
  EXPECT_THAT(Parse(src), HoldsError("unexpected token at line 2, column 24"));
}

TEST_F(ParserTest, TupleDefMissingOpeningParen) {
  std::string_view src = R"(
    comp val Foo: Type = )
  )";
  EXPECT_THAT(Parse(src), HoldsError("unexpected token at line 2, column 26"));
}

TEST_F(ParserTest, TupleDefMissingParamName) {
  std::string_view src = R"(
    comp val Foo: Type = (: Int32)
  )";
  EXPECT_THAT(Parse(src), HoldsError("expected closing parenthesis or "
                                     "parameter at line 2, column 27"));
}

TEST_F(ParserTest, TupleDefMissingParamColon) {
  std::string_view src = R"(
    comp val Foo: Type = (x Int32)
  )";
  EXPECT_THAT(Parse(src), HoldsError("unexpected token at line 2, column 28"));
}

TEST_F(ParserTest, TupleDefMissingParamType) {
  std::string_view src = R"(
    comp val Foo: Type = (x:)
  )";
  EXPECT_THAT(Parse(src),
              HoldsError("expected identifier at line 2, column 29"));
}

TEST_F(ParserTest, TupleDefMissingParamColonAndType) {
  std::string_view src = R"(
    comp val Id: Type = (x)
  )";
  EXPECT_THAT(Parse(src), HoldsError("unexpected token at line 2, column 27"));
}

TEST_F(ParserTest, TupleDefMissingNextParam) {
  std::string_view src = R"(
    comp val Foo: Type = (x: Int32,)
  )";
  EXPECT_THAT(Parse(src), HoldsError("expected closing parenthesis or "
                                     "parameter at line 2, column 36"));
}

TEST_F(ParserTest, TupleDefMissingClosingParen) {
  std::string_view src = R"(
    comp val Foo: Type = (
  )";
  EXPECT_THAT(Parse(src), HoldsError("expected closing parenthesis or "
                                     "parameter at line 2, column 27"));
}

}  // namespace
}  // namespace lucid
