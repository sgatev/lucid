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
    return ::lucid::ExtractFuncTypes(arena_, func_defs);
  }
};

TEST_F(ExtractFuncTypesTest, NoFuncDefs) {
  EXPECT_THAT(ExtractFuncTypes({}), IsEmpty());
}

TEST_F(ExtractFuncTypesTest, MultipleFuncDefs) {
  const std::vector<FuncDefStmt> func_defs = {
      {
          .name = "id",
          .result_type = Allocate(BasicType{.name = "Int32"}),
          .parameters =
              {
                  {.type = Allocate(BasicType{.name = "Int32"})},
              },
      },
      {
          .name = "foo",
          .result_type = Allocate(BasicType{.name = "Int64"}),
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
    return ::lucid::InferExprTypes(arena_, func_types, stmt);
  }
};

TEST_F(InferExprTypesTest, FromResult) {
  auto func = FuncDefStmt{
      .name = "foo",
      .result_type = Allocate(BasicType{.name = "Int32"}),
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
  EXPECT_THAT(
      func, HoldsFuncDef(MatchesFuncDefStmt({
                .name = "foo",
                .result_type = MatchesBasicType({.name = "Int32"}),
                .body =
                    {
                        .statements = {{
                            MatchesReturnStmt({
                                .value = MatchesIntLitExpr({
                                    .type = MatchesBasicType({.name = "Int32"}),
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
      .result_type = Allocate(BasicType{.name = "Void"}),
      .body =
          {
              .statements =
                  {
                      Allocate(VarDeclStmt{
                          .name = "x",
                          .type = Allocate(BasicType{.name = "Int64"}),
                          .init = Allocate(IntLitExpr{
                              .value = "21",
                          }),
                      }),
                  },
          },
  };

  EXPECT_EQ(InferExprTypes(func), std::nullopt);
  EXPECT_THAT(
      func, HoldsFuncDef(MatchesFuncDefStmt({
                .name = "foo",
                .result_type = MatchesBasicType({.name = "Void"}),
                .body =
                    {
                        .statements = {{
                            MatchesVarDeclStmt({
                                .name = "x",
                                .type = MatchesBasicType({.name = "Int64"}),
                                .init = MatchesIntLitExpr({
                                    .type = MatchesBasicType({.name = "Int64"}),
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
      .result_type = Allocate(BasicType{.name = "Void"}),
      .parameters =
          {
              {
                  .name = "n",
                  .type = Allocate(BasicType{.name = "Int32"}),
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
  EXPECT_THAT(func,
              HoldsFuncDef(MatchesFuncDefStmt({
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
                              .lhs = MatchesIdentExpr({
                                  .type = MatchesBasicType({.name = "Int32"}),
                                  .name = "n",
                              }),
                              .rhs = MatchesIntLitExpr({
                                  .type = MatchesBasicType({.name = "Int32"}),
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
      .result_type = Allocate(BasicType{.name = "Void"}),
      .body =
          {
              .statements =
                  {
                      Allocate(VarDeclStmt{
                          .name = "x",
                          .type = Allocate(BasicType{.name = "Int64"}),
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
  EXPECT_THAT(
      func,
      HoldsFuncDef(MatchesFuncDefStmt(  //
          {
              .name = "foo",
              .result_type = MatchesBasicType({.name = "Void"}),
              .body =
                  {
                      {{
                          MatchesVarDeclStmt({
                              .name = "x",
                              .type = MatchesBasicType({.name = "Int64"}),
                              .init = MatchesBinaryOpExpr({
                                  .op = BinaryOp::Add,
                                  .type = MatchesBasicType({.name = "Int64"}),
                                  .lhs = MatchesIntLitExpr({
                                      .type =
                                          MatchesBasicType({.name = "Int64"}),
                                      .value = "2",
                                  }),
                                  .rhs = MatchesIntLitExpr({
                                      .type =
                                          MatchesBasicType({.name = "Int64"}),
                                      .value = "3",
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
      .result_type = Allocate(BasicType{.name = "Void"}),
      .body =
          {
              .statements =
                  {
                      Allocate(VarDeclStmt{
                          .name = "x",
                          .type = Allocate(BasicType{.name = "Int32"}),
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
      {.type = Allocate(BasicType{.name = "Int32"})},
  };
  auto id_func_type = FuncType{
      .result_type = "Int32",
      .parameters = id_func_params,
  };

  EXPECT_EQ(InferExprTypes(func, {{"id", id_func_type}}), std::nullopt);
  EXPECT_THAT(
      func,
      HoldsFuncDef(MatchesFuncDefStmt(  //
          {
              .name = "foo",
              .result_type = MatchesBasicType({.name = "Void"}),
              .body =
                  {
                      {{
                          MatchesVarDeclStmt({
                              .name = "x",
                              .type = MatchesBasicType({.name = "Int32"}),
                              .init = MatchesFuncCallExpr({
                                  .type = MatchesBasicType({.name = "Int32"}),
                                  .func_name = "id",
                                  .arguments =
                                      {
                                          MatchesIntLitExpr({
                                              .type = MatchesBasicType(
                                                  {.name = "Int32"}),
                                              .value = "21",
                                          }),
                                      },
                              }),
                          }),
                      }},
                  },
          })));
}

TEST_F(InferExprTypesTest, UnconstrainedIntLit) {
  auto func = FuncDefStmt{
      .name = "foo",
      .result_type = Allocate(BasicType{.name = "Void"}),
      .body =
          {
              .statements =
                  {
                      Allocate(IfStmt{
                          .cond = Allocate(BinaryOpExpr{
                              .op = BinaryOp::Lt,
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
  EXPECT_THAT(
      func,
      HoldsFuncDef(MatchesFuncDefStmt(  //
          {
              .name = "foo",
              .result_type = MatchesBasicType({.name = "Void"}),
              .body =
                  {
                      {{
                          MatchesIfStmt({
                              .cond = MatchesBinaryOpExpr({
                                  .op = BinaryOp::Lt,
                                  .type = MatchesBasicType({.name = "Bool"}),
                                  .lhs = MatchesIntLitExpr({
                                      .type =
                                          MatchesBasicType({.name = "Int32"}),
                                      .value = "2",
                                  }),
                                  .rhs = MatchesIntLitExpr({
                                      .type =
                                          MatchesBasicType({.name = "Int32"}),
                                      .value = "3",
                                  }),
                              }),
                          }),
                      }},
                  },
          })));
}

TEST_F(InferExprTypesTest, ErrorBoolLitAsInt64) {
  auto func = FuncDefStmt{
      .name = "foo",
      .result_type = Allocate(BasicType{.name = "Void"}),
      .body =
          {
              .statements =
                  {
                      Allocate(VarDeclStmt{
                          .name = "x",
                          .type = Allocate(BasicType{.name = "Int64"}),
                          .init = Allocate(BoolLitExpr{
                              .value = "true",
                          }),
                      }),
                  },
          },
  };

  EXPECT_EQ(InferExprTypes(func), TypeError("expected type Int64"));
}

TEST_F(InferExprTypesTest, ErrorInt64FromInt32) {
  auto func = FuncDefStmt{
      .name = "foo",
      .result_type = Allocate(BasicType{.name = "Void"}),
      .body =
          {
              .statements =
                  {
                      Allocate(VarDeclStmt{
                          .name = "x",
                          .type = Allocate(BasicType{.name = "Int32"}),
                          .init = Allocate(IntLitExpr{
                              .value = "2",
                          }),
                      }),
                      Allocate(VarDeclStmt{
                          .name = "y",
                          .type = Allocate(BasicType{.name = "Int64"}),
                          .init = Allocate(IdentExpr{
                              .name = "x",
                          }),
                      }),
                  },
          },
  };

  EXPECT_EQ(InferExprTypes(func), TypeError("expected type Int64"));
}

}  // namespace
}  // namespace lucid
