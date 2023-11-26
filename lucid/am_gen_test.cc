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

class GenerateAbstractMachineFunctionTest : public testing::Test,
                                            public AstFixture {
 protected:
  std::vector<Instruction> Generate(
      FuncDefStmt& func,
      const std::unordered_map<std::string_view, FuncType>& func_types = {}) {
    InferExprTypes(arena_, func_types, func);
    auto graph = BuildControlFlowGraph(arena_, func);
    AbstractMachineState state;
    GenerateAbstractMachineFunction(arena_, graph, state);
    return state.func.instructions;
  }
};

TEST_F(GenerateAbstractMachineFunctionTest, ReturnInt32Lit) {
  auto func = FuncDefStmt{
      .name = "foo",
      .result_type = A(BasicType{.name = "Int32"}),
      .body = {{
          A(ReturnStmt{
              .value = A(IntLitExpr{.value = "21"}),
          }),
      }},
  };

  EXPECT_THAT(Generate(func), ElementsAre(PushStack{},
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
                                          PopStack{}, Return{}));
}

TEST_F(GenerateAbstractMachineFunctionTest, ReturnInt64Lit) {
  auto func = FuncDefStmt{
      .name = "foo",
      .result_type = A(BasicType{.name = "Int64"}),
      .body = {{
          A(ReturnStmt{
              .value = A(IntLitExpr{.value = "21"}),
          }),
      }},
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
                                          PopStack{}, Return{}));
}

TEST_F(GenerateAbstractMachineFunctionTest, FuncCallWithInt32Arg) {
  auto func = FuncDefStmt{
      .name = "foo",
      .result_type = A(BasicType{.name = "Int32"}),
      .body = {{
          A(ReturnStmt{
              .value = A(FuncCallExpr{
                  .func_name = "id",
                  .arguments =
                      {
                          A(IntLitExpr{.value = "21"}),
                      },
              }),
          }),
      }},
  };

  std::vector<FuncParam> id_func_params = {
      {.type = A(BasicType{.name = "Int32"})},
  };
  auto id_func_type = FuncType{
      .result_type = "Int32",
      .parameters = id_func_params,
  };

  EXPECT_THAT(Generate(func, {{"id", id_func_type}}),
              ElementsAre(PushStack{},
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
                              .offset = 1,
                              .dst_reg = 2,
                          },
                          PopStack{}, Return{}));
}

TEST_F(GenerateAbstractMachineFunctionTest, FuncCallWithInt64Arg) {
  auto func = FuncDefStmt{
      .name = "foo",
      .result_type = A(BasicType{.name = "Int64"}),
      .body = {{
          A(ReturnStmt{
              .value = A(FuncCallExpr{
                  .func_name = "id",
                  .arguments =
                      {
                          A(IntLitExpr{.value = "21"}),
                      },
              }),
          }),
      }},
  };

  std::vector<FuncParam> id_func_params = {
      {.type = A(BasicType{.name = "Int64"})},
  };
  auto id_func_type = FuncType{
      .result_type = "Int64",
      .parameters = id_func_params,
  };

  EXPECT_THAT(Generate(func, {{"id", id_func_type}}),
              ElementsAre(PushStack{},
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
                          SetReg64{
                              .src_val = "21",
                              .dst_reg = 1,
                          },
                          MoveReg64{
                              .src_reg = 1,
                              .dst_reg = 1,
                          },
                          Jump{
                              .label = "id",
                          },
                          MoveReg64{
                              .src_reg = 0,
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
                              .offset = 0,
                              .dst_reg = 1,
                          },
                          LoadStack64{
                              .offset = 1,
                              .dst_reg = 2,
                          },
                          PopStack{}, Return{}));
}

