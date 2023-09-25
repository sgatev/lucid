#include "lucid/type.h"

#include <optional>

#include "gmock/gmock.h"
#include "gtest/gtest.h"
#include "lucid/arena.h"
#include "lucid/ast.h"
#include "lucid/ast_fixture.h"

namespace lucid {
namespace {

MATCHER_P(HoldsFuncDef, match_stmt, "") { return match_stmt(arg); }

class InferExprTypesTest : public testing::Test, public AstFixture {
 protected:
  std::optional<TypeError> InferExprTypes(
      FuncDefStmt& stmt,
      const std::unordered_map<std::string_view, FuncType>& func_types = {}) {
    return ::lucid::InferExprTypes(arena_, func_types, stmt);
  }
};

TEST_F(InferExprTypesTest, FromResult) {
  auto func = FuncDefStmt{
      .name = "foo",
      .result_type = "Int32",
      .body =
          {
              .statements =
                  {
                      Allocate(ReturnStmt{
                          .value = Allocate(IntLitExpr{
                              .value = "21",
                          }),
                      }),
                  },
          },
  };

  EXPECT_EQ(InferExprTypes(func), std::nullopt);
  EXPECT_THAT(func, HoldsFuncDef(MatchesFuncDefStmt({
                        .name = "foo",
                        .result_type = "Int32",
                        .body =
                            {
                                .statements = {{
                                    MatchesReturnStmt({
                                        .value = MatchesIntLitExpr({
                                            .type = "Int32",
                                            .value = "21",
                                        }),
                                    }),
                                }},
                            },
                    })));
}

TEST_F(InferExprTypesTest, FromVarDecl) {
  auto func = FuncDefStmt{
      .name = "foo",
      .result_type = "Void",
      .body =
          {
              .statements =
                  {
                      Allocate(VarDeclStmt{
                          .name = "x",
                          .type = "Int64",
                          .init = Allocate(IntLitExpr{
                              .value = "21",
                          }),
                      }),
                  },
          },
  };

  EXPECT_EQ(InferExprTypes(func), std::nullopt);
  EXPECT_THAT(func, HoldsFuncDef(MatchesFuncDefStmt({
                        .name = "foo",
                        .result_type = "Void",
                        .body =
                            {
                                .statements = {{
                                    MatchesVarDeclStmt({
                                        .name = "x",
                                        .type = "Int64",
                                        .init = MatchesIntLitExpr({
                                            .type = "Int64",
                                            .value = "21",
                                        }),
                                    }),
                                }},
                            },
                    })));
}

TEST_F(InferExprTypesTest, IfStmtCond) {
  auto func = FuncDefStmt{
      .name = "fact",
      .result_type = "Void",
      .parameters =
          {
              {
                  .name = "n",
                  .type = "Int32",
              },
          },
      .body =
          {
              .statements =
                  {
                      Allocate(IfStmt{
                          .cond = Allocate(BinaryOpExpr{
                              .op = BinaryOp::Eq,
                              .lhs = Allocate(IdentExpr{.name = "n"}),
                              .rhs = Allocate(IntLitExpr{.value = "1"}),
                          }),

                      }),
                  },
          },
  };

  EXPECT_EQ(InferExprTypes(func), std::nullopt);
  EXPECT_THAT(func, HoldsFuncDef(MatchesFuncDefStmt({
                        .name = "fact",
                        .result_type = "Void",
                        .parameters =
                            {
                                {
                                    .name = "n",
                                    .type = "Int32",
                                },
                            },
                        .body = {{{
                            MatchesIfStmt({
                                .cond = MatchesBinaryOpExpr({
                                    .type = "Bool",
                                    .op = BinaryOp::Eq,
                                    .lhs = MatchesIdentExpr({
                                        .type = "Int32",
                                        .name = "n",
                                    }),
                                    .rhs = MatchesIntLitExpr({
                                        .type = "Int32",
                                        .value = "1",
                                    }),
                                }),
                            }),
                        }}},
                    })));
}

TEST_F(InferExprTypesTest, ThroughBinOpExpr) {
  auto func = FuncDefStmt{
      .name = "foo",
      .result_type = "Void",
      .body =
          {
              .statements =
                  {
                      Allocate(VarDeclStmt{
                          .name = "x",
                          .type = "Int64",
                          .init = Allocate(BinaryOpExpr{
                              .op = BinaryOp::Add,
                              .lhs = Allocate(IntLitExpr{
                                  .value = "2",
                              }),
                              .rhs = Allocate(IntLitExpr{
                                  .value = "3",
                              }),
                          }),
                      }),
                  },
          },
  };

  EXPECT_EQ(InferExprTypes(func), std::nullopt);
  EXPECT_THAT(func, HoldsFuncDef(MatchesFuncDefStmt({
                        .name = "foo",
                        .result_type = "Void",
                        .body =
                            {
                                .statements = {{
                                    MatchesVarDeclStmt({
                                        .name = "x",
                                        .type = "Int64",
                                        .init = MatchesBinaryOpExpr({
                                            .type = "Int64",
                                            .lhs = MatchesIntLitExpr({
                                                .type="Int64",
                                                .value ="2",
                                            }),
                                            .rhs= MatchesIntLitExpr({
                                                .type="Int64",
                                                .value ="3",
                                            }),
                                        }),
                                    }),
                                }},
                            },
                    })));
}

TEST_F(InferExprTypesTest, ThroughFuncCall) {
  auto func = FuncDefStmt{
      .name = "foo",
      .result_type = "Void",
      .body =
          {
              .statements =
                  {
                      Allocate(VarDeclStmt{
                          .name = "x",
                          .type = "Int32",
                          .init = Allocate(FuncCallExpr{
                              .func_name = "id",
                              .arguments =
                                  {
                                      Allocate(IntLitExpr{
                                          .value = "21",
                                      }),
                                  },
                          }),
                      }),
                  },
          },
  };

  std::vector<FuncParam> id_func_params = {
      {.type = "Int32"},
  };
  auto id_func_type = FuncType{
      .result_type = "Int32",
      .parameters = id_func_params,
  };

  EXPECT_EQ(InferExprTypes(func, {{"id", id_func_type}}), std::nullopt);
  EXPECT_THAT(func, HoldsFuncDef(MatchesFuncDefStmt({
                        .name = "foo",
                        .result_type = "Void",
                        .body =
                            {
                                {{
                                    MatchesVarDeclStmt({
                                        .name = "x",
                                        .type = "Int32",
                                        .init = MatchesFuncCallExpr({
                                            .type = "Int32",
                                            .func_name = "id",
                                            .arguments =
                                                {
                                                    MatchesIntLitExpr({
                                                        .type = "Int32",
                                                        .value = "21",
                                                    }),
                                                },
                                        }),
                                    }),
                                }},
                            },
                    })));
}

TEST_F(InferExprTypesTest, ErrorBoolLitAsInt64) {
  auto func = FuncDefStmt{
      .name = "foo",
      .result_type = "Void",
      .body =
          {
              .statements =
                  {
                      Allocate(VarDeclStmt{
                          .name = "x",
                          .type = "Int64",
                          .init = Allocate(BoolLitExpr{
                              .value = "true",
                          }),
                      }),
                  },
          },
  };

  EXPECT_EQ(InferExprTypes(func),
            TypeError("Bool literal is not of type Int64"));
}

TEST_F(InferExprTypesTest, ErrorInt64FromInt32) {
  auto func = FuncDefStmt{
      .name = "foo",
      .result_type = "Void",
      .body =
          {
              .statements =
                  {
                      Allocate(VarDeclStmt{
                          .name = "x",
                          .type = "Int32",
                          .init = Allocate(IntLitExpr{
                              .value = "2",
                          }),
                      }),
                      Allocate(VarDeclStmt{
                          .name = "y",
                          .type = "Int64",
                          .init = Allocate(IdentExpr{
                              .name = "x",
                          }),
                      }),
                  },
          },
  };

  EXPECT_EQ(InferExprTypes(func),
            TypeError("Identifier 'x' is not of type Int64"));
}

}  // namespace
}  // namespace lucid
