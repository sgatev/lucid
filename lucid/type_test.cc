#include "lucid/type.h"

#include "gmock/gmock.h"
#include "gtest/gtest.h"
#include "lucid/core/functional/result.h"
#include "lucid/syntax/ast.h"
#include "lucid/syntax/ast_fixture.h"

namespace lucid {
namespace {

using ::testing::IsEmpty;
using ::testing::Pair;
using ::testing::UnorderedElementsAre;

MATCHER_P(HoldsFuncDef, match_stmt, "") { return match_stmt(arg); }

class InferExprTypesTest : public testing::Test, public AstFixture {
 protected:
  Result<void, TypeError> InferExprTypes(
      FuncDefStmt& stmt, const std::vector<FuncDefStmt>& func_defs = {}) {
    return ::lucid::InferExprTypes(ctx_, func_defs, stmt);
  }
};

TEST_F(InferExprTypesTest, ReturnValueFromResultType) {
  auto func = FuncDefStmt{
      .name = I("foo"),
      .result_type = T(BasicType{.name = I("Int32")}),
      .stmts = StmtListOf({
          S(ReturnStmt{
              .value = E(IntLitExpr{
                  .value = I("21"),
              }),
          }),
      }),
  };

  EXPECT_TRUE(InferExprTypes(func).HasValue());
  EXPECT_THAT(func,
              HoldsFuncDef(MatchesFuncDefStmt({
                  .name = I("foo"),
                  .result_type = MatchesBasicType({.name = I("Int32")}),
                  .body = {{
                      MatchesReturnStmt({
                          .value = MatchesIntLitExpr({
                              .type = MatchesBasicType({.name = I("Int32")}),
                              .value = I("21"),
                          }),
                      }),
                  }},
              })));
}

TEST_F(InferExprTypesTest, InitExprFromVarDeclType) {
  auto func = FuncDefStmt{
      .name = I("foo"),
      .result_type = T(BasicType{.name = I("Void")}),
      .stmts = StmtListOf({
          S(VarDeclStmt{
              .name = I("x"),
              .type_constraint = T(BasicType{.name = I("Int64")}),
              .init = E(IntLitExpr{
                  .value = I("21"),
              }),
          }),
      }),
  };

  EXPECT_TRUE(InferExprTypes(func).HasValue());
  EXPECT_THAT(
      func,
      HoldsFuncDef(MatchesFuncDefStmt({
          .name = I("foo"),
          .result_type = MatchesBasicType({.name = I("Void")}),
          .body = {{
              MatchesVarDeclStmt({
                  .name = I("x"),
                  .type_constraint = MatchesBasicType({.name = I("Int64")}),
                  .init = MatchesIntLitExpr({
                      .type = MatchesBasicType({.name = I("Int64")}),
                      .value = I("21"),
                  }),
              }),
          }},
      })));
}

TEST_F(InferExprTypesTest, AssignedExprFromVarType) {
  auto func = FuncDefStmt{
      .name = I("foo"),
      .result_type = T(BasicType{.name = I("Void")}),
      .params = ParamListOf({
          P(FuncParam{
              .name = I("x"),
              .type_constraint = T(BasicType{.name = I("Int64")}),
          }),
      }),
      .stmts = StmtListOf({
          S(VarAssignStmt{
              .name = I("x"),
              .expr = E(IntLitExpr{
                  .value = I("21"),
              }),
          }),
      }),
  };

  EXPECT_TRUE(InferExprTypes(func).HasValue());
  EXPECT_THAT(
      func,
      HoldsFuncDef(MatchesFuncDefStmt({
          .name = I("foo"),
          .result_type = MatchesBasicType({.name = I("Void")}),
          .params =
              {
                  MatchesFuncParam({
                      .name = I("x"),
                      .type_constraint = MatchesBasicType({.name = I("Int64")}),
                  }),
              },
          .body = {{
              MatchesVarAssignStmt({
                  .name = I("x"),
                  .expr = MatchesIntLitExpr({
                      .type = MatchesBasicType({.name = I("Int64")}),
                      .value = I("21"),
                  }),
              }),
          }},
      })));
}

TEST_F(InferExprTypesTest, IfStmtCond) {
  auto func = FuncDefStmt{
      .name = I("fact"),
      .result_type = T(BasicType{.name = I("Void")}),
      .params = ParamListOf({
          P(FuncParam{
              .name = I("n"),
              .type_constraint = T(BasicType{.name = I("Int32")}),
          }),
      }),
      .stmts = StmtListOf({
          S(IfStmt{
              .cond = E(BinaryOpExpr{
                  .op = BinaryOp::Eq,
                  .lhs = E(IdentExpr{.name = I("n")}),
                  .rhs = E(IntLitExpr{.value = I("1")}),
              }),

          }),
      }),
  };

  EXPECT_TRUE(InferExprTypes(func).HasValue());
  EXPECT_THAT(
      func,
      HoldsFuncDef(MatchesFuncDefStmt({
          .name = I("fact"),
          .result_type = MatchesBasicType({.name = I("Void")}),
          .params =
              {
                  MatchesFuncParam({
                      .name = I("n"),
                      .type_constraint = MatchesBasicType({.name = I("Int32")}),
                  }),
              },
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

TEST_F(InferExprTypesTest, ThroughAssignedBinaryOpExprFromVarType) {
  auto func = FuncDefStmt{
      .name = I("foo"),
      .result_type = T(BasicType{.name = I("Void")}),
      .stmts = StmtListOf({
          S(VarDeclStmt{
              .name = I("x"),
              .type_constraint = T(BasicType{.name = I("Int64")}),
              .init = E(BinaryOpExpr{
                  .op = BinaryOp::Add,
                  .lhs = E(IntLitExpr{
                      .value = I("2"),
                  }),
                  .rhs = E(IntLitExpr{
                      .value = I("3"),
                  }),
              }),
          }),
      }),
  };

  EXPECT_TRUE(InferExprTypes(func).HasValue());
  EXPECT_THAT(
      func,
      HoldsFuncDef(MatchesFuncDefStmt({
          .name = I("foo"),
          .result_type = MatchesBasicType({.name = I("Void")}),
          .body = {{
              MatchesVarDeclStmt({
                  .name = I("x"),
                  .type_constraint = MatchesBasicType({.name = I("Int64")}),
                  .init = MatchesBinaryOpExpr({
                      .op = BinaryOp::Add,
                      .type = MatchesBasicType({.name = I("Int64")}),
                      .lhs = MatchesIntLitExpr({
                          .type = MatchesBasicType({.name = I("Int64")}),
                          .value = I("2"),
                      }),
                      .rhs = MatchesIntLitExpr({
                          .type = MatchesBasicType({.name = I("Int64")}),
                          .value = I("3"),
                      }),
                  }),
              }),
          }},
      })));
}

TEST_F(InferExprTypesTest, FuncArgFromParamType) {
  auto id_func = FuncDefStmt{
      .name = I("id"),
      .result_type = T(BasicType{.name = I("Int32")}),
      .params = ParamListOf({
          P(FuncParam{
              .name = I("x"),
              .type_constraint = T(BasicType{.name = I("Int32")}),
          }),
      }),
  };

  auto func = FuncDefStmt{
      .name = I("foo"),
      .result_type = T(BasicType{.name = I("Int32")}),
      .stmts = StmtListOf({
          S(ReturnStmt{
              .value = E(FuncCallExpr{
                  .func_name = I("id"),
                  .args = ExprListOf({
                      E(IntLitExpr{
                          .value = I("21"),
                      }),
                  }),
              }),
          }),
      }),
  };

  EXPECT_TRUE(InferExprTypes(func, {id_func}).HasValue());
  EXPECT_THAT(
      func, HoldsFuncDef(
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
                                                                     .value = I("21"),
                                                                 }),
                                                         }}),
                        }),
                    }},
                })));
}

