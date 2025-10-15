#include "lucid/ssa.h"

#include "gmock/gmock.h"
#include "gtest/gtest.h"
#include "lucid/ast.h"
#include "lucid/ast_fixture.h"

namespace lucid {
namespace {

using ::testing::ElementsAre;
using ::testing::IsEmpty;
using ::testing::Pair;
using ::testing::Test;
using ::testing::UnorderedElementsAre;

using BlockRef = ControlFlowGraph::BlockRef;

class ConvertToStaticSingleAssignmentTest : public Test, public AstFixture {
 protected:
  ControlFlowGraph BuildControlFlowGraph(FuncDefStmt func_def) {
    return ::lucid::BuildControlFlowGraph(ctx_, func_def);
  }
};

TEST_F(ConvertToStaticSingleAssignmentTest, Branching) {
  auto graph = BuildControlFlowGraph(FuncDefStmt{
      .name = "foo",
      .result_type = T(BasicType{.name = "Int32"}),
      .stmts = StmtListOf({
          S(VarDeclStmt{
              .type_constraint = T(BasicType{.name = "Int32"}),
              .name = "x",
              .init = E(IntLitExpr{.value = "21"}),
          }),
          S(IfStmt{
              .cond = E(BoolLitExpr{.value = "true"}),
              .then_stmts = StmtListOf({
                  S(VarAssignStmt{
                      .name = "x",
                      .expr = E(BinaryOpExpr{
                          .op = BinaryOp::Mul,
                          .lhs = E(IdentExpr{.name = "x"}),
                          .rhs = E(IntLitExpr{.value = "2"}),
                      }),
                  }),
              }),
              .else_stmts = StmtListOf({
                  S(VarAssignStmt{
                      .name = "x",
                      .expr = E(BinaryOpExpr{
                          .op = BinaryOp::Add,
                          .lhs = E(IdentExpr{.name = "x"}),
                          .rhs = E(IntLitExpr{.value = "1"}),
                      }),
                  }),
              }),
          }),
          S(ReturnStmt{.value = E(IdentExpr{.name = "x"})}),
      }),
  });

  ConvertToStaticSingleAssignment(ctx_, graph);

  ASSERT_EQ(graph.blocks().Size(), 5);

  const auto& first_block = graph.blocks().Get(0);
  EXPECT_THAT(first_block.phis, IsEmpty());

  const auto& last_block = graph.blocks().Get(1);
  EXPECT_THAT(last_block.phis, IsEmpty());

  const auto& post_if_block = graph.blocks().Get(2);
  EXPECT_THAT(post_if_block.phis, ElementsAre("x"));

  const auto& then_block = graph.blocks().Get(3);
  EXPECT_THAT(then_block.phis, IsEmpty());

  const auto& else_block = graph.blocks().Get(4);
  EXPECT_THAT(else_block.phis, IsEmpty());
}

TEST_F(ConvertToStaticSingleAssignmentTest, DoubleBranching) {
  auto graph = BuildControlFlowGraph(FuncDefStmt{
      .name = "foo",
      .result_type = T(BasicType{.name = "Int32"}),
      .stmts = StmtListOf({
          S(VarDeclStmt{
              .type_constraint = T(BasicType{.name = "Int32"}),
              .name = "x",
              .init = E(IntLitExpr{.value = "21"}),
          }),
          S(IfStmt{
              .cond = E(BoolLitExpr{.value = "true"}),
              .then_stmts = StmtListOf({
                  S(IfStmt{
                      .cond = E(BoolLitExpr{.value = "true"}),
                      .then_stmts = StmtListOf({
                          S(VarAssignStmt{
                              .name = "x",
                              .expr = E(BinaryOpExpr{
                                  .op = BinaryOp::Mul,
                                  .lhs = E(IdentExpr{.name = "x"}),
                                  .rhs = E(IntLitExpr{.value = "2"}),
                              }),
                          }),
                      }),
                      .else_stmts = StmtListOf({
                          S(VarAssignStmt{
                              .name = "x",
                              .expr = E(BinaryOpExpr{
                                  .op = BinaryOp::Add,
                                  .lhs = E(IdentExpr{.name = "x"}),
                                  .rhs = E(IntLitExpr{.value = "1"}),
                              }),
                          }),
                      }),
                  }),
              }),
              .else_stmts = StmtListOf({
                  S(VarAssignStmt{
                      .name = "x",
                      .expr = E(BinaryOpExpr{
                          .op = BinaryOp::Sub,
                          .lhs = E(IdentExpr{.name = "x"}),
                          .rhs = E(IntLitExpr{.value = "3"}),
                      }),
                  }),
              }),
          }),
          S(ReturnStmt{.value = E(IdentExpr{.name = "x"})}),
      }),
  });

  ConvertToStaticSingleAssignment(ctx_, graph);

  ASSERT_EQ(graph.blocks().Size(), 8);

  const auto& first_block = graph.blocks().Get(0);
  EXPECT_THAT(first_block.phis, IsEmpty());

  const auto& last_block = graph.blocks().Get(1);
  EXPECT_THAT(last_block.phis, IsEmpty());

  const auto& post_if_block = graph.blocks().Get(2);
  EXPECT_THAT(post_if_block.phis, ElementsAre("x"));

  const auto& then_block = graph.blocks().Get(3);
  EXPECT_THAT(then_block.phis, IsEmpty());

  const auto& nested_post_if_block = graph.blocks().Get(4);
  EXPECT_THAT(nested_post_if_block.phis, ElementsAre("x"));

  const auto& nested_then_block = graph.blocks().Get(5);
  EXPECT_THAT(nested_then_block.phis, IsEmpty());

  const auto& nested_else_block = graph.blocks().Get(6);
  EXPECT_THAT(nested_else_block.phis, IsEmpty());

  const auto& else_block = graph.blocks().Get(7);
  EXPECT_THAT(else_block.phis, IsEmpty());
}

TEST_F(ConvertToStaticSingleAssignmentTest, MultipleVariables) {
  auto graph = BuildControlFlowGraph(FuncDefStmt{
      .name = "foo",
      .result_type = T(BasicType{.name = "Int32"}),
      .stmts = StmtListOf({
          S(VarDeclStmt{
              .type_constraint = T(BasicType{.name = "Int32"}),
              .name = "x",
              .init = E(IntLitExpr{.value = "21"}),
          }),
          S(VarDeclStmt{
              .type_constraint = T(BasicType{.name = "Int32"}),
              .name = "y",
              .init = E(IntLitExpr{.value = "2"}),
          }),
          S(IfStmt{
              .cond = E(BoolLitExpr{.value = "true"}),
              .then_stmts = StmtListOf({
                  S(VarAssignStmt{
                      .name = "x",
                      .expr = E(BinaryOpExpr{
                          .op = BinaryOp::Mul,
                          .lhs = E(IdentExpr{.name = "x"}),
                          .rhs = E(IntLitExpr{.value = "2"}),
                      }),
                  }),
              }),
              .else_stmts = StmtListOf({
                  S(VarAssignStmt{
                      .name = "y",
                      .expr = E(BinaryOpExpr{
                          .op = BinaryOp::Add,
                          .lhs = E(IdentExpr{.name = "x"}),
                          .rhs = E(IntLitExpr{.value = "1"}),
                      }),
                  }),
              }),
          }),
          S(ReturnStmt{.value = E(IdentExpr{.name = "x"})}),
      }),
  });

  ConvertToStaticSingleAssignment(ctx_, graph);

  ASSERT_EQ(graph.blocks().Size(), 5);

  const auto& first_block = graph.blocks().Get(0);
  EXPECT_THAT(first_block.phis, IsEmpty());

  const auto& last_block = graph.blocks().Get(1);
  EXPECT_THAT(last_block.phis, IsEmpty());

  const auto& post_if_block = graph.blocks().Get(2);
  EXPECT_THAT(post_if_block.phis, ElementsAre("y", "x"));

  const auto& then_block = graph.blocks().Get(3);
  EXPECT_THAT(then_block.phis, IsEmpty());

  const auto& else_block = graph.blocks().Get(4);
  EXPECT_THAT(else_block.phis, IsEmpty());
}

}  // namespace
}  // namespace lucid
