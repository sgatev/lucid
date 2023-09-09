#include "lucid/am_gen.h"

#include <vector>

#include "gmock/gmock.h"
#include "gtest/gtest.h"
#include "lucid/am.h"
#include "lucid/arena.h"
#include "lucid/ast.h"
#include "lucid/cfg.h"

namespace lucid {
namespace {

using ::testing::ElementsAre;

class GenerateAbstractMachineInstructionsTest : public testing::Test {
 protected:
  template <typename T>
  StmtRef Allocate(T stmt) {
    return arena_.add(stmt);
  }

  std::vector<Instruction> Generate(const FuncDefStmt& func) {
    auto graph = BuildControlFlowGraph(arena_, func);
    AbstractMachineState state;
    GenerateAbstractMachineInstructions(arena_, graph, state);
    return state.instructions;
  }

 private:
  Arena<Stmt> arena_;
};

TEST_F(GenerateAbstractMachineInstructionsTest, ReturnIntLit) {
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
                                      .size = 0,
                                  },
                                  SetReg32{
                                      .src_val = "21",
                                      .dst_reg = 1,
                                  },
                                  MoveReg32{
                                      .src_reg = 1,
                                      .dst_reg = 0,
                                  },
                                  PopStack{
                                      .size = 0,
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
                                      .size = 0,
                                  },
                                  SetReg32{
                                      .src_val = "21",
                                      .dst_reg = 1,
                                  },
                                  Jump{
                                      .label = "id",
                                  },
                                  MoveReg32{
                                      .src_reg = 0,
                                      .dst_reg = 0,
                                  },
                                  PopStack{
                                      .size = 0,
                                  },
                                  Return{}));
}

TEST_F(GenerateAbstractMachineInstructionsTest, AddInts) {
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
                                      .size = 0,
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
                                  PopStack{
                                      .size = 0,
                                  },
                                  Return{}));
}

TEST_F(GenerateAbstractMachineInstructionsTest, SubtractInts) {
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
                                      .size = 0,
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
                                  PopStack{
                                      .size = 0,
                                  },
                                  Return{}));
}

TEST_F(GenerateAbstractMachineInstructionsTest, MultiplyInts) {
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
                                      .size = 0,
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
                                  PopStack{
                                      .size = 0,
                                  },
                                  Return{}));
}

TEST_F(GenerateAbstractMachineInstructionsTest, DivideInts) {
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
                                      .size = 0,
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
                                  PopStack{
                                      .size = 0,
                                  },
                                  Return{}));
}

TEST_F(GenerateAbstractMachineInstructionsTest, AddBools) {
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
                              .lhs = Allocate(BoolLitExpr{.value = "true"}),
                              .rhs = Allocate(BoolLitExpr{.value = "false"}),
                          }),
                      }),
                  },
          },
  };

  EXPECT_THAT(Generate(func), ElementsAre(
                                  PushStack{
                                      .size = 0,
                                  },
                                  SetReg32{
                                      .src_val = "1",
                                      .dst_reg = 1,
                                  },
                                  SetReg32{
                                      .src_val = "0",
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
                                  PopStack{
                                      .size = 0,
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
                                      .size = 0,
                                  },
                                  SetReg32{
                                      .src_val = "1",
                                      .dst_reg = 1,
                                  },
                                  CondJump{
                                      .cond_reg = 1,
                                      .then_label = 1,
                                      .else_label = 2,
                                  },
                                  Label{
                                      .id = 1,
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
                                  PopStack{
                                      .size = 0,
                                  },
                                  Return{},
                                  Label{
                                      .id = 2,
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
                                  PopStack{
                                      .size = 0,
                                  },
                                  Return{}));
}

TEST_F(GenerateAbstractMachineInstructionsTest, GtInts) {
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
                                      .size = 0,
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
                                  PopStack{
                                      .size = 0,
                                  },
                                  Return{}));
}

TEST_F(GenerateAbstractMachineInstructionsTest, LtInts) {
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
                                      .size = 0,
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
                                  PopStack{
                                      .size = 0,
                                  },
                                  Return{}));
}

TEST_F(GenerateAbstractMachineInstructionsTest, VarDecl) {
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
                                      .size = 4,
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
                                  PopStack{
                                      .size = 4,
                                  },
                                  Return{}));
}

}  // namespace
}  // namespace lucid