TEST_F(InferExprTypesTest, UnconstrainedIntLit) {
  auto func = FuncDefStmt{
      .name = I("foo"),
      .result_type = T(BasicType{.name = I("Void")}),
      .stmts = StmtListOf({
          S(IfStmt{
              .cond = E(BinaryOpExpr{
                  .op = BinaryOp::Lt,
                  .lhs = E(IntLitExpr{
                      .value = I("2"),
                  }),
                  .rhs = E(IntLitExpr{
                      .value = I("3"),
                  }),
              }),
          }),
      }),
  };

  EXPECT_TRUE(InferExprTypes(func).HasValue());
  EXPECT_THAT(
      func, HoldsFuncDef(MatchesFuncDefStmt({
                .name = I("foo"),
                .result_type = MatchesBasicType({.name = I("Void")}),
                .body = {{
                    MatchesIfStmt({
                        .cond = MatchesBinaryOpExpr({
                            .op = BinaryOp::Lt,
                            .type = MatchesBasicType({.name = I("Bool")}),
                            .lhs = MatchesIntLitExpr({
                                .type = MatchesBasicType({.name = I("Int32")}),
                                .value = I("2"),
                            }),
                            .rhs = MatchesIntLitExpr({
                                .type = MatchesBasicType({.name = I("Int32")}),
                                .value = I("3"),
                            }),
                        }),
                    }),
                }},
            })));
}

