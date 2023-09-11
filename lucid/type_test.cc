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

class InferExpressionTypesTest : public testing::Test, public AstFixture {
 protected:
  std::optional<TypeError> InferExpressionTypes(FuncDefStmt& stmt) {
    return ::lucid::InferExpressionTypes(arena_, stmt);
  }
};

TEST_F(InferExpressionTypesTest, FromResult) {
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

  EXPECT_EQ(InferExpressionTypes(func), std::nullopt);
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

TEST_F(InferExpressionTypesTest, FromVarDecl) {
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

  EXPECT_EQ(InferExpressionTypes(func), std::nullopt);
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

TEST_F(InferExpressionTypesTest, ThroughBinOpExpr) {
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

  EXPECT_EQ(InferExpressionTypes(func), std::nullopt);
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

TEST_F(InferExpressionTypesTest, ErrorBoolLitAsInt64) {
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

  EXPECT_EQ(InferExpressionTypes(func),
            TypeError("Bool literal is not of type Int64"));
}

TEST_F(InferExpressionTypesTest, ErrorInt64FromInt32) {
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

  EXPECT_EQ(InferExpressionTypes(func),
            TypeError("Identifier 'x' is not of type Int64"));
}

}  // namespace
}  // namespace lucid
