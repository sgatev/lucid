#include "lucid/cfg.h"

#include "gmock/gmock.h"
#include "gtest/gtest.h"
#include "lucid/arena.h"
#include "lucid/ast.h"
#include "lucid/ast_fixture.h"

namespace lucid {
namespace {

using ::testing::ElementsAre;
using ::testing::ElementsAreArray;
using ::testing::IsEmpty;
using ::testing::SizeIs;

class ControlFlowGraphTest : public testing::Test, public AstFixture {
 protected:
  ControlFlowGraph BuildControlFlowGraph(FuncDefStmt func_def) {
    return ::lucid::BuildControlFlowGraph(stmt_arena_, expr_arena_, func_def);
  }
};

TEST_F(ControlFlowGraphTest, FunctionName) {
  auto graph = BuildControlFlowGraph(FuncDefStmt{
      .name = "foo",
      .result_type = T(BasicType{.name = "Void"}),
  });

  EXPECT_EQ(graph.func_name, "foo");
}

TEST_F(ControlFlowGraphTest, EmptyFunction) {
  auto graph = BuildControlFlowGraph(FuncDefStmt{
      .name = "foo",
      .result_type = T(BasicType{.name = "Void"}),
  });

  ASSERT_NE(graph.first, ControlFlowGraph::kNullBlockRef);
  const auto& first_block = graph.get(graph.first);
  EXPECT_EQ(first_block.id, 0);
  EXPECT_THAT(first_block.next, ElementsAre(graph.last));
  EXPECT_THAT(first_block.sequences, IsEmpty());

  ASSERT_NE(graph.last, ControlFlowGraph::kNullBlockRef);
  const auto& last_block = graph.get(graph.last);
  EXPECT_EQ(last_block.id, 1);
  EXPECT_THAT(last_block.next, IsEmpty());
  EXPECT_THAT(last_block.sequences, IsEmpty());
}

TEST_F(ControlFlowGraphTest, FuncCallExprWithoutArgs) {
  auto func_call_expr = E(FuncCallExpr({
      .func_name = "bar",
      .args = EmptyExprList(),
  }));
  auto stmts = StmtListOf(DoStmt{.expr = func_call_expr});
  auto graph = BuildControlFlowGraph(FuncDefStmt{
      .name = "foo",
      .result_type = T(BasicType{.name = "Void"}),
      .body =
          {
              .stmts = stmts,
          },
  });

  ASSERT_NE(graph.first, ControlFlowGraph::kNullBlockRef);
  const auto& first_block = graph.get(graph.first);
  EXPECT_EQ(first_block.id, 0);
  EXPECT_THAT(first_block.next, ElementsAre(graph.last));
  ASSERT_EQ(first_block.sequences.size(), 1);
  EXPECT_THAT(first_block.sequences[0].expressions, ElementsAreArray({
                                                        func_call_expr,
                                                    }));
  EXPECT_THAT(first_block.sequences[0].stmt, stmts[0]);

  ASSERT_NE(graph.last, ControlFlowGraph::kNullBlockRef);
  const auto& last_block = graph.get(graph.last);
  EXPECT_EQ(last_block.id, 1);
  EXPECT_THAT(last_block.next, IsEmpty());
  EXPECT_THAT(last_block.sequences, IsEmpty());
}

TEST_F(ControlFlowGraphTest, FuncCallExprWithArgs) {
  auto baz_arg1_expr = IntLitExpr{
      .value = "3",
  };
  auto baz_arg2_expr = IntLitExpr{
      .value = "7",
  };
  auto baz_func_call_args = ExprListOf(baz_arg1_expr, baz_arg2_expr);
  auto baz_func_call_expr = FuncCallExpr({
      .func_name = "baz",
      .args = baz_func_call_args,
  });

  auto qux_arg1_expr = IntLitExpr{
      .value = "9",
  };
  auto qux_arg2_expr = IntLitExpr{
      .value = "21",
  };
  auto qux_func_call_args = ExprListOf(qux_arg1_expr, qux_arg2_expr);
  auto qux_func_call_expr = FuncCallExpr({
      .func_name = "qux",
      .args = qux_func_call_args,
  });

  auto bar_func_call_args = ExprListOf(baz_func_call_expr, qux_func_call_expr);
  auto bar_func_call_expr = E(FuncCallExpr({
      .func_name = "bar",
      .args = bar_func_call_args,
  }));
  auto stmts = StmtListOf(DoStmt{.expr = bar_func_call_expr});
  auto graph = BuildControlFlowGraph(FuncDefStmt{
      .name = "foo",
      .result_type = T(BasicType{.name = "Void"}),
      .body =
          {
              .stmts = stmts,
          },
  });

  ASSERT_NE(graph.first, ControlFlowGraph::kNullBlockRef);
  const auto& first_block = graph.get(graph.first);
  EXPECT_EQ(first_block.id, 0);
  EXPECT_THAT(first_block.next, ElementsAre(graph.last));
  ASSERT_EQ(first_block.sequences.size(), 1);
  EXPECT_THAT(first_block.sequences[0].expressions, ElementsAreArray({
                                                        baz_func_call_args[0],
                                                        baz_func_call_args[1],
                                                        bar_func_call_args[0],
                                                        qux_func_call_args[0],
                                                        qux_func_call_args[1],
                                                        bar_func_call_args[1],
                                                        bar_func_call_expr,
                                                    }));
  EXPECT_THAT(first_block.sequences[0].stmt, stmts[0]);

  ASSERT_NE(graph.last, ControlFlowGraph::kNullBlockRef);
  const auto& last_block = graph.get(graph.last);
  EXPECT_EQ(last_block.id, 1);
  EXPECT_THAT(last_block.next, IsEmpty());
  EXPECT_THAT(last_block.sequences, IsEmpty());
}

TEST_F(ControlFlowGraphTest, ReturnStmt) {
  auto func_call_args = ExprListOf(IntLitExpr{
      .value = "3",
  });
  auto func_call_expr = E(FuncCallExpr({
      .func_name = "bar",
      .args = func_call_args,
  }));
  auto stmts = StmtListOf(ReturnStmt{
      .value = func_call_expr,
  });
  auto graph = BuildControlFlowGraph(FuncDefStmt{
      .name = "foo",
      .result_type = T(BasicType{.name = "Void"}),
      .body =
          {
              .stmts = stmts,
          },
  });

  ASSERT_NE(graph.first, ControlFlowGraph::kNullBlockRef);
  const auto& first_block = graph.get(graph.first);
  EXPECT_EQ(first_block.id, 0);
  EXPECT_THAT(first_block.next, ElementsAre(graph.last));
  ASSERT_EQ(first_block.sequences.size(), 1);
  EXPECT_THAT(first_block.sequences[0].expressions, ElementsAreArray({
                                                        func_call_args[0],
                                                        func_call_expr,
                                                    }));
  EXPECT_THAT(first_block.sequences[0].stmt, stmts[0]);

  ASSERT_NE(graph.last, ControlFlowGraph::kNullBlockRef);
  const auto& last_block = graph.get(graph.last);
  EXPECT_EQ(last_block.id, 1);
  EXPECT_THAT(last_block.next, IsEmpty());
  EXPECT_THAT(last_block.sequences, IsEmpty());
}

TEST_F(ControlFlowGraphTest, VarDeclStmt) {
  auto func_call_stmt_ref = E(FuncCallExpr{
      .func_name = "bar",
      .args = EmptyExprList(),
  });
  auto stmts = StmtListOf(VarDeclStmt{
      .type = T(BasicType{.name = "Int32"}),
      .name = "x",
      .init = func_call_stmt_ref,
  });
  auto graph = BuildControlFlowGraph(FuncDefStmt{
      .name = "foo",
      .body =
          {
              .stmts = stmts,
          },
      .result_type = T(BasicType{.name = "Void"}),
  });

  ASSERT_NE(graph.first, ControlFlowGraph::kNullBlockRef);
  const auto& first_block = graph.get(graph.first);
  EXPECT_EQ(first_block.id, 0);
  EXPECT_THAT(first_block.next, ElementsAre(graph.last));
  ASSERT_EQ(first_block.sequences.size(), 1);
  EXPECT_THAT(first_block.sequences[0].expressions, ElementsAreArray({
                                                        func_call_stmt_ref,
                                                    }));
  EXPECT_THAT(first_block.sequences[0].stmt, stmts[0]);

  ASSERT_NE(graph.last, ControlFlowGraph::kNullBlockRef);
  const auto& last_block = graph.get(graph.last);
  EXPECT_EQ(last_block.id, 1);
  EXPECT_THAT(last_block.next, IsEmpty());
  EXPECT_THAT(last_block.sequences, IsEmpty());
}

TEST_F(ControlFlowGraphTest, BinaryOpExpr) {
  auto lhs_expr = E(IntLitExpr{.value = "2"});
  auto rhs_expr = E(IntLitExpr{.value = "3"});
  auto add_expr = E(BinaryOpExpr{
      .op = BinaryOp::Add,
      .lhs = lhs_expr,
      .rhs = rhs_expr,
  });
  auto stmts = StmtListOf(DoStmt{.expr = add_expr});
  auto graph = BuildControlFlowGraph(FuncDefStmt{
      .name = "foo",
      .body =
          {
              .stmts = stmts,
          },
      .result_type = T(BasicType{.name = "Int32"}),
  });

  ASSERT_NE(graph.first, ControlFlowGraph::kNullBlockRef);
  const auto& first_block = graph.get(graph.first);
  EXPECT_EQ(first_block.id, 0);
  EXPECT_THAT(first_block.next, ElementsAre(graph.last));
  ASSERT_EQ(first_block.sequences.size(), 1);
  EXPECT_THAT(first_block.sequences[0].expressions, ElementsAreArray({
                                                        lhs_expr,
                                                        rhs_expr,
                                                        add_expr,
                                                    }));
  EXPECT_THAT(first_block.sequences[0].stmt, stmts[0]);

  ASSERT_NE(graph.last, ControlFlowGraph::kNullBlockRef);
  const auto& last_block = graph.get(graph.last);
  EXPECT_EQ(last_block.id, 1);
  EXPECT_THAT(last_block.next, IsEmpty());
  EXPECT_THAT(last_block.sequences, IsEmpty());
}

TEST_F(ControlFlowGraphTest, IfStmt) {
  auto add_lhs_expr = E(IntLitExpr{.value = "2"});
  auto add_rhs_expr = E(IntLitExpr{.value = "3"});
  auto add_expr = E(BinaryOpExpr{
      .op = BinaryOp::Add,
      .lhs = add_lhs_expr,
      .rhs = add_rhs_expr,
  });
  auto mul_lhs_expr = E(IntLitExpr{.value = "4"});
  auto mul_rhs_expr = E(IntLitExpr{.value = "5"});
  auto mul_expr = E(BinaryOpExpr{
      .op = BinaryOp::Mul,
      .lhs = mul_lhs_expr,
      .rhs = mul_rhs_expr,
  });
  auto cond_expr = E(BoolLitExpr{.value = "true"});
  auto then_stmts = StmtListOf(DoStmt{.expr = add_expr});
  auto stmts = StmtListOf(
      IfStmt{
          .cond = cond_expr,
          .then_body =
              {
                  .stmts = then_stmts,
              },
      },
      DoStmt{.expr = mul_expr});
  auto graph = BuildControlFlowGraph(FuncDefStmt{
      .name = "foo",
      .body =
          {
              .stmts = stmts,
          },
      .result_type = T(BasicType{.name = "Int32"}),
  });

  ASSERT_NE(graph.first, ControlFlowGraph::kNullBlockRef);
  const auto& first_block = graph.get(graph.first);
  EXPECT_EQ(first_block.id, 0);
  ASSERT_THAT(first_block.next, SizeIs(2));
  ASSERT_THAT(first_block.sequences.size(), 1);
  EXPECT_THAT(first_block.sequences[0].expressions, ElementsAreArray({
                                                        cond_expr,
                                                    }));
  EXPECT_EQ(first_block.branch_cond, cond_expr);

  ASSERT_NE(first_block.next[0], ControlFlowGraph::kNullBlockRef);
  const auto& then_block = graph.get(first_block.next[0]);
  EXPECT_EQ(then_block.id, 3);
  EXPECT_THAT(then_block.next, SizeIs(1));

  ASSERT_NE(then_block.next[0], ControlFlowGraph::kNullBlockRef);
  EXPECT_EQ(then_block.next[0], first_block.next[1]);
  const auto& post_if_block = graph.get(then_block.next[0]);
  EXPECT_EQ(post_if_block.id, 2);
  ASSERT_THAT(post_if_block.sequences.size(), 1);
  EXPECT_THAT(post_if_block.sequences[0].expressions, ElementsAreArray({
                                                          mul_lhs_expr,
                                                          mul_rhs_expr,
                                                          mul_expr,
                                                      }));
  EXPECT_THAT(post_if_block.sequences[0].stmt, stmts[1]);
  EXPECT_THAT(post_if_block.next, ElementsAre(graph.last));

  ASSERT_NE(graph.last, ControlFlowGraph::kNullBlockRef);
  const auto& last_block = graph.get(graph.last);
  EXPECT_EQ(last_block.id, 1);
  EXPECT_THAT(last_block.next, IsEmpty());
  EXPECT_THAT(last_block.sequences, IsEmpty());
}

TEST_F(ControlFlowGraphTest, IfElseStmt) {
  auto add_lhs_expr = E(IntLitExpr{.value = "2"});
  auto add_rhs_expr = E(IntLitExpr{.value = "3"});
  auto add_expr = E(BinaryOpExpr{
      .op = BinaryOp::Add,
      .lhs = add_lhs_expr,
      .rhs = add_rhs_expr,
  });
  auto mul_lhs_expr = E(IntLitExpr{.value = "4"});
  auto mul_rhs_expr = E(IntLitExpr{.value = "5"});
  auto mul_expr = E(BinaryOpExpr{
      .op = BinaryOp::Mul,
      .lhs = mul_lhs_expr,
      .rhs = mul_rhs_expr,
  });
  auto cond_expr = E(BoolLitExpr{.value = "true"});
  auto then_stmts = StmtListOf(DoStmt{.expr = add_expr});
  auto else_stmts = StmtListOf(DoStmt{.expr = mul_expr});
  auto stmts = StmtListOf(IfStmt{
      .cond = cond_expr,
      .then_body =
          {
              .stmts = then_stmts,
          },
      .else_body =
          {
              .stmts = else_stmts,
          },
  });
  auto graph = BuildControlFlowGraph(FuncDefStmt{
      .name = "foo",
      .body =
          {
              .stmts = stmts,
          },
      .result_type = T(BasicType{.name = "Int32"}),
  });

  ASSERT_NE(graph.first, ControlFlowGraph::kNullBlockRef);
  const auto& first_block = graph.get(graph.first);
  EXPECT_EQ(first_block.id, 0);
  ASSERT_THAT(first_block.next, SizeIs(2));
  ASSERT_THAT(first_block.sequences.size(), 1);
  EXPECT_THAT(first_block.sequences[0].expressions, ElementsAreArray({
                                                        cond_expr,
                                                    }));
  EXPECT_EQ(first_block.branch_cond, cond_expr);

  ASSERT_NE(first_block.next[0], ControlFlowGraph::kNullBlockRef);
  const auto& then_block = graph.get(first_block.next[0]);
  EXPECT_EQ(then_block.id, 3);
  EXPECT_THAT(then_block.next, ElementsAre(2));
  ASSERT_THAT(then_block.sequences.size(), 1);
  EXPECT_THAT(then_block.sequences[0].expressions, ElementsAreArray({
                                                       add_lhs_expr,
                                                       add_rhs_expr,
                                                       add_expr,
                                                   }));
  EXPECT_THAT(then_block.sequences[0].stmt, then_stmts[0]);

  ASSERT_NE(first_block.next[1], ControlFlowGraph::kNullBlockRef);
  const auto& else_block = graph.get(first_block.next[1]);
  EXPECT_EQ(else_block.id, 4);
  EXPECT_THAT(else_block.next, ElementsAre(2));
  ASSERT_THAT(then_block.sequences.size(), 1);
  EXPECT_THAT(else_block.sequences[0].expressions, ElementsAreArray({
                                                       mul_lhs_expr,
                                                       mul_rhs_expr,
                                                       mul_expr,
                                                   }));
  EXPECT_THAT(else_block.sequences[0].stmt, else_stmts[0]);

  ASSERT_NE(graph.last, ControlFlowGraph::kNullBlockRef);
  const auto& last_block = graph.get(graph.last);
  EXPECT_EQ(last_block.id, 1);
  EXPECT_THAT(last_block.next, IsEmpty());
  EXPECT_THAT(last_block.sequences, IsEmpty());
}

TEST_F(ControlFlowGraphTest, VarDecl) {
  auto int_lit = E(IntLitExpr{
      .value = "3",
  });
  auto ident_expr = E(IdentExpr{
      .name = "x",
  });
  auto stmts = StmtListOf(
      VarDeclStmt{
          .name = "x",
          .type = T(BasicType{.name = "Int32"}),
          .init = int_lit,
      },
      ReturnStmt{
          .value = ident_expr,
      });
  auto graph = BuildControlFlowGraph(FuncDefStmt{
      .name = "foo",
      .result_type = T(BasicType{.name = "Int32"}),
      .body =
          {
              .stmts = stmts,
          },
  });

  ASSERT_NE(graph.first, ControlFlowGraph::kNullBlockRef);
  const auto& first_block = graph.get(graph.first);
  EXPECT_EQ(first_block.id, 0);
  EXPECT_THAT(first_block.next, ElementsAre(graph.last));
  ASSERT_THAT(first_block.sequences.size(), 2);
  EXPECT_THAT(first_block.sequences[0].expressions, ElementsAreArray({
                                                        int_lit,
                                                    }));
  EXPECT_THAT(first_block.sequences[0].stmt, stmts[0]);
  EXPECT_THAT(first_block.sequences[1].expressions, ElementsAreArray({
                                                        ident_expr,
                                                    }));
  EXPECT_THAT(first_block.sequences[1].stmt, stmts[1]);

  ASSERT_NE(graph.last, ControlFlowGraph::kNullBlockRef);
  const auto& last_block = graph.get(graph.last);
  EXPECT_EQ(last_block.id, 1);
  EXPECT_THAT(last_block.next, IsEmpty());
  EXPECT_THAT(last_block.sequences, IsEmpty());
}

TEST_F(ControlFlowGraphTest, Loop) {
  auto add_lhs_expr = E(IntLitExpr{.value = "2"});
  auto add_rhs_expr = E(IntLitExpr{.value = "3"});
  auto add_expr = E(BinaryOpExpr{
      .op = BinaryOp::Add,
      .lhs = add_lhs_expr,
      .rhs = add_rhs_expr,
  });
  auto loop_stmts = StmtListOf(DoStmt{.expr = add_expr});
  auto stmts = StmtListOf(LoopStmt{
      .body =
          {
              .stmts = loop_stmts,
          },
  });
  auto graph = BuildControlFlowGraph(FuncDefStmt{
      .name = "foo",
      .body =
          {
              .stmts = stmts,
          },
      .result_type = T(BasicType{.name = "Int32"}),
  });

  ASSERT_NE(graph.first, ControlFlowGraph::kNullBlockRef);
  const auto& first_block = graph.get(graph.first);
  EXPECT_EQ(first_block.id, 0);
  ASSERT_THAT(first_block.next, ElementsAre(3));
  EXPECT_THAT(first_block.sequences, IsEmpty());

  ASSERT_NE(first_block.next[0], ControlFlowGraph::kNullBlockRef);
  const auto& loop_block = graph.get(first_block.next[0]);
  EXPECT_EQ(loop_block.id, 3);
  EXPECT_THAT(loop_block.next, ElementsAre(3));
  ASSERT_THAT(loop_block.sequences.size(), 1);
  EXPECT_THAT(loop_block.sequences[0].expressions, ElementsAreArray({
                                                       add_lhs_expr,
                                                       add_rhs_expr,
                                                       add_expr,
                                                   }));
  EXPECT_THAT(loop_block.sequences[0].stmt, loop_stmts[0]);

  ASSERT_NE(graph.last, ControlFlowGraph::kNullBlockRef);
  const auto& last_block = graph.get(graph.last);
  EXPECT_EQ(last_block.id, 1);
  EXPECT_THAT(last_block.next, IsEmpty());
  EXPECT_THAT(last_block.sequences, IsEmpty());
}

TEST_F(ControlFlowGraphTest, SingleLoopAndBreak) {
  auto n_var_init_ref = E(IntLitExpr{
      .value = "0",
  });
  auto if_cond_lhs_ref = E(IdentExpr{.name = "n"});
  auto if_cond_rhs_ref = E(IntLitExpr{.value = "3"});
  auto if_cond_ref = E(BinaryOpExpr{
      .op = BinaryOp::Gt,
      .lhs = if_cond_lhs_ref,
      .rhs = if_cond_rhs_ref,
  });
  auto then_stmts = StmtListOf(BreakStmt{});
  auto var_assign_lhs_ref = E(IdentExpr{.name = "n"});
  auto var_assign_rhs_ref = E(IntLitExpr{.value = "1"});
  auto binary_op_expr_ref = E(BinaryOpExpr{
      .op = BinaryOp::Add,
      .lhs = var_assign_lhs_ref,
      .rhs = var_assign_rhs_ref,
  });
  auto loop_stmts = StmtListOf(
      IfStmt{
          .cond = if_cond_ref,
          .then_body =
              {
                  .stmts = then_stmts,
              },
      },
      VarAssignStmt{
          .name = "n",
          .expr = binary_op_expr_ref,
      });
  auto return_value_ref = E(IdentExpr{.name = "n"});
  auto stmts = StmtListOf(
      VarDeclStmt{
          .type = T(BasicType{.name = "Int32"}),
          .name = "n",
          .init = n_var_init_ref,
      },
      LoopStmt{
          .body =
              {
                  .stmts = loop_stmts,
              },
      },
      ReturnStmt{
          .value = return_value_ref,
      });
  auto graph = BuildControlFlowGraph(FuncDefStmt{
      .name = "foo",
      .body =
          {
              .stmts = stmts,
          },
      .result_type = T(BasicType{.name = "Int32"}),
  });

  ASSERT_NE(graph.first, ControlFlowGraph::kNullBlockRef);
  const auto& first_block = graph.get(graph.first);
  EXPECT_EQ(first_block.id, 0);
  ASSERT_THAT(first_block.next, ElementsAre(3));
  ASSERT_THAT(first_block.sequences.size(), 1);
  EXPECT_THAT(first_block.sequences[0].expressions, ElementsAreArray({
                                                        n_var_init_ref,
                                                    }));
  EXPECT_THAT(first_block.sequences[0].stmt, stmts[0]);

  ASSERT_NE(first_block.next[0], ControlFlowGraph::kNullBlockRef);
  const auto& loop_block = graph.get(first_block.next[0]);
  EXPECT_EQ(loop_block.id, 3);
  EXPECT_THAT(loop_block.next, ElementsAre(5, 4));
  ASSERT_THAT(loop_block.sequences.size(), 1);
  EXPECT_THAT(loop_block.sequences[0].expressions, ElementsAreArray({
                                                       if_cond_lhs_ref,
                                                       if_cond_rhs_ref,
                                                       if_cond_ref,
                                                   }));

  ASSERT_NE(loop_block.next[0], ControlFlowGraph::kNullBlockRef);
  const auto& if_then_block = graph.get(loop_block.next[0]);
  EXPECT_EQ(if_then_block.id, 5);
  EXPECT_THAT(if_then_block.next, ElementsAre(2));
  ASSERT_THAT(if_then_block.sequences.size(), 1);
  EXPECT_THAT(if_then_block.sequences[0].stmt, then_stmts[0]);

  ASSERT_NE(loop_block.next[1], ControlFlowGraph::kNullBlockRef);
  const auto& post_if_block = graph.get(loop_block.next[1]);
  EXPECT_EQ(post_if_block.id, 4);
  EXPECT_THAT(post_if_block.next, ElementsAre(3));
  ASSERT_THAT(post_if_block.sequences.size(), 1);
  EXPECT_THAT(post_if_block.sequences[0].expressions, ElementsAreArray({
                                                          var_assign_lhs_ref,
                                                          var_assign_rhs_ref,
                                                          binary_op_expr_ref,
                                                      }));
  EXPECT_THAT(post_if_block.sequences[0].stmt, loop_stmts[1]);

  ASSERT_NE(if_then_block.next[0], ControlFlowGraph::kNullBlockRef);
  const auto& post_loop_block = graph.get(if_then_block.next[0]);
  EXPECT_EQ(post_loop_block.id, 2);
  EXPECT_THAT(post_loop_block.next, ElementsAre(graph.last));
  ASSERT_THAT(post_loop_block.sequences.size(), 1);
  EXPECT_THAT(post_loop_block.sequences[0].expressions, ElementsAreArray({
                                                            return_value_ref,
                                                        }));
  EXPECT_THAT(post_loop_block.sequences[0].stmt, stmts[2]);

  ASSERT_NE(graph.last, ControlFlowGraph::kNullBlockRef);
  const auto& last_block = graph.get(graph.last);
  EXPECT_EQ(last_block.id, 1);
  EXPECT_THAT(last_block.next, IsEmpty());
  EXPECT_THAT(last_block.sequences, IsEmpty());
}

}  // namespace
}  // namespace lucid