TEST_F(InferExprTypesTest, ArrayIndex) {
  auto func = FuncDefStmt{
      .name = I("foo"),
      .result_type = T(BasicType{.name = I("Int32")}),
      .stmts = StmtListOf({
          S(VarDeclStmt{
              .name = I("a"),
              .type_constraint = T(ArrayType{
                  .element_type_constraint = T(BasicType{.name = I("Int32")}),
                  .size = IntLitExpr{.value = I("10")},
              }),
          }),
          S(ReturnStmt{
              .value = E(IndexExpr{
                  .base = E(IdentExpr{.name = I("a")}),
                  .index = E(IntLitExpr{.value = I("2")}),
              }),
          }),
      })};

  EXPECT_TRUE(InferExprTypes(func).HasValue());
  EXPECT_THAT(
      func, HoldsFuncDef(MatchesFuncDefStmt({
                .name = I("foo"),
                .result_type = MatchesBasicType({.name = I("Int32")}),
                .body = {{
                    MatchesVarDeclStmt({
                        .name = I("a"),
                        .type_constraint = MatchesArrayType({
                            .element_type_constraint =
                                MatchesBasicType({.name = I("Int32")}),
                            .size = {.value = I("10")},
                        }),
                    }),
                    MatchesReturnStmt({
                        .value = MatchesIndexExpr({
                            .base = MatchesIdentExpr({
                                .name = I("a"),
                                .type = MatchesArrayType({
                                    .element_type_constraint =
                                        MatchesBasicType({.name = I("Int32")}),
                                    .size = {.value = I("10")},
                                }),
                            }),
                            .index = MatchesIntLitExpr({
                                .value = I("2"),
                                .type = MatchesBasicType({.name = I("Int32")}),
                            }),
                            .type = MatchesBasicType({.name = I("Int32")}),
                        }),
                    }),
                }},
            })));
}

TEST_F(InferExprTypesTest, ErrorBoolLitAsInt32) {
  auto func = FuncDefStmt{
      .name = I("foo"),
      .result_type = T(BasicType{.name = I("Void")}),
      .stmts = StmtListOf({
          S(VarDeclStmt{
              .name = I("x"),
              .type_constraint = T(BasicType{.name = I("Int32")}),
              .init = E(BoolLitExpr{
                  .value = I("true"),
              }),
          }),
      }),
  };

  auto res = InferExprTypes(func);
  ASSERT_TRUE(res.HasError());
  EXPECT_EQ(res.GetError(), TypeError("expected type Int32"));
}

TEST_F(InferExprTypesTest, ErrorInt64FromInt32) {
  auto func = FuncDefStmt{
      .name = I("foo"),
      .result_type = T(BasicType{.name = I("Void")}),
      .stmts = StmtListOf({
          S(VarDeclStmt{
              .name = I("x"),
              .type_constraint = T(BasicType{.name = I("Int32")}),
              .init = E(IntLitExpr{
                  .value = I("2"),
              }),
          }),
          S(VarDeclStmt{
              .name = I("y"),
              .type_constraint = T(BasicType{.name = I("Int64")}),
              .init = E(IdentExpr{
                  .name = I("x"),
              }),
          }),
      }),
  };

  auto res = InferExprTypes(func);
  ASSERT_TRUE(res.HasError());
  EXPECT_EQ(res.GetError(), TypeError("expected type Int64"));
}

}  // namespace
}  // namespace lucid
