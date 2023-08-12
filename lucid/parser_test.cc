#include "lucid/parser.h"

#include <functional>
#include <string>
#include <string_view>
#include <variant>
#include <vector>

#include "gmock/gmock.h"
#include "gtest/gtest.h"
#include "lucid/arena.h"
#include "lucid/ast.h"
#include "lucid/lexer.h"

MATCHER_P(HoldsStmt, match_stmt, "") {
  auto* stmt = std::get_if<lucid::StmtRef>(&arg);
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

struct FuncParamPattern {
  std::string_view name;
  std::string_view type;
};

struct CompoundStmtPattern {
  std::vector<std::function<bool(StmtRef)>> statements;
};

struct FuncDefStmtPattern {
  std::string_view name;
  std::vector<FuncParamPattern> parameters;
  std::string_view result_type;
  CompoundStmtPattern body;
};

struct ReturnStmtPattern {
  std::function<bool(StmtRef)> value;
};

struct IntLitExprPattern {
  std::string_view value;
};

struct BinaryOpExprPattern {
  BinaryOp op;
  std::function<bool(ExprRef)> lhs;
  std::function<bool(ExprRef)> rhs;
};

class ParserTest : public testing::Test {
 protected:
  std::variant<StmtRef, std::string> Parse(std::string_view code) {
    auto res = Parser(arena_, code, Lexer(code)).ParseFuncDef();
    if (auto* ref = std::get_if<StmtRef>(&res)) return *ref;
    return std::get<ParserError>(res).ToString();
  }

  std::function<bool(StmtRef)> MatchesFuncDefStmt(FuncDefStmtPattern pattern) {
    return [this, pattern](StmtRef ref) {
      const Stmt& stmt = arena_.get(ref);
      auto* func_def_stmt = std::get_if<FuncDefStmt>(&stmt);
      if (func_def_stmt == nullptr) return false;

      if (func_def_stmt->name != pattern.name) return false;

      if (func_def_stmt->parameters.size() != pattern.parameters.size()) {
        return false;
      }
      for (int i = 0; i < func_def_stmt->parameters.size(); ++i) {
        if (pattern.parameters[i].name != func_def_stmt->parameters[i].name) {
          return false;
        }
        if (pattern.parameters[i].type != func_def_stmt->parameters[i].type) {
          return false;
        }
      }

      if (func_def_stmt->result_type != pattern.result_type) return false;

      if (func_def_stmt->body.statements.size() !=
          pattern.body.statements.size()) {
        return false;
      }
      for (int i = 0; i < func_def_stmt->body.statements.size(); ++i) {
        if (!pattern.body.statements[i](func_def_stmt->body.statements[i]))
          return false;
      }

      return true;
    };
  }

  std::function<bool(StmtRef)> MatchesReturnStmt(ReturnStmtPattern pattern) {
    return [this, pattern](StmtRef ref) {
      const Stmt& stmt = arena_.get(ref);
      auto* return_stmt = std::get_if<ReturnStmt>(&stmt);
      if (return_stmt == nullptr) return false;
      return pattern.value(return_stmt->value);
    };
  }

  std::function<bool(StmtRef)> MatchesIntLitExpr(IntLitExprPattern pattern) {
    return [this, pattern](StmtRef ref) {
      const Stmt& stmt = arena_.get(ref);

      auto* expr = std::get_if<Expr>(&stmt);
      if (expr == nullptr) return false;

      auto* int_lit_expr = std::get_if<IntLitExpr>(expr);
      if (int_lit_expr == nullptr) return false;
      return int_lit_expr->value == pattern.value;
    };
  }

  std::function<bool(StmtRef)> MatchesBinaryOpExpr(
      BinaryOpExprPattern pattern) {
    return [this, pattern](StmtRef ref) {
      const Stmt& stmt = arena_.get(ref);

      auto* expr = std::get_if<Expr>(&stmt);
      if (expr == nullptr) return false;

      auto* binary_op_expr = std::get_if<BinaryOpExpr>(expr);
      if (binary_op_expr == nullptr) return false;

      if (binary_op_expr->op != pattern.op) return false;
      if (!pattern.lhs(binary_op_expr->lhs)) return false;
      if (!pattern.rhs(binary_op_expr->rhs)) return false;

      return true;
    };
  }

  std::function<bool(StmtRef)> MatchesAnyExpr() {
    return [](StmtRef) { return true; };
  }

 private:
  Arena<Stmt> arena_;
};

TEST_F(ParserTest, EmptyFuncDef) {
  std::string_view src = R"(
    let main = () -> Void {
    }
  )";
  EXPECT_THAT(Parse(src), HoldsStmt(MatchesFuncDefStmt({
                              .name = "main",
                              .result_type = "Void",
                          })));
}

