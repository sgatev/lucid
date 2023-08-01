#include "lucid/cfg.h"

#include <utility>

#include "gmock/gmock.h"
#include "gtest/gtest.h"
#include "lucid/arena.h"
#include "lucid/ast.h"

namespace lucid {
namespace {

using ::testing::ElementsAreArray;
using ::testing::IsEmpty;

class ControlFlowGraphTest : public testing::Test {
 protected:
  template <typename T>
  StmtRef Allocate(T stmt) {
    return arena_.add(stmt);
  }

  ControlFlowGraph BuildControlFlowGraph(FuncDefStmt func_def) {
    return ::lucid::BuildControlFlowGraph(arena_, func_def);
  }

 private:
  Arena<Stmt> arena_;
};

TEST_F(ControlFlowGraphTest, EmptyFunction) {
  auto cfg = BuildControlFlowGraph(FuncDefStmt{
      .name = "foo",
      .result_type = "void",
  });

  auto block_ref = cfg.first;
  ASSERT_NE(block_ref, ControlFlowGraph::kNullBlockRef);

  const auto& block = cfg.get(block_ref);
  EXPECT_EQ(block.next, cfg.last);
  EXPECT_THAT(block.statements, IsEmpty());
}

TEST_F(ControlFlowGraphTest, FuncCallExprWithoutArgs) {
  auto func_call_expr = Allocate(FuncCallExpr({
      .func_name = "bar",
  }));
  auto cfg = BuildControlFlowGraph(FuncDefStmt{
      .name = "foo",
      .result_type = "void",
      .body =
          {
              .statements =
                  {
                      func_call_expr,
                  },
          },
  });

  auto block_ref = cfg.first;
  ASSERT_NE(block_ref, ControlFlowGraph::kNullBlockRef);

  const auto& block = cfg.get(block_ref);
  EXPECT_EQ(block.next, cfg.last);
  EXPECT_THAT(block.statements, ElementsAreArray({
                                    func_call_expr,
                                }));
}

TEST_F(ControlFlowGraphTest, FuncCallExprWithArgs) {
  auto arg1_expr = Allocate(IntLitExpr{
      .value = "3",
  });
  auto arg2_expr = Allocate(IntLitExpr{
      .value = "7",
  });
  auto baz_func_call_expr = Allocate(FuncCallExpr({
      .func_name = "baz",
      .arguments =
          {
              arg2_expr,
          },
  }));
  auto func_call_expr = Allocate(FuncCallExpr({
      .func_name = "bar",
      .arguments =
          {
              arg1_expr,
              baz_func_call_expr,
          },
  }));
  auto cfg = BuildControlFlowGraph(FuncDefStmt{
      .name = "foo",
      .result_type = "void",
      .body =
          {
              .statements =
                  {
                      func_call_expr,
                  },
          },
  });

  auto block_ref = cfg.first;
  ASSERT_NE(block_ref, ControlFlowGraph::kNullBlockRef);

  const auto& block = cfg.get(block_ref);
  EXPECT_EQ(block.next, cfg.last);
  EXPECT_THAT(block.statements, ElementsAreArray({
                                    arg1_expr,
                                    arg2_expr,
                                    baz_func_call_expr,
                                    func_call_expr,
                                }));
}

TEST_F(ControlFlowGraphTest, ReturnStmt) {
  auto arg1_expr = Allocate(IntLitExpr{
      .value = "3",
  });
  auto func_call_expr = Allocate(FuncCallExpr({
      .func_name = "bar",
      .arguments =
          {
              arg1_expr,
          },
  }));
  auto return_stmt = Allocate(ReturnStmt{
      .value = func_call_expr,
  });
  auto cfg = BuildControlFlowGraph(FuncDefStmt{
      .name = "foo",
      .result_type = "void",
      .body =
          {
              .statements =
                  {
                      return_stmt,
                  },
          },
  });

  auto block_ref = cfg.first;
  ASSERT_NE(block_ref, ControlFlowGraph::kNullBlockRef);

  const auto& block = cfg.get(block_ref);
  EXPECT_EQ(block.next, cfg.last);
  EXPECT_THAT(block.statements, ElementsAreArray({
                                    arg1_expr,
                                    func_call_expr,
                                    return_stmt,
                                }));
}

TEST_F(ControlFlowGraphTest, VarDeclStmt) {
  auto func_call_stmt_ref = Allocate(FuncCallExpr{.func_name = "bar"});
  auto x_var_decl_ref = Allocate(VarDeclStmt{
      .type = "int",
      .name = "x",
      .init = func_call_stmt_ref,
  });
  auto cfg = BuildControlFlowGraph(FuncDefStmt{
      .name = "foo",
      .body =
          {
              .statements = {x_var_decl_ref},
          },
      .result_type = "void",
  });

  auto block_ref = cfg.first;
  ASSERT_NE(block_ref, ControlFlowGraph::kNullBlockRef);

  const auto& block = cfg.get(block_ref);
  EXPECT_EQ(block.next, cfg.last);
  EXPECT_THAT(block.statements, ElementsAreArray({
                                    func_call_stmt_ref,
                                    x_var_decl_ref,
                                }));
}

}  // namespace
}  // namespace lucid