TEST_F(GenerateAbstractMachineFunctionTest, AddInt32) {
  auto func = FuncDefStmt{
      .name = "foo",
      .result_type = A(BasicType{.name = "Int32"}),
      .body = {{
          A(ReturnStmt{
              .value = A(BinaryOpExpr{
                  .op = BinaryOp::Add,
                  .lhs = A(IntLitExpr{.value = "2"}),
                  .rhs = A(IntLitExpr{.value = "3"}),
              }),
          }),
      }},
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
                                          StoreStack64{
                                              .offset = 2,
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
                                              .offset = 1,
                                              .dst_reg = 2,
                                          },
                                          LoadStack64{
                                              .offset = 2,
                                              .dst_reg = 3,
                                          },
                                          PopStack{}, Return{}));
}

TEST_F(GenerateAbstractMachineFunctionTest, AddInt64) {
  auto func = FuncDefStmt{
      .name = "foo",
      .result_type = A(BasicType{.name = "Int64"}),
      .body = {{
          A(ReturnStmt{
              .value = A(BinaryOpExpr{
                  .op = BinaryOp::Add,
                  .lhs = A(IntLitExpr{.value = "2"}),
                  .rhs = A(IntLitExpr{.value = "3"}),
              }),
          }),
      }},
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
                                          StoreStack64{
                                              .offset = 2,
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
                                              .offset = 1,
                                              .dst_reg = 2,
                                          },
                                          LoadStack64{
                                              .offset = 2,
                                              .dst_reg = 3,
                                          },
                                          PopStack{}, Return{}));
}

TEST_F(GenerateAbstractMachineFunctionTest, SubtractInt32) {
  auto func = FuncDefStmt{
      .name = "foo",
      .result_type = A(BasicType{.name = "Int32"}),
      .body = {{
          A(ReturnStmt{
              .value = A(BinaryOpExpr{
                  .op = BinaryOp::Sub,
                  .lhs = A(IntLitExpr{.value = "7"}),
                  .rhs = A(IntLitExpr{.value = "5"}),
              }),
          }),
      }},
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
                                          StoreStack64{
                                              .offset = 2,
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
                                              .offset = 1,
                                              .dst_reg = 2,
                                          },
                                          LoadStack64{
                                              .offset = 2,
                                              .dst_reg = 3,
                                          },
                                          PopStack{}, Return{}));
}

TEST_F(GenerateAbstractMachineFunctionTest, SubtractInt64) {
  auto func = FuncDefStmt{
      .name = "foo",
      .result_type = A(BasicType{.name = "Int64"}),
      .body = {{
          A(ReturnStmt{
              .value = A(BinaryOpExpr{
                  .op = BinaryOp::Sub,
                  .lhs = A(IntLitExpr{.value = "7"}),
                  .rhs = A(IntLitExpr{.value = "5"}),
              }),
          }),
      }},
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
                                          StoreStack64{
                                              .offset = 2,
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
                                              .offset = 1,
                                              .dst_reg = 2,
                                          },
                                          LoadStack64{
                                              .offset = 2,
                                              .dst_reg = 3,
                                          },
                                          PopStack{}, Return{}));
}

TEST_F(GenerateAbstractMachineFunctionTest, MultiplyInt32) {
  auto func = FuncDefStmt{
      .name = "foo",
      .result_type = A(BasicType{.name = "Int32"}),
      .body = {{
          A(ReturnStmt{
              .value = A(BinaryOpExpr{
                  .op = BinaryOp::Mul,
                  .lhs = A(IntLitExpr{.value = "2"}),
                  .rhs = A(IntLitExpr{.value = "3"}),
              }),
          }),
      }},
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
                                          StoreStack64{
                                              .offset = 2,
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
                                              .offset = 1,
                                              .dst_reg = 2,
                                          },
                                          LoadStack64{
                                              .offset = 2,
                                              .dst_reg = 3,
                                          },
                                          PopStack{}, Return{}));
}

