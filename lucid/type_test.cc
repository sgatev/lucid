#include "lucid/type.h"

#include <functional>
#include <utility>
#include <variant>

#include "gmock/gmock.h"
#include "gtest/gtest.h"
#include "lucid/arena.h"
#include "lucid/ast.h"
#include "lucid/ast_matchers.h"

namespace lucid {
namespace {

MATCHER_P(HoldsFuncDef, match_stmt, "") { return match_stmt(arg); }

class DeduceTypesTest : public testing::Test {
 protected:
  template <typename T>
  StmtRef Allocate(T stmt) {
    return arena_.add(stmt);
  }

  void DeduceTypes(FuncDefStmt& stmt) { ::lucid::DeduceTypes(arena_, stmt); }

  std::function<bool(FuncDefStmt)> MatchesFuncDefStmt(
      FuncDefStmtPattern pattern) {
    return [pattern](FuncDefStmt stmt) { return pattern(stmt); };
  }

  StmtRefMatcher MatchesReturnStmt(ReturnStmtPattern pattern) {
    return MatchesStmt<ReturnStmt>(std::move(pattern));
  }

  StmtRefMatcher MatchesIfStmt(IfStmtPattern pattern) {
    return MatchesStmt<IfStmt>(std::move(pattern));
  }

  ExprRefMatcher MatchesIntLitExpr(IntLitExprPattern pattern) {
    return MatchesExpr<IntLitExpr>(std::move(pattern));
  }

  ExprRefMatcher MatchesBoolLitExpr(BoolLitExprPattern pattern) {
    return MatchesExpr<BoolLitExpr>(std::move(pattern));
  }

  ExprRefMatcher MatchesBinaryOpExpr(BinaryOpExprPattern pattern) {
    return MatchesExpr<BinaryOpExpr>(std::move(pattern));
  }

  ExprRefMatcher MatchesIdentExpr(IdentExprPattern pattern) {
    return MatchesExpr<IdentExpr>(std::move(pattern));
  }

  ExprRefMatcher MatchesFuncCallExpr(FuncCallExprPattern pattern) {
    return MatchesExpr<FuncCallExpr>(std::move(pattern));
  }

  StmtRefMatcher MatchesVarDeclStmt(VarDeclStmtPattern pattern) {
    return MatchesStmt<VarDeclStmt>(std::move(pattern));
  }

 private:
  template <typename S, typename P>
  ExprRefMatcher MatchesStmt(P pattern) {
    return [this, pattern](ExprRef ref) {
      if (auto* stmt = std::get_if<S>(&arena_.get(ref))) return pattern(*stmt);
      return false;
    };
  }

  template <typename E, typename P>
  ExprRefMatcher MatchesExpr(P pattern) {
    return MatchesStmt<Expr>([pattern](const Expr& stmt) {
      if (auto* expr = std::get_if<E>(&stmt)) return pattern(*expr);
      return false;
    });
  }

  Arena<Stmt> arena_;
};

TEST_F(DeduceTypesTest, FromResult) {
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

  DeduceTypes(func);
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

TEST_F(DeduceTypesTest, FromVarDecl) {
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

  DeduceTypes(func);
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

TEST_F(DeduceTypesTest, ThroughBinOpExpr) {
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

  DeduceTypes(func);
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

}  // namespace
}  // namespace lucid
