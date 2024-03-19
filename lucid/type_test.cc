#include "lucid/type.h"

#include <optional>

#include "gmock/gmock.h"
#include "gtest/gtest.h"
#include "lucid/arena.h"
#include "lucid/ast.h"
#include "lucid/ast_fixture.h"

namespace lucid {
namespace {

using ::testing::IsEmpty;
using ::testing::Pair;
using ::testing::UnorderedElementsAre;

class ExtractFuncTypesTest : public testing::Test, public AstFixture {
 protected:
  std::unordered_map<std::string_view, FuncType> ExtractFuncTypes(
      const std::vector<FuncDefStmt>& func_defs) {
    return ::lucid::ExtractFuncTypes(stmt_arena_, expr_arena_, type_arena_,
                                     func_defs);
  }
};

TEST_F(ExtractFuncTypesTest, NoFuncDefs) {
  EXPECT_THAT(ExtractFuncTypes({}), IsEmpty());
}

TEST_F(ExtractFuncTypesTest, MultipleFuncDefs) {
  const std::vector<FuncDefStmt> func_defs = {
      {
          .name = "id",
          .result_type = T(BasicType{.name = "Int32"}),
          .parameters =
              {
                  {.type = T(BasicType{.name = "Int32"})},
              },
      },
      {
          .name = "foo",
          .result_type = T(BasicType{.name = "Int64"}),
      },
  };

  EXPECT_THAT(
      ExtractFuncTypes(func_defs),
      UnorderedElementsAre(Pair("id",
                                FuncType{
                                    .result_type = "Int32",
                                    .parameters = func_defs[0].parameters,
                                }),
                           Pair("foo", FuncType{
                                           .result_type = "Int64",
                                       })));
}

MATCHER_P(HoldsFuncDef, match_stmt, "") { return match_stmt(arg); }

class InferExprTypesTest : public testing::Test, public AstFixture {
 protected:
  std::optional<TypeError> InferExprTypes(
      FuncDefStmt& stmt,
      const std::unordered_map<std::string_view, FuncType>& func_types = {}) {
    return ::lucid::InferExprTypes(stmt_arena_, expr_arena_, type_arena_,
                                   func_types, stmt);
  }
};

TEST_F(InferExprTypesTest, ReturnValueFromResultType) {
  auto func = FuncDefStmt{
      .name = "foo",
      .result_type = T(BasicType{.name = "Int32"}),
      .body =
          {
              .stmts = StmtListOf({
                  S(ReturnStmt{
                      .value = E(IntLitExpr{
                          .value = "21",
                      }),
                  }),
              }),
          },
  };

  EXPECT_EQ(InferExprTypes(func), std::nullopt);
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
      .body =
          {
              .stmts = StmtListOf({
                  S(VarDeclStmt{
                      .name = "x",
                      .type = T(BasicType{.name = "Int64"}),
                      .init = E(IntLitExpr{
                          .value = "21",
                      }),
                  }),
              }),
          },
  };

