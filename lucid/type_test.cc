#include "lucid/type.h"

#include <optional>

#include "gmock/gmock.h"
#include "gtest/gtest.h"
#include "lucid/ast.h"
#include "lucid/ast_fixture.h"
#include "lucid/result.h"

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
      .name = "foo",
      .result_type = T(BasicType{.name = "Int32"}),
      .stmts = StmtListOf({
          S(ReturnStmt{
              .value = E(IntLitExpr{
                  .value = "21",
              }),
          }),
      }),
  };

  EXPECT_TRUE(InferExprTypes(func).HasValue());
  EXPECT_THAT(func, HoldsFuncDef(MatchesFuncDefStmt({
                        .name = "foo",
                        .result_type = MatchesBasicType({.name = "Int32"}),
                        .body = {{
                            MatchesReturnStmt({
                                .value = MatchesIntLitExpr({
                                    .type = MatchesBasicType({.name = "Int32"}),
                                    .value = "21",
                                }),
                            }),
                        }},
                    })));
}

TEST_F(InferExprTypesTest, InitExprFromVarDeclType) {
  auto func = FuncDefStmt{
      .name = "foo",
      .result_type = T(BasicType{.name = "Void"}),
      .stmts = StmtListOf({
          S(VarDeclStmt{
              .name = "x",
              .type_constraint = T(BasicType{.name = "Int64"}),
              .init = E(IntLitExpr{
                  .value = "21",
              }),
          }),
      }),
  };

  EXPECT_TRUE(InferExprTypes(func).HasValue());
  EXPECT_THAT(
      func, HoldsFuncDef(MatchesFuncDefStmt({
                .name = "foo",
                .result_type = MatchesBasicType({.name = "Void"}),
                .body = {{
                    MatchesVarDeclStmt({
                        .name = "x",
                        .type_constraint = MatchesBasicType({.name = "Int64"}),
                        .init = MatchesIntLitExpr({
                            .type = MatchesBasicType({.name = "Int64"}),
                            .value = "21",
                        }),
                    }),
                }},
            })));
}

TEST_F(InferExprTypesTest, AssignedExprFromVarType) {
  auto func = FuncDefStmt{
      .name = "foo",
      .result_type = T(BasicType{.name = "Void"}),
      .params = ParamListOf({
          P(FuncParam{
              .name = I("x"),
              .type_constraint = T(BasicType{.name = "Int64"}),
          }),
      }),
      .stmts = StmtListOf({
          S(VarAssignStmt{
              .name = "x",
              .expr = E(IntLitExpr{
                  .value = "21",
              }),
          }),
      }),
  };

  EXPECT_TRUE(InferExprTypes(func).HasValue());
  EXPECT_THAT(
      func,
      HoldsFuncDef(MatchesFuncDefStmt({
          .name = "foo",
          .result_type = MatchesBasicType({.name = "Void"}),
          .params =
              {
                  MatchesFuncParam({
                      .name = I("x"),
                      .type_constraint = MatchesBasicType({.name = "Int64"}),
                  }),
              },
          .body = {{
              MatchesVarAssignStmt({
                  .name = "x",
                  .expr = MatchesIntLitExpr({
                      .type = MatchesBasicType({.name = "Int64"}),
                      .value = "21",
                  }),
              }),
          }},
      })));
}

