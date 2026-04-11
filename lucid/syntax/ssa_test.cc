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

using BlockRef = SyntaxControlFlowGraph::BlockRef;
using Phi = SyntaxControlFlowGraph::Phi;

class ConvertToStaticSingleAssignmentTest : public Test, public AstFixture {
 protected:
  SyntaxControlFlowGraph BuildControlFlowGraph(FuncDefStmt func_def) {
    return ::lucid::BuildControlFlowGraph(ctx_, func_def);
  }
};

TEST_F(ConvertToStaticSingleAssignmentTest, Branching) {
  auto scfg = BuildControlFlowGraph(FuncDefStmt{
      .name = I("foo"),
      .result_type = T(BasicType{.name = I("Int32")}),
      .stmts = StmtListOf({
          S(VarDeclStmt{
              .name = I("x"),
              .type_constraint = T(BasicType{.name = I("Int32")}),
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

  ConvertToStaticSingleAssignment(ctx_, scfg);

  ASSERT_EQ(scfg.blocks().Size(), 5);

  const auto& first_block = scfg.blocks().Get(0);
  EXPECT_THAT(first_block.phis, IsEmpty());

  const auto& last_block = scfg.blocks().Get(1);
  EXPECT_THAT(last_block.phis, IsEmpty());

  const auto& post_if_block = scfg.blocks().Get(2);
  EXPECT_THAT(post_if_block.phis, SizeIs(1));

  const auto& then_block = scfg.blocks().Get(3);
  EXPECT_THAT(then_block.phis, IsEmpty());

  const auto& else_block = scfg.blocks().Get(4);
  EXPECT_THAT(else_block.phis, IsEmpty());
}

TEST_F(ConvertToStaticSingleAssignmentTest, DoubleBranching) {
  auto scfg = BuildControlFlowGraph(FuncDefStmt{
      .name = I("foo"),
      .result_type = T(BasicType{.name = I("Int32")}),
      .stmts = StmtListOf({
          S(VarDeclStmt{
              .name = I("x"),
              .type_constraint = T(BasicType{.name = I("Int32")}),
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

  ConvertToStaticSingleAssignment(ctx_, scfg);

  ASSERT_EQ(scfg.blocks().Size(), 8);

  const auto& first_block = scfg.blocks().Get(0);
  EXPECT_THAT(first_block.phis, IsEmpty());

  const auto& last_block = scfg.blocks().Get(1);
  EXPECT_THAT(last_block.phis, IsEmpty());

  const auto& post_if_block = scfg.blocks().Get(2);
  EXPECT_THAT(post_if_block.phis, SizeIs(1));

  const auto& then_block = scfg.blocks().Get(3);
  EXPECT_THAT(then_block.phis, IsEmpty());

  const auto& nested_post_if_block = scfg.blocks().Get(4);
  EXPECT_THAT(nested_post_if_block.phis, SizeIs(1));

  const auto& nested_then_block = scfg.blocks().Get(5);
  EXPECT_THAT(nested_then_block.phis, IsEmpty());

  const auto& nested_else_block = scfg.blocks().Get(6);
  EXPECT_THAT(nested_else_block.phis, IsEmpty());

  const auto& else_block = scfg.blocks().Get(7);
  EXPECT_THAT(else_block.phis, IsEmpty());
}

TEST_F(ConvertToStaticSingleAssignmentTest, MultipleVariables) {
  auto scfg = BuildControlFlowGraph(FuncDefStmt{
      .name = I("foo"),
      .result_type = T(BasicType{.name = I("Int32")}),
      .stmts = StmtListOf({
          S(VarDeclStmt{
              .name = I("x"),
              .type_constraint = T(BasicType{.name = I("Int32")}),
              .init = E(IntLitExpr{.value = I("21")}),
          }),
          S(VarDeclStmt{
              .name = I("y"),
              .type_constraint = T(BasicType{.name = I("Int32")}),
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

  ConvertToStaticSingleAssignment(ctx_, scfg);

  ASSERT_EQ(scfg.blocks().Size(), 5);

  const auto& first_block = scfg.blocks().Get(0);
  EXPECT_THAT(first_block.phis, IsEmpty());

  const auto& last_block = scfg.blocks().Get(1);
  EXPECT_THAT(last_block.phis, IsEmpty());

  const auto& post_if_block = scfg.blocks().Get(2);
  EXPECT_THAT(post_if_block.phis, SizeIs(2));

  const auto& then_block = scfg.blocks().Get(3);
  EXPECT_THAT(then_block.phis, IsEmpty());

  const auto& else_block = scfg.blocks().Get(4);
  EXPECT_THAT(else_block.phis, IsEmpty());
}

TEST_F(ConvertToStaticSingleAssignmentTest, Looping) {
  auto scfg = BuildControlFlowGraph(FuncDefStmt{
      .name = I("foo"),
      .result_type = T(BasicType{.name = I("Int32")}),
      .stmts = StmtListOf({
          S(VarDeclStmt{
              .name = I("x"),
              .type_constraint = T(BasicType{.name = I("Int32")}),
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

  ConvertToStaticSingleAssignment(ctx_, scfg);

  ASSERT_EQ(scfg.blocks().Size(), 7);

  const auto& first_block = scfg.blocks().Get(0);
  EXPECT_THAT(first_block.phis, IsEmpty());

  const auto& last_block = scfg.blocks().Get(1);
  EXPECT_THAT(last_block.phis, IsEmpty());

  const auto& loop_block = scfg.blocks().Get(2);
  EXPECT_THAT(loop_block.phis, IsEmpty());

  const auto& post_loop_block = scfg.blocks().Get(3);
  EXPECT_THAT(post_loop_block.phis, SizeIs(1));

  const auto& post_if_block = scfg.blocks().Get(4);
  EXPECT_THAT(post_if_block.phis, IsEmpty());

  const auto& then_block = scfg.blocks().Get(5);
  EXPECT_THAT(then_block.phis, IsEmpty());

  const auto& else_block = scfg.blocks().Get(6);
  EXPECT_THAT(else_block.phis, IsEmpty());
}

}  // namespace
}  // namespace lucid
