#include "lucid/syntax/type.h"

#include <expected>

#include "lucid/core/testing/testing.h"
#include "lucid/syntax/ast.h"
#include "lucid/syntax/ast_fixture.h"

namespace lucid {
namespace {

class InferExprTypesTest : public Test, public AstFixture {
 protected:
  std::expected<void, TypeError> InferExprTypes(FuncDefStmt& stmt) {
    return ::lucid::InferExprTypes(syn_ctx_, stmt);
  }
};

TEST(InferExprTypesTest, ReturnValueFromResultType) {
  auto func = FuncDefStmt{
      .name = I("foo"),
      .result_type = T("Int32"),
      .stmts = StmtListOf({
          S(ReturnStmt{
              .value = E(IntLitExpr{
                  .value = 21,
              }),
          }),
      }),
  };

  EXPECT_TRUE(InferExprTypes(func).has_value());
  EXPECT_THAT(func,
              Truly(MatchesFuncDefStmt({
                  .name = I("foo"),
                  .result_type = MatchesBasicType({.name = I("Int32")}),
                  .body = {{
                      MatchesReturnStmt({
                          .value = MatchesIntLitExpr({
                              .type = MatchesBasicType({.name = I("Int32")}),
                              .value = 21,
                          }),
                      }),
                  }},
              })));
}

TEST(InferExprTypesTest, InitExprFromVarDeclType) {
  auto func = FuncDefStmt{
      .name = I("foo"),
      .result_type = T("Void"),
      .stmts = StmtListOf({
          S(VarDeclStmt{
              .name = I("x"),
              .type_constraint = T("Int64"),
              .init = E(IntLitExpr{
                  .value = 21,
              }),
          }),
      }),
  };

  EXPECT_TRUE(InferExprTypes(func).has_value());
  EXPECT_THAT(
      func,
      Truly(MatchesFuncDefStmt({
          .name = I("foo"),
          .result_type = MatchesBasicType({.name = I("Void")}),
          .body = {{
              MatchesVarDeclStmt({
                  .name = I("x"),
                  .type_constraint = MatchesBasicType({.name = I("Int64")}),
                  .init = MatchesIntLitExpr({
                      .type = MatchesBasicType({.name = I("Int64")}),
                      .value = 21,
                  }),
              }),
          }},
      })));
}

TEST(InferExprTypesTest, AssignedExprFromVarType) {
  auto func = FuncDefStmt{
      .name = I("foo"),
      .params = ParamListOf({
          P(FuncParam{
              .name = I("x"),
              .type_constraint = T("Int64"),
          }),
      }),
      .result_type = T("Void"),
      .stmts = StmtListOf({
          S(VarAssignStmt{
              .name = I("x"),
              .expr = E(IntLitExpr{
                  .value = 21,
              }),
          }),
      }),
  };

  EXPECT_TRUE(InferExprTypes(func).has_value());
  EXPECT_THAT(
      func,
      Truly(MatchesFuncDefStmt({
          .name = I("foo"),
          .params =
              {
                  MatchesFuncParam({
                      .name = I("x"),
                      .type_constraint = MatchesBasicType({.name = I("Int64")}),
                  }),
              },
          .result_type = MatchesBasicType({.name = I("Void")}),
          .body = {{
              MatchesVarAssignStmt({
                  .name = I("x"),
                  .expr = MatchesIntLitExpr({
                      .type = MatchesBasicType({.name = I("Int64")}),
                      .value = 21,
                  }),
              }),
          }},
      })));
}

TEST(InferExprTypesTest, IfStmtCond) {
  auto func = FuncDefStmt{
      .name = I("fact"),
      .params = ParamListOf({
          P(FuncParam{
              .name = I("n"),
              .type_constraint = T("Int32"),
          }),
      }),
      .result_type = T("Void"),
      .stmts = StmtListOf({
          S(IfStmt{
              .cond = E(BinaryOpExpr{
                  .op = BinaryOp::Eq,
                  .lhs = E(IdentExpr{.name = I("n")}),
                  .rhs = E(IntLitExpr{.value = 1}),
              }),

          }),
      }),
  };

  EXPECT_TRUE(InferExprTypes(func).has_value());
  EXPECT_THAT(
      func,
      Truly(MatchesFuncDefStmt({
          .name = I("fact"),
          .params =
              {
                  MatchesFuncParam({
                      .name = I("n"),
                      .type_constraint = MatchesBasicType({.name = I("Int32")}),
                  }),
              },
          .result_type = MatchesBasicType({.name = I("Void")}),
          .body = {{{
              MatchesIfStmt({
                  .cond = MatchesBinaryOpExpr({
                      .type = MatchesBasicType({.name = I("Bool")}),
                      .op = BinaryOp::Eq,
                      .lhs = MatchesAnyExpr(),
                      .rhs = MatchesAnyExpr(),
                  }),
              }),
          }}},
      })));
}

TEST(InferExprTypesTest, ThroughAssignedBinaryOpExprFromVarType) {
  auto func = FuncDefStmt{
      .name = I("foo"),
      .result_type = T("Void"),
      .stmts = StmtListOf({
          S(VarDeclStmt{
              .name = I("x"),
              .type_constraint = T("Int64"),
              .init = E(BinaryOpExpr{
                  .op = BinaryOp::Add,
                  .lhs = E(IntLitExpr{
                      .value = 2,
                  }),
                  .rhs = E(IntLitExpr{
                      .value = 3,
                  }),
              }),
          }),
      }),
  };

  EXPECT_TRUE(InferExprTypes(func).has_value());
  EXPECT_THAT(
      func,
      Truly(MatchesFuncDefStmt({
          .name = I("foo"),
          .result_type = MatchesBasicType({.name = I("Void")}),
          .body = {{
              MatchesVarDeclStmt({
                  .name = I("x"),
                  .type_constraint = MatchesBasicType({.name = I("Int64")}),
                  .init = MatchesBinaryOpExpr({
                      .type = MatchesBasicType({.name = I("Int64")}),
                      .op = BinaryOp::Add,
                      .lhs = MatchesIntLitExpr({
                          .type = MatchesBasicType({.name = I("Int64")}),
                          .value = 2,
                      }),
                      .rhs = MatchesIntLitExpr({
                          .type = MatchesBasicType({.name = I("Int64")}),
                          .value = 3,
                      }),
                  }),
              }),
          }},
      })));
}

TEST(InferExprTypesTest, FuncArgFromParamType) {
  auto id_func = FuncDefStmt{
      .name = I("id"),
      .params = ParamListOf({
          P(FuncParam{
              .name = I("x"),
              .type_constraint = T("Int32"),
          }),
      }),
      .result_type = T("Int32"),
  };
  syn_ctx_.AddFuncDef(id_func);

  auto func = FuncDefStmt{
      .name = I("foo"),
      .result_type = T("Int32"),
      .stmts = StmtListOf({
          S(ReturnStmt{
              .value = E(FuncCallExpr{
                  .func_name = I("id"),
                  .args = ExprListOf({
                      E(IntLitExpr{
                          .value = 21,
                      }),
                  }),
              }),
          }),
      }),
  };

  EXPECT_TRUE(InferExprTypes(func).has_value());
  EXPECT_THAT(
      func, Truly(
                MatchesFuncDefStmt({
                    .name = I("foo"),
                    .result_type = MatchesBasicType({.name = I("Int32")}),
                    .body = {{
                        MatchesReturnStmt({
                            .value =
                                MatchesFuncCallExpr({.type = MatchesBasicType(
                                                         {.name = I("Int32")}),
                                                     .func_name = I("id"),
                                                     .args =
                                                         {
                                                             MatchesIntLitExpr(
                                                                 {
                                                                     .type = MatchesBasicType(
                                                                         {.name =
                                                                              I("Int32")}),
                                                                     .value = 21,
                                                                 }),
                                                         }}),
                        }),
                    }},
                })));
}

TEST(InferExprTypesTest, UnconstrainedIntLit) {
  auto func = FuncDefStmt{
      .name = I("foo"),
      .result_type = T("Void"),
      .stmts = StmtListOf({
          S(IfStmt{
              .cond = E(BinaryOpExpr{
                  .op = BinaryOp::Lt,
                  .lhs = E(IntLitExpr{
                      .value = 2,
                  }),
                  .rhs = E(IntLitExpr{
                      .value = 3,
                  }),
              }),
          }),
      }),
  };

  EXPECT_TRUE(InferExprTypes(func).has_value());
  EXPECT_THAT(
      func, Truly(MatchesFuncDefStmt({
                .name = I("foo"),
                .result_type = MatchesBasicType({.name = I("Void")}),
                .body = {{
                    MatchesIfStmt({
                        .cond = MatchesBinaryOpExpr({
                            .type = MatchesBasicType({.name = I("Bool")}),
                            .op = BinaryOp::Lt,
                            .lhs = MatchesIntLitExpr({
                                .type = MatchesBasicType({.name = I("Int32")}),
                                .value = 2,
                            }),
                            .rhs = MatchesIntLitExpr({
                                .type = MatchesBasicType({.name = I("Int32")}),
                                .value = 3,
                            }),
                        }),
                    }),
                }},
            })));
}

TEST(InferExprTypesTest, NestedIntLitOperation) {
  auto func = FuncDefStmt{
      .name = I("foo"),
      .result_type = T("Int32"),
      .stmts = StmtListOf({
          S(ReturnStmt{
              .value = E(BinaryOpExpr{
                  .op = BinaryOp::Add,
                  .lhs = E(IntLitExpr{
                      .value = 1,
                  }),
                  .rhs = E(BinaryOpExpr{
                      .op = BinaryOp::Mul,
                      .lhs = E(IntLitExpr{
                          .value = 2,
                      }),
                      .rhs = E(IntLitExpr{
                          .value = 3,
                      }),
                  }),
              }),
          }),
      }),
  };

  EXPECT_TRUE(InferExprTypes(func).has_value());
  EXPECT_THAT(
      func,
      Truly(MatchesFuncDefStmt({
          .name = I("foo"),
          .result_type = MatchesBasicType({.name = I("Int32")}),
          .body = {{
              MatchesReturnStmt({
                  .value = MatchesBinaryOpExpr({
                      .type = MatchesBasicType({.name = I("Int32")}),
                      .op = BinaryOp::Add,
                      .lhs = MatchesIntLitExpr({
                          .type = MatchesBasicType({.name = I("Int32")}),
                          .value = 1,
                      }),
                      // The operation standing under another one is the one
                      // that had no type: what reached it was written over.
                      .rhs = MatchesBinaryOpExpr({
                          .type = MatchesBasicType({.name = I("Int32")}),
                          .op = BinaryOp::Mul,
                          .lhs = MatchesIntLitExpr({
                              .type = MatchesBasicType({.name = I("Int32")}),
                              .value = 2,
                          }),
                          .rhs = MatchesIntLitExpr({
                              .type = MatchesBasicType({.name = I("Int32")}),
                              .value = 3,
                          }),
                      }),
                  }),
              }),
          }},
      })));
}

TEST(InferExprTypesTest, ArrayIndex) {
  auto func = FuncDefStmt{.name = I("foo"),
                          .result_type = T("Int32"),
                          .stmts = StmtListOf({
                              S(VarDeclStmt{
                                  .name = I("a"),
                                  .type_constraint = T(T("Int32"), 10),
                              }),
                              S(ReturnStmt{
                                  .value = E(IndexExpr{
                                      .base = E(IdentExpr{.name = I("a")}),
                                      .index = E(IntLitExpr{.value = 2}),
                                  }),
                              }),
                          })};

  EXPECT_TRUE(InferExprTypes(func).has_value());
  EXPECT_THAT(
      func, Truly(MatchesFuncDefStmt({
                .name = I("foo"),
                .result_type = MatchesBasicType({.name = I("Int32")}),
                .body = {{
                    MatchesVarDeclStmt({
                        .name = I("a"),
                        .type_constraint = MatchesArrayType({
                            .element_type_constraint =
                                MatchesBasicType({.name = I("Int32")}),
                            .size = {.value = 10},
                        }),
                    }),
                    MatchesReturnStmt({
                        .value = MatchesIndexExpr({
                            .type = MatchesBasicType({.name = I("Int32")}),
                            .base = MatchesIdentExpr({
                                .type = MatchesArrayType({
                                    .element_type_constraint =
                                        MatchesBasicType({.name = I("Int32")}),
                                    .size = {.value = 10},
                                }),
                                .name = I("a"),
                            }),
                            .index = MatchesIntLitExpr({
                                .type = MatchesBasicType({.name = I("Int32")}),
                                .value = 2,
                            }),
                        }),
                    }),
                }},
            })));
}

TEST(InferExprTypesTest, ErrorBoolLitAsInt32) {
  auto func = FuncDefStmt{
      .name = I("foo"),
      .result_type = T("Void"),
      .stmts = StmtListOf({
          S(VarDeclStmt{
              .name = I("x"),
              .type_constraint = T("Int32"),
              .init = E(BoolLitExpr{
                  .value = true,
              }),
          }),
      }),
  };

  auto res = InferExprTypes(func);
  ASSERT_TRUE(!res.has_value());
  EXPECT_EQ(res.error(), TypeError("expected type Int32"));
}

TEST(InferExprTypesTest, ErrorInt64FromInt32) {
  auto func = FuncDefStmt{
      .name = I("foo"),
      .result_type = T("Void"),
      .stmts = StmtListOf({
          S(VarDeclStmt{
              .name = I("x"),
              .type_constraint = T("Int32"),
              .init = E(IntLitExpr{
                  .value = 2,
              }),
          }),
          S(VarDeclStmt{
              .name = I("y"),
              .type_constraint = T("Int64"),
              .init = E(IdentExpr{
                  .name = I("x"),
              }),
          }),
      }),
  };

  auto res = InferExprTypes(func);
  ASSERT_TRUE(!res.has_value());
  EXPECT_EQ(res.error(), TypeError("expected type Int64"));
}

}  // namespace
}  // namespace lucid
