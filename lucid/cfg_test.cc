#include "lucid/cfg.h"

#include "gmock/gmock.h"
#include "gtest/gtest.h"
#include "lucid/ast.h"
#include "lucid/ast_fixture.h"

namespace lucid {
namespace {

using ::testing::ElementsAre;
using ::testing::ElementsAreArray;
using ::testing::IsEmpty;
using ::testing::Optional;
using ::testing::SizeIs;

class ControlFlowGraphTest : public testing::Test, public AstFixture {
 protected:
  ControlFlowGraph BuildControlFlowGraph(FuncDefStmt func_def) {
    return ::lucid::BuildControlFlowGraph(ctx_, func_def);
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

  ASSERT_EQ(graph.blocks().Size(), 2);
  const auto& first_block = graph.blocks().Get(0);
  const auto& last_block = graph.blocks().Get(1);

  EXPECT_EQ(graph.first, first_block.id);
  EXPECT_EQ(first_block.id, 0);
  EXPECT_THAT(first_block.next, ElementsAre(last_block.id));
  EXPECT_THAT(first_block.preds, IsEmpty());
  EXPECT_THAT(first_block.sequences, IsEmpty());

  EXPECT_EQ(graph.last, last_block.id);
  EXPECT_EQ(last_block.id, 1);
  EXPECT_THAT(last_block.next, IsEmpty());
  EXPECT_THAT(last_block.preds, ElementsAre(first_block.id));
  EXPECT_THAT(last_block.sequences, IsEmpty());
}

TEST_F(ControlFlowGraphTest, FuncCallExprWithoutArgs) {
  auto func_call_expr = E(FuncCallExpr({
      .func_name = "bar",
      .args = EmptyList<Expr>(),
  }));
  auto do_stmt = S(DoStmt{.expr = func_call_expr});
  auto graph = BuildControlFlowGraph(FuncDefStmt{
      .name = "foo",
      .result_type = T(BasicType{.name = "Void"}),
      .stmts = StmtListOf({
          do_stmt,
      }),
  });

  ASSERT_EQ(graph.blocks().Size(), 2);
  const auto& first_block = graph.blocks().Get(0);
  const auto& last_block = graph.blocks().Get(1);

  EXPECT_EQ(graph.first, first_block.id);
  EXPECT_EQ(first_block.id, 0);
  EXPECT_THAT(first_block.next, ElementsAre(last_block.id));
  EXPECT_THAT(first_block.preds, IsEmpty());
  ASSERT_EQ(first_block.sequences.size(), 1);
  EXPECT_THAT(first_block.sequences[0].expressions, ElementsAreArray({
                                                        func_call_expr,
                                                    }));
  EXPECT_THAT(first_block.sequences[0].stmt, Optional(StmtEquivTo(do_stmt)));

  EXPECT_EQ(graph.last, last_block.id);
  EXPECT_EQ(last_block.id, 1);
  EXPECT_THAT(last_block.next, IsEmpty());
  EXPECT_THAT(last_block.preds, ElementsAre(first_block.id));
  EXPECT_THAT(last_block.sequences, IsEmpty());
}

TEST_F(ControlFlowGraphTest, FuncCallExprWithArgs) {
  auto baz_arg1_expr = IntLitExpr{
      .value = "3",
  };
  auto baz_arg2_expr = IntLitExpr{
      .value = "7",
  };
  auto baz_func_call_args = ExprListOf({E(baz_arg1_expr), E(baz_arg2_expr)});
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
  auto qux_func_call_args = ExprListOf({E(qux_arg1_expr), E(qux_arg2_expr)});
  auto qux_func_call_expr = FuncCallExpr({
      .func_name = "qux",
      .args = qux_func_call_args,
  });

  auto bar_func_call_args =
      ExprListOf({E(baz_func_call_expr), E(qux_func_call_expr)});
  auto bar_func_call_expr = E(FuncCallExpr({
      .func_name = "bar",
      .args = bar_func_call_args,
  }));
  auto do_stmt = S(DoStmt{.expr = bar_func_call_expr});
  auto graph = BuildControlFlowGraph(FuncDefStmt{
      .name = "foo",
      .result_type = T(BasicType{.name = "Void"}),
      .stmts = StmtListOf({
          do_stmt,
      }),
  });

  ASSERT_EQ(graph.blocks().Size(), 2);
  const auto& first_block = graph.blocks().Get(0);
  const auto& last_block = graph.blocks().Get(1);

  EXPECT_EQ(graph.first, first_block.id);
  EXPECT_EQ(first_block.id, 0);
  EXPECT_THAT(first_block.next, ElementsAre(last_block.id));
  EXPECT_THAT(first_block.preds, IsEmpty());
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
  EXPECT_THAT(first_block.sequences[0].stmt, Optional(StmtEquivTo(do_stmt)));

  EXPECT_EQ(graph.last, last_block.id);
  EXPECT_EQ(last_block.id, 1);
  EXPECT_THAT(last_block.next, IsEmpty());
  EXPECT_THAT(last_block.preds, ElementsAre(first_block.id));
  EXPECT_THAT(last_block.sequences, IsEmpty());
}

TEST_F(ControlFlowGraphTest, ReturnStmt) {
  auto func_call_args = ExprListOf({
      E(IntLitExpr{
          .value = "3",
      }),
  });
  auto func_call_expr = E(FuncCallExpr({
      .func_name = "bar",
      .args = func_call_args,
  }));
  auto return_stmt = S(ReturnStmt{
      .value = func_call_expr,
  });
  auto graph = BuildControlFlowGraph(FuncDefStmt{
      .name = "foo",
      .result_type = T(BasicType{.name = "Void"}),
      .stmts = StmtListOf({return_stmt}),
  });

  ASSERT_EQ(graph.blocks().Size(), 2);
  const auto& first_block = graph.blocks().Get(0);
  const auto& last_block = graph.blocks().Get(1);

  EXPECT_EQ(graph.first, first_block.id);
  EXPECT_EQ(first_block.id, 0);
  EXPECT_THAT(first_block.next, ElementsAre(last_block.id));
  EXPECT_THAT(first_block.preds, IsEmpty());
  ASSERT_EQ(first_block.sequences.size(), 1);
  EXPECT_THAT(first_block.sequences[0].expressions, ElementsAreArray({
                                                        func_call_args[0],
                                                        func_call_expr,
                                                    }));
  EXPECT_THAT(first_block.sequences[0].stmt,
              Optional(StmtEquivTo(return_stmt)));

  EXPECT_EQ(graph.last, last_block.id);
  EXPECT_EQ(last_block.id, 1);
  EXPECT_THAT(last_block.next, IsEmpty());
  EXPECT_THAT(last_block.preds, ElementsAre(first_block.id));
  EXPECT_THAT(last_block.sequences, IsEmpty());
}

TEST_F(ControlFlowGraphTest, VarDeclStmt) {
  auto func_call_stmt_ref = E(FuncCallExpr{
      .func_name = "bar",
      .args = EmptyList<Expr>(),
  });
  auto var_decl_stmt = S(VarDeclStmt{
      .type_constraint = T(BasicType{.name = "Int32"}),
      .name = "x",
      .init = func_call_stmt_ref,
  });
  auto graph = BuildControlFlowGraph(FuncDefStmt{
      .name = "foo",
      .stmts = StmtListOf({var_decl_stmt}),
      .result_type = T(BasicType{.name = "Void"}),
  });

  ASSERT_EQ(graph.blocks().Size(), 2);
  const auto& first_block = graph.blocks().Get(0);
  const auto& last_block = graph.blocks().Get(1);

  EXPECT_EQ(graph.first, first_block.id);
  EXPECT_EQ(first_block.id, 0);
  EXPECT_THAT(first_block.next, ElementsAre(last_block.id));
  EXPECT_THAT(first_block.preds, IsEmpty());
  ASSERT_EQ(first_block.sequences.size(), 1);
  EXPECT_THAT(first_block.sequences[0].expressions, ElementsAreArray({
                                                        func_call_stmt_ref,
                                                    }));
  EXPECT_THAT(first_block.sequences[0].stmt,
              Optional(StmtEquivTo(var_decl_stmt)));

  EXPECT_EQ(graph.last, last_block.id);
  EXPECT_EQ(last_block.id, 1);
  EXPECT_THAT(last_block.next, IsEmpty());
  EXPECT_THAT(last_block.preds, ElementsAre(first_block.id));
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
  auto do_stmt = S(DoStmt{.expr = add_expr});
  auto graph = BuildControlFlowGraph(FuncDefStmt{
      .name = "foo",
      .stmts = StmtListOf({do_stmt}),
      .result_type = T(BasicType{.name = "Int32"}),
  });

  ASSERT_EQ(graph.blocks().Size(), 2);
  const auto& first_block = graph.blocks().Get(0);
  const auto& last_block = graph.blocks().Get(1);

  EXPECT_EQ(graph.first, first_block.id);
  EXPECT_EQ(first_block.id, 0);
  EXPECT_THAT(first_block.next, ElementsAre(last_block.id));
  EXPECT_THAT(first_block.preds, IsEmpty());
  ASSERT_EQ(first_block.sequences.size(), 1);
  EXPECT_THAT(first_block.sequences[0].expressions, ElementsAreArray({
                                                        lhs_expr,
                                                        rhs_expr,
                                                        add_expr,
                                                    }));
  EXPECT_THAT(first_block.sequences[0].stmt, Optional(StmtEquivTo(do_stmt)));

  EXPECT_EQ(graph.last, last_block.id);
  EXPECT_EQ(last_block.id, 1);
  EXPECT_THAT(last_block.next, IsEmpty());
  EXPECT_THAT(last_block.preds, ElementsAre(first_block.id));
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
  auto do_mul_stmt = S(DoStmt{.expr = mul_expr});
  auto graph = BuildControlFlowGraph(FuncDefStmt{
      .name = "foo",
      .stmts = StmtListOf({
          S(IfStmt{
              .cond = cond_expr,
              .then_stmts = StmtListOf({
                  S(DoStmt{.expr = add_expr}),
              }),
          }),
          do_mul_stmt,
      }),
      .result_type = T(BasicType{.name = "Int32"}),
  });

  ASSERT_EQ(graph.blocks().Size(), 4);
  const auto& first_block = graph.blocks().Get(0);
  const auto& last_block = graph.blocks().Get(1);
  const auto& post_if_block = graph.blocks().Get(2);
  const auto& then_block = graph.blocks().Get(3);

  EXPECT_EQ(graph.first, first_block.id);
  EXPECT_EQ(first_block.id, 0);
  EXPECT_THAT(first_block.next, ElementsAre(then_block.id, post_if_block.id));
  EXPECT_THAT(first_block.preds, IsEmpty());
  ASSERT_THAT(first_block.sequences.size(), 1);
  EXPECT_THAT(first_block.sequences[0].expressions, ElementsAreArray({
                                                        cond_expr,
                                                    }));
  EXPECT_EQ(first_block.branch_cond, cond_expr);

  EXPECT_EQ(then_block.id, 3);
  EXPECT_THAT(then_block.next, ElementsAre(post_if_block.id));
  EXPECT_THAT(then_block.preds, ElementsAre(first_block.id));

  EXPECT_EQ(post_if_block.id, 2);
  ASSERT_THAT(post_if_block.sequences.size(), 1);
  EXPECT_THAT(post_if_block.sequences[0].expressions, ElementsAreArray({
                                                          mul_lhs_expr,
                                                          mul_rhs_expr,
                                                          mul_expr,
                                                      }));
  EXPECT_THAT(post_if_block.sequences[0].stmt,
              Optional(StmtEquivTo(do_mul_stmt)));
  EXPECT_THAT(post_if_block.next, ElementsAre(last_block.id));
  EXPECT_THAT(post_if_block.preds, ElementsAre(then_block.id, graph.first));

  EXPECT_EQ(graph.last, last_block.id);
  EXPECT_EQ(last_block.id, 1);
  EXPECT_THAT(last_block.next, IsEmpty());
  EXPECT_THAT(last_block.preds, ElementsAre(post_if_block.id));
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
  auto do_add_stmt = S(DoStmt{.expr = add_expr});
  auto do_mul_stmt = S(DoStmt{.expr = mul_expr});
  auto graph = BuildControlFlowGraph(FuncDefStmt{
      .name = "foo",
      .stmts = StmtListOf({
          S(IfStmt{
              .cond = cond_expr,
              .then_stmts = StmtListOf({do_add_stmt}),
              .else_stmts = StmtListOf({do_mul_stmt}),
          }),
      }),
      .result_type = T(BasicType{.name = "Int32"}),
  });

  ASSERT_EQ(graph.blocks().Size(), 5);
  const auto& first_block = graph.blocks().Get(0);
  const auto& last_block = graph.blocks().Get(1);
  const auto& post_if_block = graph.blocks().Get(2);
  const auto& then_block = graph.blocks().Get(3);
  const auto& else_block = graph.blocks().Get(4);

  EXPECT_EQ(graph.first, first_block.id);
  EXPECT_EQ(first_block.id, 0);
  EXPECT_THAT(first_block.next, ElementsAre(then_block.id, else_block.id));
  EXPECT_THAT(first_block.preds, IsEmpty());
  ASSERT_THAT(first_block.sequences.size(), 1);
  EXPECT_THAT(first_block.sequences[0].expressions, ElementsAreArray({
                                                        cond_expr,
                                                    }));
  EXPECT_EQ(first_block.branch_cond, cond_expr);

  EXPECT_EQ(then_block.id, 3);
  EXPECT_THAT(then_block.next, ElementsAre(post_if_block.id));
  EXPECT_THAT(then_block.preds, ElementsAre(first_block.id));
  ASSERT_THAT(then_block.sequences.size(), 1);
  EXPECT_THAT(then_block.sequences[0].expressions, ElementsAreArray({
                                                       add_lhs_expr,
                                                       add_rhs_expr,
                                                       add_expr,
                                                   }));
  EXPECT_THAT(then_block.sequences[0].stmt, Optional(StmtEquivTo(do_add_stmt)));

  EXPECT_EQ(else_block.id, 4);
  EXPECT_THAT(else_block.next, ElementsAre(post_if_block.id));
  EXPECT_THAT(else_block.preds, ElementsAre(first_block.id));
  ASSERT_THAT(then_block.sequences.size(), 1);
  EXPECT_THAT(else_block.sequences[0].expressions, ElementsAreArray({
                                                       mul_lhs_expr,
                                                       mul_rhs_expr,
                                                       mul_expr,
                                                   }));
  EXPECT_THAT(else_block.sequences[0].stmt, Optional(StmtEquivTo(do_mul_stmt)));

  EXPECT_EQ(post_if_block.id, 2);
  EXPECT_THAT(post_if_block.sequences.size(), 0);
  EXPECT_THAT(post_if_block.next, ElementsAre(last_block.id));
  EXPECT_THAT(post_if_block.preds, ElementsAre(then_block.id, else_block.id));

  EXPECT_EQ(graph.last, last_block.id);
  EXPECT_EQ(last_block.id, 1);
  EXPECT_THAT(last_block.next, IsEmpty());
  EXPECT_THAT(last_block.preds, ElementsAre(post_if_block.id));
  EXPECT_THAT(last_block.sequences, IsEmpty());
}

TEST_F(ControlFlowGraphTest, VarDecl) {
  auto int_lit = E(IntLitExpr{
      .value = "3",
  });
  auto ident_expr = E(IdentExpr{
      .name = "x",
  });
  auto var_decl_stmt = S(VarDeclStmt{
      .name = "x",
      .type_constraint = T(BasicType{.name = "Int32"}),
      .init = int_lit,
  });
  auto return_stmt = S(ReturnStmt{
      .value = ident_expr,
  });
  auto graph = BuildControlFlowGraph(FuncDefStmt{
      .name = "foo",
      .result_type = T(BasicType{.name = "Int32"}),
      .stmts = StmtListOf({var_decl_stmt, return_stmt}),
  });

  ASSERT_EQ(graph.blocks().Size(), 2);
  const auto& first_block = graph.blocks().Get(0);
  const auto& last_block = graph.blocks().Get(1);

  EXPECT_EQ(graph.first, first_block.id);
  EXPECT_EQ(first_block.id, 0);
  EXPECT_THAT(first_block.next, ElementsAre(last_block.id));
  EXPECT_THAT(first_block.preds, IsEmpty());
  ASSERT_THAT(first_block.sequences.size(), 2);
  EXPECT_THAT(first_block.sequences[0].expressions, ElementsAreArray({
                                                        int_lit,
                                                    }));
  EXPECT_THAT(first_block.sequences[0].stmt,
              Optional(StmtEquivTo(var_decl_stmt)));
  EXPECT_THAT(first_block.sequences[1].expressions, ElementsAreArray({
                                                        ident_expr,
                                                    }));
  EXPECT_THAT(first_block.sequences[1].stmt,
              Optional(StmtEquivTo(return_stmt)));

  EXPECT_EQ(graph.last, last_block.id);
  EXPECT_EQ(last_block.id, 1);
  EXPECT_THAT(last_block.next, IsEmpty());
  EXPECT_THAT(last_block.preds, ElementsAre(first_block.id));
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
  auto do_add_stmt = S(DoStmt{.expr = add_expr});
  auto graph = BuildControlFlowGraph(FuncDefStmt{
      .name = "foo",
      .stmts = StmtListOf({
          S(LoopStmt{
              .stmts = StmtListOf({do_add_stmt}),
          }),
      }),
      .result_type = T(BasicType{.name = "Int32"}),
  });

  ASSERT_EQ(graph.blocks().Size(), 4);
  const auto& first_block = graph.blocks().Get(0);
  const auto& last_block = graph.blocks().Get(1);
  const auto& post_loop_block = graph.blocks().Get(2);
  const auto& loop_block = graph.blocks().Get(3);

  EXPECT_EQ(graph.first, first_block.id);
  EXPECT_EQ(first_block.id, 0);
  EXPECT_THAT(first_block.next, ElementsAre(loop_block.id));
  EXPECT_THAT(first_block.preds, IsEmpty());
  EXPECT_THAT(first_block.sequences, IsEmpty());

  EXPECT_EQ(loop_block.id, 3);
  EXPECT_THAT(loop_block.next, ElementsAre(loop_block.id));
  EXPECT_THAT(loop_block.preds, ElementsAre(loop_block.id, first_block.id));
  ASSERT_THAT(loop_block.sequences.size(), 1);
  EXPECT_THAT(loop_block.sequences[0].expressions, ElementsAreArray({
                                                       add_lhs_expr,
                                                       add_rhs_expr,
                                                       add_expr,
                                                   }));
  EXPECT_THAT(loop_block.sequences[0].stmt, Optional(StmtEquivTo(do_add_stmt)));

  EXPECT_EQ(post_loop_block.id, 2);
  EXPECT_THAT(post_loop_block.next, ElementsAre(last_block.id));
  EXPECT_THAT(post_loop_block.preds, IsEmpty());
  EXPECT_THAT(post_loop_block.sequences, IsEmpty());

  EXPECT_EQ(graph.last, last_block.id);
  EXPECT_EQ(last_block.id, 1);
  EXPECT_THAT(last_block.next, IsEmpty());
  EXPECT_THAT(loop_block.preds, ElementsAre(loop_block.id, first_block.id));
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
  auto var_assign_lhs_ref = E(IdentExpr{.name = "n"});
  auto var_assign_rhs_ref = E(IntLitExpr{.value = "1"});
  auto binary_op_expr_ref = E(BinaryOpExpr{
      .op = BinaryOp::Add,
      .lhs = var_assign_lhs_ref,
      .rhs = var_assign_rhs_ref,
  });
  auto break_stmt = S(BreakStmt{});
  auto return_value_ref = E(IdentExpr{.name = "n"});
  auto var_decl_stmt = S(VarDeclStmt{
      .type_constraint = T(BasicType{.name = "Int32"}),
      .name = "n",
      .init = n_var_init_ref,
  });
  auto var_assign_stmt = S(VarAssignStmt{
      .name = "n",
      .expr = binary_op_expr_ref,
  });
  auto return_stmt = S(ReturnStmt{
      .value = return_value_ref,
  });
  auto graph = BuildControlFlowGraph(FuncDefStmt{
      .name = "foo",
      .stmts = StmtListOf({
          var_decl_stmt,
          S(LoopStmt{
              .stmts = StmtListOf({
                  S(IfStmt{
                      .cond = if_cond_ref,
                      .then_stmts = StmtListOf({break_stmt}),
                  }),
                  var_assign_stmt,
              }),
          }),
          return_stmt,
      }),
      .result_type = T(BasicType{.name = "Int32"}),
  });

  ASSERT_EQ(graph.blocks().Size(), 6);
  const auto& first_block = graph.blocks().Get(0);
  const auto& last_block = graph.blocks().Get(1);
  const auto& post_loop_block = graph.blocks().Get(2);
  const auto& loop_block = graph.blocks().Get(3);
  const auto& post_if_block = graph.blocks().Get(4);
  const auto& then_block = graph.blocks().Get(5);

  EXPECT_EQ(graph.first, first_block.id);
  EXPECT_EQ(first_block.id, 0);
  ASSERT_THAT(first_block.next, ElementsAre(loop_block.id));
  EXPECT_THAT(first_block.preds, IsEmpty());
  ASSERT_THAT(first_block.sequences.size(), 1);
  EXPECT_THAT(first_block.sequences[0].expressions, ElementsAreArray({
                                                        n_var_init_ref,
                                                    }));
  EXPECT_THAT(first_block.sequences[0].stmt,
              Optional(StmtEquivTo(var_decl_stmt)));

  EXPECT_EQ(loop_block.id, 3);
  EXPECT_THAT(loop_block.next, ElementsAre(then_block.id, post_if_block.id));
  EXPECT_THAT(loop_block.preds, ElementsAre(post_if_block.id, first_block.id));
  ASSERT_THAT(loop_block.sequences.size(), 1);
  EXPECT_THAT(loop_block.sequences[0].expressions, ElementsAreArray({
                                                       if_cond_lhs_ref,
                                                       if_cond_rhs_ref,
                                                       if_cond_ref,
                                                   }));

  EXPECT_EQ(then_block.id, 5);
  EXPECT_THAT(then_block.next, ElementsAre(post_loop_block.id));
  EXPECT_THAT(then_block.preds, ElementsAre(loop_block.id));
  ASSERT_THAT(then_block.sequences.size(), 1);
  EXPECT_THAT(then_block.sequences[0].stmt, Optional(StmtEquivTo(break_stmt)));

  EXPECT_EQ(post_if_block.id, 4);
  EXPECT_THAT(post_if_block.next, ElementsAre(loop_block.id));
  EXPECT_THAT(post_if_block.preds, ElementsAre(loop_block.id));
  ASSERT_THAT(post_if_block.sequences.size(), 1);
  EXPECT_THAT(post_if_block.sequences[0].expressions, ElementsAreArray({
                                                          var_assign_lhs_ref,
                                                          var_assign_rhs_ref,
                                                          binary_op_expr_ref,
                                                      }));
  EXPECT_THAT(post_if_block.sequences[0].stmt,
              Optional(StmtEquivTo(var_assign_stmt)));

  EXPECT_EQ(post_loop_block.id, 2);
  EXPECT_THAT(post_loop_block.next, ElementsAre(last_block.id));
  EXPECT_THAT(post_loop_block.preds, ElementsAre(then_block.id));
  ASSERT_THAT(post_loop_block.sequences.size(), 1);
  EXPECT_THAT(post_loop_block.sequences[0].expressions, ElementsAreArray({
                                                            return_value_ref,
                                                        }));
  EXPECT_THAT(post_loop_block.sequences[0].stmt,
              Optional(StmtEquivTo(return_stmt)));

  EXPECT_EQ(graph.last, last_block.id);
  EXPECT_EQ(last_block.id, 1);
  EXPECT_THAT(last_block.next, IsEmpty());
  EXPECT_THAT(last_block.preds, ElementsAre(post_loop_block.id));
  EXPECT_THAT(last_block.sequences, IsEmpty());
}

}  // namespace
}  // namespace lucid