TEST_F(ParserTest, ReturnIntLit) {
  std::string_view src = R"(
    let main = () -> Int {
      return 0
    }
  )";
  EXPECT_THAT(Parse(src), HoldsStmt(MatchesFuncDefStmt({
                              .name = "main",
                              .result_type = "Int",
                              .body =
                                  {
                                      .statements =
                                          {
                                              MatchesReturnStmt({
                                                  .value = MatchesIntLitExpr({
                                                      .value = "0",
                                                  }),
                                              }),
                                          },
                                  },
                          })));
}

TEST_F(ParserTest, ReturnAddBinaryOpExpr) {
  std::string_view src = R"(
    let main = () -> Int {
      return 3 + 2
    }
  )";
  EXPECT_THAT(Parse(src), HoldsStmt(
                              MatchesFuncDefStmt(
                                  {
                                      .name = "main",
                                      .result_type = "Int",
                                      .body =
                                          {
                                              .statements =
                                                  {
                                                      MatchesReturnStmt(
                                                          {
                                                              .value =
                                                                  MatchesBinaryOpExpr(
                                                                      {
                                                                          .op =
                                                                              BinaryOp::Add,
                                                                          .lhs =
                                                                              MatchesIntLitExpr(
                                                                                  {
                                                                                      .value =
                                                                                          "3",
                                                                                  }),
                                                                          .rhs =
                                                                              MatchesIntLitExpr(
                                                                                  {
                                                                                      .value =
                                                                                          "2",
                                                                                  }),
                                                                      }),
                                                          }),
                                                  },
                                          },
                                  })));
}

TEST_F(ParserTest, ReturnSubBinaryOpExpr) {
  std::string_view src = R"(
    let main = () -> Int {
      return 3 - 2
    }
  )";
  EXPECT_THAT(Parse(src), HoldsStmt(
                              MatchesFuncDefStmt(
                                  {
                                      .name = "main",
                                      .result_type = "Int",
                                      .body =
                                          {
                                              .statements =
                                                  {
                                                      MatchesReturnStmt(
                                                          {
                                                              .value =
                                                                  MatchesBinaryOpExpr(
                                                                      {
                                                                          .op =
                                                                              BinaryOp::Sub,
                                                                          .lhs =
                                                                              MatchesIntLitExpr(
                                                                                  {
                                                                                      .value =
                                                                                          "3",
                                                                                  }),
                                                                          .rhs =
                                                                              MatchesIntLitExpr(
                                                                                  {
                                                                                      .value =
                                                                                          "2",
                                                                                  }),
                                                                      }),
                                                          }),
                                                  },
                                          },
                                  })));
}

TEST_F(ParserTest, ReturnMulBinaryOpExpr) {
  std::string_view src = R"(
    let main = () -> Int {
      return 3 * 2
    }
  )";
  EXPECT_THAT(Parse(src), HoldsStmt(
                              MatchesFuncDefStmt(
                                  {
                                      .name = "main",
                                      .result_type = "Int",
                                      .body =
                                          {
                                              .statements =
                                                  {
                                                      MatchesReturnStmt(
                                                          {
                                                              .value =
                                                                  MatchesBinaryOpExpr(
                                                                      {
                                                                          .op =
                                                                              BinaryOp::Mul,
                                                                          .lhs =
                                                                              MatchesIntLitExpr(
                                                                                  {
                                                                                      .value =
                                                                                          "3",
                                                                                  }),
                                                                          .rhs =
                                                                              MatchesIntLitExpr(
                                                                                  {
                                                                                      .value =
                                                                                          "2",
                                                                                  }),
                                                                      }),
                                                          }),
                                                  },
                                          },
                                  })));
}

