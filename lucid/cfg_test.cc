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
    return ::lucid::BuildControlFlowGraph(arena_, func_def);
  }
};

TEST_F(ControlFlowGraphTest, FunctionName) {
  auto graph = BuildControlFlowGraph(FuncDefStmt{
      .name = "foo",
      .result_type = A(BasicType{.name = "Void"}),
  });

  EXPECT_EQ(graph.func_name, "foo");
}

TEST_F(ControlFlowGraphTest, EmptyFunction) {
  auto graph = BuildControlFlowGraph(FuncDefStmt{
      .name = "foo",
      .result_type = A(BasicType{.name = "Void"}),
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
  auto func_call_expr = A(FuncCallExpr({
      .func_name = "bar",
  }));
  auto do_stmt = A(DoStmt{.expr = func_call_expr});
  auto graph = BuildControlFlowGraph(FuncDefStmt{
      .name = "foo",
      .result_type = A(BasicType{.name = "Void"}),
      .body = {{
          do_stmt,
      }},
  });

  ASSERT_NE(graph.first, ControlFlowGraph::kNullBlockRef);
  const auto& first_block = graph.get(graph.first);
  EXPECT_EQ(first_block.id, 0);
  EXPECT_THAT(first_block.next, ElementsAre(graph.last));
  ASSERT_EQ(first_block.sequences.size(), 1);
  EXPECT_THAT(first_block.sequences[0].expressions, ElementsAreArray({
                                                        func_call_expr,
                                                    }));
  EXPECT_THAT(first_block.sequences[0].stmt, do_stmt);

  ASSERT_NE(graph.last, ControlFlowGraph::kNullBlockRef);
  const auto& last_block = graph.get(graph.last);
  EXPECT_EQ(last_block.id, 1);
  EXPECT_THAT(last_block.next, IsEmpty());
  EXPECT_THAT(last_block.sequences, IsEmpty());
}

TEST_F(ControlFlowGraphTest, FuncCallExprWithArgs) {
  auto arg1_expr = A(IntLitExpr{
      .value = "3",
  });
  auto arg2_expr = A(IntLitExpr{
      .value = "7",
  });
  auto baz_func_call_expr = A(FuncCallExpr({
      .func_name = "baz",
      .arguments =
          {
              arg2_expr,
          },
  }));
  auto func_call_expr = A(FuncCallExpr({
      .func_name = "bar",
      .arguments =
          {
              arg1_expr,
              baz_func_call_expr,
          },
  }));
  auto do_stmt = A(DoStmt{.expr = func_call_expr});
  auto graph = BuildControlFlowGraph(FuncDefStmt{
      .name = "foo",
      .result_type = A(BasicType{.name = "Void"}),
      .body = {{
          do_stmt,
      }},
  });

  ASSERT_NE(graph.first, ControlFlowGraph::kNullBlockRef);
  const auto& first_block = graph.get(graph.first);
  EXPECT_EQ(first_block.id, 0);
  EXPECT_THAT(first_block.next, ElementsAre(graph.last));
  ASSERT_EQ(first_block.sequences.size(), 1);
  EXPECT_THAT(first_block.sequences[0].expressions, ElementsAreArray({
                                                        arg1_expr,
                                                        arg2_expr,
                                                        baz_func_call_expr,
                                                        func_call_expr,
                                                    }));
  EXPECT_THAT(first_block.sequences[0].stmt, do_stmt);

  ASSERT_NE(graph.last, ControlFlowGraph::kNullBlockRef);
  const auto& last_block = graph.get(graph.last);
  EXPECT_EQ(last_block.id, 1);
  EXPECT_THAT(last_block.next, IsEmpty());
  EXPECT_THAT(last_block.sequences, IsEmpty());
}

TEST_F(ControlFlowGraphTest, ReturnStmt) {
  auto arg1_expr = A(IntLitExpr{
      .value = "3",
  });
  auto func_call_expr = A(FuncCallExpr({
      .func_name = "bar",
      .arguments =
          {
              arg1_expr,
          },
  }));
  auto return_stmt = A(ReturnStmt{
      .value = func_call_expr,
  });
  auto graph = BuildControlFlowGraph(FuncDefStmt{
      .name = "foo",
      .result_type = A(BasicType{.name = "Void"}),
      .body = {{
          return_stmt,
      }},
  });

  ASSERT_NE(graph.first, ControlFlowGraph::kNullBlockRef);
  const auto& first_block = graph.get(graph.first);
  EXPECT_EQ(first_block.id, 0);
  EXPECT_THAT(first_block.next, ElementsAre(graph.last));
  ASSERT_EQ(first_block.sequences.size(), 1);
  EXPECT_THAT(first_block.sequences[0].expressions, ElementsAreArray({
                                                        arg1_expr,
                                                        func_call_expr,
                                                    }));
  EXPECT_THAT(first_block.sequences[0].stmt, return_stmt);

  ASSERT_NE(graph.last, ControlFlowGraph::kNullBlockRef);
  const auto& last_block = graph.get(graph.last);
  EXPECT_EQ(last_block.id, 1);
  EXPECT_THAT(last_block.next, IsEmpty());
  EXPECT_THAT(last_block.sequences, IsEmpty());
}

TEST_F(ControlFlowGraphTest, VarDeclStmt) {
  auto func_call_stmt_ref = A(FuncCallExpr{.func_name = "bar"});
  auto x_var_decl_ref = A(VarDeclStmt{
      .type = A(BasicType{.name = "Int32"}),
      .name = "x",
      .init = func_call_stmt_ref,
  });
  auto graph = BuildControlFlowGraph(FuncDefStmt{
      .name = "foo",
      .body = {{
          x_var_decl_ref,
      }},
      .result_type = A(BasicType{.name = "Void"}),
  });

  ASSERT_NE(graph.first, ControlFlowGraph::kNullBlockRef);
  const auto& first_block = graph.get(graph.first);
  EXPECT_EQ(first_block.id, 0);
  EXPECT_THAT(first_block.next, ElementsAre(graph.last));
  ASSERT_EQ(first_block.sequences.size(), 1);
  EXPECT_THAT(first_block.sequences[0].expressions, ElementsAreArray({
                                                        func_call_stmt_ref,
                                                    }));
  EXPECT_THAT(first_block.sequences[0].stmt, x_var_decl_ref);

  ASSERT_NE(graph.last, ControlFlowGraph::kNullBlockRef);
  const auto& last_block = graph.get(graph.last);
  EXPECT_EQ(last_block.id, 1);
  EXPECT_THAT(last_block.next, IsEmpty());
  EXPECT_THAT(last_block.sequences, IsEmpty());
}

TEST_F(ControlFlowGraphTest, BinaryOpExpr) {
  auto lhs_expr = A(IntLitExpr{.value = "2"});
  auto rhs_expr = A(IntLitExpr{.value = "3"});
  auto add_expr = A(BinaryOpExpr{
      .op = BinaryOp::Add,
      .lhs = lhs_expr,
      .rhs = rhs_expr,
  });
  auto do_stmt = A(DoStmt{.expr = add_expr});
  auto graph = BuildControlFlowGraph(FuncDefStmt{
      .name = "foo",
      .body = {{
          do_stmt,
      }},
      .result_type = A(BasicType{.name = "Int32"}),
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
  EXPECT_THAT(first_block.sequences[0].stmt, do_stmt);

  ASSERT_NE(graph.last, ControlFlowGraph::kNullBlockRef);
  const auto& last_block = graph.get(graph.last);
  EXPECT_EQ(last_block.id, 1);
  EXPECT_THAT(last_block.next, IsEmpty());
  EXPECT_THAT(last_block.sequences, IsEmpty());
}

TEST_F(ControlFlowGraphTest, IfStmt) {
  auto add_lhs_expr = A(IntLitExpr{.value = "2"});
  auto add_rhs_expr = A(IntLitExpr{.value = "3"});
  auto add_expr = A(BinaryOpExpr{
      .op = BinaryOp::Add,
      .lhs = add_lhs_expr,
      .rhs = add_rhs_expr,
  });
  auto mul_lhs_expr = A(IntLitExpr{.value = "4"});
  auto mul_rhs_expr = A(IntLitExpr{.value = "5"});
  auto mul_expr = A(BinaryOpExpr{
      .op = BinaryOp::Mul,
      .lhs = mul_lhs_expr,
      .rhs = mul_rhs_expr,
  });
  auto cond_expr = A(BoolLitExpr{.value = "true"});
  auto if_stmt = A(IfStmt{
      .cond = cond_expr,
      .then_body = {{
          add_expr,
      }},
  });
  auto do_stmt = A(DoStmt{.expr = mul_expr});
  auto graph = BuildControlFlowGraph(FuncDefStmt{
      .name = "foo",
      .body = {{
          if_stmt,
          do_stmt,
      }},
      .result_type = A(BasicType{.name = "Int32"}),
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
  EXPECT_THAT(post_if_block.sequences[0].stmt, do_stmt);
  EXPECT_THAT(post_if_block.next, ElementsAre(graph.last));

  ASSERT_NE(graph.last, ControlFlowGraph::kNullBlockRef);
  const auto& last_block = graph.get(graph.last);
  EXPECT_EQ(last_block.id, 1);
  EXPECT_THAT(last_block.next, IsEmpty());
  EXPECT_THAT(last_block.sequences, IsEmpty());
}

TEST_F(ControlFlowGraphTest, IfElseStmt) {
  auto add_lhs_expr = A(IntLitExpr{.value = "2"});
  auto add_rhs_expr = A(IntLitExpr{.value = "3"});
  auto add_expr = A(BinaryOpExpr{
      .op = BinaryOp::Add,
      .lhs = add_lhs_expr,
      .rhs = add_rhs_expr,
  });
  auto mul_lhs_expr = A(IntLitExpr{.value = "4"});
  auto mul_rhs_expr = A(IntLitExpr{.value = "5"});
  auto mul_expr = A(BinaryOpExpr{
      .op = BinaryOp::Mul,
      .lhs = mul_lhs_expr,
      .rhs = mul_rhs_expr,
  });
  auto cond_expr = A(BoolLitExpr{.value = "true"});
  auto then_do_stmt = A(DoStmt{.expr = add_expr});
  auto else_do_stmt = A(DoStmt{.expr = mul_expr});
  auto if_stmt = A(IfStmt{
      .cond = cond_expr,
      .then_body = {{
          then_do_stmt,
      }},
      .else_body = {{
          else_do_stmt,
      }},
  });
  auto graph = BuildControlFlowGraph(FuncDefStmt{
      .name = "foo",
      .body = {{
          if_stmt,
      }},
      .result_type = A(BasicType{.name = "Int32"}),
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
  EXPECT_THAT(then_block.sequences[0].stmt, then_do_stmt);

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
  EXPECT_THAT(else_block.sequences[0].stmt, else_do_stmt);

  ASSERT_NE(graph.last, ControlFlowGraph::kNullBlockRef);
  const auto& last_block = graph.get(graph.last);
  EXPECT_EQ(last_block.id, 1);
  EXPECT_THAT(last_block.next, IsEmpty());
  EXPECT_THAT(last_block.sequences, IsEmpty());
}

TEST_F(ControlFlowGraphTest, VarDecl) {
  auto int_lit = A(IntLitExpr{
      .value = "3",
  });
  auto const_decl_stmt = A(VarDeclStmt{
      .name = "x",
      .type = A(BasicType{.name = "Int32"}),
      .init = int_lit,
  });
  auto ident_expr = A(IdentExpr{
      .name = "x",
  });
  auto return_stmt = A(ReturnStmt{
      .value = ident_expr,
  });
  auto graph = BuildControlFlowGraph(FuncDefStmt{
      .name = "foo",
      .result_type = A(BasicType{.name = "Int32"}),
      .body = {{
          const_decl_stmt,
          return_stmt,
      }},
  });

  ASSERT_NE(graph.first, ControlFlowGraph::kNullBlockRef);
  const auto& first_block = graph.get(graph.first);
  EXPECT_EQ(first_block.id, 0);
  EXPECT_THAT(first_block.next, ElementsAre(graph.last));
  ASSERT_THAT(first_block.sequences.size(), 2);
  EXPECT_THAT(first_block.sequences[0].expressions, ElementsAreArray({
                                                        int_lit,
                                                    }));
  EXPECT_THAT(first_block.sequences[0].stmt, const_decl_stmt);
  EXPECT_THAT(first_block.sequences[1].expressions, ElementsAreArray({
                                                        ident_expr,
                                                    }));
  EXPECT_THAT(first_block.sequences[1].stmt, return_stmt);

  ASSERT_NE(graph.last, ControlFlowGraph::kNullBlockRef);
  const auto& last_block = graph.get(graph.last);
  EXPECT_EQ(last_block.id, 1);
  EXPECT_THAT(last_block.next, IsEmpty());
  EXPECT_THAT(last_block.sequences, IsEmpty());
}

TEST_F(ControlFlowGraphTest, Loop) {
  auto add_lhs_expr = A(IntLitExpr{.value = "2"});
  auto add_rhs_expr = A(IntLitExpr{.value = "3"});
  auto add_expr = A(BinaryOpExpr{
      .op = BinaryOp::Add,
      .lhs = add_lhs_expr,
      .rhs = add_rhs_expr,
  });
  auto do_stmt = A(DoStmt{.expr = add_expr});
  auto loop_stmt = A(LoopStmt{
      .body = {{
          do_stmt,
      }},
  });
  auto graph = BuildControlFlowGraph(FuncDefStmt{
      .name = "foo",
      .body = {{
          loop_stmt,
      }},
      .result_type = A(BasicType{.name = "Int32"}),
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
  EXPECT_THAT(loop_block.sequences[0].stmt, do_stmt);

  ASSERT_NE(graph.last, ControlFlowGraph::kNullBlockRef);
  const auto& last_block = graph.get(graph.last);
  EXPECT_EQ(last_block.id, 1);
  EXPECT_THAT(last_block.next, IsEmpty());
  EXPECT_THAT(last_block.sequences, IsEmpty());
}

TEST_F(ControlFlowGraphTest, SingleLoopAndBreak) {
  auto n_var_init_ref = A(IntLitExpr{
      .value = "0",
  });
  auto n_var_decl_ref = A(VarDeclStmt{
      .type = A(BasicType{.name = "Int32"}),
      .name = "n",
      .init = n_var_init_ref,
  });
  auto if_cond_lhs_ref = A(IdentExpr{.name = "n"});
  auto if_cond_rhs_ref = A(IntLitExpr{.value = "3"});
  auto if_cond_ref = A(BinaryOpExpr{
      .op = BinaryOp::Gt,
      .lhs = if_cond_lhs_ref,
      .rhs = if_cond_rhs_ref,
  });
  auto break_stmt_ref = A(BreakStmt{});
  auto if_stmt = A(IfStmt{
      .cond = if_cond_ref,
      .then_body = {{
          break_stmt_ref,
      }},
  });
  auto var_assign_lhs_ref = A(IdentExpr{.name = "n"});
  auto var_assign_rhs_ref = A(IntLitExpr{.value = "1"});
  auto binary_op_expr_ref = A(BinaryOpExpr{
      .op = BinaryOp::Add,
      .lhs = var_assign_lhs_ref,
      .rhs = var_assign_rhs_ref,
  });
  auto var_assign_stmt_ref = A(VarAssignStmt{
      .name = "n",
      .expr = binary_op_expr_ref,
  });
  auto loop_stmt = A(LoopStmt{
      .body = {{
          if_stmt,
          var_assign_stmt_ref,
      }},
  });
  auto return_value_ref = A(IdentExpr{.name = "n"});
  auto return_stmt = A(ReturnStmt{
      .value = return_value_ref,
  });
  auto graph = BuildControlFlowGraph(FuncDefStmt{
      .name = "foo",
      .body = {{
          n_var_decl_ref,
          loop_stmt,
          return_stmt,
      }},
      .result_type = A(BasicType{.name = "Int32"}),
  });

  ASSERT_NE(graph.first, ControlFlowGraph::kNullBlockRef);
  const auto& first_block = graph.get(graph.first);
  EXPECT_EQ(first_block.id, 0);
  ASSERT_THAT(first_block.next, ElementsAre(3));
  ASSERT_THAT(first_block.sequences.size(), 1);
  EXPECT_THAT(first_block.sequences[0].expressions, ElementsAreArray({
                                                        n_var_init_ref,
                                                    }));
  EXPECT_THAT(first_block.sequences[0].stmt, n_var_decl_ref);

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
  EXPECT_THAT(if_then_block.sequences[0].stmt, break_stmt_ref);

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
  EXPECT_THAT(post_if_block.sequences[0].stmt, var_assign_stmt_ref);

  ASSERT_NE(if_then_block.next[0], ControlFlowGraph::kNullBlockRef);
  const auto& post_loop_block = graph.get(if_then_block.next[0]);
  EXPECT_EQ(post_loop_block.id, 2);
  EXPECT_THAT(post_loop_block.next, ElementsAre(graph.last));
  ASSERT_THAT(post_loop_block.sequences.size(), 1);
  EXPECT_THAT(post_loop_block.sequences[0].expressions, ElementsAreArray({
                                                            return_value_ref,
                                                        }));
  EXPECT_THAT(post_loop_block.sequences[0].stmt, return_stmt);

  ASSERT_NE(graph.last, ControlFlowGraph::kNullBlockRef);
  const auto& last_block = graph.get(graph.last);
  EXPECT_EQ(last_block.id, 1);
  EXPECT_THAT(last_block.next, IsEmpty());
  EXPECT_THAT(last_block.sequences, IsEmpty());
}

}  // namespace
}  // namespace lucid