TEST_F(GenerateAbstractMachineFunctionTest, MultiplyInt64) {
  auto func = FuncDefStmt{
      .name = "foo",
      .result_type = A(BasicType{.name = "Int64"}),
      .body = {{
          A(ReturnStmt{
              .value = A(BinaryOpExpr{
                  .op = BinaryOp::Mul,
                  .lhs = A(IntLitExpr{.value = "2"}),
                  .rhs = A(IntLitExpr{.value = "3"}),
              }),
          }),
      }},
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
                                          StoreStack64{
                                              .offset = 2,
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
                                              .offset = 1,
                                              .dst_reg = 2,
                                          },
                                          LoadStack64{
                                              .offset = 2,
                                              .dst_reg = 3,
                                          },
                                          PopStack{}, Return{}));
}

TEST_F(GenerateAbstractMachineFunctionTest, DivideInt32) {
  auto func = FuncDefStmt{
      .name = "foo",
      .result_type = A(BasicType{.name = "Int32"}),
      .body = {{
          A(ReturnStmt{
              .value = A(BinaryOpExpr{
                  .op = BinaryOp::Div,
                  .lhs = A(IntLitExpr{.value = "8"}),
                  .rhs = A(IntLitExpr{.value = "2"}),
              }),
          }),
      }},
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
                                          StoreStack64{
                                              .offset = 2,
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
                                              .offset = 1,
                                              .dst_reg = 2,
                                          },
                                          LoadStack64{
                                              .offset = 2,
                                              .dst_reg = 3,
                                          },
                                          PopStack{}, Return{}));
}

TEST_F(GenerateAbstractMachineFunctionTest, DivideInt64) {
  auto func = FuncDefStmt{
      .name = "foo",
      .result_type = A(BasicType{.name = "Int64"}),
      .body = {{
          A(ReturnStmt{
              .value = A(BinaryOpExpr{
                  .op = BinaryOp::Div,
                  .lhs = A(IntLitExpr{.value = "8"}),
                  .rhs = A(IntLitExpr{.value = "2"}),
              }),
          }),
      }},
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
                                          StoreStack64{
                                              .offset = 2,
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
                                              .offset = 1,
                                              .dst_reg = 2,
                                          },
                                          LoadStack64{
                                              .offset = 2,
                                              .dst_reg = 3,
                                          },
                                          PopStack{}, Return{}));
}

TEST_F(GenerateAbstractMachineFunctionTest, IfStmt) {
  auto func = FuncDefStmt{
      .name = "foo",
      .result_type = A(BasicType{.name = "Int32"}),
      .body = {{
          A(IfStmt{
              .cond = A(BoolLitExpr{.value = "true"}),
              .then_body = {{
                  A(ReturnStmt{
                      .value = A(BinaryOpExpr{
                          .op = BinaryOp::Add,
                          .lhs = A(IntLitExpr{.value = "2"}),
                          .rhs = A(IntLitExpr{.value = "3"}),
                      }),
                  }),
              }},
          }),
          A(ReturnStmt{
              .value = A(BinaryOpExpr{
                  .op = BinaryOp::Mul,
                  .lhs = A(IntLitExpr{.value = "4"}),
                  .rhs = A(IntLitExpr{.value = "5"}),
              }),
          }),
      }},
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
                                          StoreStack64{
                                              .offset = 2,
                                              .src_reg = 3,
                                          },
                                          StoreStack64{
                                              .offset = 3,
                                              .src_reg = 4,
                                          },
                                          StoreStack64{
                                              .offset = 4,
                                              .src_reg = 5,
                                          },
                                          StoreStack64{
                                              .offset = 5,
                                              .src_reg = 6,
                                          },
                                          StoreStack64{
                                              .offset = 6,
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
                                              .then_label = 3,
                                              .else_label = 2,
                                          },
                                          Label{
                                              .id = 1,
                                          },
                                          LoadStack64{
                                              .offset = 0,
                                              .dst_reg = 1,
                                          },
                                          LoadStack64{
                                              .offset = 1,
                                              .dst_reg = 2,
                                          },
                                          LoadStack64{
                                              .offset = 2,
                                              .dst_reg = 3,
                                          },
                                          LoadStack64{
                                              .offset = 3,
                                              .dst_reg = 4,
                                          },
                                          LoadStack64{
                                              .offset = 4,
                                              .dst_reg = 5,
                                          },
                                          LoadStack64{
                                              .offset = 5,
                                              .dst_reg = 6,
                                          },
                                          LoadStack64{
                                              .offset = 6,
                                              .dst_reg = 7,
                                          },
                                          PopStack{}, Return{},
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
                                          }));
}