  EXPECT_EQ(InferExprTypes(func), std::nullopt);
  EXPECT_THAT(func, HoldsFuncDef(MatchesFuncDefStmt({
                        .name = "foo",
                        .result_type = MatchesBasicType({.name = "Void"}),
                        .body = {{
                            MatchesVarDeclStmt({
                                .name = "x",
                                .type = MatchesBasicType({.name = "Int64"}),
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
      .parameters =
          {
              {
                  .name = "x",
                  .type = T(BasicType{.name = "Int64"}),
              },
          },
      .body =
          {
              .stmts = StmtListOf({
                  S(VarAssignStmt{
                      .name = "x",
                      .expr = E(IntLitExpr{
                          .value = "21",
                      }),
                  }),
              }),
          },
  };

  EXPECT_EQ(InferExprTypes(func), std::nullopt);
  EXPECT_THAT(func, HoldsFuncDef(MatchesFuncDefStmt({
                        .name = "foo",
                        .result_type = MatchesBasicType({.name = "Void"}),
                        .parameters =
                            {
                                {
                                    .name = "x",
                                    .type = MatchesBasicType({.name = "Int64"}),
                                },
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
      .parameters =
          {
              {
                  .name = "n",
                  .type = T(BasicType{.name = "Int32"}),
              },
          },
      .body =
          {
              .stmts = StmtListOf({
                  S(IfStmt{
                      .cond = E(BinaryOpExpr{
                          .op = BinaryOp::Eq,
                          .lhs = E(IdentExpr{.name = "n"}),
                          .rhs = E(IntLitExpr{.value = "1"}),
                      }),

                  }),
              }),
          },
  };

  EXPECT_EQ(InferExprTypes(func), std::nullopt);
  EXPECT_THAT(func, HoldsFuncDef(MatchesFuncDefStmt({
                        .name = "fact",
                        .result_type = MatchesBasicType({.name = "Void"}),
                        .parameters =
                            {
                                {
                                    .name = "n",
                                    .type = MatchesBasicType({.name = "Int32"}),
                                },
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
      .body =
          {
              .stmts = StmtListOf({
                  S(VarDeclStmt{
                      .name = "x",
                      .type = T(BasicType{.name = "Int64"}),
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
          },
  };

  EXPECT_EQ(InferExprTypes(func), std::nullopt);
  EXPECT_THAT(func,
              HoldsFuncDef(MatchesFuncDefStmt({
                  .name = "foo",
                  .result_type = MatchesBasicType({.name = "Void"}),
                  .body = {{
                      MatchesVarDeclStmt({
                          .name = "x",
                          .type = MatchesBasicType({.name = "Int64"}),
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
  auto func = FuncDefStmt{
      .name = "foo",
      .result_type = T(BasicType{.name = "Int32"}),
      .body =
          {
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
          },
  };

  std::vector<FuncParam> id_func_params = {
      {.type = T(BasicType{.name = "Int32"})},
  };
  auto id_func_type = FuncType{
      .result_type = "Int32",
      .parameters = id_func_params,
  };

  EXPECT_EQ(InferExprTypes(func, {{"id", id_func_type}}), std::nullopt);
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
      .body =
          {
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
          },
  };

  EXPECT_EQ(InferExprTypes(func), std::nullopt);
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
  auto func =
      FuncDefStmt{.name = "foo",
                  .result_type = T(BasicType{.name = "Int32"}),
                  .body = {
                      .stmts = StmtListOf({
                          S(VarDeclStmt{
                              .name = "a",
                              .type = T(ArrayType{
                                  .element_type = T(BasicType{.name = "Int32"}),
                                  .size = IntLitExpr{.value = "10"},
                              }),
                          }),
                          S(ReturnStmt{
                              .value = E(IndexExpr{
                                  .base = E(IdentExpr{.name = "a"}),
                                  .index = E(IntLitExpr{.value = "2"}),
                              }),
                          }),
                      }),
                  }};

  EXPECT_EQ(InferExprTypes(func), std::nullopt);
  EXPECT_THAT(
      func, HoldsFuncDef(MatchesFuncDefStmt({
                .name = "foo",
                .result_type = MatchesBasicType({.name = "Int32"}),
                .body = {{
                    MatchesVarDeclStmt({
                        .name = "a",
                        .type = MatchesArrayType({
                            .element_type = MatchesBasicType({.name = "Int32"}),
                            .size = {.value = "10"},
                        }),
                    }),
                    MatchesReturnStmt({
                        .value = MatchesIndexExpr({
                            .base = MatchesIdentExpr({
                                .name = "a",
                                .type = MatchesArrayType({
                                    .element_type =
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
      .body =
          {
              .stmts = StmtListOf({
                  S(VarDeclStmt{
                      .name = "x",
                      .type = T(BasicType{.name = "Int32"}),
                      .init = E(BoolLitExpr{
                          .value = "true",
                      }),
                  }),
              }),
          },
  };

  EXPECT_EQ(InferExprTypes(func), TypeError("expected type Int32"));
}

TEST_F(InferExprTypesTest, ErrorInt64FromInt32) {
  auto func = FuncDefStmt{
      .name = "foo",
      .result_type = T(BasicType{.name = "Void"}),
      .body =
          {
              .stmts = StmtListOf({
                  S(VarDeclStmt{
                      .name = "x",
                      .type = T(BasicType{.name = "Int32"}),
                      .init = E(IntLitExpr{
                          .value = "2",
                      }),
                  }),
                  S(VarDeclStmt{
                      .name = "y",
                      .type = T(BasicType{.name = "Int64"}),
                      .init = E(IdentExpr{
                          .name = "x",
                      }),
                  }),
              }),
          },
  };

  EXPECT_EQ(InferExprTypes(func), TypeError("expected type Int64"));
}

}  // namespace
}  // namespace lucid