TEST_F(ParserTest, ReturnDivBinaryOpExpr) {
  std::string_view src = R"(
    let main = () -> Int {
      return 3 / 2
    }
  )";
  EXPECT_THAT(Parse(src), HoldsStmt(
                              MatchesFuncDefStmt(
                                  {
                                      .name = "main",
                                      .result_type = "Int",
                                      .body =
                                          {
                                              .statements =
                                                  {
                                                      MatchesReturnStmt(
                                                          {
                                                              .value =
                                                                  MatchesBinaryOpExpr(
                                                                      {
                                                                          .op =
                                                                              BinaryOp::Div,
                                                                          .lhs =
                                                                              MatchesIntLitExpr(
                                                                                  {
                                                                                      .value =
                                                                                          "3",
                                                                                  }),
                                                                          .rhs =
                                                                              MatchesIntLitExpr(
                                                                                  {
                                                                                      .value =
                                                                                          "2",
                                                                                  }),
                                                                      }),
                                                          }),
                                                  },
                                          },
                                  })));
}

TEST_F(ParserTest, SingleParam) {
  std::string_view src = R"(
    let id = (x: Int) -> Void {
    }
  )";
  EXPECT_THAT(Parse(src), HoldsStmt(MatchesFuncDefStmt({
                              .name = "id",
                              .parameters =
                                  {
                                      {.name = "x", .type = "Int"},
                                  },
                              .result_type = "Void",
                          })));
}

TEST_F(ParserTest, MultipleParams) {
  std::string_view src = R"(
    let foo = (a: Int, b: Double, c: Bool) -> Void {
    }
  )";
  EXPECT_THAT(Parse(src), HoldsStmt(MatchesFuncDefStmt({
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

TEST_F(ParserTest, FuncDefMissingLet) {
  std::string_view src = R"(
    = () -> Void {
    }
  )";
  EXPECT_THAT(
      Parse(src),
      HoldsError("parse error: expected `let` keyword at line 2, column 5"));
}

TEST_F(ParserTest, FuncDefMissingName) {
  std::string_view src = R"(
    let = () -> Void {
    }
  )";
  EXPECT_THAT(
      Parse(src),
      HoldsError("parse error: expected identifier at line 2, column 9"));
}

TEST_F(ParserTest, FuncDefMissingEqual) {
  std::string_view src = R"(
    let main () -> Void {
    }
  )";
  EXPECT_THAT(Parse(src),
              HoldsError("parse error: unexpected token at line 2, column 14"));
}

TEST_F(ParserTest, FuncDefMissingOpeningParen) {
  std::string_view src = R"(
    let main = ) -> Void {
    }
  )";
  EXPECT_THAT(Parse(src),
              HoldsError("parse error: unexpected token at line 2, column 16"));
}

TEST_F(ParserTest, FuncDefMissingClosingParen) {
  std::string_view src = R"(
    let main = ( -> Void {
    }
  )";
  EXPECT_THAT(Parse(src),
              HoldsError("parse error: expected closing parenthesis or "
                         "parameter at line 2, column 18"));
}

TEST_F(ParserTest, FuncDefMissingResultArrowDash) {
  std::string_view src = R"(
    let main = () > Void {
    }
  )";
  EXPECT_THAT(Parse(src),
              HoldsError("parse error: unexpected token at line 2, column 19"));
}

TEST_F(ParserTest, FuncDefMissingResultArrowHead) {
  std::string_view src = R"(
    let main = () - Void {
    }
  )";
  EXPECT_THAT(Parse(src),
              HoldsError("parse error: unexpected token at line 2, column 21"));
}

TEST_F(ParserTest, FuncDefMissingResultType) {
  std::string_view src = R"(
    let main = () -> {
    }
  )";
  EXPECT_THAT(
      Parse(src),
      HoldsError("parse error: expected identifier at line 2, column 22"));
}

TEST_F(ParserTest, FuncDefMissingOpenBrace) {
  std::string_view src = R"(
    let main = () -> Void
    }
  )";
  EXPECT_THAT(Parse(src),
              HoldsError("parse error: unexpected token at line 3, column 5"));
}

TEST_F(ParserTest, FuncDefMissingClosingBrace) {
  std::string_view src = R"(
    let main = () -> Void {
  )";
  EXPECT_THAT(Parse(src),
              HoldsError("parse error: unexpected token at line 3, column 3"));
}

}  // namespace
}  // namespace lucid
