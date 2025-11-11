#include "lucid/dom.h"

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

class ComputeImmediateDominatorsTest : public Test, public AstFixture {
 protected:
  ControlFlowGraph BuildControlFlowGraph(FuncDefStmt func_def) {
    return ::lucid::BuildControlFlowGraph(ctx_, func_def);
  }
};

TEST_F(ComputeImmediateDominatorsTest, Simple) {
  auto graph = BuildControlFlowGraph(FuncDefStmt{
      .name = I("foo"),
      .result_type = T(BasicType{.name = "Void"}),
  });

  EXPECT_THAT(ComputeImmediateDominators(graph), ElementsAre(0, 0));
}

TEST_F(ComputeImmediateDominatorsTest, Branching) {
  auto graph = BuildControlFlowGraph(FuncDefStmt{
      .name = I("foo"),
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
                  S(VarAssignStmt{.name = "x",
                                  .expr = E(BinaryOpExpr{
                                      .op = BinaryOp::Mul,
                                      .lhs = E(IdentExpr{.name = "x"}),
                                      .rhs = E(IntLitExpr{.value = "2"}),
                                  })}),
              }),
              .else_stmts = StmtListOf({
                  S(VarAssignStmt{.name = "x",
                                  .expr = E(BinaryOpExpr{
                                      .op = BinaryOp::Add,
                                      .lhs = E(IdentExpr{.name = "x"}),
                                      .rhs = E(IntLitExpr{.value = "1"}),
                                  })}),
              }),
          }),
          S(ReturnStmt{.value = E(IdentExpr{.name = "x"})}),
      }),
  });

  EXPECT_THAT(ComputeImmediateDominators(graph), ElementsAre(0, 2, 0, 0, 0));
}

TEST_F(ComputeImmediateDominatorsTest, NestedBranching) {
  auto graph = BuildControlFlowGraph(FuncDefStmt{
      .name = I("foo"),
      .result_type = T(BasicType{.name = "Int32"}),
      .stmts = StmtListOf({
          S(VarDeclStmt{
              .type_constraint = T(BasicType{.name = "Int32"}),
              .name = "x",
              .init = E(IntLitExpr{.value = "21"}),
          }),
          S(LoopStmt{
              .stmts = StmtListOf({S(IfStmt{
                  .cond = E(BoolLitExpr{.value = "true"}),
                  .then_stmts = StmtListOf({
                      S(VarAssignStmt{.name = "x",
                                      .expr = E(BinaryOpExpr{
                                          .op = BinaryOp::Mul,
                                          .lhs = E(IdentExpr{.name = "x"}),
                                          .rhs = E(IntLitExpr{.value = "2"}),
                                      })}),
                  }),
                  .else_stmts = StmtListOf({
                      S(BreakStmt{}),
                  }),
              })}),
          }),
          S(ReturnStmt{.value = E(IdentExpr{.name = "x"})}),
      }),
  });

  EXPECT_THAT(ComputeImmediateDominators(graph),
              ElementsAre(0, 2, 6, 0, 5, 3, 3));
}

class ComputeDominanceFrontiersTest : public Test, public AstFixture {
 protected:
  ControlFlowGraph BuildControlFlowGraph(FuncDefStmt func_def) {
    return ::lucid::BuildControlFlowGraph(ctx_, func_def);
  }

  std::unordered_map<BlockRef, std::unordered_set<BlockRef>>
  ComputeDominanceFrontiers(const ControlFlowGraph& cfg) {
    return ::lucid::ComputeDominanceFrontiers(cfg,
                                              ComputeImmediateDominators(cfg));
  }
};

TEST_F(ComputeDominanceFrontiersTest, Simple) {
  auto graph = BuildControlFlowGraph(FuncDefStmt{
      .name = I("foo"),
      .result_type = T(BasicType{.name = "Void"}),
  });

  EXPECT_THAT(ComputeDominanceFrontiers(graph), IsEmpty());
}

TEST_F(ComputeDominanceFrontiersTest, Branching) {
  auto graph = BuildControlFlowGraph(FuncDefStmt{
      .name = I("foo"),
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
                  S(VarAssignStmt{.name = "x",
                                  .expr = E(BinaryOpExpr{
                                      .op = BinaryOp::Mul,
                                      .lhs = E(IdentExpr{.name = "x"}),
                                      .rhs = E(IntLitExpr{.value = "2"}),
                                  })}),
              }),
              .else_stmts = StmtListOf({
                  S(VarAssignStmt{.name = "x",
                                  .expr = E(BinaryOpExpr{
                                      .op = BinaryOp::Add,
                                      .lhs = E(IdentExpr{.name = "x"}),
                                      .rhs = E(IntLitExpr{.value = "1"}),
                                  })}),
              }),
          }),
          S(ReturnStmt{.value = E(IdentExpr{.name = "x"})}),
      }),
  });

  EXPECT_THAT(ComputeDominanceFrontiers(graph),
              UnorderedElementsAre(Pair(3, UnorderedElementsAre(2)),
                                   Pair(4, UnorderedElementsAre(2))));
}

TEST_F(ComputeDominanceFrontiersTest, NestedBranching) {
  auto graph = BuildControlFlowGraph(FuncDefStmt{
      .name = I("foo"),
      .result_type = T(BasicType{.name = "Int32"}),
      .stmts = StmtListOf({
          S(VarDeclStmt{
              .type_constraint = T(BasicType{.name = "Int32"}),
              .name = "x",
              .init = E(IntLitExpr{.value = "21"}),
          }),
          S(LoopStmt{
              .stmts = StmtListOf({S(IfStmt{
                  .cond = E(BoolLitExpr{.value = "true"}),
                  .then_stmts = StmtListOf({
                      S(VarAssignStmt{.name = "x",
                                      .expr = E(BinaryOpExpr{
                                          .op = BinaryOp::Mul,
                                          .lhs = E(IdentExpr{.name = "x"}),
                                          .rhs = E(IntLitExpr{.value = "2"}),
                                      })}),
                  }),
                  .else_stmts = StmtListOf({
                      S(BreakStmt{}),
                  }),
              })}),
          }),
          S(ReturnStmt{.value = E(IdentExpr{.name = "x"})}),
      }),
  });

  EXPECT_THAT(ComputeDominanceFrontiers(graph),
              UnorderedElementsAre(Pair(3, UnorderedElementsAre(3)),
                                   Pair(4, UnorderedElementsAre(3)),
                                   Pair(5, UnorderedElementsAre(3))));
}

}  // namespace
}  // namespace lucid
