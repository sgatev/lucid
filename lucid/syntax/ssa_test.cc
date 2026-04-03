#include "lucid/syntax/ssa.h"

#include "gmock/gmock.h"
#include "gtest/gtest.h"
#include "lucid/syntax/ast.h"
#include "lucid/syntax/ast_fixture.h"
#include "lucid/syntax/cfg.h"

namespace lucid {
namespace {

using ::testing::IsEmpty;
using ::testing::Pair;
using ::testing::SizeIs;
using ::testing::Test;
using ::testing::UnorderedElementsAre;

using BlockRef = ControlFlowGraph::BlockRef;
using Phi = ControlFlowGraph::Phi;

class ConvertToStaticSingleAssignmentTest : public Test, public AstFixture {
 protected:
  ControlFlowGraph BuildControlFlowGraph(FuncDefStmt func_def) {
    return ::lucid::BuildControlFlowGraph(ctx_, func_def);
  }
};

TEST_F(ConvertToStaticSingleAssignmentTest, Branching) {
  auto graph = BuildControlFlowGraph(FuncDefStmt{
      .name = I("foo"),
      .result_type = T(BasicType{.name = I("Int32")}),
      .stmts = StmtListOf({
          S(VarDeclStmt{
              .type_constraint = T(BasicType{.name = I("Int32")}),
              .name = I("x"),
              .init = E(IntLitExpr{.value = I("21")}),
          }),
          S(IfStmt{
              .cond = E(BoolLitExpr{.value = I("true")}),
              .then_stmts = StmtListOf({
                  S(VarAssignStmt{
                      .name = I("x"),
                      .expr = E(BinaryOpExpr{
                          .op = BinaryOp::Mul,
                          .lhs = E(IdentExpr{.name = I("x")}),
                          .rhs = E(IntLitExpr{.value = I("2")}),
                      }),
                  }),
              }),
              .else_stmts = StmtListOf({
                  S(VarAssignStmt{
                      .name = I("x"),
                      .expr = E(BinaryOpExpr{
                          .op = BinaryOp::Add,
                          .lhs = E(IdentExpr{.name = I("x")}),
                          .rhs = E(IntLitExpr{.value = I("1")}),
                      }),
                  }),
              }),
          }),
          S(ReturnStmt{.value = E(IdentExpr{.name = I("x")})}),
      }),
  });

  ConvertToStaticSingleAssignment(ctx_, graph);

  ASSERT_EQ(graph.blocks().Size(), 5);

  const auto& first_block = graph.blocks().Get(0);
  EXPECT_THAT(first_block.phis, IsEmpty());

  const auto& last_block = graph.blocks().Get(1);
  EXPECT_THAT(last_block.phis, IsEmpty());

  const auto& post_if_block = graph.blocks().Get(2);
  EXPECT_THAT(post_if_block.phis, SizeIs(1));

  const auto& then_block = graph.blocks().Get(3);
  EXPECT_THAT(then_block.phis, IsEmpty());

  const auto& else_block = graph.blocks().Get(4);
  EXPECT_THAT(else_block.phis, IsEmpty());
}

TEST_F(ConvertToStaticSingleAssignmentTest, DoubleBranching) {
  auto graph = BuildControlFlowGraph(FuncDefStmt{
      .name = I("foo"),
      .result_type = T(BasicType{.name = I("Int32")}),
      .stmts = StmtListOf({
          S(VarDeclStmt{
              .type_constraint = T(BasicType{.name = I("Int32")}),
              .name = I("x"),
              .init = E(IntLitExpr{.value = I("21")}),
          }),
          S(IfStmt{
              .cond = E(BoolLitExpr{.value = I("true")}),
              .then_stmts = StmtListOf({
                  S(IfStmt{
                      .cond = E(BoolLitExpr{.value = I("true")}),
                      .then_stmts = StmtListOf({
                          S(VarAssignStmt{
                              .name = I("x"),
                              .expr = E(BinaryOpExpr{
                                  .op = BinaryOp::Mul,
                                  .lhs = E(IdentExpr{.name = I("x")}),
                                  .rhs = E(IntLitExpr{.value = I("2")}),
                              }),
                          }),
                      }),
                      .else_stmts = StmtListOf({
                          S(VarAssignStmt{
                              .name = I("x"),
                              .expr = E(BinaryOpExpr{
                                  .op = BinaryOp::Add,
                                  .lhs = E(IdentExpr{.name = I("x")}),
                                  .rhs = E(IntLitExpr{.value = I("1")}),
                              }),
                          }),
                      }),
                  }),
              }),
              .else_stmts = StmtListOf({
                  S(VarAssignStmt{
                      .name = I("x"),
                      .expr = E(BinaryOpExpr{
                          .op = BinaryOp::Sub,
                          .lhs = E(IdentExpr{.name = I("x")}),
                          .rhs = E(IntLitExpr{.value = I("3")}),
                      }),
                  }),
              }),
          }),
          S(ReturnStmt{.value = E(IdentExpr{.name = I("x")})}),
      }),
  });

  ConvertToStaticSingleAssignment(ctx_, graph);

  ASSERT_EQ(graph.blocks().Size(), 8);

  const auto& first_block = graph.blocks().Get(0);
  EXPECT_THAT(first_block.phis, IsEmpty());

  const auto& last_block = graph.blocks().Get(1);
  EXPECT_THAT(last_block.phis, IsEmpty());

  const auto& post_if_block = graph.blocks().Get(2);
  EXPECT_THAT(post_if_block.phis, SizeIs(1));

  const auto& then_block = graph.blocks().Get(3);
  EXPECT_THAT(then_block.phis, IsEmpty());

  const auto& nested_post_if_block = graph.blocks().Get(4);
  EXPECT_THAT(nested_post_if_block.phis, SizeIs(1));

  const auto& nested_then_block = graph.blocks().Get(5);
  EXPECT_THAT(nested_then_block.phis, IsEmpty());

  const auto& nested_else_block = graph.blocks().Get(6);
  EXPECT_THAT(nested_else_block.phis, IsEmpty());

  const auto& else_block = graph.blocks().Get(7);
  EXPECT_THAT(else_block.phis, IsEmpty());
}

TEST_F(ConvertToStaticSingleAssignmentTest, MultipleVariables) {
  auto graph = BuildControlFlowGraph(FuncDefStmt{
      .name = I("foo"),
      .result_type = T(BasicType{.name = I("Int32")}),
      .stmts = StmtListOf({
          S(VarDeclStmt{
              .type_constraint = T(BasicType{.name = I("Int32")}),
              .name = I("x"),
              .init = E(IntLitExpr{.value = I("21")}),
          }),
          S(VarDeclStmt{
              .type_constraint = T(BasicType{.name = I("Int32")}),
              .name = I("y"),
              .init = E(IntLitExpr{.value = I("2")}),
          }),
          S(IfStmt{
              .cond = E(BoolLitExpr{.value = I("true")}),
              .then_stmts = StmtListOf({
                  S(VarAssignStmt{
                      .name = I("x"),
                      .expr = E(BinaryOpExpr{
                          .op = BinaryOp::Mul,
                          .lhs = E(IdentExpr{.name = I("x")}),
                          .rhs = E(IntLitExpr{.value = I("2")}),
                      }),
                  }),
              }),
              .else_stmts = StmtListOf({
                  S(VarAssignStmt{
                      .name = I("y"),
                      .expr = E(BinaryOpExpr{
                          .op = BinaryOp::Add,
                          .lhs = E(IdentExpr{.name = I("x")}),
                          .rhs = E(IntLitExpr{.value = I("1")}),
                      }),
                  }),
              }),
          }),
          S(ReturnStmt{.value = E(IdentExpr{.name = I("x")})}),
      }),
  });

  ConvertToStaticSingleAssignment(ctx_, graph);

  ASSERT_EQ(graph.blocks().Size(), 5);

  const auto& first_block = graph.blocks().Get(0);
  EXPECT_THAT(first_block.phis, IsEmpty());

  const auto& last_block = graph.blocks().Get(1);
  EXPECT_THAT(last_block.phis, IsEmpty());

  const auto& post_if_block = graph.blocks().Get(2);
  EXPECT_THAT(post_if_block.phis, SizeIs(2));

  const auto& then_block = graph.blocks().Get(3);
  EXPECT_THAT(then_block.phis, IsEmpty());

  const auto& else_block = graph.blocks().Get(4);
  EXPECT_THAT(else_block.phis, IsEmpty());
}

TEST_F(ConvertToStaticSingleAssignmentTest, Looping) {
  auto graph = BuildControlFlowGraph(FuncDefStmt{
      .name = I("foo"),
      .result_type = T(BasicType{.name = I("Int32")}),
      .stmts = StmtListOf({
          S(VarDeclStmt{
              .type_constraint = T(BasicType{.name = I("Int32")}),
              .name = I("x"),
              .init = E(IntLitExpr{.value = I("21")}),
          }),
          S(LoopStmt{
              .stmts = StmtListOf({
                  S(IfStmt{
                      .cond = E(BoolLitExpr{.value = I("true")}),
                      .then_stmts = StmtListOf({
                          S(VarAssignStmt{
                              .name = I("x"),
                              .expr = E(BinaryOpExpr{
                                  .op = BinaryOp::Mul,
                                  .lhs = E(IdentExpr{.name = I("x")}),
                                  .rhs = E(IntLitExpr{.value = I("2")}),
                              }),
                          }),
                      }),
                      .else_stmts = StmtListOf({
                          S(BreakStmt{}),
                      }),
                  }),
              }),
          }),
          S(ReturnStmt{.value = E(IdentExpr{.name = I("x")})}),
      }),
  });

  ConvertToStaticSingleAssignment(ctx_, graph);

  ASSERT_EQ(graph.blocks().Size(), 7);

  const auto& first_block = graph.blocks().Get(0);
  EXPECT_THAT(first_block.phis, IsEmpty());

  const auto& last_block = graph.blocks().Get(1);
  EXPECT_THAT(last_block.phis, IsEmpty());

  const auto& loop_block = graph.blocks().Get(2);
  EXPECT_THAT(loop_block.phis, IsEmpty());

  const auto& post_loop_block = graph.blocks().Get(3);
  EXPECT_THAT(post_loop_block.phis, SizeIs(1));

  const auto& post_if_block = graph.blocks().Get(4);
  EXPECT_THAT(post_if_block.phis, IsEmpty());

  const auto& then_block = graph.blocks().Get(5);
  EXPECT_THAT(then_block.phis, IsEmpty());

  const auto& else_block = graph.blocks().Get(6);
  EXPECT_THAT(else_block.phis, IsEmpty());
}

}  // namespace
}  // namespace lucid