TEST_F(GenerateAbstractMachineFunctionTest, IfElseStmt) {
  auto func = FuncDefStmt{
      .name = "foo",
      .result_type = A(BasicType{.name = "Int32"}),
      .body = {{
          A(IfStmt{
              .cond = A(BoolLitExpr{.value = "true"}),
              .then_body = {{
                  A(ReturnStmt{
                      .value = A(BinaryOpExpr{
                          .op = BinaryOp::Add,
                          .lhs = A(IntLitExpr{.value = "2"}),
                          .rhs = A(IntLitExpr{.value = "3"}),
                      }),
                  }),
              }},
              .else_body = {{
                  A(ReturnStmt{
                      .value = A(BinaryOpExpr{
                          .op = BinaryOp::Mul,
                          .lhs = A(IntLitExpr{.value = "4"}),
                          .rhs = A(IntLitExpr{.value = "5"}),
                      }),
                  }),
              }},
          }),
      }},
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
                                          StoreStack64{
                                              .offset = 2,
                                              .src_reg = 3,
                                          },
                                          StoreStack64{
                                              .offset = 3,
                                              .src_reg = 4,
                                          },
                                          StoreStack64{
                                              .offset = 4,
                                              .src_reg = 5,
                                          },
                                          StoreStack64{
                                              .offset = 5,
                                              .src_reg = 6,
                                          },
                                          StoreStack64{
                                              .offset = 6,
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
                                              .then_label = 3,
                                              .else_label = 4,
                                          },
                                          Label{
                                              .id = 1,
                                          },
                                          LoadStack64{
                                              .offset = 0,
                                              .dst_reg = 1,
                                          },
                                          LoadStack64{
                                              .offset = 1,
                                              .dst_reg = 2,
                                          },
                                          LoadStack64{
                                              .offset = 2,
                                              .dst_reg = 3,
                                          },
                                          LoadStack64{
                                              .offset = 3,
                                              .dst_reg = 4,
                                          },
                                          LoadStack64{
                                              .offset = 4,
                                              .dst_reg = 5,
                                          },
                                          LoadStack64{
                                              .offset = 5,
                                              .dst_reg = 6,
                                          },
                                          LoadStack64{
                                              .offset = 6,
                                              .dst_reg = 7,
                                          },
                                          PopStack{}, Return{},
                                          Label{
                                              .id = 2,
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
                                          }));
}

TEST_F(GenerateAbstractMachineFunctionTest, GtInt) {
  auto func = FuncDefStmt{
      .name = "foo",
      .result_type = A(BasicType{.name = "Bool"}),
      .body = {{
          A(ReturnStmt{
              .value = A(BinaryOpExpr{
                  .op = BinaryOp::Gt,
                  .lhs = A(IntLitExpr{.value = "3"}),
                  .rhs = A(IntLitExpr{.value = "2"}),
              }),
          }),
      }},
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
                                          StoreStack64{
                                              .offset = 2,
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
                                              .offset = 1,
                                              .dst_reg = 2,
                                          },
                                          LoadStack64{
                                              .offset = 2,
                                              .dst_reg = 3,
                                          },
                                          PopStack{}, Return{}));
}

