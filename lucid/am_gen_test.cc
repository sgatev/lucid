#include "lucid/am_gen.h"

#include <vector>

#include "gmock/gmock.h"
#include "gtest/gtest.h"
#include "lucid/am.h"
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
      FuncDefStmt& func, const std::vector<FuncDefStmt>& func_defs = {}) {
    InferExprTypes(ctx_, func_defs, func);
    auto graph = BuildControlFlowGraph(ctx_, func);
    AbstractMachineState state = {
        .func =
            {
                .name = func.name,
            },
    };
    GenerateAbstractMachineFunction(ctx_, graph, state);
    return state.func.instructions;
  }
};

TEST_F(GenerateAbstractMachineFunctionTest, ReturnInt32Lit) {
  auto func = FuncDefStmt{
      .name = I("foo"),
      .result_type = T(BasicType{.name = "Int32"}),
      .stmts = StmtListOf({
          S(ReturnStmt{
              .value = E(IntLitExpr{.value = "21"}),
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
      .result_type = T(BasicType{.name = "Int64"}),
      .stmts = StmtListOf({
          S(ReturnStmt{
              .value = E(IntLitExpr{.value = "21"}),
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
      .result_type = T(BasicType{.name = "Int32"}),
      .params = ParamListOf({
          P(FuncParam{
              .name = I("x"),
              .type_constraint = T(BasicType{.name = "Int32"}),
          }),
      }),
  };

  auto func = FuncDefStmt{
      .name = I("foo"),
      .result_type = T(BasicType{.name = "Int32"}),
      .stmts = StmtListOf({
          S(ReturnStmt{
              .value = E(FuncCallExpr{
                  .func_name = I("id"),
                  .args = ExprListOf({
                      E(IntLitExpr{.value = "21"}),
                  }),
              }),
          }),
      }),
  };

  EXPECT_THAT(Generate(func, {id_func}), ElementsAre(PushStack{},
                                                     StoreStack64{
                                                         .offset = 11,
                                                         .src_reg = 12,
                                                     },
                                                     StoreStack64{
                                                         .offset = 10,
                                                         .src_reg = 11,
                                                     },
                                                     StoreStack64{
                                                         .offset = 9,
                                                         .src_reg = 10,
                                                     },
                                                     StoreStack64{
                                                         .offset = 8,
                                                         .src_reg = 9,
                                                     },
                                                     StoreStack64{
                                                         .offset = 7,
                                                         .src_reg = 8,
                                                     },
                                                     StoreStack64{
                                                         .offset = 6,
                                                         .src_reg = 7,
                                                     },
                                                     StoreStack64{
                                                         .offset = 5,
                                                         .src_reg = 6,
                                                     },
                                                     StoreStack64{
                                                         .offset = 4,
                                                         .src_reg = 5,
                                                     },
                                                     StoreStack64{
                                                         .offset = 3,
                                                         .src_reg = 4,
                                                     },
                                                     StoreStack64{
                                                         .offset = 2,
                                                         .src_reg = 3,
                                                     },
                                                     StoreStack64{
                                                         .offset = 1,
                                                         .src_reg = 2,
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
                                                         .offset = 11,
                                                         .dst_reg = 12,
                                                     },
                                                     LoadStack64{
                                                         .offset = 10,
                                                         .dst_reg = 11,
                                                     },
                                                     LoadStack64{
                                                         .offset = 9,
                                                         .dst_reg = 10,
                                                     },
                                                     LoadStack64{
                                                         .offset = 8,
                                                         .dst_reg = 9,
                                                     },
                                                     LoadStack64{
                                                         .offset = 7,
                                                         .dst_reg = 8,
                                                     },
                                                     LoadStack64{
                                                         .offset = 6,
                                                         .dst_reg = 7,
                                                     },
                                                     LoadStack64{
                                                         .offset = 5,
                                                         .dst_reg = 6,
                                                     },
                                                     LoadStack64{
                                                         .offset = 4,
                                                         .dst_reg = 5,
                                                     },
                                                     LoadStack64{
                                                         .offset = 3,
                                                         .dst_reg = 4,
                                                     },
                                                     LoadStack64{
                                                         .offset = 2,
                                                         .dst_reg = 3,
                                                     },
                                                     LoadStack64{
                                                         .offset = 1,
                                                         .dst_reg = 2,
                                                     },
                                                     LoadStack64{
                                                         .offset = 0,
                                                         .dst_reg = 1,
                                                     },
                                                     PopStack{}, Return{}));
}

TEST_F(GenerateAbstractMachineFunctionTest, FuncCallWithInt64Arg) {
  auto id_func = FuncDefStmt{
      .name = I("id"),
      .result_type = T(BasicType{.name = "Int64"}),
      .params = ParamListOf({
          P(FuncParam{
              .name = I("x"),
              .type_constraint = T(BasicType{.name = "Int64"}),
          }),
      }),
  };

  auto func = FuncDefStmt{
      .name = I("foo"),
      .result_type = T(BasicType{.name = "Int64"}),
      .stmts = StmtListOf({
          S(ReturnStmt{
              .value = E(FuncCallExpr{
                  .func_name = I("id"),
                  .args = ExprListOf({
                      E(IntLitExpr{.value = "21"}),
                  }),
              }),
          }),
      }),
  };

  EXPECT_THAT(Generate(func, {id_func}), ElementsAre(PushStack{},
                                                     StoreStack64{
                                                         .offset = 11,
                                                         .src_reg = 12,
                                                     },
                                                     StoreStack64{
                                                         .offset = 10,
                                                         .src_reg = 11,
                                                     },
                                                     StoreStack64{
                                                         .offset = 9,
                                                         .src_reg = 10,
                                                     },
                                                     StoreStack64{
                                                         .offset = 8,
                                                         .src_reg = 9,
                                                     },
                                                     StoreStack64{
                                                         .offset = 7,
                                                         .src_reg = 8,
                                                     },
                                                     StoreStack64{
                                                         .offset = 6,
                                                         .src_reg = 7,
                                                     },
                                                     StoreStack64{
                                                         .offset = 5,
                                                         .src_reg = 6,
                                                     },
                                                     StoreStack64{
                                                         .offset = 4,
                                                         .src_reg = 5,
                                                     },
                                                     StoreStack64{
                                                         .offset = 3,
                                                         .src_reg = 4,
                                                     },
                                                     StoreStack64{
                                                         .offset = 2,
                                                         .src_reg = 3,
                                                     },
                                                     StoreStack64{
                                                         .offset = 1,
                                                         .src_reg = 2,
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
                                                         .offset = 11,
                                                         .dst_reg = 12,
                                                     },
                                                     LoadStack64{
                                                         .offset = 10,
                                                         .dst_reg = 11,
                                                     },
                                                     LoadStack64{
                                                         .offset = 9,
                                                         .dst_reg = 10,
                                                     },
                                                     LoadStack64{
                                                         .offset = 8,
                                                         .dst_reg = 9,
                                                     },
                                                     LoadStack64{
                                                         .offset = 7,
                                                         .dst_reg = 8,
                                                     },
                                                     LoadStack64{
                                                         .offset = 6,
                                                         .dst_reg = 7,
                                                     },
                                                     LoadStack64{
                                                         .offset = 5,
                                                         .dst_reg = 6,
                                                     },
                                                     LoadStack64{
                                                         .offset = 4,
                                                         .dst_reg = 5,
                                                     },
                                                     LoadStack64{
                                                         .offset = 3,
                                                         .dst_reg = 4,
                                                     },
                                                     LoadStack64{
                                                         .offset = 2,
                                                         .dst_reg = 3,
                                                     },
                                                     LoadStack64{
                                                         .offset = 1,
                                                         .dst_reg = 2,
                                                     },
                                                     LoadStack64{
                                                         .offset = 0,
                                                         .dst_reg = 1,
                                                     },
                                                     PopStack{}, Return{}));
}

TEST_F(GenerateAbstractMachineFunctionTest, AddInt32) {
  auto func = FuncDefStmt{
      .name = I("foo"),
      .result_type = T(BasicType{.name = "Int32"}),
      .stmts = StmtListOf({
          S(ReturnStmt{
              .value = E(BinaryOpExpr{
                  .op = BinaryOp::Add,
                  .lhs = E(IntLitExpr{.value = "2"}),
                  .rhs = E(IntLitExpr{.value = "3"}),
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
      .result_type = T(BasicType{.name = "Int64"}),
      .stmts = StmtListOf({
          S(ReturnStmt{
              .value = E(BinaryOpExpr{
                  .op = BinaryOp::Add,
                  .lhs = E(IntLitExpr{.value = "2"}),
                  .rhs = E(IntLitExpr{.value = "3"}),
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
      .result_type = T(BasicType{.name = "Int32"}),
      .stmts = StmtListOf({
          S(ReturnStmt{
              .value = E(BinaryOpExpr{
                  .op = BinaryOp::Sub,
                  .lhs = E(IntLitExpr{.value = "7"}),
                  .rhs = E(IntLitExpr{.value = "5"}),
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
      .result_type = T(BasicType{.name = "Int64"}),
      .stmts = StmtListOf({
          S(ReturnStmt{
              .value = E(BinaryOpExpr{
                  .op = BinaryOp::Sub,
                  .lhs = E(IntLitExpr{.value = "7"}),
                  .rhs = E(IntLitExpr{.value = "5"}),
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
      .result_type = T(BasicType{.name = "Int32"}),
      .stmts = StmtListOf({
          S(ReturnStmt{
              .value = E(BinaryOpExpr{
                  .op = BinaryOp::Mul,
                  .lhs = E(IntLitExpr{.value = "2"}),
                  .rhs = E(IntLitExpr{.value = "3"}),
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
      .result_type = T(BasicType{.name = "Int64"}),
      .stmts = StmtListOf({
          S(ReturnStmt{
              .value = E(BinaryOpExpr{
                  .op = BinaryOp::Mul,
                  .lhs = E(IntLitExpr{.value = "2"}),
                  .rhs = E(IntLitExpr{.value = "3"}),
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
      .result_type = T(BasicType{.name = "Int32"}),
      .stmts = StmtListOf({
          S(ReturnStmt{
              .value = E(BinaryOpExpr{
                  .op = BinaryOp::Div,
                  .lhs = E(IntLitExpr{.value = "8"}),
                  .rhs = E(IntLitExpr{.value = "2"}),
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
      .result_type = T(BasicType{.name = "Int64"}),
      .stmts = StmtListOf({
          S(ReturnStmt{
              .value = E(BinaryOpExpr{
                  .op = BinaryOp::Div,
                  .lhs = E(IntLitExpr{.value = "8"}),
                  .rhs = E(IntLitExpr{.value = "2"}),
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
      .result_type = T(BasicType{.name = "Int32"}),
      .stmts = StmtListOf({
          S(ReturnStmt{
              .value = E(BinaryOpExpr{
                  .op = BinaryOp::Mod,
                  .lhs = E(IntLitExpr{.value = "8"}),
                  .rhs = E(IntLitExpr{.value = "2"}),
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
      .result_type = T(BasicType{.name = "Int64"}),
      .stmts = StmtListOf({
          S(ReturnStmt{
              .value = E(BinaryOpExpr{
                  .op = BinaryOp::Mod,
                  .lhs = E(IntLitExpr{.value = "8"}),
                  .rhs = E(IntLitExpr{.value = "2"}),
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
      .result_type = T(BasicType{.name = "Int32"}),
      .stmts = StmtListOf({
          S(IfStmt{
              .cond = E(BoolLitExpr{.value = "true"}),
              .then_stmts = StmtListOf({
                  S(ReturnStmt{
                      .value = E(BinaryOpExpr{
                          .op = BinaryOp::Add,
                          .lhs = E(IntLitExpr{.value = "2"}),
                          .rhs = E(IntLitExpr{.value = "3"}),
                      }),
                  }),
              }),
          }),
          S(ReturnStmt{
              .value = E(BinaryOpExpr{
                  .op = BinaryOp::Mul,
                  .lhs = E(IntLitExpr{.value = "4"}),
                  .rhs = E(IntLitExpr{.value = "5"}),
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
                                              .id = 1,
                                          },
                                          PopStack{}, Return{},
                                          Label{
                                              .id = 2,
                                          },
                                          SetReg32{
                                              .src_val = "4",
                                              .dst_reg = 1,
                                          },
                                          SetReg32{
                                              .src_val = "5",
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
                                              .id = 3,
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
                                          }));
}

TEST_F(GenerateAbstractMachineFunctionTest, IfElseStmt) {
  auto func = FuncDefStmt{
      .name = I("foo"),
      .result_type = T(BasicType{.name = "Int32"}),
      .stmts = StmtListOf({
          S(IfStmt{
              .cond = E(BoolLitExpr{.value = "true"}),
              .then_stmts = StmtListOf({
                  S(ReturnStmt{
                      .value = E(BinaryOpExpr{
                          .op = BinaryOp::Add,
                          .lhs = E(IntLitExpr{.value = "2"}),
                          .rhs = E(IntLitExpr{.value = "3"}),
                      }),
                  }),
              }),
              .else_stmts = StmtListOf({
                  S(ReturnStmt{
                      .value = E(BinaryOpExpr{
                          .op = BinaryOp::Mul,
                          .lhs = E(IntLitExpr{.value = "4"}),
                          .rhs = E(IntLitExpr{.value = "5"}),
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
                                              .id = 1,
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
                                              .id = 4,
                                          },
                                          SetReg32{
                                              .src_val = "4",
                                              .dst_reg = 1,
                                          },
                                          SetReg32{
                                              .src_val = "5",
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
                                          }));
}

TEST_F(GenerateAbstractMachineFunctionTest, GtInt) {
  auto func = FuncDefStmt{
      .name = I("foo"),
      .result_type = T(BasicType{.name = "Bool"}),
      .stmts = StmtListOf({
          S(ReturnStmt{
              .value = E(BinaryOpExpr{
                  .op = BinaryOp::Gt,
                  .lhs = E(IntLitExpr{.value = "3"}),
                  .rhs = E(IntLitExpr{.value = "2"}),
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
              .type_constraint = T(BasicType{.name = "Int32"}),
          }),
          P(FuncParam{
              .name = I("y"),
              .type_constraint = T(BasicType{.name = "Int32"}),
          }),
      }),
      .result_type = T(BasicType{.name = "Bool"}),
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
              .type_constraint = T(BasicType{.name = "Int64"}),
          }),
          P(FuncParam{
              .name = I("y"),
              .type_constraint = T(BasicType{.name = "Int64"}),
          }),
      }),
      .result_type = T(BasicType{.name = "Bool"}),
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
      .result_type = T(BasicType{.name = "Bool"}),
      .stmts = StmtListOf({
          S(ReturnStmt{
              .value = E(BinaryOpExpr{
                  .op = BinaryOp::Lt,
                  .lhs = E(IntLitExpr{.value = "3"}),
                  .rhs = E(IntLitExpr{.value = "2"}),
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
      .result_type = T(BasicType{.name = "Bool"}),
      .params = ParamListOf({
          P(FuncParam{
              .name = I("x"),
              .type_constraint = T(BasicType{.name = "Int32"}),
          }),
          P(FuncParam{
              .name = I("y"),
              .type_constraint = T(BasicType{.name = "Int32"}),
          }),
      }),
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
      .result_type = T(BasicType{.name = "Bool"}),
      .params = ParamListOf({
          P(FuncParam{
              .name = I("x"),
              .type_constraint = T(BasicType{.name = "Int64"}),
          }),
          P(FuncParam{
              .name = I("y"),
              .type_constraint = T(BasicType{.name = "Int64"}),
          }),
      }),
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
      .result_type = T(BasicType{.name = "Bool"}),
      .params = ParamListOf({
          P(FuncParam{
              .name = I("x"),
              .type_constraint = T(BasicType{.name = "Int32"}),
          }),
          P(FuncParam{
              .name = I("y"),
              .type_constraint = T(BasicType{.name = "Int32"}),
          }),
      }),
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
      .result_type = T(BasicType{.name = "Bool"}),
      .params = ParamListOf({
          P(FuncParam{
              .name = I("x"),
              .type_constraint = T(BasicType{.name = "Int64"}),
          }),
          P(FuncParam{
              .name = I("y"),
              .type_constraint = T(BasicType{.name = "Int64"}),
          }),
      }),
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
      .result_type = T(BasicType{.name = "Bool"}),
      .params = ParamListOf({
          P(FuncParam{
              .name = I("x"),
              .type_constraint = T(BasicType{.name = "Int32"}),
          }),
          P(FuncParam{
              .name = I("y"),
              .type_constraint = T(BasicType{.name = "Int32"}),
          }),
      }),
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
              .type_constraint = T(BasicType{.name = "Int64"}),
              .name = I("x"),
          }),
          P(FuncParam{
              .type_constraint = T(BasicType{.name = "Int64"}),
              .name = I("y"),
          }),
      }),
      .result_type = T(BasicType{.name = "Bool"}),
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
      .result_type = T(BasicType{.name = "Int32"}),
      .stmts = StmtListOf({
          S(VarDeclStmt{
              .type_constraint = T(BasicType{.name = "Int32"}),
              .name = I("x"),
              .init = E(IntLitExpr{.value = "2"}),
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
      .result_type = T(BasicType{.name = "Int64"}),
      .stmts = StmtListOf({
          S(VarDeclStmt{
              .type_constraint = T(BasicType{.name = "Int64"}),
              .name = I("x"),
              .init = E(IntLitExpr{.value = "2"}),
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
      .result_type = T(BasicType{.name = "Int32"}),
      .stmts = StmtListOf({
          S(VarDeclStmt{
              .type_constraint = T(ArrayType{
                  .element_type_constraint = T(BasicType{.name = "Int32"}),
                  .size = IntLitExpr{.value = "10"},
              }),
              .name = I("x"),
          }),
          S(ReturnStmt{
              .value = E(IntLitExpr{
                  .value = "0",
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
      .result_type = T(BasicType{.name = "Int64"}),
      .stmts = StmtListOf({
          S(VarDeclStmt{
              .type_constraint = T(ArrayType{
                  .element_type_constraint = T(BasicType{.name = "Int64"}),
                  .size = IntLitExpr{.value = "10"},
              }),
              .name = I("x"),
          }),
          S(ReturnStmt{
              .value = E(IntLitExpr{
                  .value = "0",
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
              .type_constraint = T(BasicType{.name = "Int32"}),
              .name = I("x"),
          }),
      }),
      .result_type = T(BasicType{.name = "Void"}),
      .stmts = StmtListOf({
          S(VarAssignStmt{
              .name = I("x"),
              .expr = E(IntLitExpr{.value = "2"}),
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
              .type_constraint = T(BasicType{.name = "Int64"}),
              .name = I("x"),
          }),
      }),
      .result_type = T(BasicType{.name = "Void"}),
      .stmts = StmtListOf({
          S(VarAssignStmt{
              .name = I("x"),
              .expr = E(IntLitExpr{.value = "2"}),
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
      .result_type = T(BasicType{.name = "Int32"}),
      .stmts = StmtListOf({
          S(LoopStmt{
              .stmts = StmtListOf({
                  S(ReturnStmt{
                      .value = E(IntLitExpr{.value = "1"}),
                  }),
              }),
          }),
          S(ReturnStmt{
              .value = E(IntLitExpr{.value = "2"}),
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
                                          },
                                          Label{
                                              .id = 3,
                                          },
                                          SetReg32{
                                              .src_val = "1",
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
      .result_type = T(BasicType{.name = "Int32"}),
      .stmts = StmtListOf({
          S(VarDeclStmt{
              .type_constraint = T(BasicType{.name = "Int32"}),
              .name = I("n"),
              .init = E(IntLitExpr{
                  .value = "0",
              }),
          }),
          S(LoopStmt{
              .stmts = StmtListOf({
                  S(IfStmt{
                      .cond = E(BinaryOpExpr{
                          .op = BinaryOp::Gt,
                          .lhs = E(IdentExpr{.name = I("n")}),
                          .rhs = E(IntLitExpr{.value = "3"}),
                      }),
                      .then_stmts = StmtListOf({S(BreakStmt{})}),
                  }),
                  S(VarAssignStmt{
                      .name = I("n"),
                      .expr = E(BinaryOpExpr{
                          .op = BinaryOp::Add,
                          .lhs = E(IdentExpr{.name = I("n")}),
                          .rhs = E(IntLitExpr{.value = "1"}),
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
                                              .id = 1,
                                          },
                                          PopStack{}, Return{},
                                          Label{
                                              .id = 2,
                                          },
                                          LoadStack32{
                                              .offset = 0,
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
                                              .id = 3,
                                          },
                                          LoadStack32{
                                              .offset = 0,
                                              .dst_reg = 1,
                                          },
                                          SetReg32{
                                              .src_val = "3",
                                              .dst_reg = 2,
                                          },
                                          GtReg32{
                                              .res_reg = 3,
                                              .lhs_reg = 1,
                                              .rhs_reg = 2,
                                          },
                                          CondJump{
                                              .cond_reg = 3,
                                              .then_label = 5,
                                              .else_label = 4,
                                          },
                                          Label{
                                              .id = 4,
                                          },
                                          LoadStack32{
                                              .offset = 0,
                                              .dst_reg = 1,
                                          },
                                          SetReg32{
                                              .src_val = "1",
                                              .dst_reg = 2,
                                          },
                                          AddReg32{
                                              .res_reg = 3,
                                              .lhs_reg = 1,
                                              .rhs_reg = 2,
                                          },
                                          StoreStack32{
                                              .offset = 0,
                                              .src_reg = 3,
                                          },
                                          UncondJump{
                                              .label = 3,
                                          },
                                          Label{
                                              .id = 5,
                                          },
                                          UncondJump{
                                              .label = 2,
                                          }));
}

}  // namespace
}  // namespace lucid