TEST_F(InferExprTypesTest, IfStmtCond) {
  auto func = FuncDefStmt{
      .name = "fact",
      .result_type = T(BasicType{.name = "Void"}),
      .params = ParamListOf({
          P(FuncParam{
              .name = I("n"),
              .type_constraint = T(BasicType{.name = "Int32"}),
          }),
      }),
      .stmts = StmtListOf({
          S(IfStmt{
              .cond = E(BinaryOpExpr{
                  .op = BinaryOp::Eq,
                  .lhs = E(IdentExpr{.name = "n"}),
                  .rhs = E(IntLitExpr{.value = "1"}),
              }),

          }),
      }),
  };

  EXPECT_TRUE(InferExprTypes(func).HasValue());
  EXPECT_THAT(
      func,
      HoldsFuncDef(MatchesFuncDefStmt({
          .name = "fact",
          .result_type = MatchesBasicType({.name = "Void"}),
          .params =
              {
                  MatchesFuncParam({
                      .name = I("n"),
                      .type_constraint = MatchesBasicType({.name = "Int32"}),
                  }),
              },
          .body = {{{
              MatchesIfStmt({
                  .cond = MatchesBinaryOpExpr({
                      .type = MatchesBasicType({.name = "Bool"}),
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
      .name = "foo",
      .result_type = T(BasicType{.name = "Void"}),
      .stmts = StmtListOf({
          S(VarDeclStmt{
              .name = "x",
              .type_constraint = T(BasicType{.name = "Int64"}),
              .init = E(BinaryOpExpr{
                  .op = BinaryOp::Add,
                  .lhs = E(IntLitExpr{
                      .value = "2",
                  }),
                  .rhs = E(IntLitExpr{
                      .value = "3",
                  }),
              }),
          }),
      }),
  };

  EXPECT_TRUE(InferExprTypes(func).HasValue());
  EXPECT_THAT(
      func, HoldsFuncDef(MatchesFuncDefStmt({
                .name = "foo",
                .result_type = MatchesBasicType({.name = "Void"}),
                .body = {{
                    MatchesVarDeclStmt({
                        .name = "x",
                        .type_constraint = MatchesBasicType({.name = "Int64"}),
                        .init = MatchesBinaryOpExpr({
                            .op = BinaryOp::Add,
                            .type = MatchesBasicType({.name = "Int64"}),
                            .lhs = MatchesIntLitExpr({
                                .type = MatchesBasicType({.name = "Int64"}),
                                .value = "2",
                            }),
                            .rhs = MatchesIntLitExpr({
                                .type = MatchesBasicType({.name = "Int64"}),
                                .value = "3",
                            }),
                        }),
                    }),
                }},
            })));
}

TEST_F(InferExprTypesTest, FuncArgFromParamType) {
  auto id_func = FuncDefStmt{
      .name = "id",
      .result_type = T(BasicType{.name = "Int32"}),
      .params = ParamListOf({
          P(FuncParam{
              .name = I("x"),
              .type_constraint = T(BasicType{.name = "Int32"}),
          }),
      }),
  };

  auto func = FuncDefStmt{
      .name = "foo",
      .result_type = T(BasicType{.name = "Int32"}),
      .stmts = StmtListOf({
          S(ReturnStmt{
              .value = E(FuncCallExpr{
                  .func_name = "id",
                  .args = ExprListOf({
                      E(IntLitExpr{
                          .value = "21",
                      }),
                  }),
              }),
          }),
      }),
  };

  EXPECT_TRUE(InferExprTypes(func, {id_func}).HasValue());
  EXPECT_THAT(
      func,
      HoldsFuncDef(MatchesFuncDefStmt({
          .name = "foo",
          .result_type = MatchesBasicType({.name = "Int32"}),
          .body = {{
              MatchesReturnStmt({
                  .value = MatchesFuncCallExpr(
                      {.type = MatchesBasicType({.name = "Int32"}),
                       .func_name = "id",
                       .args =
                           {
                               MatchesIntLitExpr({
                                   .type = MatchesBasicType({.name = "Int32"}),
                                   .value = "21",
                               }),
                           }}),
              }),
          }},
      })));
}

TEST_F(InferExprTypesTest, UnconstrainedIntLit) {
  auto func = FuncDefStmt{
      .name = "foo",
      .result_type = T(BasicType{.name = "Void"}),
      .stmts = StmtListOf({
          S(IfStmt{
              .cond = E(BinaryOpExpr{
                  .op = BinaryOp::Lt,
                  .lhs = E(IntLitExpr{
                      .value = "2",
                  }),
                  .rhs = E(IntLitExpr{
                      .value = "3",
                  }),
              }),
          }),
      }),
  };

  EXPECT_TRUE(InferExprTypes(func).HasValue());
  EXPECT_THAT(func,
              HoldsFuncDef(MatchesFuncDefStmt({
                  .name = "foo",
                  .result_type = MatchesBasicType({.name = "Void"}),
                  .body = {{
                      MatchesIfStmt({
                          .cond = MatchesBinaryOpExpr({
                              .op = BinaryOp::Lt,
                              .type = MatchesBasicType({.name = "Bool"}),
                              .lhs = MatchesIntLitExpr({
                                  .type = MatchesBasicType({.name = "Int32"}),
                                  .value = "2",
                              }),
                              .rhs = MatchesIntLitExpr({
                                  .type = MatchesBasicType({.name = "Int32"}),
                                  .value = "3",
                              }),
                          }),
                      }),
                  }},
              })));
}

TEST_F(InferExprTypesTest, ArrayIndex) {
  auto func = FuncDefStmt{
      .name = "foo",
      .result_type = T(BasicType{.name = "Int32"}),
      .stmts = StmtListOf({
          S(VarDeclStmt{
              .name = "a",
              .type_constraint = T(ArrayType{
                  .element_type_constraint = T(BasicType{.name = "Int32"}),
                  .size = IntLitExpr{.value = "10"},
              }),
          }),
          S(ReturnStmt{
              .value = E(IndexExpr{
                  .base = E(IdentExpr{.name = "a"}),
                  .index = E(IntLitExpr{.value = "2"}),
              }),
          }),
      })};

  EXPECT_TRUE(InferExprTypes(func).HasValue());
  EXPECT_THAT(func,
              HoldsFuncDef(MatchesFuncDefStmt({
                  .name = "foo",
                  .result_type = MatchesBasicType({.name = "Int32"}),
                  .body = {{
                      MatchesVarDeclStmt({
                          .name = "a",
                          .type_constraint = MatchesArrayType({
                              .element_type_constraint =
                                  MatchesBasicType({.name = "Int32"}),
                              .size = {.value = "10"},
                          }),
                      }),
                      MatchesReturnStmt({
                          .value = MatchesIndexExpr({
                              .base = MatchesIdentExpr({
                                  .name = "a",
                                  .type = MatchesArrayType({
                                      .element_type_constraint =
                                          MatchesBasicType({.name = "Int32"}),
                                      .size = {.value = "10"},
                                  }),
                              }),
                              .index = MatchesIntLitExpr({
                                  .value = "2",
                                  .type = MatchesBasicType({.name = "Int32"}),
                              }),
                              .type = MatchesBasicType({.name = "Int32"}),
                          }),
                      }),
                  }},
              })));
}

TEST_F(InferExprTypesTest, ErrorBoolLitAsInt32) {
  auto func = FuncDefStmt{
      .name = "foo",
      .result_type = T(BasicType{.name = "Void"}),
      .stmts = StmtListOf({
          S(VarDeclStmt{
              .name = "x",
              .type_constraint = T(BasicType{.name = "Int32"}),
              .init = E(BoolLitExpr{
                  .value = "true",
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
      .name = "foo",
      .result_type = T(BasicType{.name = "Void"}),
      .stmts = StmtListOf({
          S(VarDeclStmt{
              .name = "x",
              .type_constraint = T(BasicType{.name = "Int32"}),
              .init = E(IntLitExpr{
                  .value = "2",
              }),
          }),
          S(VarDeclStmt{
              .name = "y",
              .type_constraint = T(BasicType{.name = "Int64"}),
              .init = E(IdentExpr{
                  .name = "x",
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