TEST_F(GenerateAbstractMachineFunctionTest, GtInt32) {
  auto func = FuncDefStmt{
      .name = "foo",
      .result_type = A(BasicType{.name = "Bool"}),
      .parameters =
          {
              {
                  .name = "x",
                  .type = A(BasicType{.name = "Int32"}),
              },
              {
                  .name = "y",
                  .type = A(BasicType{.name = "Int32"}),
              },
          },
      .body = {{
          A(ReturnStmt{
              .value = A(BinaryOpExpr{
                  .op = BinaryOp::Gt,
                  .lhs = A(IdentExpr{.name = "x"}),
                  .rhs = A(IdentExpr{.name = "y"}),
              }),
          }),
      }},
  };

  EXPECT_THAT(Generate(func), ElementsAre(PushStack{},
                                          StoreStack64{
                                              .offset = 2,
                                              .src_reg = 1,
                                          },
                                          StoreStack64{
                                              .offset = 3,
                                              .src_reg = 2,
                                          },
                                          StoreStack64{
                                              .offset = 4,
                                              .src_reg = 3,
                                          },
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
                                          LoadStack64{
                                              .offset = 2,
                                              .dst_reg = 1,
                                          },
                                          LoadStack64{
                                              .offset = 3,
                                              .dst_reg = 2,
                                          },
                                          LoadStack64{
                                              .offset = 4,
                                              .dst_reg = 3,
                                          },
                                          PopStack{}, Return{}));
}

TEST_F(GenerateAbstractMachineFunctionTest, GtInt64) {
  auto func = FuncDefStmt{
      .name = "foo",
      .result_type = A(BasicType{.name = "Bool"}),
      .parameters =
          {
              {
                  .name = "x",
                  .type = A(BasicType{.name = "Int64"}),
              },
              {
                  .name = "y",
                  .type = A(BasicType{.name = "Int64"}),
              },
          },
      .body = {{
          A(ReturnStmt{
              .value = A(BinaryOpExpr{
                  .op = BinaryOp::Gt,
                  .lhs = A(IdentExpr{.name = "x"}),
                  .rhs = A(IdentExpr{.name = "y"}),
              }),
          }),
      }},
  };

  EXPECT_THAT(Generate(func), ElementsAre(PushStack{},
                                          StoreStack64{
                                              .offset = 2,
                                              .src_reg = 1,
                                          },
                                          StoreStack64{
                                              .offset = 3,
                                              .src_reg = 2,
                                          },
                                          StoreStack64{
                                              .offset = 4,
                                              .src_reg = 3,
                                          },
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
                                          LoadStack64{
                                              .offset = 2,
                                              .dst_reg = 1,
                                          },
                                          LoadStack64{
                                              .offset = 3,
                                              .dst_reg = 2,
                                          },
                                          LoadStack64{
                                              .offset = 4,
                                              .dst_reg = 3,
                                          },
                                          PopStack{}, Return{}));
}

TEST_F(GenerateAbstractMachineFunctionTest, LtInt) {
  auto func = FuncDefStmt{
      .name = "foo",
      .result_type = A(BasicType{.name = "Bool"}),
      .body = {{
          A(ReturnStmt{
              .value = A(BinaryOpExpr{
                  .op = BinaryOp::Lt,
                  .lhs = A(IntLitExpr{.value = "3"}),
                  .rhs = A(IntLitExpr{.value = "2"}),
              }),
          }),
      }},
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
                                          StoreStack64{
                                              .offset = 2,
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
                                              .offset = 1,
                                              .dst_reg = 2,
                                          },
                                          LoadStack64{
                                              .offset = 2,
                                              .dst_reg = 3,
                                          },
                                          PopStack{}, Return{}));
}

