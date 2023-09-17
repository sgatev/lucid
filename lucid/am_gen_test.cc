#include "lucid/am_gen.h"

#include <vector>

#include "gmock/gmock.h"
#include "gtest/gtest.h"
#include "lucid/am.h"
#include "lucid/arena.h"
#include "lucid/ast.h"
#include "lucid/ast_fixture.h"
#include "lucid/cfg.h"
#include "lucid/type.h"

namespace lucid {
namespace {

using ::testing::ElementsAre;

class GenerateAbstractMachineInstructionsTest : public testing::Test,
                                                public AstFixture {
 protected:
  std::vector<Instruction> Generate(FuncDefStmt& func) {
    InferExpressionTypes(arena_, func);
    auto graph = BuildControlFlowGraph(arena_, func);
    AbstractMachineState state;
    GenerateAbstractMachineInstructions(arena_, graph, state);
    return state.instructions;
  }
};

TEST_F(GenerateAbstractMachineInstructionsTest, ReturnInt32Lit) {
  auto func = FuncDefStmt{
      .name = "foo",
      .result_type = "Int32",
      .body =
          {
              .statements =
                  {
                      Allocate(ReturnStmt{
                          .value = Allocate(IntLitExpr{.value = "21"}),
                      }),
                  },
          },
  };

  EXPECT_THAT(Generate(func), ElementsAre(
                                  PushStack{
                                      .size = 8,
                                  },
                                  StoreStack64{
                                      .offset = 0,
                                      .src_reg = 1,
                                  },
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
                                  LoadStack64{
                                      .offset = 0,
                                      .dst_reg = 1,
                                  },
                                  PopStack{
                                      .size = 8,
                                  },
                                  Return{}));
}

TEST_F(GenerateAbstractMachineInstructionsTest, ReturnInt64Lit) {
  auto func = FuncDefStmt{
      .name = "foo",
      .result_type = "Int64",
      .body =
          {
              .statements =
                  {
                      Allocate(ReturnStmt{
                          .value = Allocate(IntLitExpr{.value = "21"}),
                      }),
                  },
          },
  };

  EXPECT_THAT(Generate(func), ElementsAre(
                                  PushStack{
                                      .size = 8,
                                  },
                                  StoreStack64{
                                      .offset = 0,
                                      .src_reg = 1,
                                  },
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
                                  LoadStack64{
                                      .offset = 0,
                                      .dst_reg = 1,
                                  },
                                  PopStack{
                                      .size = 8,
                                  },
                                  Return{}));
}

TEST_F(GenerateAbstractMachineInstructionsTest, FuncCallWithArg) {
  auto func = FuncDefStmt{
      .name = "foo",
      .result_type = "Int32",
      .body =
          {
              .statements =
                  {
                      Allocate(ReturnStmt{
                          .value = Allocate(FuncCallExpr{
                              .func_name = "id",
                              .arguments =
                                  {
                                      Allocate(IntLitExpr{.value = "21"}),
                                  },
                          }),
                      }),
                  },
          },
  };

  EXPECT_THAT(Generate(func), ElementsAre(
                                  PushStack{
                                      .size = 16,
                                  },
                                  StoreStack64{
                                      .offset = 0,
                                      .src_reg = 1,
                                  },
                                  StoreStack64{
                                      .offset = 8,
                                      .src_reg = 2,
                                  },
                                  Label{
                                      .id = 0,
                                  },
                                  SetReg32{
                                      .src_val = "21",
                                      .dst_reg = 1,
                                  },
                                  MoveReg32{
                                      .src_reg = 1,
                                      .dst_reg = 1,
                                  },
                                  Jump{
                                      .label = "id",
                                  },
                                  MoveReg32{
                                      .src_reg = 0,
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
                                  LoadStack64{
                                      .offset = 0,
                                      .dst_reg = 1,
                                  },
                                  LoadStack64{
                                      .offset = 8,
                                      .dst_reg = 2,
                                  },
                                  PopStack{
                                      .size = 16,
                                  },
                                  Return{}));
}

TEST_F(GenerateAbstractMachineInstructionsTest, AddInt32) {
  auto func = FuncDefStmt{
      .name = "foo",
      .result_type = "Int32",
      .body =
          {
              .statements =
                  {
                      Allocate(ReturnStmt{
                          .value = Allocate(BinaryOpExpr{
                              .op = BinaryOp::Add,
                              .lhs = Allocate(IntLitExpr{.value = "2"}),
                              .rhs = Allocate(IntLitExpr{.value = "3"}),
                          }),
                      }),
                  },
          },
  };

  EXPECT_THAT(Generate(func), ElementsAre(
                                  PushStack{
                                      .size = 24,
                                  },
                                  StoreStack64{
                                      .offset = 0,
                                      .src_reg = 1,
                                  },
                                  StoreStack64{
                                      .offset = 8,
                                      .src_reg = 2,
                                  },
                                  StoreStack64{
                                      .offset = 16,
                                      .src_reg = 3,
                                  },
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
                                  LoadStack64{
                                      .offset = 0,
                                      .dst_reg = 1,
                                  },
                                  LoadStack64{
                                      .offset = 8,
                                      .dst_reg = 2,
                                  },
                                  LoadStack64{
                                      .offset = 16,
                                      .dst_reg = 3,
                                  },
                                  PopStack{
                                      .size = 24,
                                  },
                                  Return{}));
}

TEST_F(GenerateAbstractMachineInstructionsTest, AddInt64) {
  auto func = FuncDefStmt{
      .name = "foo",
      .result_type = "Int64",
      .body =
          {
              .statements =
                  {
                      Allocate(ReturnStmt{
                          .value = Allocate(BinaryOpExpr{
                              .op = BinaryOp::Add,
                              .lhs = Allocate(IntLitExpr{.value = "2"}),
                              .rhs = Allocate(IntLitExpr{.value = "3"}),
                          }),
                      }),
                  },
          },
  };

  EXPECT_THAT(Generate(func), ElementsAre(
                                  PushStack{
                                      .size = 24,
                                  },
                                  StoreStack64{
                                      .offset = 0,
                                      .src_reg = 1,
                                  },
                                  StoreStack64{
                                      .offset = 8,
                                      .src_reg = 2,
                                  },
                                  StoreStack64{
                                      .offset = 16,
                                      .src_reg = 3,
                                  },
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
                                  LoadStack64{
                                      .offset = 0,
                                      .dst_reg = 1,
                                  },
                                  LoadStack64{
                                      .offset = 8,
                                      .dst_reg = 2,
                                  },
                                  LoadStack64{
                                      .offset = 16,
                                      .dst_reg = 3,
                                  },
                                  PopStack{
                                      .size = 24,
                                  },
                                  Return{}));
}

TEST_F(GenerateAbstractMachineInstructionsTest, SubtractInt32) {
  auto func = FuncDefStmt{
      .name = "foo",
      .result_type = "Int32",
      .body =
          {
              .statements =
                  {
                      Allocate(ReturnStmt{
                          .value = Allocate(BinaryOpExpr{
                              .op = BinaryOp::Sub,
                              .lhs = Allocate(IntLitExpr{.value = "7"}),
                              .rhs = Allocate(IntLitExpr{.value = "5"}),
                          }),
                      }),
                  },
          },
  };

  EXPECT_THAT(Generate(func), ElementsAre(
                                  PushStack{
                                      .size = 24,
                                  },
                                  StoreStack64{
                                      .offset = 0,
                                      .src_reg = 1,
                                  },
                                  StoreStack64{
                                      .offset = 8,
                                      .src_reg = 2,
                                  },
                                  StoreStack64{
                                      .offset = 16,
                                      .src_reg = 3,
                                  },
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
                                  LoadStack64{
                                      .offset = 0,
                                      .dst_reg = 1,
                                  },
                                  LoadStack64{
                                      .offset = 8,
                                      .dst_reg = 2,
                                  },
                                  LoadStack64{
                                      .offset = 16,
                                      .dst_reg = 3,
                                  },
                                  PopStack{
                                      .size = 24,
                                  },
                                  Return{}));
}

TEST_F(GenerateAbstractMachineInstructionsTest, SubtractInt64) {
  auto func = FuncDefStmt{
      .name = "foo",
      .result_type = "Int64",
      .body =
          {
              .statements =
                  {
                      Allocate(ReturnStmt{
                          .value = Allocate(BinaryOpExpr{
                              .op = BinaryOp::Sub,
                              .lhs = Allocate(IntLitExpr{.value = "7"}),
                              .rhs = Allocate(IntLitExpr{.value = "5"}),
                          }),
                      }),
                  },
          },
  };

  EXPECT_THAT(Generate(func), ElementsAre(
                                  PushStack{
                                      .size = 24,
                                  },
                                  StoreStack64{
                                      .offset = 0,
                                      .src_reg = 1,
                                  },
                                  StoreStack64{
                                      .offset = 8,
                                      .src_reg = 2,
                                  },
                                  StoreStack64{
                                      .offset = 16,
                                      .src_reg = 3,
                                  },
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
                                  LoadStack64{
                                      .offset = 0,
                                      .dst_reg = 1,
                                  },
                                  LoadStack64{
                                      .offset = 8,
                                      .dst_reg = 2,
                                  },
                                  LoadStack64{
                                      .offset = 16,
                                      .dst_reg = 3,
                                  },
                                  PopStack{
                                      .size = 24,
                                  },
                                  Return{}));
}

TEST_F(GenerateAbstractMachineInstructionsTest, MultiplyInt32) {
  auto func = FuncDefStmt{
      .name = "foo",
      .result_type = "Int32",
      .body =
          {
              .statements =
                  {
                      Allocate(ReturnStmt{
                          .value = Allocate(BinaryOpExpr{
                              .op = BinaryOp::Mul,
                              .lhs = Allocate(IntLitExpr{.value = "2"}),
                              .rhs = Allocate(IntLitExpr{.value = "3"}),
                          }),
                      }),
                  },
          },
  };

  EXPECT_THAT(Generate(func), ElementsAre(
                                  PushStack{
                                      .size = 24,
                                  },
                                  StoreStack64{
                                      .offset = 0,
                                      .src_reg = 1,
                                  },
                                  StoreStack64{
                                      .offset = 8,
                                      .src_reg = 2,
                                  },
                                  StoreStack64{
                                      .offset = 16,
                                      .src_reg = 3,
                                  },
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
                                  LoadStack64{
                                      .offset = 0,
                                      .dst_reg = 1,
                                  },
                                  LoadStack64{
                                      .offset = 8,
                                      .dst_reg = 2,
                                  },
                                  LoadStack64{
                                      .offset = 16,
                                      .dst_reg = 3,
                                  },
                                  PopStack{
                                      .size = 24,
                                  },
                                  Return{}));
}

TEST_F(GenerateAbstractMachineInstructionsTest, MultiplyInt64) {
  auto func = FuncDefStmt{
      .name = "foo",
      .result_type = "Int64",
      .body =
          {
              .statements =
                  {
                      Allocate(ReturnStmt{
                          .value = Allocate(BinaryOpExpr{
                              .op = BinaryOp::Mul,
                              .lhs = Allocate(IntLitExpr{.value = "2"}),
                              .rhs = Allocate(IntLitExpr{.value = "3"}),
                          }),
                      }),
                  },
          },
  };

  EXPECT_THAT(Generate(func), ElementsAre(
                                  PushStack{
                                      .size = 24,
                                  },
                                  StoreStack64{
                                      .offset = 0,
                                      .src_reg = 1,
                                  },
                                  StoreStack64{
                                      .offset = 8,
                                      .src_reg = 2,
                                  },
                                  StoreStack64{
                                      .offset = 16,
                                      .src_reg = 3,
                                  },
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
                                  LoadStack64{
                                      .offset = 0,
                                      .dst_reg = 1,
                                  },
                                  LoadStack64{
                                      .offset = 8,
                                      .dst_reg = 2,
                                  },
                                  LoadStack64{
                                      .offset = 16,
                                      .dst_reg = 3,
                                  },
                                  PopStack{
                                      .size = 24,
                                  },
                                  Return{}));
}

TEST_F(GenerateAbstractMachineInstructionsTest, DivideInt32) {
  auto func = FuncDefStmt{
      .name = "foo",
      .result_type = "Int32",
      .body =
          {
              .statements =
                  {
                      Allocate(ReturnStmt{
                          .value = Allocate(BinaryOpExpr{
                              .op = BinaryOp::Div,
                              .lhs = Allocate(IntLitExpr{.value = "8"}),
                              .rhs = Allocate(IntLitExpr{.value = "2"}),
                          }),
                      }),
                  },
          },
  };

  EXPECT_THAT(Generate(func), ElementsAre(
                                  PushStack{
                                      .size = 24,
                                  },
                                  StoreStack64{
                                      .offset = 0,
                                      .src_reg = 1,
                                  },
                                  StoreStack64{
                                      .offset = 8,
                                      .src_reg = 2,
                                  },
                                  StoreStack64{
                                      .offset = 16,
                                      .src_reg = 3,
                                  },
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
                                  LoadStack64{
                                      .offset = 0,
                                      .dst_reg = 1,
                                  },
                                  LoadStack64{
                                      .offset = 8,
                                      .dst_reg = 2,
                                  },
                                  LoadStack64{
                                      .offset = 16,
                                      .dst_reg = 3,
                                  },
                                  PopStack{
                                      .size = 24,
                                  },
                                  Return{}));
}

TEST_F(GenerateAbstractMachineInstructionsTest, DivideInt64) {
  auto func = FuncDefStmt{
      .name = "foo",
      .result_type = "Int64",
      .body =
          {
              .statements =
                  {
                      Allocate(ReturnStmt{
                          .value = Allocate(BinaryOpExpr{
                              .op = BinaryOp::Div,
                              .lhs = Allocate(IntLitExpr{.value = "8"}),
                              .rhs = Allocate(IntLitExpr{.value = "2"}),
                          }),
                      }),
                  },
          },
  };

  EXPECT_THAT(Generate(func), ElementsAre(
                                  PushStack{
                                      .size = 24,
                                  },
                                  StoreStack64{
                                      .offset = 0,
                                      .src_reg = 1,
                                  },
                                  StoreStack64{
                                      .offset = 8,
                                      .src_reg = 2,
                                  },
                                  StoreStack64{
                                      .offset = 16,
                                      .src_reg = 3,
                                  },
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
                                  LoadStack64{
                                      .offset = 0,
                                      .dst_reg = 1,
                                  },
                                  LoadStack64{
                                      .offset = 8,
                                      .dst_reg = 2,
                                  },
                                  LoadStack64{
                                      .offset = 16,
                                      .dst_reg = 3,
                                  },
                                  PopStack{
                                      .size = 24,
                                  },
                                  Return{}));
}

TEST_F(GenerateAbstractMachineInstructionsTest, IfStmt) {
  auto return_add_expr = Allocate(ReturnStmt{
      .value = Allocate(BinaryOpExpr{
          .op = BinaryOp::Add,
          .lhs = Allocate(IntLitExpr{.value = "2"}),
          .rhs = Allocate(IntLitExpr{.value = "3"}),
      }),
  });
  auto return_mul_expr = Allocate(ReturnStmt{
      .value = Allocate(BinaryOpExpr{
          .op = BinaryOp::Mul,
          .lhs = Allocate(IntLitExpr{.value = "4"}),
          .rhs = Allocate(IntLitExpr{.value = "5"}),
      }),
  });
  auto if_stmt = Allocate(IfStmt{
      .cond = Allocate(BoolLitExpr{.value = "true"}),
      .then_body = {.statements =
                        {
                            return_add_expr,
                        }},
      .else_body = {.statements =
                        {
                            return_mul_expr,
                        }},
  });
  auto func = FuncDefStmt{
      .name = "foo",
      .result_type = "Int32",
      .body =
          {
              .statements =
                  {
                      if_stmt,
                  },
          },
  };

  EXPECT_THAT(Generate(func), ElementsAre(
                                  PushStack{
                                      .size = 56,
                                  },
                                  StoreStack64{
                                      .offset = 0,
                                      .src_reg = 1,
                                  },
                                  StoreStack64{
                                      .offset = 8,
                                      .src_reg = 2,
                                  },
                                  StoreStack64{
                                      .offset = 16,
                                      .src_reg = 3,
                                  },
                                  StoreStack64{
                                      .offset = 24,
                                      .src_reg = 4,
                                  },
                                  StoreStack64{
                                      .offset = 32,
                                      .src_reg = 5,
                                  },
                                  StoreStack64{
                                      .offset = 40,
                                      .src_reg = 6,
                                  },
                                  StoreStack64{
                                      .offset = 48,
                                      .src_reg = 7,
                                  },
                                  Label{
                                      .id = 0,
                                  },
                                  SetReg32{
                                      .src_val = "1",
                                      .dst_reg = 1,
                                  },
                                  CondJump{
                                      .cond_reg = 1,
                                      .then_label = 2,
                                      .else_label = 3,
                                  },
                                  Label{
                                      .id = 1,
                                  },
                                  LoadStack64{
                                      .offset = 0,
                                      .dst_reg = 1,
                                  },
                                  LoadStack64{
                                      .offset = 8,
                                      .dst_reg = 2,
                                  },
                                  LoadStack64{
                                      .offset = 16,
                                      .dst_reg = 3,
                                  },
                                  LoadStack64{
                                      .offset = 24,
                                      .dst_reg = 4,
                                  },
                                  LoadStack64{
                                      .offset = 32,
                                      .dst_reg = 5,
                                  },
                                  LoadStack64{
                                      .offset = 40,
                                      .dst_reg = 6,
                                  },
                                  LoadStack64{
                                      .offset = 48,
                                      .dst_reg = 7,
                                  },
                                  PopStack{
                                      .size = 56,
                                  },
                                  Return{},
                                  Label{
                                      .id = 2,
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
                                      .id = 3,
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
                                  }));
}

TEST_F(GenerateAbstractMachineInstructionsTest, GtInt32) {
  auto func = FuncDefStmt{
      .name = "foo",
      .result_type = "Int32",
      .body =
          {
              .statements =
                  {
                      Allocate(ReturnStmt{
                          .value = Allocate(BinaryOpExpr{
                              .op = BinaryOp::Gt,
                              .lhs = Allocate(IntLitExpr{.value = "3"}),
                              .rhs = Allocate(IntLitExpr{.value = "2"}),
                          }),
                      }),
                  },
          },
  };

  EXPECT_THAT(Generate(func), ElementsAre(
                                  PushStack{
                                      .size = 24,
                                  },
                                  StoreStack64{
                                      .offset = 0,
                                      .src_reg = 1,
                                  },
                                  StoreStack64{
                                      .offset = 8,
                                      .src_reg = 2,
                                  },
                                  StoreStack64{
                                      .offset = 16,
                                      .src_reg = 3,
                                  },
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
                                  LoadStack64{
                                      .offset = 0,
                                      .dst_reg = 1,
                                  },
                                  LoadStack64{
                                      .offset = 8,
                                      .dst_reg = 2,
                                  },
                                  LoadStack64{
                                      .offset = 16,
                                      .dst_reg = 3,
                                  },
                                  PopStack{
                                      .size = 24,
                                  },
                                  Return{}));
}

TEST_F(GenerateAbstractMachineInstructionsTest, GtInt64) {
  auto func = FuncDefStmt{
      .name = "foo",
      .result_type = "Int64",
      .body =
          {
              .statements =
                  {
                      Allocate(ReturnStmt{
                          .value = Allocate(BinaryOpExpr{
                              .op = BinaryOp::Gt,
                              .lhs = Allocate(IntLitExpr{.value = "3"}),
                              .rhs = Allocate(IntLitExpr{.value = "2"}),
                          }),
                      }),
                  },
          },
  };

  EXPECT_THAT(Generate(func), ElementsAre(
                                  PushStack{
                                      .size = 24,
                                  },
                                  StoreStack64{
                                      .offset = 0,
                                      .src_reg = 1,
                                  },
                                  StoreStack64{
                                      .offset = 8,
                                      .src_reg = 2,
                                  },
                                  StoreStack64{
                                      .offset = 16,
                                      .src_reg = 3,
                                  },
                                  Label{
                                      .id = 0,
                                  },
                                  SetReg64{
                                      .src_val = "3",
                                      .dst_reg = 1,
                                  },
                                  SetReg64{
                                      .src_val = "2",
                                      .dst_reg = 2,
                                  },
                                  GtReg64{
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
                                  LoadStack64{
                                      .offset = 0,
                                      .dst_reg = 1,
                                  },
                                  LoadStack64{
                                      .offset = 8,
                                      .dst_reg = 2,
                                  },
                                  LoadStack64{
                                      .offset = 16,
                                      .dst_reg = 3,
                                  },
                                  PopStack{
                                      .size = 24,
                                  },
                                  Return{}));
}

TEST_F(GenerateAbstractMachineInstructionsTest, LtInt32) {
  auto func = FuncDefStmt{
      .name = "foo",
      .result_type = "Int32",
      .body =
          {
              .statements =
                  {
                      Allocate(ReturnStmt{
                          .value = Allocate(BinaryOpExpr{
                              .op = BinaryOp::Lt,
                              .lhs = Allocate(IntLitExpr{.value = "3"}),
                              .rhs = Allocate(IntLitExpr{.value = "2"}),
                          }),
                      }),
                  },
          },
  };

  EXPECT_THAT(Generate(func), ElementsAre(
                                  PushStack{
                                      .size = 24,
                                  },
                                  StoreStack64{
                                      .offset = 0,
                                      .src_reg = 1,
                                  },
                                  StoreStack64{
                                      .offset = 8,
                                      .src_reg = 2,
                                  },
                                  StoreStack64{
                                      .offset = 16,
                                      .src_reg = 3,
                                  },
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
                                  LoadStack64{
                                      .offset = 0,
                                      .dst_reg = 1,
                                  },
                                  LoadStack64{
                                      .offset = 8,
                                      .dst_reg = 2,
                                  },
                                  LoadStack64{
                                      .offset = 16,
                                      .dst_reg = 3,
                                  },
                                  PopStack{
                                      .size = 24,
                                  },
                                  Return{}));
}

TEST_F(GenerateAbstractMachineInstructionsTest, LtInt64) {
  auto func = FuncDefStmt{
      .name = "foo",
      .result_type = "Int64",
      .body =
          {
              .statements =
                  {
                      Allocate(ReturnStmt{
                          .value = Allocate(BinaryOpExpr{
                              .op = BinaryOp::Lt,
                              .lhs = Allocate(IntLitExpr{.value = "3"}),
                              .rhs = Allocate(IntLitExpr{.value = "2"}),
                          }),
                      }),
                  },
          },
  };

  EXPECT_THAT(Generate(func), ElementsAre(
                                  PushStack{
                                      .size = 24,
                                  },
                                  StoreStack64{
                                      .offset = 0,
                                      .src_reg = 1,
                                  },
                                  StoreStack64{
                                      .offset = 8,
                                      .src_reg = 2,
                                  },
                                  StoreStack64{
                                      .offset = 16,
                                      .src_reg = 3,
                                  },
                                  Label{
                                      .id = 0,
                                  },
                                  SetReg64{
                                      .src_val = "3",
                                      .dst_reg = 1,
                                  },
                                  SetReg64{
                                      .src_val = "2",
                                      .dst_reg = 2,
                                  },
                                  LtReg64{
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
                                  LoadStack64{
                                      .offset = 0,
                                      .dst_reg = 1,
                                  },
                                  LoadStack64{
                                      .offset = 8,
                                      .dst_reg = 2,
                                  },
                                  LoadStack64{
                                      .offset = 16,
                                      .dst_reg = 3,
                                  },
                                  PopStack{
                                      .size = 24,
                                  },
                                  Return{}));
}

TEST_F(GenerateAbstractMachineInstructionsTest, EqInt32) {
  auto func = FuncDefStmt{
      .name = "foo",
      .result_type = "Int32",
      .parameters =
          {
              {
                  {
                      .name = "x",
                      .type = "Int32",
                  },
                  {
                      .name = "y",
                      .type = "Int32",
                  },
              },
          },
      .body =
          {
              .statements =
                  {
                      Allocate(ReturnStmt{
                          .value = Allocate(BinaryOpExpr{
                              .op = BinaryOp::Eq,
                              .lhs = Allocate(IdentExpr{.name = "x"}),
                              .rhs = Allocate(IdentExpr{.name = "y"}),
                          }),
                      }),
                  },
          },
  };

  EXPECT_THAT(Generate(func), ElementsAre(
                                  PushStack{
                                      .size = 32,
                                  },
                                  StoreStack64{
                                      .offset = 8,
                                      .src_reg = 1,
                                  },
                                  StoreStack64{
                                      .offset = 16,
                                      .src_reg = 2,
                                  },
                                  StoreStack64{
                                      .offset = 24,
                                      .src_reg = 3,
                                  },
                                  StoreStack32{
                                      .offset = 0,
                                      .src_reg = 1,
                                  },
                                  StoreStack32{
                                      .offset = 4,
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
                                      .offset = 4,
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
                                  LoadStack64{
                                      .offset = 8,
                                      .dst_reg = 1,
                                  },
                                  LoadStack64{
                                      .offset = 16,
                                      .dst_reg = 2,
                                  },
                                  LoadStack64{
                                      .offset = 24,
                                      .dst_reg = 3,
                                  },
                                  PopStack{
                                      .size = 32,
                                  },
                                  Return{}));
}

TEST_F(GenerateAbstractMachineInstructionsTest, EqInt64) {
  auto func = FuncDefStmt{
      .name = "foo",
      .result_type = "Int64",
      .parameters =
          {
              {
                  {
                      .name = "x",
                      .type = "Int64",
                  },
                  {
                      .name = "y",
                      .type = "Int64",
                  },
              },
          },
      .body =
          {
              .statements =
                  {
                      Allocate(ReturnStmt{
                          .value = Allocate(BinaryOpExpr{
                              .op = BinaryOp::Eq,
                              .lhs = Allocate(IdentExpr{.name = "x"}),
                              .rhs = Allocate(IdentExpr{.name = "y"}),
                          }),
                      }),
                  },
          },
  };

  EXPECT_THAT(Generate(func), ElementsAre(
                                  PushStack{
                                      .size = 40,
                                  },
                                  StoreStack64{
                                      .offset = 16,
                                      .src_reg = 1,
                                  },
                                  StoreStack64{
                                      .offset = 24,
                                      .src_reg = 2,
                                  },
                                  StoreStack64{
                                      .offset = 32,
                                      .src_reg = 3,
                                  },
                                  StoreStack64{
                                      .offset = 0,
                                      .src_reg = 1,
                                  },
                                  StoreStack64{
                                      .offset = 8,
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
                                      .offset = 8,
                                      .dst_reg = 2,
                                  },
                                  EqReg64{
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
                                  LoadStack64{
                                      .offset = 16,
                                      .dst_reg = 1,
                                  },
                                  LoadStack64{
                                      .offset = 24,
                                      .dst_reg = 2,
                                  },
                                  LoadStack64{
                                      .offset = 32,
                                      .dst_reg = 3,
                                  },
                                  PopStack{
                                      .size = 40,
                                  },
                                  Return{}));
}

TEST_F(GenerateAbstractMachineInstructionsTest, VarDeclInt32) {
  auto func = FuncDefStmt{
      .name = "foo",
      .result_type = "Int32",
      .body =
          {
              .statements =
                  {
                      Allocate(VarDeclStmt{
                          .name = "x",
                          .type = "Int32",
                          .init = Allocate(IntLitExpr{.value = "2"}),
                      }),
                      Allocate(ReturnStmt{
                          .value = Allocate(IdentExpr{
                              .name = "x",
                          }),
                      }),
                  },
          },
  };

  EXPECT_THAT(Generate(func), ElementsAre(
                                  PushStack{
                                      .size = 20,
                                  },
                                  StoreStack64{
                                      .offset = 4,
                                      .src_reg = 1,
                                  },
                                  StoreStack64{
                                      .offset = 12,
                                      .src_reg = 2,
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
                                  LoadStack64{
                                      .offset = 4,
                                      .dst_reg = 1,
                                  },
                                  LoadStack64{
                                      .offset = 12,
                                      .dst_reg = 2,
                                  },
                                  PopStack{
                                      .size = 20,
                                  },
                                  Return{}));
}

TEST_F(GenerateAbstractMachineInstructionsTest, VarDeclInt64) {
  auto func = FuncDefStmt{
      .name = "foo",
      .result_type = "Int64",
      .body =
          {
              .statements =
                  {
                      Allocate(VarDeclStmt{
                          .name = "x",
                          .type = "Int64",
                          .init = Allocate(IntLitExpr{.value = "2"}),
                      }),
                      Allocate(ReturnStmt{
                          .value = Allocate(IdentExpr{
                              .name = "x",
                          }),
                      }),
                  },
          },
  };

  EXPECT_THAT(Generate(func), ElementsAre(
                                  PushStack{
                                      .size = 20,
                                  },
                                  StoreStack64{
                                      .offset = 4,
                                      .src_reg = 1,
                                  },
                                  StoreStack64{
                                      .offset = 12,
                                      .src_reg = 2,
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
                                  LoadStack64{
                                      .offset = 4,
                                      .dst_reg = 1,
                                  },
                                  LoadStack64{
                                      .offset = 12,
                                      .dst_reg = 2,
                                  },
                                  PopStack{
                                      .size = 20,
                                  },
                                  Return{}));
}

}  // namespace
}  // namespace lucid
