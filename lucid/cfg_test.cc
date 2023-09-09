#include "lucid/cfg.h"

#include "gmock/gmock.h"
#include "gtest/gtest.h"
#include "lucid/arena.h"
#include "lucid/ast.h"

namespace lucid {
namespace {

using ::testing::ElementsAre;
using ::testing::ElementsAreArray;
using ::testing::IsEmpty;
using ::testing::SizeIs;

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

TEST_F(ControlFlowGraphTest, FunctionName) {
  auto graph = BuildControlFlowGraph(FuncDefStmt{
      .name = "foo",
      .result_type = "Void",
  });

  EXPECT_EQ(graph.func_name, "foo");
}

TEST_F(ControlFlowGraphTest, EmptyFunction) {
  auto graph = BuildControlFlowGraph(FuncDefStmt{
      .name = "foo",
      .result_type = "Void",
  });

  auto block_ref = graph.first;
  ASSERT_NE(block_ref, ControlFlowGraph::kNullBlockRef);

  const auto& block = graph.get(block_ref);
  EXPECT_THAT(block.next, ElementsAre(graph.last));
  EXPECT_THAT(block.statements, IsEmpty());
}

TEST_F(ControlFlowGraphTest, FuncCallExprWithoutArgs) {
  auto func_call_expr = Allocate(FuncCallExpr({
      .func_name = "bar",
  }));
  auto graph = BuildControlFlowGraph(FuncDefStmt{
      .name = "foo",
      .result_type = "Void",
      .body =
          {
              .statements =
                  {
                      func_call_expr,
                  },
          },
  });

  auto block_ref = graph.first;
  ASSERT_NE(block_ref, ControlFlowGraph::kNullBlockRef);

  const auto& block = graph.get(block_ref);
  EXPECT_THAT(block.next, ElementsAre(graph.last));
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
  auto graph = BuildControlFlowGraph(FuncDefStmt{
      .name = "foo",
      .result_type = "Void",
      .body =
          {
              .statements =
                  {
                      func_call_expr,
                  },
          },
  });

  auto block_ref = graph.first;
  ASSERT_NE(block_ref, ControlFlowGraph::kNullBlockRef);

  const auto& block = graph.get(block_ref);
  EXPECT_THAT(block.next, ElementsAre(graph.last));
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
  auto graph = BuildControlFlowGraph(FuncDefStmt{
      .name = "foo",
      .result_type = "Void",
      .body =
          {
              .statements =
                  {
                      return_stmt,
                  },
          },
  });

  auto block_ref = graph.first;
  ASSERT_NE(block_ref, ControlFlowGraph::kNullBlockRef);

  const auto& block = graph.get(block_ref);
  EXPECT_THAT(block.next, ElementsAre(graph.last));
  EXPECT_THAT(block.statements, ElementsAreArray({
                                    arg1_expr,
                                    func_call_expr,
                                    return_stmt,
                                }));
}

TEST_F(ControlFlowGraphTest, VarDeclStmt) {
  auto func_call_stmt_ref = Allocate(FuncCallExpr{.func_name = "bar"});
  auto x_var_decl_ref = Allocate(VarDeclStmt{
      .type = "Int32",
      .name = "x",
      .init = func_call_stmt_ref,
  });
  auto graph = BuildControlFlowGraph(FuncDefStmt{
      .name = "foo",
      .body =
          {
              .statements = {x_var_decl_ref},
          },
      .result_type = "Void",
  });

  auto block_ref = graph.first;
  ASSERT_NE(block_ref, ControlFlowGraph::kNullBlockRef);

  const auto& block = graph.get(block_ref);
  EXPECT_THAT(block.next, ElementsAre(graph.last));
  EXPECT_THAT(block.statements, ElementsAreArray({
                                    func_call_stmt_ref,
                                    x_var_decl_ref,
                                }));
}

TEST_F(ControlFlowGraphTest, BinaryOpExpr) {
  auto lhs_expr = Allocate(IntLitExpr{.value = "2"});
  auto rhs_expr = Allocate(IntLitExpr{.value = "3"});
  auto add_expr = Allocate(BinaryOpExpr{
      .op = BinaryOp::Add,
      .lhs = lhs_expr,
      .rhs = rhs_expr,
  });
  auto graph = BuildControlFlowGraph(FuncDefStmt{
      .name = "foo",
      .body =
          {
              .statements = {add_expr},
          },
      .result_type = "Int32",
  });

  auto block_ref = graph.first;
  ASSERT_NE(block_ref, ControlFlowGraph::kNullBlockRef);

  const auto& block = graph.get(block_ref);
  EXPECT_THAT(block.next, ElementsAre(graph.last));
  EXPECT_THAT(block.statements, ElementsAreArray({
                                    lhs_expr,
                                    rhs_expr,
                                    add_expr,
                                }));
}

TEST_F(ControlFlowGraphTest, IfStmt) {
  auto add_lhs_expr = Allocate(IntLitExpr{.value = "2"});
  auto add_rhs_expr = Allocate(IntLitExpr{.value = "3"});
  auto add_expr = Allocate(BinaryOpExpr{
      .op = BinaryOp::Add,
      .lhs = add_lhs_expr,
      .rhs = add_rhs_expr,
  });
  auto mul_lhs_expr = Allocate(IntLitExpr{.value = "4"});
  auto mul_rhs_expr = Allocate(IntLitExpr{.value = "5"});
  auto mul_expr = Allocate(BinaryOpExpr{
      .op = BinaryOp::Mul,
      .lhs = mul_lhs_expr,
      .rhs = mul_rhs_expr,
  });
  auto cond_expr = Allocate(BoolLitExpr{.value = "true"});
  auto if_stmt = Allocate(IfStmt{
      .cond = cond_expr,
      .then_body =
          {
              .statements{add_expr},
          },
      .else_body =
          {
              .statements{mul_expr},
          },
  });
  auto graph = BuildControlFlowGraph(FuncDefStmt{
      .name = "foo",
      .body =
          {
              .statements = {if_stmt},
          },
      .result_type = "Int32",
  });

  auto block_ref = graph.first;
  ASSERT_NE(block_ref, ControlFlowGraph::kNullBlockRef);

  const auto& block = graph.get(block_ref);
  ASSERT_THAT(block.next, SizeIs(2));
  EXPECT_THAT(block.statements, ElementsAreArray({
                                    cond_expr,
                                }));
  EXPECT_EQ(block.branch_cond, cond_expr);

  const auto& then_block = graph.get(block.next[0]);
  EXPECT_THAT(then_block.next, ElementsAre(graph.last));
  EXPECT_THAT(then_block.statements, ElementsAreArray({
                                         add_lhs_expr,
                                         add_rhs_expr,
                                         add_expr,
                                     }));

  const auto& else_block = graph.get(block.next[1]);
  EXPECT_THAT(else_block.next, ElementsAre(graph.last));
  EXPECT_THAT(else_block.statements, ElementsAreArray({
                                         mul_lhs_expr,
                                         mul_rhs_expr,
                                         mul_expr,
                                     }));
}

TEST_F(ControlFlowGraphTest, VarDecl) {
  auto int_lit = Allocate(IntLitExpr{
      .value = "3",
  });
  auto const_decl_stmt = Allocate(VarDeclStmt{
      .name = "x",
      .type = "Int32",
      .init = int_lit,
  });
  auto ident_expr = Allocate(IdentExpr{
      .name = "x",
  });
  auto return_stmt = Allocate(ReturnStmt{
      .value = ident_expr,
  });
  auto graph = BuildControlFlowGraph(FuncDefStmt{
      .name = "foo",
      .result_type = "Int32",
      .body =
          {
              .statements =
                  {
                      const_decl_stmt,
                      return_stmt,
                  },
          },
  });

  auto block_ref = graph.first;
  ASSERT_NE(block_ref, ControlFlowGraph::kNullBlockRef);

  const auto& block = graph.get(block_ref);
  EXPECT_THAT(block.next, ElementsAre(graph.last));
  EXPECT_THAT(block.statements, ElementsAreArray({
                                    int_lit,
                                    const_decl_stmt,
                                    ident_expr,
                                    return_stmt,
                                }));
}

}  // namespace
}  // namespace lucid