TEST_F(GenerateAbstractMachineFunctionTest, LtInt32) {
  auto func = FuncDefStmt{
      .name = "foo",
      .result_type = A(BasicType{.name = "Bool"}),
      .parameters =
          {
              {
                  .name = "x",
                  .type = A(BasicType{.name = "Int32"}),
              },
              {
                  .name = "y",
                  .type = A(BasicType{.name = "Int32"}),
              },
          },
      .body = {{
          A(ReturnStmt{
              .value = A(BinaryOpExpr{
                  .op = BinaryOp::Lt,
                  .lhs = A(IdentExpr{.name = "x"}),
                  .rhs = A(IdentExpr{.name = "y"}),
              }),
          }),
      }},
  };

  EXPECT_THAT(Generate(func), ElementsAre(PushStack{},
                                          StoreStack64{
                                              .offset = 2,
                                              .src_reg = 1,
                                          },
                                          StoreStack64{
                                              .offset = 3,
                                              .src_reg = 2,
                                          },
                                          StoreStack64{
                                              .offset = 4,
                                              .src_reg = 3,
                                          },
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
                                          LoadStack64{
                                              .offset = 2,
                                              .dst_reg = 1,
                                          },
                                          LoadStack64{
                                              .offset = 3,
                                              .dst_reg = 2,
                                          },
                                          LoadStack64{
                                              .offset = 4,
                                              .dst_reg = 3,
                                          },
                                          PopStack{}, Return{}));
}

TEST_F(GenerateAbstractMachineFunctionTest, LtInt64) {
  auto func = FuncDefStmt{
      .name = "foo",
      .result_type = A(BasicType{.name = "Bool"}),
      .parameters =
          {
              {
                  .name = "x",
                  .type = A(BasicType{.name = "Int64"}),
              },
              {
                  .name = "y",
                  .type = A(BasicType{.name = "Int64"}),
              },
          },
      .body = {{
          A(ReturnStmt{
              .value = A(BinaryOpExpr{
                  .op = BinaryOp::Lt,
                  .lhs = A(IdentExpr{.name = "x"}),
                  .rhs = A(IdentExpr{.name = "y"}),
              }),
          }),
      }},
  };

  EXPECT_THAT(Generate(func), ElementsAre(PushStack{},
                                          StoreStack64{
                                              .offset = 2,
                                              .src_reg = 1,
                                          },
                                          StoreStack64{
                                              .offset = 3,
                                              .src_reg = 2,
                                          },
                                          StoreStack64{
                                              .offset = 4,
                                              .src_reg = 3,
                                          },
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
                                          LoadStack64{
                                              .offset = 2,
                                              .dst_reg = 1,
                                          },
                                          LoadStack64{
                                              .offset = 3,
                                              .dst_reg = 2,
                                          },
                                          LoadStack64{
                                              .offset = 4,
                                              .dst_reg = 3,
                                          },
                                          PopStack{}, Return{}));
}

TEST_F(GenerateAbstractMachineFunctionTest, EqInt32) {
  auto func = FuncDefStmt{
      .name = "foo",
      .result_type = A(BasicType{.name = "Bool"}),
      .parameters =
          {
              {
                  .name = "x",
                  .type = A(BasicType{.name = "Int32"}),
              },
              {
                  .name = "y",
                  .type = A(BasicType{.name = "Int32"}),
              },
          },
      .body = {{
          A(ReturnStmt{
              .value = A(BinaryOpExpr{
                  .op = BinaryOp::Eq,
                  .lhs = A(IdentExpr{.name = "x"}),
                  .rhs = A(IdentExpr{.name = "y"}),
              }),
          }),
      }},
  };

  EXPECT_THAT(Generate(func), ElementsAre(PushStack{},
                                          StoreStack64{
                                              .offset = 2,
                                              .src_reg = 1,
                                          },
                                          StoreStack64{
                                              .offset = 3,
                                              .src_reg = 2,
                                          },
                                          StoreStack64{
                                              .offset = 4,
                                              .src_reg = 3,
                                          },
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
                                          LoadStack64{
                                              .offset = 2,
                                              .dst_reg = 1,
                                          },
                                          LoadStack64{
                                              .offset = 3,
                                              .dst_reg = 2,
                                          },
                                          LoadStack64{
                                              .offset = 4,
                                              .dst_reg = 3,
                                          },
                                          PopStack{}, Return{}));
}

