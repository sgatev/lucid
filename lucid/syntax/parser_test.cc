#include "lucid/syntax/parser.h"

#include <expected>
#include <optional>
#include <sstream>
#include <string>
#include <string_view>
#include <utility>
#include <variant>

#include "lucid/core/testing/testing.h"
#include "lucid/syntax/ast.h"
#include "lucid/syntax/ast_fixture.h"
#include "lucid/syntax/lexer.h"

namespace lucid {
namespace {

using namespace std::string_literals;

template <typename M>
auto HoldsFuncDef(M matcher) {
  return Variant<Def>(Variant<FuncDefStmt>(Truly(std::move(matcher))));
}

template <typename M>
auto HoldsTypeDef(M matcher) {
  return Variant<Def>(Variant<TypeDefStmt>(Truly(std::move(matcher))));
}

auto HoldsError(const std::string& error) {
  return Variant<std::string>(Equals(error));
}

class ParserTest : public Test, public AstFixture {
 protected:
  std::variant<Def, std::string> Parse(std::string_view src) {
    std::string code_with_null(src);
    code_with_null.append("\0"s);

    std::expected<std::optional<Def>, ParserError> def_or_error =
        Parser(syn_ctx_, src, Lexer(code_with_null)).Parse();
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

TEST(ParserTest, EmptyFuncDefStmt) {
  std::string_view src = R"(
    fun main(): Void {
    }
  )";
  EXPECT_THAT(Parse(src),
              Variant<Def>(Variant<FuncDefStmt>(Truly(MatchesFuncDefStmt({
                  .name = I("main"),
                  .result_type = MatchesBasicType({.name = I("Void")}),
              })))));
}

TEST(ParserTest, ReturnIntLitExpr) {
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

TEST(ParserTest, CompFuncDef) {
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

TEST(ParserTest, Comment) {
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

TEST(ParserTest, ReturnAddExpr) {
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

TEST(ParserTest, ReturnSubExpr) {
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

TEST(ParserTest, ReturnMulExpr) {
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

TEST(ParserTest, ReturnDivExpr) {
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

TEST(ParserTest, ReturnModExpr) {
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

TEST(ParserTest, ReturnGtExpr) {
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

TEST(ParserTest, Precedence) {
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

TEST(ParserTest, SingleFuncParam) {
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

TEST(ParserTest, MultipleFuncParams) {
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

TEST(ParserTest, FuncCallExprIntLitArg) {
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

TEST(ParserTest, FuncCallExprStringLitArg) {
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

TEST(ParserTest, FuncCallExprNestedArg) {
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

TEST(ParserTest, ReturnFuncCallExpr) {
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

TEST(ParserTest, ReturnTrueBoolLit) {
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

TEST(ParserTest, ReturnFalseBoolLit) {
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

TEST(ParserTest, IfStmt) {
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

TEST(ParserTest, IfElseStmt) {
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

TEST(ParserTest, IfElseIfElseStmt) {
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

TEST(ParserTest, GtInts) {
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

TEST(ParserTest, LtInts) {
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

TEST(ParserTest, EqInts) {
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

TEST(ParserTest, NotEqInts) {
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

TEST(ParserTest, VarDecl) {
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

TEST(ParserTest, CompVarDecl) {
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

TEST(ParserTest, VarAssignment) {
  std::string_view src = R"(
    fun foo(n: Int32): Void {
      &n = 3
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

TEST(ParserTest, LoopAndBreakStmt) {
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

TEST(ParserTest, EqOverMod) {
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

TEST(ParserTest, NotEqOverAdd) {
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

TEST(ParserTest, GtOverMul) {
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

TEST(ParserTest, LtOverSub) {
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

TEST(ParserTest, ArrayParam) {
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

TEST(ParserTest, EmptyTuple) {
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

TEST(ParserTest, Tuple) {
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

TEST(ParserTest, FuncDefMissingLet) {
  std::string_view src = R"(
    = () Void {
    }
  )";
  EXPECT_THAT(Parse(src), HoldsError("unexpected token at line 2, column 5"));
}

TEST(ParserTest, FuncDefMissingName) {
  std::string_view src = R"(
    fun = () Void {
    }
  )";
  EXPECT_THAT(Parse(src),
              HoldsError("expected identifier at line 2, column 9"));
}

TEST(ParserTest, FuncDefMissingOpeningParen) {
  std::string_view src = R"(
    fun main ) Void {
    }
  )";
  EXPECT_THAT(Parse(src), HoldsError("unexpected token at line 2, column 14"));
}

TEST(ParserTest, FuncDefMissingParamName) {
  std::string_view src = R"(
    fun id(: Int32) Int32 {
      return x
    }
  )";
  EXPECT_THAT(Parse(src), HoldsError("expected closing parenthesis or "
                                     "parameter at line 2, column 12"));
}

TEST(ParserTest, FuncDefMissingParamColon) {
  std::string_view src = R"(
    fun id(x Int32) Int32 {
      return x
    }
  )";
  EXPECT_THAT(Parse(src), HoldsError("unexpected token at line 2, column 14"));
}

TEST(ParserTest, FuncDefMissingParamType) {
  std::string_view src = R"(
    fun id(x:) Int32 {
      return x
    }
  )";
  EXPECT_THAT(Parse(src),
              HoldsError("expected identifier at line 2, column 14"));
}

TEST(ParserTest, FuncDefMissingParamColonAndType) {
  std::string_view src = R"(
    fun id(x) Int32 {
      return x
    }
  )";
  EXPECT_THAT(Parse(src), HoldsError("unexpected token at line 2, column 13"));
}

TEST(ParserTest, FuncDefMissingNextParam) {
  std::string_view src = R"(
    fun id(x: Int32,) Int32 {
      return x
    }
  )";
  EXPECT_THAT(Parse(src), HoldsError("expected closing parenthesis or "
                                     "parameter at line 2, column 21"));
}

TEST(ParserTest, FuncDefMissingClosingParen) {
  std::string_view src = R"(
    fun main( Void {
    }
  )";
  EXPECT_THAT(Parse(src), HoldsError("unexpected token at line 2, column 20"));
}

TEST(ParserTest, FuncDefMissingResultColon) {
  std::string_view src = R"(
    fun main() Void {
    }
  )";
  EXPECT_THAT(Parse(src), HoldsError("unexpected token at line 2, column 16"));
}

TEST(ParserTest, FuncDefMissingResultType) {
  std::string_view src = R"(
    fun main(): {
    }
  )";
  EXPECT_THAT(Parse(src),
              HoldsError("expected identifier at line 2, column 17"));
}

TEST(ParserTest, FuncDefMissingOpenBrace) {
  std::string_view src = R"(
    fun main(): Void
    }
  )";
  EXPECT_THAT(Parse(src), HoldsError("unexpected token at line 3, column 5"));
}

TEST(ParserTest, FuncDefMissingClosingBrace) {
  std::string_view src = R"(
    fun main(): Void {
  )";
  EXPECT_THAT(Parse(src), HoldsError("unexpected token at line 3, column 3"));
}

TEST(ParserTest, ReturnMissingValue) {
  std::string_view src = R"(
    fun id(x: Int32): Int32 {
      return
    }
  )";
  EXPECT_THAT(Parse(src), HoldsError("unexpected token at line 4, column 5"));
}

TEST(ParserTest, VarDeclMissingLet) {
  std::string_view src = R"(
    fun main(): Void {
      m: Int32 = 1
    }
  )";
  EXPECT_THAT(Parse(src), HoldsError("unexpected token at line 3, column 8"));
}

TEST(ParserTest, VarDeclMissingName) {
  std::string_view src = R"(
    fun main(): Void {
      val : Int32 = 1
    }
  )";
  EXPECT_THAT(Parse(src),
              HoldsError("expected identifier at line 3, column 11"));
}

TEST(ParserTest, VarDeclMissingColon) {
  std::string_view src = R"(
    fun main(): Void {
      val m Int32 = 1
    }
  )";
  EXPECT_THAT(Parse(src), HoldsError("unexpected token at line 3, column 13"));
}

TEST(ParserTest, VarDeclMissingType) {
  std::string_view src = R"(
    fun main(): Void {
      val m: = 1
    }
  )";
  EXPECT_THAT(Parse(src),
              HoldsError("expected identifier at line 3, column 14"));
}

TEST(ParserTest, VarDeclMissingColonAndType) {
  std::string_view src = R"(
    fun main(): Void {
      val m = 1
    }
  )";
  EXPECT_THAT(Parse(src), HoldsError("unexpected token at line 3, column 13"));
}

TEST(ParserTest, VarDeclMissingEqual) {
  std::string_view src = R"(
    fun main(): Void {
      val m: Int32 1
    }
  )";
  EXPECT_THAT(Parse(src), HoldsError("unexpected token at line 3, column 20"));
}

TEST(ParserTest, VarDeclMissingInit) {
  std::string_view src = R"(
    fun main(): Void {
      val m: Int32 =
    }
  )";
  EXPECT_THAT(Parse(src), HoldsError("unexpected token at line 4, column 5"));
}

TEST(ParserTest, VarAssignMissingValue) {
  std::string_view src = R"(
    fun foo(n: Int32): Void {
      &n =
    }
  )";
  EXPECT_THAT(Parse(src), HoldsError("unexpected token at line 4, column 5"));
}

TEST(ParserTest, MissingEqualSign) {
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

TEST(ParserTest, SpaceBetweenEqualSigns) {
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

TEST(ParserTest, MissingStringClosingQuote) {
  std::string_view src = R"(
    fun foo(): Void {
      do bar("foo)
    }
  )";
  EXPECT_THAT(Parse(src),
              HoldsError("incomplete string literal at line 3, column 14"));
}

TEST(ParserTest, MissingLoopOpenBrace) {
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

TEST(ParserTest, MissingLoopCloseBrace) {
  std::string_view src = R"(
    fun main(): Int32 {
      loop {
        return 1
      return 2
    }
  )";
  EXPECT_THAT(Parse(src), HoldsError("unexpected token at line 7, column 3"));
}

TEST(ParserTest, TupleDefMissingCompVal) {
  std::string_view src = R"(
    : Type = (x: Int32, y: Int32)
  )";
  EXPECT_THAT(Parse(src), HoldsError("unexpected token at line 2, column 5"));
}

TEST(ParserTest, TupleDefMissingName) {
  std::string_view src = R"(
    comp val : Type = ()
  )";
  EXPECT_THAT(Parse(src),
              HoldsError("expected identifier at line 2, column 14"));
}

TEST(ParserTest, TupleDefMissingColon) {
  std::string_view src = R"(
    comp val Foo Type = ()
  )";
  EXPECT_THAT(Parse(src), HoldsError("unexpected token at line 2, column 18"));
}

TEST(ParserTest, TupleDefMissingType) {
  std::string_view src = R"(
    comp val Foo: = ()
  )";
  EXPECT_THAT(Parse(src),
              HoldsError("expected 'Type' keyword at line 2, column 19"));
}

TEST(ParserTest, TupleDefMissingEqual) {
  std::string_view src = R"(
    comp val Foo: Type ()
  )";
  EXPECT_THAT(Parse(src), HoldsError("unexpected token at line 2, column 24"));
}

TEST(ParserTest, TupleDefMissingOpeningParen) {
  std::string_view src = R"(
    comp val Foo: Type = )
  )";
  EXPECT_THAT(Parse(src), HoldsError("unexpected token at line 2, column 26"));
}

TEST(ParserTest, TupleDefMissingParamName) {
  std::string_view src = R"(
    comp val Foo: Type = (: Int32)
  )";
  EXPECT_THAT(Parse(src), HoldsError("expected closing parenthesis or "
                                     "parameter at line 2, column 27"));
}

TEST(ParserTest, TupleDefMissingParamColon) {
  std::string_view src = R"(
    comp val Foo: Type = (x Int32)
  )";
  EXPECT_THAT(Parse(src), HoldsError("unexpected token at line 2, column 29"));
}

TEST(ParserTest, TupleDefMissingParamType) {
  std::string_view src = R"(
    comp val Foo: Type = (x:)
  )";
  EXPECT_THAT(Parse(src),
              HoldsError("expected identifier at line 2, column 29"));
}

TEST(ParserTest, TupleDefMissingParamColonAndType) {
  std::string_view src = R"(
    comp val Id: Type = (x)
  )";
  EXPECT_THAT(Parse(src), HoldsError("unexpected token at line 2, column 27"));
}

TEST(ParserTest, TupleDefMissingNextParam) {
  std::string_view src = R"(
    comp val Foo: Type = (x: Int32,)
  )";
  EXPECT_THAT(Parse(src), HoldsError("expected closing parenthesis or "
                                     "parameter at line 2, column 36"));
}

TEST(ParserTest, TupleDefMissingClosingParen) {
  std::string_view src = R"(
    comp val Foo: Type = (
  )";
  EXPECT_THAT(Parse(src), HoldsError("expected closing parenthesis or "
                                     "parameter at line 3, column 3"));
}

}  // namespace
}  // namespace lucid
