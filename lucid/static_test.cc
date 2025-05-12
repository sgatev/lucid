#include "lucid/static.h"

#include "gtest/gtest.h"
#include "lucid/ast.h"
#include "lucid/ast_fixture.h"

namespace lucid {
namespace {

class InferStaticExprsTest : public testing::Test, public AstFixture {
 protected:
  ControlFlowGraph BuildControlFlowGraph(FuncDefStmt func_def) {
    return ::lucid::BuildControlFlowGraph(ctx_, func_def);
  }
};

TEST_F(InferStaticExprsTest, BoolLitExpr) {
  auto expr = E(BoolLitExpr{});

  auto graph = BuildControlFlowGraph(FuncDefStmt{
      .name = "foo",
      .result_type = T(BasicType{.name = "Void"}),
      .stmts = StmtListOf({
          S(DoStmt{.expr = expr}),
      }),
  });

  ASSERT_TRUE(InferStaticExprs(ctx_, graph).HasValue());
  EXPECT_TRUE(std::get<BoolLitExpr>(ctx_.DerefExpr(expr)).is_static);
}

TEST_F(InferStaticExprsTest, IntLitExpr) {
  auto expr = E(IntLitExpr{});

  auto graph = BuildControlFlowGraph(FuncDefStmt{
      .name = "foo",
      .result_type = T(BasicType{.name = "Void"}),
      .stmts = StmtListOf({
          S(DoStmt{.expr = expr}),
      }),
  });

  ASSERT_TRUE(InferStaticExprs(ctx_, graph).HasValue());
  EXPECT_TRUE(std::get<IntLitExpr>(ctx_.DerefExpr(expr)).is_static);
}

}  // namespace
}  // namespace lucid