TEST_F(GenerateAbstractMachineFunctionTest, EqInt64) {
  auto func = FuncDefStmt{
      .name = "foo",
      .result_type = A(BasicType{.name = "Bool"}),
      .parameters =
          {
              {
                  .name = "x",
                  .type = A(BasicType{.name = "Int64"}),
              },
              {
                  .name = "y",
                  .type = A(BasicType{.name = "Int64"}),
              },
          },
      .body = {{
          A(ReturnStmt{
              .value = A(BinaryOpExpr{
                  .op = BinaryOp::Eq,
                  .lhs = A(IdentExpr{.name = "x"}),
                  .rhs = A(IdentExpr{.name = "y"}),
              }),
          }),
      }},
  };

  EXPECT_THAT(Generate(func), ElementsAre(PushStack{},
                                          StoreStack64{
                                              .offset = 2,
                                              .src_reg = 1,
                                          },
                                          StoreStack64{
                                              .offset = 3,
                                              .src_reg = 2,
                                          },
                                          StoreStack64{
                                              .offset = 4,
                                              .src_reg = 3,
                                          },
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
                                          LoadStack64{
                                              .offset = 2,
                                              .dst_reg = 1,
                                          },
                                          LoadStack64{
                                              .offset = 3,
                                              .dst_reg = 2,
                                          },
                                          LoadStack64{
                                              .offset = 4,
                                              .dst_reg = 3,
                                          },
                                          PopStack{}, Return{}));
}

TEST_F(GenerateAbstractMachineFunctionTest, NotEqInt32) {
  auto func = FuncDefStmt{
      .name = "foo",
      .result_type = A(BasicType{.name = "Bool"}),
      .parameters =
          {
              {
                  .name = "x",
                  .type = A(BasicType{.name = "Int32"}),
              },
              {
                  .name = "y",
                  .type = A(BasicType{.name = "Int32"}),
              },
          },
      .body = {{
          A(ReturnStmt{
              .value = A(BinaryOpExpr{
                  .op = BinaryOp::NotEq,
                  .lhs = A(IdentExpr{.name = "x"}),
                  .rhs = A(IdentExpr{.name = "y"}),
              }),
          }),
      }},
  };

  EXPECT_THAT(Generate(func), ElementsAre(PushStack{},
                                          StoreStack64{
                                              .offset = 2,
                                              .src_reg = 1,
                                          },
                                          StoreStack64{
                                              .offset = 3,
                                              .src_reg = 2,
                                          },
                                          StoreStack64{
                                              .offset = 4,
                                              .src_reg = 3,
                                          },
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
                                          LoadStack64{
                                              .offset = 2,
                                              .dst_reg = 1,
                                          },
                                          LoadStack64{
                                              .offset = 3,
                                              .dst_reg = 2,
                                          },
                                          LoadStack64{
                                              .offset = 4,
                                              .dst_reg = 3,
                                          },
                                          PopStack{}, Return{}));
}

TEST_F(GenerateAbstractMachineFunctionTest, NotEqInt64) {
  auto func = FuncDefStmt{
      .name = "foo",
      .result_type = A(BasicType{.name = "Bool"}),
      .parameters =
          {
              {
                  .name = "x",
                  .type = A(BasicType{.name = "Int64"}),
              },
              {
                  .name = "y",
                  .type = A(BasicType{.name = "Int64"}),
              },
          },
      .body = {{
          A(ReturnStmt{
              .value = A(BinaryOpExpr{
                  .op = BinaryOp::NotEq,
                  .lhs = A(IdentExpr{.name = "x"}),
                  .rhs = A(IdentExpr{.name = "y"}),
              }),
          }),
      }},
  };

  EXPECT_THAT(Generate(func), ElementsAre(PushStack{},
                                          StoreStack64{
                                              .offset = 2,
                                              .src_reg = 1,
                                          },
                                          StoreStack64{
                                              .offset = 3,
                                              .src_reg = 2,
                                          },
                                          StoreStack64{
                                              .offset = 4,
                                              .src_reg = 3,
                                          },
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
                                          LoadStack64{
                                              .offset = 2,
                                              .dst_reg = 1,
                                          },
                                          LoadStack64{
                                              .offset = 3,
                                              .dst_reg = 2,
                                          },
                                          LoadStack64{
                                              .offset = 4,
                                              .dst_reg = 3,
                                          },
                                          PopStack{}, Return{}));
}

