#include "lucid/am/translator.h"

#include <vector>

#include "gmock/gmock.h"
#include "gtest/gtest.h"
#include "lucid/am/instructions.h"
#include "lucid/core/container/graph/order.h"
#include "lucid/syntax/ast.h"
#include "lucid/syntax/ast_fixture.h"
#include "lucid/syntax/cfg.h"
#include "lucid/syntax/type.h"

namespace lucid {
namespace {

using ::testing::ElementsAre;

class GenerateAbstractMachineFunctionTest : public testing::Test,
                                            public AstFixture {
 protected:
  std::vector<Instruction> Generate(
      FuncDefStmt& func, const std::vector<FuncDefStmt>& func_defs = {}) {
    InferExprTypes(ctx_, func_defs, func);
    auto graph = BuildControlFlowGraph(ctx_, func);
    AbstractMachineState state;
    AbstractMachineControlFlowGraph am_cfg =
        GenerateAbstractMachineFunction(ctx_, graph, state);

    std::vector<AbstractMachineControlFlowGraph::BlockRef> block_refs =
        Vertices(am_cfg);
    const CompareVertexOrder<AbstractMachineControlFlowGraph> compare(
        am_cfg, ComputeReversePostOrder(am_cfg));
    std::sort(block_refs.begin(), block_refs.end(), compare);

    std::vector<Instruction> instructions;
    for (const auto& ref : block_refs) {
      const auto& block = am_cfg.get(ref);
      instructions.insert(instructions.end(), block.instructions.begin(),
                          block.instructions.end());
    }
    return instructions;
  }
};

TEST_F(GenerateAbstractMachineFunctionTest, ReturnInt32Lit) {
  auto func = FuncDefStmt{
      .name = I("foo"),
      .result_type = T(BasicType{.name = I("Int32")}),
      .stmts = StmtListOf({
          S(ReturnStmt{
              .value = E(IntLitExpr{.value = I("21")}),
          }),
      }),
  };

  EXPECT_THAT(Generate(func), ElementsAre(PushStack{},
                                          Label{
                                              .id = 0,
                                          },
                                          SetReg32{
                                              .src_val = "21",
                                              .dst_reg = 1,
                                          },
                                          MoveReg32{
                                              .src_reg = 1,
                                              .dst_reg = 0,
                                          },
                                          UncondJump{
                                              .label = 1,
                                          },
                                          Label{
                                              .id = 1,
                                          },
                                          PopStack{}, Return{}));
}

TEST_F(GenerateAbstractMachineFunctionTest, ReturnInt64Lit) {
  auto func = FuncDefStmt{
      .name = I("foo"),
      .result_type = T(BasicType{.name = I("Int64")}),
      .stmts = StmtListOf({
          S(ReturnStmt{
              .value = E(IntLitExpr{.value = I("21")}),
          }),
      }),
  };

  EXPECT_THAT(Generate(func), ElementsAre(PushStack{},
                                          Label{
                                              .id = 0,
                                          },
                                          SetReg64{
                                              .src_val = "21",
                                              .dst_reg = 1,
                                          },
                                          MoveReg64{
                                              .src_reg = 1,
                                              .dst_reg = 0,
                                          },
                                          UncondJump{
                                              .label = 1,
                                          },
                                          Label{
                                              .id = 1,
                                          },
                                          PopStack{}, Return{}));
}

TEST_F(GenerateAbstractMachineFunctionTest, FuncCallWithInt32Arg) {
  auto id_func = FuncDefStmt{
      .name = I("id"),
      .params = ParamListOf({
          P(FuncParam{
              .name = I("x"),
              .type_constraint = T(BasicType{.name = I("Int32")}),
          }),
      }),
      .result_type = T(BasicType{.name = I("Int32")}),
  };

  auto func = FuncDefStmt{
      .name = I("foo"),
      .result_type = T(BasicType{.name = I("Int32")}),
      .stmts = StmtListOf({
          S(ReturnStmt{
              .value = E(FuncCallExpr{
                  .func_name = I("id"),
                  .args = ExprListOf({
                      E(IntLitExpr{.value = I("21")}),
                  }),
              }),
          }),
      }),
  };

  EXPECT_THAT(Generate(func, {id_func}), ElementsAre(PushStack{},
                                                     Label{
                                                         .id = 0,
                                                     },
                                                     SetReg32{
                                                         .src_val = "21",
                                                         .dst_reg = 1,
                                                     },
                                                     FuncCall{
                                                         .label = "id",
                                                         .args =
                                                             {
                                                                 {
                                                                     .reg = 1,
                                                                     .bits = 32,
                                                                 },
                                                             },
                                                         .res =
                                                             {
                                                                 .reg = 2,
                                                                 .bits = 32,
                                                             },
                                                     },
                                                     MoveReg32{
                                                         .src_reg = 2,
                                                         .dst_reg = 0,
                                                     },
                                                     UncondJump{
                                                         .label = 1,
                                                     },
                                                     Label{
                                                         .id = 1,
                                                     },
                                                     PopStack{}, Return{}));
}

TEST_F(GenerateAbstractMachineFunctionTest, FuncCallWithInt64Arg) {
  auto id_func = FuncDefStmt{
      .name = I("id"),
      .params = ParamListOf({
          P(FuncParam{
              .name = I("x"),
              .type_constraint = T(BasicType{.name = I("Int64")}),
          }),
      }),
      .result_type = T(BasicType{.name = I("Int64")}),
  };

  auto func = FuncDefStmt{
      .name = I("foo"),
      .result_type = T(BasicType{.name = I("Int64")}),
      .stmts = StmtListOf({
          S(ReturnStmt{
              .value = E(FuncCallExpr{
                  .func_name = I("id"),
                  .args = ExprListOf({
                      E(IntLitExpr{.value = I("21")}),
                  }),
              }),
          }),
      }),
  };

  EXPECT_THAT(Generate(func, {id_func}), ElementsAre(PushStack{},
                                                     Label{
                                                         .id = 0,
                                                     },
                                                     SetReg64{
                                                         .src_val = "21",
                                                         .dst_reg = 1,
                                                     },
                                                     FuncCall{
                                                         .label = "id",
                                                         .args =
                                                             {
                                                                 {
                                                                     .reg = 1,
                                                                     .bits = 64,
                                                                 },
                                                             },
                                                         .res =
                                                             {
                                                                 .reg = 2,
                                                                 .bits = 64,
                                                             },
                                                     },
                                                     MoveReg64{
                                                         .src_reg = 2,
                                                         .dst_reg = 0,
                                                     },
                                                     UncondJump{
                                                         .label = 1,
                                                     },
                                                     Label{
                                                         .id = 1,
                                                     },
                                                     PopStack{}, Return{}));
}

TEST_F(GenerateAbstractMachineFunctionTest, AddInt32) {
  auto func = FuncDefStmt{
      .name = I("foo"),
      .result_type = T(BasicType{.name = I("Int32")}),
      .stmts = StmtListOf({
          S(ReturnStmt{
              .value = E(BinaryOpExpr{
                  .op = BinaryOp::Add,
                  .lhs = E(IntLitExpr{.value = I("2")}),
                  .rhs = E(IntLitExpr{.value = I("3")}),
              }),
          }),
      }),
  };

  EXPECT_THAT(Generate(func), ElementsAre(PushStack{},
                                          Label{
                                              .id = 0,
                                          },
                                          SetReg32{
                                              .src_val = "2",
                                              .dst_reg = 1,
                                          },
                                          SetReg32{
                                              .src_val = "3",
                                              .dst_reg = 2,
                                          },
                                          AddReg32{
                                              .res_reg = 3,
                                              .lhs_reg = 1,
                                              .rhs_reg = 2,
                                          },
                                          MoveReg32{
                                              .src_reg = 3,
                                              .dst_reg = 0,
                                          },
                                          UncondJump{
                                              .label = 1,
                                          },
                                          Label{
                                              .id = 1,
                                          },
                                          PopStack{}, Return{}));
}

TEST_F(GenerateAbstractMachineFunctionTest, AddInt64) {
  auto func = FuncDefStmt{
      .name = I("foo"),
      .result_type = T(BasicType{.name = I("Int64")}),
      .stmts = StmtListOf({
          S(ReturnStmt{
              .value = E(BinaryOpExpr{
                  .op = BinaryOp::Add,
                  .lhs = E(IntLitExpr{.value = I("2")}),
                  .rhs = E(IntLitExpr{.value = I("3")}),
              }),
          }),
      }),
  };

  EXPECT_THAT(Generate(func), ElementsAre(PushStack{},
                                          Label{
                                              .id = 0,
                                          },
                                          SetReg64{
                                              .src_val = "2",
                                              .dst_reg = 1,
                                          },
                                          SetReg64{
                                              .src_val = "3",
                                              .dst_reg = 2,
                                          },
                                          AddReg64{
                                              .res_reg = 3,
                                              .lhs_reg = 1,
                                              .rhs_reg = 2,
                                          },
                                          MoveReg64{
                                              .src_reg = 3,
                                              .dst_reg = 0,
                                          },
                                          UncondJump{
                                              .label = 1,
                                          },
                                          Label{
                                              .id = 1,
                                          },
                                          PopStack{}, Return{}));
}

TEST_F(GenerateAbstractMachineFunctionTest, SubtractInt32) {
  auto func = FuncDefStmt{
      .name = I("foo"),
      .result_type = T(BasicType{.name = I("Int32")}),
      .stmts = StmtListOf({
          S(ReturnStmt{
              .value = E(BinaryOpExpr{
                  .op = BinaryOp::Sub,
                  .lhs = E(IntLitExpr{.value = I("7")}),
                  .rhs = E(IntLitExpr{.value = I("5")}),
              }),
          }),
      }),
  };

  EXPECT_THAT(Generate(func), ElementsAre(PushStack{},
                                          Label{
                                              .id = 0,
                                          },
                                          SetReg32{
                                              .src_val = "7",
                                              .dst_reg = 1,
                                          },
                                          SetReg32{
                                              .src_val = "5",
                                              .dst_reg = 2,
                                          },
                                          SubReg32{
                                              .res_reg = 3,
                                              .lhs_reg = 1,
                                              .rhs_reg = 2,
                                          },
                                          MoveReg32{
                                              .src_reg = 3,
                                              .dst_reg = 0,
                                          },
                                          UncondJump{
                                              .label = 1,
                                          },
                                          Label{
                                              .id = 1,
                                          },
                                          PopStack{}, Return{}));
}

TEST_F(GenerateAbstractMachineFunctionTest, SubtractInt64) {
  auto func = FuncDefStmt{
      .name = I("foo"),
      .result_type = T(BasicType{.name = I("Int64")}),
      .stmts = StmtListOf({
          S(ReturnStmt{
              .value = E(BinaryOpExpr{
                  .op = BinaryOp::Sub,
                  .lhs = E(IntLitExpr{.value = I("7")}),
                  .rhs = E(IntLitExpr{.value = I("5")}),
              }),
          }),
      }),
  };

  EXPECT_THAT(Generate(func), ElementsAre(PushStack{},
                                          Label{
                                              .id = 0,
                                          },
                                          SetReg64{
                                              .src_val = "7",
                                              .dst_reg = 1,
                                          },
                                          SetReg64{
                                              .src_val = "5",
                                              .dst_reg = 2,
                                          },
                                          SubReg64{
                                              .res_reg = 3,
                                              .lhs_reg = 1,
                                              .rhs_reg = 2,
                                          },
                                          MoveReg64{
                                              .src_reg = 3,
                                              .dst_reg = 0,
                                          },
                                          UncondJump{
                                              .label = 1,
                                          },
                                          Label{
                                              .id = 1,
                                          },
                                          PopStack{}, Return{}));
}

TEST_F(GenerateAbstractMachineFunctionTest, MultiplyInt32) {
  auto func = FuncDefStmt{
      .name = I("foo"),
      .result_type = T(BasicType{.name = I("Int32")}),
      .stmts = StmtListOf({
          S(ReturnStmt{
              .value = E(BinaryOpExpr{
                  .op = BinaryOp::Mul,
                  .lhs = E(IntLitExpr{.value = I("2")}),
                  .rhs = E(IntLitExpr{.value = I("3")}),
              }),
          }),
      }),
  };

  EXPECT_THAT(Generate(func), ElementsAre(PushStack{},
                                          Label{
                                              .id = 0,
                                          },
                                          SetReg32{
                                              .src_val = "2",
                                              .dst_reg = 1,
                                          },
                                          SetReg32{
                                              .src_val = "3",
                                              .dst_reg = 2,
                                          },
                                          MulReg32{
                                              .res_reg = 3,
                                              .lhs_reg = 1,
                                              .rhs_reg = 2,
                                          },
                                          MoveReg32{
                                              .src_reg = 3,
                                              .dst_reg = 0,
                                          },
                                          UncondJump{
                                              .label = 1,
                                          },
                                          Label{
                                              .id = 1,
                                          },
                                          PopStack{}, Return{}));
}

TEST_F(GenerateAbstractMachineFunctionTest, MultiplyInt64) {
  auto func = FuncDefStmt{
      .name = I("foo"),
      .result_type = T(BasicType{.name = I("Int64")}),
      .stmts = StmtListOf({
          S(ReturnStmt{
              .value = E(BinaryOpExpr{
                  .op = BinaryOp::Mul,
                  .lhs = E(IntLitExpr{.value = I("2")}),
                  .rhs = E(IntLitExpr{.value = I("3")}),
              }),
          }),
      }),
  };

  EXPECT_THAT(Generate(func), ElementsAre(PushStack{},
                                          Label{
                                              .id = 0,
                                          },
                                          SetReg64{
                                              .src_val = "2",
                                              .dst_reg = 1,
                                          },
                                          SetReg64{
                                              .src_val = "3",
                                              .dst_reg = 2,
                                          },
                                          MulReg64{
                                              .res_reg = 3,
                                              .lhs_reg = 1,
                                              .rhs_reg = 2,
                                          },
                                          MoveReg64{
                                              .src_reg = 3,
                                              .dst_reg = 0,
                                          },
                                          UncondJump{
                                              .label = 1,
                                          },
                                          Label{
                                              .id = 1,
                                          },
                                          PopStack{}, Return{}));
}

TEST_F(GenerateAbstractMachineFunctionTest, DivideInt32) {
  auto func = FuncDefStmt{
      .name = I("foo"),
      .result_type = T(BasicType{.name = I("Int32")}),
      .stmts = StmtListOf({
          S(ReturnStmt{
              .value = E(BinaryOpExpr{
                  .op = BinaryOp::Div,
                  .lhs = E(IntLitExpr{.value = I("8")}),
                  .rhs = E(IntLitExpr{.value = I("2")}),
              }),
          }),
      }),
  };

  EXPECT_THAT(Generate(func), ElementsAre(PushStack{},
                                          Label{
                                              .id = 0,
                                          },
                                          SetReg32{
                                              .src_val = "8",
                                              .dst_reg = 1,
                                          },
                                          SetReg32{
                                              .src_val = "2",
                                              .dst_reg = 2,
                                          },
                                          DivReg32{
                                              .res_reg = 3,
                                              .lhs_reg = 1,
                                              .rhs_reg = 2,
                                          },
                                          MoveReg32{
                                              .src_reg = 3,
                                              .dst_reg = 0,
                                          },
                                          UncondJump{
                                              .label = 1,
                                          },
                                          Label{
                                              .id = 1,
                                          },
                                          PopStack{}, Return{}));
}

TEST_F(GenerateAbstractMachineFunctionTest, DivideInt64) {
  auto func = FuncDefStmt{
      .name = I("foo"),
      .result_type = T(BasicType{.name = I("Int64")}),
      .stmts = StmtListOf({
          S(ReturnStmt{
              .value = E(BinaryOpExpr{
                  .op = BinaryOp::Div,
                  .lhs = E(IntLitExpr{.value = I("8")}),
                  .rhs = E(IntLitExpr{.value = I("2")}),
              }),
          }),
      }),
  };

  EXPECT_THAT(Generate(func), ElementsAre(PushStack{},
                                          Label{
                                              .id = 0,
                                          },
                                          SetReg64{
                                              .src_val = "8",
                                              .dst_reg = 1,
                                          },
                                          SetReg64{
                                              .src_val = "2",
                                              .dst_reg = 2,
                                          },
                                          DivReg64{
                                              .res_reg = 3,
                                              .lhs_reg = 1,
                                              .rhs_reg = 2,
                                          },
                                          MoveReg64{
                                              .src_reg = 3,
                                              .dst_reg = 0,
                                          },
                                          UncondJump{
                                              .label = 1,
                                          },
                                          Label{
                                              .id = 1,
                                          },
                                          PopStack{}, Return{}));
}

TEST_F(GenerateAbstractMachineFunctionTest, ModuloInt32) {
  auto func = FuncDefStmt{
      .name = I("foo"),
      .result_type = T(BasicType{.name = I("Int32")}),
      .stmts = StmtListOf({
          S(ReturnStmt{
              .value = E(BinaryOpExpr{
                  .op = BinaryOp::Mod,
                  .lhs = E(IntLitExpr{.value = I("8")}),
                  .rhs = E(IntLitExpr{.value = I("2")}),
              }),
          }),
      }),
  };

  EXPECT_THAT(Generate(func), ElementsAre(PushStack{},
                                          Label{
                                              .id = 0,
                                          },
                                          SetReg32{
                                              .src_val = "8",
                                              .dst_reg = 1,
                                          },
                                          SetReg32{
                                              .src_val = "2",
                                              .dst_reg = 2,
                                          },
                                          ModReg32{
                                              .res_reg = 3,
                                              .lhs_reg = 1,
                                              .rhs_reg = 2,
                                          },
                                          MoveReg32{
                                              .src_reg = 3,
                                              .dst_reg = 0,
                                          },
                                          UncondJump{
                                              .label = 1,
                                          },
                                          Label{
                                              .id = 1,
                                          },
                                          PopStack{}, Return{}));
}

TEST_F(GenerateAbstractMachineFunctionTest, ModuloInt64) {
  auto func = FuncDefStmt{
      .name = I("foo"),
      .result_type = T(BasicType{.name = I("Int64")}),
      .stmts = StmtListOf({
          S(ReturnStmt{
              .value = E(BinaryOpExpr{
                  .op = BinaryOp::Mod,
                  .lhs = E(IntLitExpr{.value = I("8")}),
                  .rhs = E(IntLitExpr{.value = I("2")}),
              }),
          }),
      }),
  };

  EXPECT_THAT(Generate(func), ElementsAre(PushStack{},
                                          Label{
                                              .id = 0,
                                          },
                                          SetReg64{
                                              .src_val = "8",
                                              .dst_reg = 1,
                                          },
                                          SetReg64{
                                              .src_val = "2",
                                              .dst_reg = 2,
                                          },
                                          ModReg64{
                                              .res_reg = 3,
                                              .lhs_reg = 1,
                                              .rhs_reg = 2,
                                          },
                                          MoveReg64{
                                              .src_reg = 3,
                                              .dst_reg = 0,
                                          },
                                          UncondJump{
                                              .label = 1,
                                          },
                                          Label{
                                              .id = 1,
                                          },
                                          PopStack{}, Return{}));
}

TEST_F(GenerateAbstractMachineFunctionTest, IfStmt) {
  auto func = FuncDefStmt{
      .name = I("foo"),
      .result_type = T(BasicType{.name = I("Int32")}),
      .stmts = StmtListOf({
          S(IfStmt{
              .cond = E(BoolLitExpr{.value = I("true")}),
              .then_stmts = StmtListOf({
                  S(ReturnStmt{
                      .value = E(BinaryOpExpr{
                          .op = BinaryOp::Add,
                          .lhs = E(IntLitExpr{.value = I("2")}),
                          .rhs = E(IntLitExpr{.value = I("3")}),
                      }),
                  }),
              }),
          }),
          S(ReturnStmt{
              .value = E(BinaryOpExpr{
                  .op = BinaryOp::Mul,
                  .lhs = E(IntLitExpr{.value = I("4")}),
                  .rhs = E(IntLitExpr{.value = I("5")}),
              }),
          }),
      }),
  };

  EXPECT_THAT(Generate(func), ElementsAre(PushStack{},
                                          Label{
                                              .id = 0,
                                          },
                                          SetReg32{
                                              .src_val = "1",
                                              .dst_reg = 1,
                                          },
                                          CondJump{
                                              .cond_reg = 1,
                                              .then_label = 3,
                                              .else_label = 2,
                                          },
                                          Label{
                                              .id = 2,
                                          },
                                          SetReg32{
                                              .src_val = "4",
                                              .dst_reg = 2,
                                          },
                                          SetReg32{
                                              .src_val = "5",
                                              .dst_reg = 3,
                                          },
                                          MulReg32{
                                              .res_reg = 4,
                                              .lhs_reg = 2,
                                              .rhs_reg = 3,
                                          },
                                          MoveReg32{
                                              .src_reg = 4,
                                              .dst_reg = 0,
                                          },
                                          UncondJump{
                                              .label = 1,
                                          },
                                          Label{
                                              .id = 3,
                                          },
                                          SetReg32{
                                              .src_val = "2",
                                              .dst_reg = 5,
                                          },
                                          SetReg32{
                                              .src_val = "3",
                                              .dst_reg = 6,
                                          },
                                          AddReg32{
                                              .res_reg = 7,
                                              .lhs_reg = 5,
                                              .rhs_reg = 6,
                                          },
                                          MoveReg32{
                                              .src_reg = 7,
                                              .dst_reg = 0,
                                          },
                                          UncondJump{
                                              .label = 1,
                                          },
                                          Label{
                                              .id = 1,
                                          },
                                          PopStack{}, Return{}));
}

TEST_F(GenerateAbstractMachineFunctionTest, IfElseStmt) {
  auto func = FuncDefStmt{
      .name = I("foo"),
      .result_type = T(BasicType{.name = I("Int32")}),
      .stmts = StmtListOf({
          S(IfStmt{
              .cond = E(BoolLitExpr{.value = I("true")}),
              .then_stmts = StmtListOf({
                  S(ReturnStmt{
                      .value = E(BinaryOpExpr{
                          .op = BinaryOp::Add,
                          .lhs = E(IntLitExpr{.value = I("2")}),
                          .rhs = E(IntLitExpr{.value = I("3")}),
                      }),
                  }),
              }),
              .else_stmts = StmtListOf({
                  S(ReturnStmt{
                      .value = E(BinaryOpExpr{
                          .op = BinaryOp::Mul,
                          .lhs = E(IntLitExpr{.value = I("4")}),
                          .rhs = E(IntLitExpr{.value = I("5")}),
                      }),
                  }),
              }),
          }),
      }),
  };

  EXPECT_THAT(Generate(func), ElementsAre(PushStack{},
                                          Label{
                                              .id = 0,
                                          },
                                          SetReg32{
                                              .src_val = "1",
                                              .dst_reg = 1,
                                          },
                                          CondJump{
                                              .cond_reg = 1,
                                              .then_label = 3,
                                              .else_label = 4,
                                          },
                                          Label{
                                              .id = 4,
                                          },
                                          SetReg32{
                                              .src_val = "4",
                                              .dst_reg = 5,
                                          },
                                          SetReg32{
                                              .src_val = "5",
                                              .dst_reg = 6,
                                          },
                                          MulReg32{
                                              .res_reg = 7,
                                              .lhs_reg = 5,
                                              .rhs_reg = 6,
                                          },
                                          MoveReg32{
                                              .src_reg = 7,
                                              .dst_reg = 0,
                                          },
                                          UncondJump{
                                              .label = 1,
                                          },
                                          Label{
                                              .id = 3,
                                          },
                                          SetReg32{
                                              .src_val = "2",
                                              .dst_reg = 2,
                                          },
                                          SetReg32{
                                              .src_val = "3",
                                              .dst_reg = 3,
                                          },
                                          AddReg32{
                                              .res_reg = 4,
                                              .lhs_reg = 2,
                                              .rhs_reg = 3,
                                          },
                                          MoveReg32{
                                              .src_reg = 4,
                                              .dst_reg = 0,
                                          },
                                          UncondJump{
                                              .label = 1,
                                          },
                                          Label{
                                              .id = 1,
                                          },
                                          PopStack{}, Return{},
                                          Label{
                                              .id = 2,
                                          }));
}

TEST_F(GenerateAbstractMachineFunctionTest, GtInt) {
  auto func = FuncDefStmt{
      .name = I("foo"),
      .result_type = T(BasicType{.name = I("Bool")}),
      .stmts = StmtListOf({
          S(ReturnStmt{
              .value = E(BinaryOpExpr{
                  .op = BinaryOp::Gt,
                  .lhs = E(IntLitExpr{.value = I("3")}),
                  .rhs = E(IntLitExpr{.value = I("2")}),
              }),
          }),
      }),
  };

  EXPECT_THAT(Generate(func), ElementsAre(PushStack{},
                                          Label{
                                              .id = 0,
                                          },
                                          SetReg32{
                                              .src_val = "3",
                                              .dst_reg = 1,
                                          },
                                          SetReg32{
                                              .src_val = "2",
                                              .dst_reg = 2,
                                          },
                                          GtReg32{
                                              .res_reg = 3,
                                              .lhs_reg = 1,
                                              .rhs_reg = 2,
                                          },
                                          MoveReg32{
                                              .src_reg = 3,
                                              .dst_reg = 0,
                                          },
                                          UncondJump{
                                              .label = 1,
                                          },
                                          Label{
                                              .id = 1,
                                          },
                                          PopStack{}, Return{}));
}

TEST_F(GenerateAbstractMachineFunctionTest, GtInt32) {
  auto func = FuncDefStmt{
      .name = I("foo"),
      .params = ParamListOf({
          P(FuncParam{
              .name = I("x"),
              .type_constraint = T(BasicType{.name = I("Int32")}),
          }),
          P(FuncParam{
              .name = I("y"),
              .type_constraint = T(BasicType{.name = I("Int32")}),
          }),
      }),
      .result_type = T(BasicType{.name = I("Bool")}),
      .stmts = StmtListOf({
          S(ReturnStmt{
              .value = E(BinaryOpExpr{
                  .op = BinaryOp::Gt,
                  .lhs = E(IdentExpr{.name = I("x")}),
                  .rhs = E(IdentExpr{.name = I("y")}),
              }),
          }),
      }),
  };

  EXPECT_THAT(Generate(func), ElementsAre(PushStack{},
                                          StoreStack32{
                                              .offset = 0,
                                              .src_reg = 1,
                                          },
                                          StoreStack32{
                                              .offset = 1,
                                              .src_reg = 2,
                                          },
                                          Label{
                                              .id = 0,
                                          },
                                          LoadStack32{
                                              .offset = 0,
                                              .dst_reg = 1,
                                          },
                                          LoadStack32{
                                              .offset = 1,
                                              .dst_reg = 2,
                                          },
                                          GtReg32{
                                              .res_reg = 3,
                                              .lhs_reg = 1,
                                              .rhs_reg = 2,
                                          },
                                          MoveReg32{
                                              .src_reg = 3,
                                              .dst_reg = 0,
                                          },
                                          UncondJump{
                                              .label = 1,
                                          },
                                          Label{
                                              .id = 1,
                                          },
                                          PopStack{}, Return{}));
}

TEST_F(GenerateAbstractMachineFunctionTest, GtInt64) {
  auto func = FuncDefStmt{
      .name = I("foo"),
      .params = ParamListOf({
          P(FuncParam{
              .name = I("x"),
              .type_constraint = T(BasicType{.name = I("Int64")}),
          }),
          P(FuncParam{
              .name = I("y"),
              .type_constraint = T(BasicType{.name = I("Int64")}),
          }),
      }),
      .result_type = T(BasicType{.name = I("Bool")}),
      .stmts = StmtListOf({
          S(ReturnStmt{
              .value = E(BinaryOpExpr{
                  .op = BinaryOp::Gt,
                  .lhs = E(IdentExpr{.name = I("x")}),
                  .rhs = E(IdentExpr{.name = I("y")}),
              }),
          }),
      }),
  };

  EXPECT_THAT(Generate(func), ElementsAre(PushStack{},
                                          StoreStack64{
                                              .offset = 0,
                                              .src_reg = 1,
                                          },
                                          StoreStack64{
                                              .offset = 1,
                                              .src_reg = 2,
                                          },
                                          Label{
                                              .id = 0,
                                          },
                                          LoadStack64{
                                              .offset = 0,
                                              .dst_reg = 1,
                                          },
                                          LoadStack64{
                                              .offset = 1,
                                              .dst_reg = 2,
                                          },
                                          GtReg64{
                                              .res_reg = 3,
                                              .lhs_reg = 1,
                                              .rhs_reg = 2,
                                          },
                                          MoveReg32{
                                              .src_reg = 3,
                                              .dst_reg = 0,
                                          },
                                          UncondJump{
                                              .label = 1,
                                          },
                                          Label{
                                              .id = 1,
                                          },
                                          PopStack{}, Return{}));
}

TEST_F(GenerateAbstractMachineFunctionTest, LtInt) {
  auto func = FuncDefStmt{
      .name = I("foo"),
      .result_type = T(BasicType{.name = I("Bool")}),
      .stmts = StmtListOf({
          S(ReturnStmt{
              .value = E(BinaryOpExpr{
                  .op = BinaryOp::Lt,
                  .lhs = E(IntLitExpr{.value = I("3")}),
                  .rhs = E(IntLitExpr{.value = I("2")}),
              }),
          }),
      }),
  };

  EXPECT_THAT(Generate(func), ElementsAre(PushStack{},
                                          Label{
                                              .id = 0,
                                          },
                                          SetReg32{
                                              .src_val = "3",
                                              .dst_reg = 1,
                                          },
                                          SetReg32{
                                              .src_val = "2",
                                              .dst_reg = 2,
                                          },
                                          LtReg32{
                                              .res_reg = 3,
                                              .lhs_reg = 1,
                                              .rhs_reg = 2,
                                          },
                                          MoveReg32{
                                              .src_reg = 3,
                                              .dst_reg = 0,
                                          },
                                          UncondJump{
                                              .label = 1,
                                          },
                                          Label{
                                              .id = 1,
                                          },
                                          PopStack{}, Return{}));
}

TEST_F(GenerateAbstractMachineFunctionTest, LtInt32) {
  auto func = FuncDefStmt{
      .name = I("foo"),
      .params = ParamListOf({
          P(FuncParam{
              .name = I("x"),
              .type_constraint = T(BasicType{.name = I("Int32")}),
          }),
          P(FuncParam{
              .name = I("y"),
              .type_constraint = T(BasicType{.name = I("Int32")}),
          }),
      }),
      .result_type = T(BasicType{.name = I("Bool")}),
      .stmts = StmtListOf({
          S(ReturnStmt{
              .value = E(BinaryOpExpr{
                  .op = BinaryOp::Lt,
                  .lhs = E(IdentExpr{.name = I("x")}),
                  .rhs = E(IdentExpr{.name = I("y")}),
              }),
          }),
      }),
  };

  EXPECT_THAT(Generate(func), ElementsAre(PushStack{},
                                          StoreStack32{
                                              .offset = 0,
                                              .src_reg = 1,
                                          },
                                          StoreStack32{
                                              .offset = 1,
                                              .src_reg = 2,
                                          },
                                          Label{
                                              .id = 0,
                                          },
                                          LoadStack32{
                                              .offset = 0,
                                              .dst_reg = 1,
                                          },
                                          LoadStack32{
                                              .offset = 1,
                                              .dst_reg = 2,
                                          },
                                          LtReg32{
                                              .res_reg = 3,
                                              .lhs_reg = 1,
                                              .rhs_reg = 2,
                                          },
                                          MoveReg32{
                                              .src_reg = 3,
                                              .dst_reg = 0,
                                          },
                                          UncondJump{
                                              .label = 1,
                                          },
                                          Label{
                                              .id = 1,
                                          },
                                          PopStack{}, Return{}));
}

TEST_F(GenerateAbstractMachineFunctionTest, LtInt64) {
  auto func = FuncDefStmt{
      .name = I("foo"),
      .params = ParamListOf({
          P(FuncParam{
              .name = I("x"),
              .type_constraint = T(BasicType{.name = I("Int64")}),
          }),
          P(FuncParam{
              .name = I("y"),
              .type_constraint = T(BasicType{.name = I("Int64")}),
          }),
      }),
      .result_type = T(BasicType{.name = I("Bool")}),
      .stmts = StmtListOf({
          S(ReturnStmt{
              .value = E(BinaryOpExpr{
                  .op = BinaryOp::Lt,
                  .lhs = E(IdentExpr{.name = I("x")}),
                  .rhs = E(IdentExpr{.name = I("y")}),
              }),
          }),
      }),
  };

  EXPECT_THAT(Generate(func), ElementsAre(PushStack{},
                                          StoreStack64{
                                              .offset = 0,
                                              .src_reg = 1,
                                          },
                                          StoreStack64{
                                              .offset = 1,
                                              .src_reg = 2,
                                          },
                                          Label{
                                              .id = 0,
                                          },
                                          LoadStack64{
                                              .offset = 0,
                                              .dst_reg = 1,
                                          },
                                          LoadStack64{
                                              .offset = 1,
                                              .dst_reg = 2,
                                          },
                                          LtReg64{
                                              .res_reg = 3,
                                              .lhs_reg = 1,
                                              .rhs_reg = 2,
                                          },
                                          MoveReg32{
                                              .src_reg = 3,
                                              .dst_reg = 0,
                                          },
                                          UncondJump{
                                              .label = 1,
                                          },
                                          Label{
                                              .id = 1,
                                          },
                                          PopStack{}, Return{}));
}

TEST_F(GenerateAbstractMachineFunctionTest, EqInt32) {
  auto func = FuncDefStmt{
      .name = I("foo"),
      .params = ParamListOf({
          P(FuncParam{
              .name = I("x"),
              .type_constraint = T(BasicType{.name = I("Int32")}),
          }),
          P(FuncParam{
              .name = I("y"),
              .type_constraint = T(BasicType{.name = I("Int32")}),
          }),
      }),
      .result_type = T(BasicType{.name = I("Bool")}),
      .stmts = StmtListOf({
          S(ReturnStmt{
              .value = E(BinaryOpExpr{
                  .op = BinaryOp::Eq,
                  .lhs = E(IdentExpr{.name = I("x")}),
                  .rhs = E(IdentExpr{.name = I("y")}),
              }),
          }),
      }),
  };

  EXPECT_THAT(Generate(func), ElementsAre(PushStack{},
                                          StoreStack32{
                                              .offset = 0,
                                              .src_reg = 1,
                                          },
                                          StoreStack32{
                                              .offset = 1,
                                              .src_reg = 2,
                                          },
                                          Label{
                                              .id = 0,
                                          },
                                          LoadStack32{
                                              .offset = 0,
                                              .dst_reg = 1,
                                          },
                                          LoadStack32{
                                              .offset = 1,
                                              .dst_reg = 2,
                                          },
                                          EqReg32{
                                              .res_reg = 3,
                                              .lhs_reg = 1,
                                              .rhs_reg = 2,
                                          },
                                          MoveReg32{
                                              .src_reg = 3,
                                              .dst_reg = 0,
                                          },
                                          UncondJump{
                                              .label = 1,
                                          },
                                          Label{
                                              .id = 1,
                                          },
                                          PopStack{}, Return{}));
}

TEST_F(GenerateAbstractMachineFunctionTest, EqInt64) {
  auto func = FuncDefStmt{
      .name = I("foo"),
      .params = ParamListOf({
          P(FuncParam{
              .name = I("x"),
              .type_constraint = T(BasicType{.name = I("Int64")}),
          }),
          P(FuncParam{
              .name = I("y"),
              .type_constraint = T(BasicType{.name = I("Int64")}),
          }),
      }),
      .result_type = T(BasicType{.name = I("Bool")}),
      .stmts = StmtListOf({
          S(ReturnStmt{
              .value = E(BinaryOpExpr{
                  .op = BinaryOp::Eq,
                  .lhs = E(IdentExpr{.name = I("x")}),
                  .rhs = E(IdentExpr{.name = I("y")}),
              }),
          }),
      }),
  };

  EXPECT_THAT(Generate(func), ElementsAre(PushStack{},
                                          StoreStack64{
                                              .offset = 0,
                                              .src_reg = 1,
                                          },
                                          StoreStack64{
                                              .offset = 1,
                                              .src_reg = 2,
                                          },
                                          Label{
                                              .id = 0,
                                          },
                                          LoadStack64{
                                              .offset = 0,
                                              .dst_reg = 1,
                                          },
                                          LoadStack64{
                                              .offset = 1,
                                              .dst_reg = 2,
                                          },
                                          EqReg64{
                                              .res_reg = 3,
                                              .lhs_reg = 1,
                                              .rhs_reg = 2,
                                          },
                                          MoveReg32{
                                              .src_reg = 3,
                                              .dst_reg = 0,
                                          },
                                          UncondJump{
                                              .label = 1,
                                          },
                                          Label{
                                              .id = 1,
                                          },
                                          PopStack{}, Return{}));
}

TEST_F(GenerateAbstractMachineFunctionTest, NotEqInt32) {
  auto func = FuncDefStmt{
      .name = I("foo"),
      .params = ParamListOf({
          P(FuncParam{
              .name = I("x"),
              .type_constraint = T(BasicType{.name = I("Int32")}),
          }),
          P(FuncParam{
              .name = I("y"),
              .type_constraint = T(BasicType{.name = I("Int32")}),
          }),
      }),
      .result_type = T(BasicType{.name = I("Bool")}),
      .stmts = StmtListOf({
          S(ReturnStmt{
              .value = E(BinaryOpExpr{
                  .op = BinaryOp::NotEq,
                  .lhs = E(IdentExpr{.name = I("x")}),
                  .rhs = E(IdentExpr{.name = I("y")}),
              }),
          }),
      }),
  };

  EXPECT_THAT(Generate(func), ElementsAre(PushStack{},
                                          StoreStack32{
                                              .offset = 0,
                                              .src_reg = 1,
                                          },
                                          StoreStack32{
                                              .offset = 1,
                                              .src_reg = 2,
                                          },
                                          Label{
                                              .id = 0,
                                          },
                                          LoadStack32{
                                              .offset = 0,
                                              .dst_reg = 1,
                                          },
                                          LoadStack32{
                                              .offset = 1,
                                              .dst_reg = 2,
                                          },
                                          NotEqReg32{
                                              .res_reg = 3,
                                              .lhs_reg = 1,
                                              .rhs_reg = 2,
                                          },
                                          MoveReg32{
                                              .src_reg = 3,
                                              .dst_reg = 0,
                                          },
                                          UncondJump{
                                              .label = 1,
                                          },
                                          Label{
                                              .id = 1,
                                          },
                                          PopStack{}, Return{}));
}

TEST_F(GenerateAbstractMachineFunctionTest, NotEqInt64) {
  auto func = FuncDefStmt{
      .name = I("foo"),
      .params = ParamListOf({
          P(FuncParam{
              .name = I("x"),
              .type_constraint = T(BasicType{.name = I("Int64")}),
          }),
          P(FuncParam{
              .name = I("y"),
              .type_constraint = T(BasicType{.name = I("Int64")}),
          }),
      }),
      .result_type = T(BasicType{.name = I("Bool")}),
      .stmts = StmtListOf({
          S(ReturnStmt{
              .value = E(BinaryOpExpr{
                  .op = BinaryOp::NotEq,
                  .lhs = E(IdentExpr{.name = I("x")}),
                  .rhs = E(IdentExpr{.name = I("y")}),
              }),
          }),
      }),
  };

  EXPECT_THAT(Generate(func), ElementsAre(PushStack{},
                                          StoreStack64{
                                              .offset = 0,
                                              .src_reg = 1,
                                          },
                                          StoreStack64{
                                              .offset = 1,
                                              .src_reg = 2,
                                          },
                                          Label{
                                              .id = 0,
                                          },
                                          LoadStack64{
                                              .offset = 0,
                                              .dst_reg = 1,
                                          },
                                          LoadStack64{
                                              .offset = 1,
                                              .dst_reg = 2,
                                          },
                                          NotEqReg64{
                                              .res_reg = 3,
                                              .lhs_reg = 1,
                                              .rhs_reg = 2,
                                          },
                                          MoveReg32{
                                              .src_reg = 3,
                                              .dst_reg = 0,
                                          },
                                          UncondJump{
                                              .label = 1,
                                          },
                                          Label{
                                              .id = 1,
                                          },
                                          PopStack{}, Return{}));
}

TEST_F(GenerateAbstractMachineFunctionTest, VarDeclInt32) {
  auto func = FuncDefStmt{
      .name = I("foo"),
      .result_type = T(BasicType{.name = I("Int32")}),
      .stmts = StmtListOf({
          S(VarDeclStmt{
              .name = I("x"),
              .type_constraint = T(BasicType{.name = I("Int32")}),
              .init = E(IntLitExpr{.value = I("2")}),
          }),
          S(ReturnStmt{
              .value = E(IdentExpr{
                  .name = I("x"),
              }),
          }),
      }),
  };

  EXPECT_THAT(Generate(func), ElementsAre(PushStack{},
                                          Label{
                                              .id = 0,
                                          },
                                          SetReg32{
                                              .src_val = "2",
                                              .dst_reg = 1,
                                          },
                                          StoreStack32{
                                              .offset = 0,
                                              .src_reg = 1,
                                          },
                                          LoadStack32{
                                              .offset = 0,
                                              .dst_reg = 2,
                                          },
                                          MoveReg32{
                                              .src_reg = 2,
                                              .dst_reg = 0,
                                          },
                                          UncondJump{
                                              .label = 1,
                                          },
                                          Label{
                                              .id = 1,
                                          },
                                          PopStack{}, Return{}));
}

TEST_F(GenerateAbstractMachineFunctionTest, VarDeclInt64) {
  auto func = FuncDefStmt{
      .name = I("foo"),
      .result_type = T(BasicType{.name = I("Int64")}),
      .stmts = StmtListOf({
          S(VarDeclStmt{
              .name = I("x"),
              .type_constraint = T(BasicType{.name = I("Int64")}),
              .init = E(IntLitExpr{.value = I("2")}),
          }),
          S(ReturnStmt{
              .value = E(IdentExpr{
                  .name = I("x"),
              }),
          }),
      }),
  };

  EXPECT_THAT(Generate(func), ElementsAre(PushStack{},
                                          Label{
                                              .id = 0,
                                          },
                                          SetReg64{
                                              .src_val = "2",
                                              .dst_reg = 1,
                                          },
                                          StoreStack64{
                                              .offset = 0,
                                              .src_reg = 1,
                                          },
                                          LoadStack64{
                                              .offset = 0,
                                              .dst_reg = 2,
                                          },
                                          MoveReg64{
                                              .src_reg = 2,
                                              .dst_reg = 0,
                                          },
                                          UncondJump{
                                              .label = 1,
                                          },
                                          Label{
                                              .id = 1,
                                          },
                                          PopStack{}, Return{}));
}

TEST_F(GenerateAbstractMachineFunctionTest, VarDeclInt32Array) {
  auto func = FuncDefStmt{
      .name = I("foo"),
      .result_type = T(BasicType{.name = I("Int32")}),
      .stmts = StmtListOf({
          S(VarDeclStmt{
              .name = I("x"),
              .type_constraint = T(ArrayType{
                  .element_type_constraint = T(BasicType{.name = I("Int32")}),
                  .size = IntLitExpr{.value = I("10")},
              }),
          }),
          S(ReturnStmt{
              .value = E(IntLitExpr{
                  .value = I("0"),
              }),
          }),
      }),
  };

  EXPECT_THAT(Generate(func), ElementsAre(PushStack{},
                                          Label{
                                              .id = 0,
                                          },
                                          SetReg32{
                                              .src_val = "0",
                                              .dst_reg = 1,
                                          },
                                          MoveReg32{
                                              .src_reg = 1,
                                              .dst_reg = 0,
                                          },
                                          UncondJump{
                                              .label = 1,
                                          },
                                          Label{
                                              .id = 1,
                                          },
                                          PopStack{}, Return{}));
}

TEST_F(GenerateAbstractMachineFunctionTest, VarDeclInt64Array) {
  auto func = FuncDefStmt{
      .name = I("foo"),
      .result_type = T(BasicType{.name = I("Int64")}),
      .stmts = StmtListOf({
          S(VarDeclStmt{
              .name = I("x"),
              .type_constraint = T(ArrayType{
                  .element_type_constraint = T(BasicType{.name = I("Int64")}),
                  .size = IntLitExpr{.value = I("10")},
              }),
          }),
          S(ReturnStmt{
              .value = E(IntLitExpr{
                  .value = I("0"),
              }),
          }),
      }),
  };

  EXPECT_THAT(Generate(func), ElementsAre(PushStack{},
                                          Label{
                                              .id = 0,
                                          },
                                          SetReg64{
                                              .src_val = "0",
                                              .dst_reg = 1,
                                          },
                                          MoveReg64{
                                              .src_reg = 1,
                                              .dst_reg = 0,
                                          },
                                          UncondJump{
                                              .label = 1,
                                          },
                                          Label{
                                              .id = 1,
                                          },
                                          PopStack{}, Return{}));
}

TEST_F(GenerateAbstractMachineFunctionTest, VarAssignInt32) {
  auto func = FuncDefStmt{
      .name = I("foo"),
      .params = ParamListOf({
          P(FuncParam{
              .name = I("x"),
              .type_constraint = T(BasicType{.name = I("Int32")}),
          }),
      }),
      .result_type = T(BasicType{.name = I("Void")}),
      .stmts = StmtListOf({
          S(VarAssignStmt{
              .name = I("x"),
              .expr = E(IntLitExpr{.value = I("2")}),
          }),
      }),
  };

  EXPECT_THAT(Generate(func), ElementsAre(PushStack{},
                                          StoreStack32{
                                              .offset = 0,
                                              .src_reg = 1,
                                          },
                                          Label{
                                              .id = 0,
                                          },
                                          SetReg32{
                                              .src_val = "2",
                                              .dst_reg = 1,
                                          },
                                          StoreStack32{
                                              .offset = 0,
                                              .src_reg = 1,
                                          },
                                          UncondJump{
                                              .label = 1,
                                          },
                                          Label{
                                              .id = 1,
                                          },
                                          PopStack{}, Return{}));
}

TEST_F(GenerateAbstractMachineFunctionTest, VarAssignInt64) {
  auto func = FuncDefStmt{
      .name = I("foo"),
      .params = ParamListOf({
          P(FuncParam{
              .name = I("x"),
              .type_constraint = T(BasicType{.name = I("Int64")}),
          }),
      }),
      .result_type = T(BasicType{.name = I("Void")}),
      .stmts = StmtListOf({
          S(VarAssignStmt{
              .name = I("x"),
              .expr = E(IntLitExpr{.value = I("2")}),
          }),
      }),
  };

  EXPECT_THAT(Generate(func), ElementsAre(PushStack{},
                                          StoreStack64{
                                              .offset = 0,
                                              .src_reg = 1,
                                          },
                                          Label{
                                              .id = 0,
                                          },
                                          SetReg64{
                                              .src_val = "2",
                                              .dst_reg = 1,
                                          },
                                          StoreStack64{
                                              .offset = 0,
                                              .src_reg = 1,
                                          },
                                          UncondJump{
                                              .label = 1,
                                          },
                                          Label{
                                              .id = 1,
                                          },
                                          PopStack{}, Return{}));
}

TEST_F(GenerateAbstractMachineFunctionTest, Loop) {
  auto func = FuncDefStmt{
      .name = I("foo"),
      .result_type = T(BasicType{.name = I("Int32")}),
      .stmts = StmtListOf({
          S(LoopStmt{
              .stmts = StmtListOf({
                  S(ReturnStmt{
                      .value = E(IntLitExpr{.value = I("1")}),
                  }),
              }),
          }),
          S(ReturnStmt{
              .value = E(IntLitExpr{.value = I("2")}),
          }),
      }),
  };

  EXPECT_THAT(Generate(func), ElementsAre(PushStack{},
                                          Label{
                                              .id = 0,
                                          },
                                          UncondJump{
                                              .label = 3,
                                          },
                                          Label{
                                              .id = 3,
                                          },
                                          SetReg32{
                                              .src_val = "1",
                                              .dst_reg = 2,
                                          },
                                          MoveReg32{
                                              .src_reg = 2,
                                              .dst_reg = 0,
                                          },
                                          UncondJump{
                                              .label = 1,
                                          },
                                          Label{
                                              .id = 1,
                                          },
                                          PopStack{}, Return{},
                                          Label{
                                              .id = 2,
                                          },
                                          SetReg32{
                                              .src_val = "2",
                                              .dst_reg = 1,
                                          },
                                          MoveReg32{
                                              .src_reg = 1,
                                              .dst_reg = 0,
                                          },
                                          UncondJump{
                                              .label = 1,
                                          }));
}

TEST_F(GenerateAbstractMachineFunctionTest, SingleLoopAndBreak) {
  auto func = FuncDefStmt{
      .name = I("foo"),
      .result_type = T(BasicType{.name = I("Int32")}),
      .stmts = StmtListOf({
          S(VarDeclStmt{
              .name = I("n"),
              .type_constraint = T(BasicType{.name = I("Int32")}),
              .init = E(IntLitExpr{
                  .value = I("0"),
              }),
          }),
          S(LoopStmt{
              .stmts = StmtListOf({
                  S(IfStmt{
                      .cond = E(BinaryOpExpr{
                          .op = BinaryOp::Gt,
                          .lhs = E(IdentExpr{.name = I("n")}),
                          .rhs = E(IntLitExpr{.value = I("3")}),
                      }),
                      .then_stmts = StmtListOf({S(BreakStmt{})}),
                  }),
                  S(VarAssignStmt{
                      .name = I("n"),
                      .expr = E(BinaryOpExpr{
                          .op = BinaryOp::Add,
                          .lhs = E(IdentExpr{.name = I("n")}),
                          .rhs = E(IntLitExpr{.value = I("1")}),
                      }),
                  }),
              }),
          }),
          S(ReturnStmt{
              .value = E(IdentExpr{.name = I("n")}),
          }),
      }),
  };

  EXPECT_THAT(Generate(func), ElementsAre(PushStack{},
                                          Label{
                                              .id = 0,
                                          },
                                          SetReg32{
                                              .src_val = "0",
                                              .dst_reg = 1,
                                          },
                                          StoreStack32{
                                              .offset = 0,
                                              .src_reg = 1,
                                          },
                                          UncondJump{
                                              .label = 3,
                                          },
                                          Label{
                                              .id = 3,
                                          },
                                          LoadStack32{
                                              .offset = 0,
                                              .dst_reg = 3,
                                          },
                                          SetReg32{
                                              .src_val = "3",
                                              .dst_reg = 4,
                                          },
                                          GtReg32{
                                              .res_reg = 5,
                                              .lhs_reg = 3,
                                              .rhs_reg = 4,
                                          },
                                          CondJump{
                                              .cond_reg = 5,
                                              .then_label = 5,
                                              .else_label = 4,
                                          },
                                          Label{
                                              .id = 4,
                                          },
                                          LoadStack32{
                                              .offset = 0,
                                              .dst_reg = 6,
                                          },
                                          SetReg32{
                                              .src_val = "1",
                                              .dst_reg = 7,
                                          },
                                          AddReg32{
                                              .res_reg = 8,
                                              .lhs_reg = 6,
                                              .rhs_reg = 7,
                                          },
                                          StoreStack32{
                                              .offset = 0,
                                              .src_reg = 8,
                                          },
                                          UncondJump{
                                              .label = 3,
                                          },
                                          Label{
                                              .id = 5,
                                          },
                                          UncondJump{
                                              .label = 2,
                                          },
                                          Label{
                                              .id = 2,
                                          },
                                          LoadStack32{
                                              .offset = 0,
                                              .dst_reg = 2,
                                          },
                                          MoveReg32{
                                              .src_reg = 2,
                                              .dst_reg = 0,
                                          },
                                          UncondJump{
                                              .label = 1,
                                          },
                                          Label{
                                              .id = 1,
                                          },
                                          PopStack{}, Return{}));
}

}  // namespace
}  // namespace lucid