TEST_F(GenerateAbstractMachineFunctionTest, VarDeclInt32) {
  auto func = FuncDefStmt{
      .name = "foo",
      .result_type = A(BasicType{.name = "Int32"}),
      .body = {{
          A(VarDeclStmt{
              .name = "x",
              .type = A(BasicType{.name = "Int32"}),
              .init = A(IntLitExpr{.value = "2"}),
          }),
          A(ReturnStmt{
              .value = A(IdentExpr{
                  .name = "x",
              }),
          }),
      }},
  };

  EXPECT_THAT(Generate(func), ElementsAre(PushStack{},
                                          StoreStack64{
                                              .offset = 1,
                                              .src_reg = 1,
                                          },
                                          StoreStack64{
                                              .offset = 2,
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
                                              .offset = 1,
                                              .dst_reg = 1,
                                          },
                                          LoadStack64{
                                              .offset = 2,
                                              .dst_reg = 2,
                                          },
                                          PopStack{}, Return{}));
}

TEST_F(GenerateAbstractMachineFunctionTest, VarDeclInt64) {
  auto func = FuncDefStmt{
      .name = "foo",
      .result_type = A(BasicType{.name = "Int64"}),
      .body = {{
          A(VarDeclStmt{
              .name = "x",
              .type = A(BasicType{.name = "Int64"}),
              .init = A(IntLitExpr{.value = "2"}),
          }),
          A(ReturnStmt{
              .value = A(IdentExpr{
                  .name = "x",
              }),
          }),
      }},
  };

  EXPECT_THAT(Generate(func), ElementsAre(PushStack{},
                                          StoreStack64{
                                              .offset = 1,
                                              .src_reg = 1,
                                          },
                                          StoreStack64{
                                              .offset = 2,
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
                                              .offset = 1,
                                              .dst_reg = 1,
                                          },
                                          LoadStack64{
                                              .offset = 2,
                                              .dst_reg = 2,
                                          },
                                          PopStack{}, Return{}));
}

TEST_F(GenerateAbstractMachineFunctionTest, VarAssignInt32) {
  auto func = FuncDefStmt{
      .name = "foo",
      .result_type = A(BasicType{.name = "Void"}),
      .parameters =
          {
              {
                  .name = "x",
                  .type = A(BasicType{.name = "Int32"}),
              },
          },
      .body = {{
          A(VarAssignStmt{
              .name = "x",
              .expr = A(IntLitExpr{.value = "2"}),
          }),
      }},
  };

  EXPECT_THAT(Generate(func), ElementsAre(PushStack{},
                                          StoreStack64{
                                              .offset = 1,
                                              .src_reg = 1,
                                          },
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
                                          LoadStack64{
                                              .offset = 1,
                                              .dst_reg = 1,
                                          },
                                          PopStack{}, Return{}));
}

TEST_F(GenerateAbstractMachineFunctionTest, VarAssignInt64) {
  auto func = FuncDefStmt{
      .name = "foo",
      .result_type = A(BasicType{.name = "Void"}),
      .parameters =
          {
              {
                  .name = "x",
                  .type = A(BasicType{.name = "Int64"}),
              },
          },
      .body = {{
          A(VarAssignStmt{
              .name = "x",
              .expr = A(IntLitExpr{.value = "2"}),
          }),
      }},
  };

  EXPECT_THAT(Generate(func), ElementsAre(PushStack{},
                                          StoreStack64{
                                              .offset = 1,
                                              .src_reg = 1,
                                          },
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
                                          LoadStack64{
                                              .offset = 1,
                                              .dst_reg = 1,
                                          },
                                          PopStack{}, Return{}));
}

}  // namespace
}  // namespace lucid
