#include "lucid/am/translator.h"

#include <algorithm>
#include <optional>
#include <utility>
#include <variant>
#include <vector>

#include "lucid/am/cfg.h"
#include "lucid/am/instructions.h"
#include "lucid/am/state.h"
#include "lucid/core/container/graph/order.h"
#include "lucid/core/testing/testing.h"
#include "lucid/syntax/ast.h"
#include "lucid/syntax/ast_fixture.h"
#include "lucid/syntax/cfg.h"
#include "lucid/syntax/type.h"

namespace lucid {
namespace {

class GenerateAbstractMachineFunctionTest : public Test, public AstFixture {
 protected:
  AbstractMachineControlFlowGraph GenerateGraph(FuncDefStmt& func) {
    EXPECT_TRUE(InferExprTypes(syn_ctx_, func).has_value());
    auto graph = BuildControlFlowGraph(syn_ctx_, func);
    AbstractMachineState state;
    auto am_cfg_or_error =
        GenerateAbstractMachineFunction(/*am_cfgs=*/{}, syn_ctx_, graph, state);
    EXPECT_TRUE(am_cfg_or_error.has_value());
    return std::move(am_cfg_or_error).value();
  }

  std::vector<Instruction> Generate(FuncDefStmt& func) {
    AbstractMachineControlFlowGraph am_cfg = GenerateGraph(func);

    std::vector<AbstractMachineControlFlowGraph::BlockRef> block_refs =
        Vertices(am_cfg);
    std::sort(block_refs.begin(), block_refs.end(),
              CompareReversePostOrder(am_cfg));

    std::vector<Instruction> instructions;
    for (const auto& ref : block_refs) {
      const auto& block = am_cfg.GetBlock(ref);
      instructions.insert(instructions.end(), block.instructions.begin(),
                          block.instructions.end());
    }
    return instructions;
  }
};

TEST(GenerateAbstractMachineFunctionTest, ReturnInt32Lit) {
  auto func = FuncDefStmt{
      .name = I("foo"),
      .result_type = T("Int32"),
      .stmts = StmtListOf({
          S(ReturnStmt{
              .value = E(IntLitExpr{.value = 21}),
          }),
      }),
  };

  EXPECT_THAT(Generate(func), ElementsEqual(
                                  SetReg{
                                      .src_val = 21,
                                      .dst_reg = Reg(2, RegSize::RegSize32),
                                  },
                                  Return{
                                      .res_reg = Reg(1, RegSize::RegSize32),
                                  }));
}

TEST(GenerateAbstractMachineFunctionTest, ReturnInt64Lit) {
  auto func = FuncDefStmt{
      .name = I("foo"),
      .result_type = T("Int64"),
      .stmts = StmtListOf({
          S(ReturnStmt{
              .value = E(IntLitExpr{.value = 21}),
          }),
      }),
  };

  EXPECT_THAT(Generate(func), ElementsEqual(
                                  SetReg{
                                      .src_val = 21,
                                      .dst_reg = Reg(2, RegSize::RegSize64),
                                  },
                                  Return{
                                      .res_reg = Reg(1, RegSize::RegSize64),
                                  }));
}

TEST(GenerateAbstractMachineFunctionTest, FuncCallWithInt32Arg) {
  auto id_func = FuncDefStmt{
      .name = I("id"),
      .params = ParamListOf({
          P(FuncParam{
              .name = I("x"),
              .type_constraint = T("Int32"),
          }),
      }),
      .result_type = T("Int32"),
  };
  syn_ctx_.AddFuncDef(id_func);

  auto func = FuncDefStmt{
      .name = I("foo"),
      .result_type = T("Int32"),
      .stmts = StmtListOf({
          S(ReturnStmt{
              .value = E(FuncCallExpr{
                  .func_name = I("id"),
                  .args = ExprListOf({
                      E(IntLitExpr{.value = 21}),
                  }),
              }),
          }),
      }),
  };

  EXPECT_THAT(Generate(func), ElementsEqual(
                                  SetReg{
                                      .src_val = 21,
                                      .dst_reg = Reg(2, RegSize32),
                                  },
                                  FuncCall{
                                      .label = "id",
                                      .args =
                                          {
                                              {
                                                  .reg = Reg(2, RegSize32),
                                              },
                                          },
                                      .res = std::optional<FuncCall::Slot>({
                                          .reg = Reg(3, RegSize32),
                                      }),
                                  },
                                  Return{
                                      .res_reg = Reg(1, RegSize32),
                                  }));
}

TEST(GenerateAbstractMachineFunctionTest, FuncCallWithInt64Arg) {
  auto id_func = FuncDefStmt{
      .name = I("id"),
      .params = ParamListOf({
          P(FuncParam{
              .name = I("x"),
              .type_constraint = T("Int64"),
          }),
      }),
      .result_type = T("Int64"),
  };
  syn_ctx_.AddFuncDef(id_func);

  auto func = FuncDefStmt{
      .name = I("foo"),
      .result_type = T("Int64"),
      .stmts = StmtListOf({
          S(ReturnStmt{
              .value = E(FuncCallExpr{
                  .func_name = I("id"),
                  .args = ExprListOf({
                      E(IntLitExpr{.value = 21}),
                  }),
              }),
          }),
      }),
  };

  EXPECT_THAT(Generate(func), ElementsEqual(
                                  SetReg{
                                      .src_val = 21,
                                      .dst_reg = Reg(2, RegSize64),
                                  },
                                  FuncCall{
                                      .label = "id",
                                      .args =
                                          {
                                              {
                                                  .reg = Reg(2, RegSize64),
                                              },
                                          },
                                      .res = std::optional<FuncCall::Slot>({
                                          .reg = Reg(3, RegSize64),
                                      }),
                                  },
                                  Return{
                                      .res_reg = Reg(1, RegSize64),
                                  }));
}

TEST(GenerateAbstractMachineFunctionTest, AddInt32) {
  auto func = FuncDefStmt{
      .name = I("foo"),
      .result_type = T("Int32"),
      .stmts = StmtListOf({
          S(ReturnStmt{
              .value = E(BinaryOpExpr{
                  .op = BinaryOp::Add,
                  .lhs = E(IntLitExpr{.value = 2}),
                  .rhs = E(IntLitExpr{.value = 3}),
              }),
          }),
      }),
  };

  EXPECT_THAT(Generate(func), ElementsEqual(
                                  SetReg{
                                      .src_val = 2,
                                      .dst_reg = Reg(2, RegSize32),
                                  },
                                  SetReg{
                                      .src_val = 3,
                                      .dst_reg = Reg(3, RegSize32),
                                  },
                                  AddReg{
                                      .res_reg = Reg(4, RegSize32),
                                      .lhs_reg = Reg(2, RegSize32),
                                      .rhs_reg = Reg(3, RegSize32),
                                  },
                                  Return{
                                      .res_reg = Reg(1, RegSize32),
                                  }));
}

TEST(GenerateAbstractMachineFunctionTest, AddInt64) {
  auto func = FuncDefStmt{
      .name = I("foo"),
      .result_type = T("Int64"),
      .stmts = StmtListOf({
          S(ReturnStmt{
              .value = E(BinaryOpExpr{
                  .op = BinaryOp::Add,
                  .lhs = E(IntLitExpr{.value = 2}),
                  .rhs = E(IntLitExpr{.value = 3}),
              }),
          }),
      }),
  };

  EXPECT_THAT(Generate(func), ElementsEqual(
                                  SetReg{
                                      .src_val = 2,
                                      .dst_reg = Reg(2, RegSize64),
                                  },
                                  SetReg{
                                      .src_val = 3,
                                      .dst_reg = Reg(3, RegSize64),
                                  },
                                  AddReg{
                                      .res_reg = Reg(4, RegSize64),
                                      .lhs_reg = Reg(2, RegSize64),
                                      .rhs_reg = Reg(3, RegSize64),
                                  },
                                  Return{
                                      .res_reg = Reg(1, RegSize64),
                                  }));
}

TEST(GenerateAbstractMachineFunctionTest, SubtractInt32) {
  auto func = FuncDefStmt{
      .name = I("foo"),
      .result_type = T("Int32"),
      .stmts = StmtListOf({
          S(ReturnStmt{
              .value = E(BinaryOpExpr{
                  .op = BinaryOp::Sub,
                  .lhs = E(IntLitExpr{.value = 7}),
                  .rhs = E(IntLitExpr{.value = 5}),
              }),
          }),
      }),
  };

  EXPECT_THAT(Generate(func), ElementsEqual(
                                  SetReg{
                                      .src_val = 7,
                                      .dst_reg = Reg(2, RegSize32),
                                  },
                                  SetReg{
                                      .src_val = 5,
                                      .dst_reg = Reg(3, RegSize32),
                                  },
                                  SubReg{
                                      .res_reg = Reg(4, RegSize32),
                                      .lhs_reg = Reg(2, RegSize32),
                                      .rhs_reg = Reg(3, RegSize32),
                                  },
                                  Return{
                                      .res_reg = Reg(1, RegSize32),
                                  }));
}

TEST(GenerateAbstractMachineFunctionTest, SubtractInt64) {
  auto func = FuncDefStmt{
      .name = I("foo"),
      .result_type = T("Int64"),
      .stmts = StmtListOf({
          S(ReturnStmt{
              .value = E(BinaryOpExpr{
                  .op = BinaryOp::Sub,
                  .lhs = E(IntLitExpr{.value = 7}),
                  .rhs = E(IntLitExpr{.value = 5}),
              }),
          }),
      }),
  };

  EXPECT_THAT(Generate(func), ElementsEqual(
                                  SetReg{
                                      .src_val = 7,
                                      .dst_reg = Reg(2, RegSize64),
                                  },
                                  SetReg{
                                      .src_val = 5,
                                      .dst_reg = Reg(3, RegSize64),
                                  },
                                  SubReg{
                                      .res_reg = Reg(4, RegSize64),
                                      .lhs_reg = Reg(2, RegSize64),
                                      .rhs_reg = Reg(3, RegSize64),
                                  },
                                  Return{
                                      .res_reg = Reg(1, RegSize64),
                                  }));
}

TEST(GenerateAbstractMachineFunctionTest, MultiplyInt32) {
  auto func = FuncDefStmt{
      .name = I("foo"),
      .result_type = T("Int32"),
      .stmts = StmtListOf({
          S(ReturnStmt{
              .value = E(BinaryOpExpr{
                  .op = BinaryOp::Mul,
                  .lhs = E(IntLitExpr{.value = 2}),
                  .rhs = E(IntLitExpr{.value = 3}),
              }),
          }),
      }),
  };

  EXPECT_THAT(Generate(func), ElementsEqual(
                                  SetReg{
                                      .src_val = 2,
                                      .dst_reg = Reg(2, RegSize32),
                                  },
                                  SetReg{
                                      .src_val = 3,
                                      .dst_reg = Reg(3, RegSize32),
                                  },
                                  MulReg{
                                      .res_reg = Reg(4, RegSize32),
                                      .lhs_reg = Reg(2, RegSize32),
                                      .rhs_reg = Reg(3, RegSize32),
                                  },
                                  Return{
                                      .res_reg = Reg(1, RegSize32),
                                  }));
}

TEST(GenerateAbstractMachineFunctionTest, MultiplyInt64) {
  auto func = FuncDefStmt{
      .name = I("foo"),
      .result_type = T("Int64"),
      .stmts = StmtListOf({
          S(ReturnStmt{
              .value = E(BinaryOpExpr{
                  .op = BinaryOp::Mul,
                  .lhs = E(IntLitExpr{.value = 2}),
                  .rhs = E(IntLitExpr{.value = 3}),
              }),
          }),
      }),
  };

  EXPECT_THAT(Generate(func), ElementsEqual(
                                  SetReg{
                                      .src_val = 2,
                                      .dst_reg = Reg(2, RegSize64),
                                  },
                                  SetReg{
                                      .src_val = 3,
                                      .dst_reg = Reg(3, RegSize64),
                                  },
                                  MulReg{
                                      .res_reg = Reg(4, RegSize64),
                                      .lhs_reg = Reg(2, RegSize64),
                                      .rhs_reg = Reg(3, RegSize64),
                                  },
                                  Return{
                                      .res_reg = Reg(1, RegSize64),
                                  }));
}

TEST(GenerateAbstractMachineFunctionTest, DivideInt32) {
  auto func = FuncDefStmt{
      .name = I("foo"),
      .result_type = T("Int32"),
      .stmts = StmtListOf({
          S(ReturnStmt{
              .value = E(BinaryOpExpr{
                  .op = BinaryOp::Div,
                  .lhs = E(IntLitExpr{.value = 8}),
                  .rhs = E(IntLitExpr{.value = 2}),
              }),
          }),
      }),
  };

  EXPECT_THAT(Generate(func), ElementsEqual(
                                  SetReg{
                                      .src_val = 8,
                                      .dst_reg = Reg(2, RegSize32),
                                  },
                                  SetReg{
                                      .src_val = 2,
                                      .dst_reg = Reg(3, RegSize32),
                                  },
                                  DivReg{
                                      .res_reg = Reg(4, RegSize32),
                                      .lhs_reg = Reg(2, RegSize32),
                                      .rhs_reg = Reg(3, RegSize32),
                                  },
                                  Return{
                                      .res_reg = Reg(1, RegSize32),
                                  }));
}

TEST(GenerateAbstractMachineFunctionTest, DivideInt64) {
  auto func = FuncDefStmt{
      .name = I("foo"),
      .result_type = T("Int64"),
      .stmts = StmtListOf({
          S(ReturnStmt{
              .value = E(BinaryOpExpr{
                  .op = BinaryOp::Div,
                  .lhs = E(IntLitExpr{.value = 8}),
                  .rhs = E(IntLitExpr{.value = 2}),
              }),
          }),
      }),
  };

  EXPECT_THAT(Generate(func), ElementsEqual(
                                  SetReg{
                                      .src_val = 8,
                                      .dst_reg = Reg(2, RegSize64),
                                  },
                                  SetReg{
                                      .src_val = 2,
                                      .dst_reg = Reg(3, RegSize64),
                                  },
                                  DivReg{
                                      .res_reg = Reg(4, RegSize64),
                                      .lhs_reg = Reg(2, RegSize64),
                                      .rhs_reg = Reg(3, RegSize64),
                                  },
                                  Return{
                                      .res_reg = Reg(1, RegSize64),
                                  }));
}

TEST(GenerateAbstractMachineFunctionTest, ModuloInt32) {
  auto func = FuncDefStmt{
      .name = I("foo"),
      .result_type = T("Int32"),
      .stmts = StmtListOf({
          S(ReturnStmt{
              .value = E(BinaryOpExpr{
                  .op = BinaryOp::Mod,
                  .lhs = E(IntLitExpr{.value = 8}),
                  .rhs = E(IntLitExpr{.value = 2}),
              }),
          }),
      }),
  };

  EXPECT_THAT(Generate(func), ElementsEqual(
                                  SetReg{
                                      .src_val = 8,
                                      .dst_reg = Reg(2, RegSize32),
                                  },
                                  SetReg{
                                      .src_val = 2,
                                      .dst_reg = Reg(3, RegSize32),
                                  },
                                  ModReg{
                                      .res_reg = Reg(4, RegSize32),
                                      .lhs_reg = Reg(2, RegSize32),
                                      .rhs_reg = Reg(3, RegSize32),
                                  },
                                  Return{
                                      .res_reg = Reg(1, RegSize32),
                                  }));
}

TEST(GenerateAbstractMachineFunctionTest, ModuloInt64) {
  auto func = FuncDefStmt{
      .name = I("foo"),
      .result_type = T("Int64"),
      .stmts = StmtListOf({
          S(ReturnStmt{
              .value = E(BinaryOpExpr{
                  .op = BinaryOp::Mod,
                  .lhs = E(IntLitExpr{.value = 8}),
                  .rhs = E(IntLitExpr{.value = 2}),
              }),
          }),
      }),
  };

  EXPECT_THAT(Generate(func), ElementsEqual(
                                  SetReg{
                                      .src_val = 8,
                                      .dst_reg = Reg(2, RegSize64),
                                  },
                                  SetReg{
                                      .src_val = 2,
                                      .dst_reg = Reg(3, RegSize64),
                                  },
                                  ModReg{
                                      .res_reg = Reg(4, RegSize64),
                                      .lhs_reg = Reg(2, RegSize64),
                                      .rhs_reg = Reg(3, RegSize64),
                                  },
                                  Return{
                                      .res_reg = Reg(1, RegSize64),
                                  }));
}

TEST(GenerateAbstractMachineFunctionTest, IfStmt) {
  auto func = FuncDefStmt{
      .name = I("foo"),
      .result_type = T("Int32"),
      .stmts = StmtListOf({
          S(IfStmt{
              .cond = E(BoolLitExpr{.value = true}),
              .then_stmts = StmtListOf({
                  S(ReturnStmt{
                      .value = E(BinaryOpExpr{
                          .op = BinaryOp::Add,
                          .lhs = E(IntLitExpr{.value = 2}),
                          .rhs = E(IntLitExpr{.value = 3}),
                      }),
                  }),
              }),
          }),
          S(ReturnStmt{
              .value = E(BinaryOpExpr{
                  .op = BinaryOp::Mul,
                  .lhs = E(IntLitExpr{.value = 4}),
                  .rhs = E(IntLitExpr{.value = 5}),
              }),
          }),
      }),
  };

  EXPECT_THAT(Generate(func), ElementsEqual(
                                  SetReg{
                                      .src_val = 1,
                                      .dst_reg = Reg(2, RegSize32),
                                  },
                                  SetReg{
                                      .src_val = 4,
                                      .dst_reg = Reg(3, RegSize32),
                                  },
                                  SetReg{
                                      .src_val = 5,
                                      .dst_reg = Reg(4, RegSize32),
                                  },
                                  MulReg{
                                      .res_reg = Reg(5, RegSize32),
                                      .lhs_reg = Reg(3, RegSize32),
                                      .rhs_reg = Reg(4, RegSize32),
                                  },
                                  SetReg{
                                      .src_val = 2,
                                      .dst_reg = Reg(6, RegSize32),
                                  },
                                  SetReg{
                                      .src_val = 3,
                                      .dst_reg = Reg(7, RegSize32),
                                  },
                                  AddReg{
                                      .res_reg = Reg(8, RegSize32),
                                      .lhs_reg = Reg(6, RegSize32),
                                      .rhs_reg = Reg(7, RegSize32),
                                  },
                                  Return{
                                      .res_reg = Reg(1, RegSize32),
                                  }));
}

TEST(GenerateAbstractMachineFunctionTest, IfElseStmt) {
  auto func = FuncDefStmt{
      .name = I("foo"),
      .result_type = T("Int32"),
      .stmts = StmtListOf({
          S(IfStmt{
              .cond = E(BoolLitExpr{.value = true}),
              .then_stmts = StmtListOf({
                  S(ReturnStmt{
                      .value = E(BinaryOpExpr{
                          .op = BinaryOp::Add,
                          .lhs = E(IntLitExpr{.value = 2}),
                          .rhs = E(IntLitExpr{.value = 3}),
                      }),
                  }),
              }),
              .else_stmts = StmtListOf({
                  S(ReturnStmt{
                      .value = E(BinaryOpExpr{
                          .op = BinaryOp::Mul,
                          .lhs = E(IntLitExpr{.value = 4}),
                          .rhs = E(IntLitExpr{.value = 5}),
                      }),
                  }),
              }),
          }),
      }),
  };

  EXPECT_THAT(Generate(func), ElementsEqual(
                                  SetReg{
                                      .src_val = 1,
                                      .dst_reg = Reg(2, RegSize32),
                                  },
                                  SetReg{
                                      .src_val = 4,
                                      .dst_reg = Reg(6, RegSize32),
                                  },
                                  SetReg{
                                      .src_val = 5,
                                      .dst_reg = Reg(7, RegSize32),
                                  },
                                  MulReg{
                                      .res_reg = Reg(8, RegSize32),
                                      .lhs_reg = Reg(6, RegSize32),
                                      .rhs_reg = Reg(7, RegSize32),
                                  },
                                  SetReg{
                                      .src_val = 2,
                                      .dst_reg = Reg(3, RegSize32),
                                  },
                                  SetReg{
                                      .src_val = 3,
                                      .dst_reg = Reg(4, RegSize32),
                                  },
                                  AddReg{
                                      .res_reg = Reg(5, RegSize32),
                                      .lhs_reg = Reg(3, RegSize32),
                                      .rhs_reg = Reg(4, RegSize32),
                                  },
                                  Return{
                                      .res_reg = Reg(1, RegSize32),
                                  }));
}

TEST(GenerateAbstractMachineFunctionTest, GtInt) {
  auto func = FuncDefStmt{
      .name = I("foo"),
      .result_type = T("Bool"),
      .stmts = StmtListOf({
          S(ReturnStmt{
              .value = E(BinaryOpExpr{
                  .op = BinaryOp::Gt,
                  .lhs = E(IntLitExpr{.value = 3}),
                  .rhs = E(IntLitExpr{.value = 2}),
              }),
          }),
      }),
  };

  EXPECT_THAT(Generate(func), ElementsEqual(
                                  SetReg{
                                      .src_val = 3,
                                      .dst_reg = Reg(2, RegSize32),
                                  },
                                  SetReg{
                                      .src_val = 2,
                                      .dst_reg = Reg(3, RegSize32),
                                  },
                                  GtReg{
                                      .res_reg = Reg(4, RegSize32),
                                      .lhs_reg = Reg(2, RegSize32),
                                      .rhs_reg = Reg(3, RegSize32),
                                  },
                                  Return{
                                      .res_reg = Reg(1, RegSize32),
                                  }));
}

TEST(GenerateAbstractMachineFunctionTest, GtInt32) {
  auto func = FuncDefStmt{
      .name = I("foo"),
      .params = ParamListOf({
          P(FuncParam{
              .name = I("x"),
              .type_constraint = T("Int32"),
          }),
          P(FuncParam{
              .name = I("y"),
              .type_constraint = T("Int32"),
          }),
      }),
      .result_type = T("Bool"),
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

  EXPECT_THAT(Generate(func), ElementsEqual(
                                  MoveReg{
                                      .src_reg = Reg(2, RegSize32),
                                      .dst_reg = Reg(4, RegSize32),
                                  },
                                  MoveReg{
                                      .src_reg = Reg(3, RegSize32),
                                      .dst_reg = Reg(5, RegSize32),
                                  },
                                  GtReg{
                                      .res_reg = Reg(6, RegSize32),
                                      .lhs_reg = Reg(4, RegSize32),
                                      .rhs_reg = Reg(5, RegSize32),
                                  },
                                  Return{
                                      .res_reg = Reg(1, RegSize32),
                                  }));
}

TEST(GenerateAbstractMachineFunctionTest, GtInt64) {
  auto func = FuncDefStmt{
      .name = I("foo"),
      .params = ParamListOf({
          P(FuncParam{
              .name = I("x"),
              .type_constraint = T("Int64"),
          }),
          P(FuncParam{
              .name = I("y"),
              .type_constraint = T("Int64"),
          }),
      }),
      .result_type = T("Bool"),
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

  EXPECT_THAT(Generate(func), ElementsEqual(
                                  MoveReg{
                                      .src_reg = Reg(2, RegSize64),
                                      .dst_reg = Reg(4, RegSize64),
                                  },
                                  MoveReg{
                                      .src_reg = Reg(3, RegSize64),
                                      .dst_reg = Reg(5, RegSize64),
                                  },
                                  GtReg{
                                      .res_reg = Reg(6, RegSize32),
                                      .lhs_reg = Reg(4, RegSize64),
                                      .rhs_reg = Reg(5, RegSize64),
                                  },
                                  Return{
                                      .res_reg = Reg(1, RegSize32),
                                  }));
}

TEST(GenerateAbstractMachineFunctionTest, LtInt) {
  auto func = FuncDefStmt{
      .name = I("foo"),
      .result_type = T("Bool"),
      .stmts = StmtListOf({
          S(ReturnStmt{
              .value = E(BinaryOpExpr{
                  .op = BinaryOp::Lt,
                  .lhs = E(IntLitExpr{.value = 3}),
                  .rhs = E(IntLitExpr{.value = 2}),
              }),
          }),
      }),
  };

  EXPECT_THAT(Generate(func), ElementsEqual(
                                  SetReg{
                                      .src_val = 3,
                                      .dst_reg = Reg(2, RegSize32),
                                  },
                                  SetReg{
                                      .src_val = 2,
                                      .dst_reg = Reg(3, RegSize32),
                                  },
                                  LtReg{
                                      .res_reg = Reg(4, RegSize32),
                                      .lhs_reg = Reg(2, RegSize32),
                                      .rhs_reg = Reg(3, RegSize32),
                                  },
                                  Return{
                                      .res_reg = Reg(1, RegSize32),
                                  }));
}

TEST(GenerateAbstractMachineFunctionTest, LtInt32) {
  auto func = FuncDefStmt{
      .name = I("foo"),
      .params = ParamListOf({
          P(FuncParam{
              .name = I("x"),
              .type_constraint = T("Int32"),
          }),
          P(FuncParam{
              .name = I("y"),
              .type_constraint = T("Int32"),
          }),
      }),
      .result_type = T("Bool"),
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

  EXPECT_THAT(Generate(func), ElementsEqual(
                                  MoveReg{
                                      .src_reg = Reg(2, RegSize32),
                                      .dst_reg = Reg(4, RegSize32),
                                  },
                                  MoveReg{
                                      .src_reg = Reg(3, RegSize32),
                                      .dst_reg = Reg(5, RegSize32),
                                  },
                                  LtReg{
                                      .res_reg = Reg(6, RegSize32),
                                      .lhs_reg = Reg(4, RegSize32),
                                      .rhs_reg = Reg(5, RegSize32),
                                  },
                                  Return{
                                      .res_reg = Reg(1, RegSize32),
                                  }));
}

TEST(GenerateAbstractMachineFunctionTest, LtInt64) {
  auto func = FuncDefStmt{
      .name = I("foo"),
      .params = ParamListOf({
          P(FuncParam{
              .name = I("x"),
              .type_constraint = T("Int64"),
          }),
          P(FuncParam{
              .name = I("y"),
              .type_constraint = T("Int64"),
          }),
      }),
      .result_type = T("Bool"),
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

  EXPECT_THAT(Generate(func), ElementsEqual(
                                  MoveReg{
                                      .src_reg = Reg(2, RegSize64),
                                      .dst_reg = Reg(4, RegSize64),
                                  },
                                  MoveReg{
                                      .src_reg = Reg(3, RegSize64),
                                      .dst_reg = Reg(5, RegSize64),
                                  },
                                  LtReg{
                                      .res_reg = Reg(6, RegSize32),
                                      .lhs_reg = Reg(4, RegSize64),
                                      .rhs_reg = Reg(5, RegSize64),
                                  },
                                  Return{
                                      .res_reg = Reg(1, RegSize32),
                                  }));
}

TEST(GenerateAbstractMachineFunctionTest, EqInt32) {
  auto func = FuncDefStmt{
      .name = I("foo"),
      .params = ParamListOf({
          P(FuncParam{
              .name = I("x"),
              .type_constraint = T("Int32"),
          }),
          P(FuncParam{
              .name = I("y"),
              .type_constraint = T("Int32"),
          }),
      }),
      .result_type = T("Bool"),
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

  EXPECT_THAT(Generate(func), ElementsEqual(
                                  MoveReg{
                                      .src_reg = Reg(2, RegSize32),
                                      .dst_reg = Reg(4, RegSize32),
                                  },
                                  MoveReg{
                                      .src_reg = Reg(3, RegSize32),
                                      .dst_reg = Reg(5, RegSize32),
                                  },
                                  EqReg{
                                      .res_reg = Reg(6, RegSize32),
                                      .lhs_reg = Reg(4, RegSize32),
                                      .rhs_reg = Reg(5, RegSize32),
                                  },
                                  Return{
                                      .res_reg = Reg(1, RegSize32),
                                  }));
}

TEST(GenerateAbstractMachineFunctionTest, EqInt64) {
  auto func = FuncDefStmt{
      .name = I("foo"),
      .params = ParamListOf({
          P(FuncParam{
              .name = I("x"),
              .type_constraint = T("Int64"),
          }),
          P(FuncParam{
              .name = I("y"),
              .type_constraint = T("Int64"),
          }),
      }),
      .result_type = T("Bool"),
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

  EXPECT_THAT(Generate(func), ElementsEqual(
                                  MoveReg{
                                      .src_reg = Reg(2, RegSize64),
                                      .dst_reg = Reg(4, RegSize64),
                                  },
                                  MoveReg{
                                      .src_reg = Reg(3, RegSize64),
                                      .dst_reg = Reg(5, RegSize64),
                                  },
                                  EqReg{
                                      .res_reg = Reg(6, RegSize32),
                                      .lhs_reg = Reg(4, RegSize64),
                                      .rhs_reg = Reg(5, RegSize64),
                                  },
                                  Return{
                                      .res_reg = Reg(1, RegSize32),
                                  }));
}

TEST(GenerateAbstractMachineFunctionTest, NotEqInt32) {
  auto func = FuncDefStmt{
      .name = I("foo"),
      .params = ParamListOf({
          P(FuncParam{
              .name = I("x"),
              .type_constraint = T("Int32"),
          }),
          P(FuncParam{
              .name = I("y"),
              .type_constraint = T("Int32"),
          }),
      }),
      .result_type = T("Bool"),
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

  EXPECT_THAT(Generate(func), ElementsEqual(
                                  MoveReg{
                                      .src_reg = Reg(2, RegSize32),
                                      .dst_reg = Reg(4, RegSize32),
                                  },
                                  MoveReg{
                                      .src_reg = Reg(3, RegSize32),
                                      .dst_reg = Reg(5, RegSize32),
                                  },
                                  NotEqReg{
                                      .res_reg = Reg(6, RegSize32),
                                      .lhs_reg = Reg(4, RegSize32),
                                      .rhs_reg = Reg(5, RegSize32),
                                  },
                                  Return{
                                      .res_reg = Reg(1, RegSize32),
                                  }));
}

TEST(GenerateAbstractMachineFunctionTest, NotEqInt64) {
  auto func = FuncDefStmt{
      .name = I("foo"),
      .params = ParamListOf({
          P(FuncParam{
              .name = I("x"),
              .type_constraint = T("Int64"),
          }),
          P(FuncParam{
              .name = I("y"),
              .type_constraint = T("Int64"),
          }),
      }),
      .result_type = T("Bool"),
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

  EXPECT_THAT(Generate(func), ElementsEqual(
                                  MoveReg{
                                      .src_reg = Reg(2, RegSize64),
                                      .dst_reg = Reg(4, RegSize64),
                                  },
                                  MoveReg{
                                      .src_reg = Reg(3, RegSize64),
                                      .dst_reg = Reg(5, RegSize64),
                                  },
                                  NotEqReg{
                                      .res_reg = Reg(6, RegSize32),
                                      .lhs_reg = Reg(4, RegSize64),
                                      .rhs_reg = Reg(5, RegSize64),
                                  },
                                  Return{
                                      .res_reg = Reg(1, RegSize32),
                                  }));
}

TEST(GenerateAbstractMachineFunctionTest, VarDeclInt32) {
  auto func = FuncDefStmt{
      .name = I("foo"),
      .result_type = T("Int32"),
      .stmts = StmtListOf({
          S(VarDeclStmt{
              .name = I("x"),
              .type_constraint = T("Int32"),
              .init = E(IntLitExpr{.value = 2}),
          }),
          S(ReturnStmt{
              .value = E(IdentExpr{
                  .name = I("x"),
              }),
          }),
      }),
  };

  EXPECT_THAT(Generate(func), ElementsEqual(
                                  SetReg{
                                      .src_val = 2,
                                      .dst_reg = Reg(2, RegSize32),
                                  },
                                  MoveReg{
                                      .src_reg = Reg(2, RegSize32),
                                      .dst_reg = Reg(3, RegSize32),
                                  },
                                  MoveReg{
                                      .src_reg = Reg(3, RegSize32),
                                      .dst_reg = Reg(4, RegSize32),
                                  },
                                  Return{
                                      .res_reg = Reg(1, RegSize32),
                                  }));
}

TEST(GenerateAbstractMachineFunctionTest, VarDeclInt64) {
  auto func = FuncDefStmt{
      .name = I("foo"),
      .result_type = T("Int64"),
      .stmts = StmtListOf({
          S(VarDeclStmt{
              .name = I("x"),
              .type_constraint = T("Int64"),
              .init = E(IntLitExpr{.value = 2}),
          }),
          S(ReturnStmt{
              .value = E(IdentExpr{
                  .name = I("x"),
              }),
          }),
      }),
  };

  EXPECT_THAT(Generate(func), ElementsEqual(
                                  SetReg{
                                      .src_val = 2,
                                      .dst_reg = Reg(2, RegSize64),
                                  },
                                  MoveReg{
                                      .src_reg = Reg(2, RegSize64),
                                      .dst_reg = Reg(3, RegSize64),
                                  },
                                  MoveReg{
                                      .src_reg = Reg(3, RegSize64),
                                      .dst_reg = Reg(4, RegSize64),
                                  },
                                  Return{
                                      .res_reg = Reg(1, RegSize64),
                                  }));
}

TEST(GenerateAbstractMachineFunctionTest, VarDeclInt32Array) {
  auto func = FuncDefStmt{
      .name = I("foo"),
      .result_type = T("Int32"),
      .stmts = StmtListOf({
          S(VarDeclStmt{
              .name = I("x"),
              .type_constraint = T(T("Int32"), 10),
          }),
          S(ReturnStmt{
              .value = E(IntLitExpr{
                  .value = 0,
              }),
          }),
      }),
  };

  EXPECT_THAT(Generate(func), ElementsEqual(
                                  SetReg{
                                      .src_val = 0,
                                      .dst_reg = Reg(2, RegSize32),
                                  },
                                  Return{
                                      .res_reg = Reg(1, RegSize32),
                                  }));
}

TEST(GenerateAbstractMachineFunctionTest, VarDeclInt64Array) {
  auto func = FuncDefStmt{
      .name = I("foo"),
      .result_type = T("Int64"),
      .stmts = StmtListOf({
          S(VarDeclStmt{
              .name = I("x"),
              .type_constraint = T(T("Int64"), 10),
          }),
          S(ReturnStmt{
              .value = E(IntLitExpr{
                  .value = 0,
              }),
          }),
      }),
  };

  EXPECT_THAT(Generate(func), ElementsEqual(
                                  SetReg{
                                      .src_val = 0,
                                      .dst_reg = Reg(2, RegSize64),
                                  },
                                  Return{
                                      .res_reg = Reg(1, RegSize64),
                                  }));
}

TEST(GenerateAbstractMachineFunctionTest, IndexGoesToTheLoadAndStoreAsItIs) {
  // `a[i]` is `i` elements into `a`, which a load or a store reaches by
  // scaling `i` itself, so nothing works out how many bytes that is first.
  auto func = FuncDefStmt{
      .name = I("foo"),
      .params = ParamListOf({
          P(FuncParam{
              .name = I("i"),
              .type_constraint = T("Int32"),
          }),
          P(FuncParam{
              .name = I("x"),
              .type_constraint = T("Int64"),
          }),
      }),
      .result_type = T("Int64"),
      .stmts = StmtListOf({
          S(VarDeclStmt{
              .name = I("a"),
              .type_constraint = T(T("Int64"), 10),
              .is_mutable = true,
          }),
          S(ArrayAssignStmt{
              .name = I("a"),
              .index = E(IdentExpr{.name = I("i")}),
              .expr = E(IdentExpr{.name = I("x")}),
          }),
          S(ReturnStmt{
              .value = E(IndexExpr{
                  .base = E(IdentExpr{.name = I("a")}),
                  .index = E(IdentExpr{.name = I("i")}),
              }),
          }),
      }),
  };

  const std::vector<Instruction> instructions = Generate(func);
  EXPECT_TRUE(std::ranges::none_of(instructions, [](const auto& inst) {
    return std::holds_alternative<MulReg>(inst);
  }));
  const auto store = std::ranges::find_if(instructions, [](const auto& inst) {
    return std::holds_alternative<StoreStackReg>(inst);
  });
  ASSERT_TRUE(store != instructions.end());
  EXPECT_EQ(std::get<StoreStackReg>(*store).offset_scale, 8u);
  const auto load = std::ranges::find_if(instructions, [](const auto& inst) {
    return std::holds_alternative<LoadStackReg>(inst);
  });
  ASSERT_TRUE(load != instructions.end());
  EXPECT_EQ(std::get<LoadStackReg>(*load).offset_scale, 8u);
}

TEST(GenerateAbstractMachineFunctionTest, VarAssignInt32) {
  auto func = FuncDefStmt{
      .name = I("foo"),
      .params = ParamListOf({
          P(FuncParam{
              .name = I("x"),
              .type_constraint = T("Int32"),
          }),
      }),
      .result_type = T("Int32"),
      .stmts = StmtListOf({
          S(VarAssignStmt{
              .name = I("x"),
              .expr = E(IntLitExpr{.value = 2}),
          }),
          S(ReturnStmt{
              .value = E(IdentExpr{.name = I("x")}),
          }),
      }),
  };

  EXPECT_THAT(Generate(func), ElementsEqual(
                                  SetReg{
                                      .src_val = 2,
                                      .dst_reg = Reg(3, RegSize32),
                                  },
                                  MoveReg{
                                      .src_reg = Reg(3, RegSize32),
                                      .dst_reg = Reg(2, RegSize32),
                                  },
                                  MoveReg{
                                      .src_reg = Reg(2, RegSize32),
                                      .dst_reg = Reg(4, RegSize32),
                                  },
                                  Return{
                                      .res_reg = Reg(1, RegSize32),
                                  }));
}

TEST(GenerateAbstractMachineFunctionTest, VarAssignInt64) {
  auto func = FuncDefStmt{
      .name = I("foo"),
      .params = ParamListOf({
          P(FuncParam{
              .name = I("x"),
              .type_constraint = T("Int64"),
          }),
      }),
      .result_type = T("Int64"),
      .stmts = StmtListOf({
          S(VarAssignStmt{
              .name = I("x"),
              .expr = E(IntLitExpr{.value = 2}),
          }),
          S(ReturnStmt{
              .value = E(IdentExpr{.name = I("x")}),
          }),
      }),
  };

  EXPECT_THAT(Generate(func), ElementsEqual(
                                  SetReg{
                                      .src_val = 2,
                                      .dst_reg = Reg(3, RegSize64),
                                  },
                                  MoveReg{
                                      .src_reg = Reg(3, RegSize64),
                                      .dst_reg = Reg(2, RegSize64),
                                  },
                                  MoveReg{
                                      .src_reg = Reg(2, RegSize64),
                                      .dst_reg = Reg(4, RegSize64),
                                  },
                                  Return{
                                      .res_reg = Reg(1, RegSize64),
                                  }));
}

// The `return` after a loop with no way out is never reached, so nothing of
// it is carried over.
TEST(GenerateAbstractMachineFunctionTest, Loop) {
  auto func = FuncDefStmt{
      .name = I("foo"),
      .result_type = T("Int32"),
      .stmts = StmtListOf({
          S(LoopStmt{
              .stmts = StmtListOf({
                  S(ReturnStmt{
                      .value = E(IntLitExpr{.value = 1}),
                  }),
              }),
          }),
          S(ReturnStmt{
              .value = E(IntLitExpr{.value = 2}),
          }),
      }),
  };

  EXPECT_THAT(Generate(func), ElementsEqual(
                                  SetReg{
                                      .src_val = 1,
                                      .dst_reg = Reg(2, RegSize32),
                                  },
                                  Return{
                                      .res_reg = Reg(1, RegSize32),
                                  }));
}

// What a function returns is settled where it ends, by one phi function
// taking each way in the value returned along it, so that the register
// holding the result is written once. Here the only way in returns from
// inside the loop: the `return` after it is never reached, and has no say.
TEST(GenerateAbstractMachineFunctionTest, ReturnsMeetWhereTheFunctionEnds) {
  auto func = FuncDefStmt{
      .name = I("foo"),
      .result_type = T("Int32"),
      .stmts = StmtListOf({
          S(LoopStmt{
              .stmts = StmtListOf({
                  S(ReturnStmt{
                      .value = E(IntLitExpr{.value = 1}),
                  }),
              }),
          }),
          S(ReturnStmt{
              .value = E(IntLitExpr{.value = 2}),
          }),
      }),
  };

  const AbstractMachineControlFlowGraph am_cfg = GenerateGraph(func);
  const auto& phis = am_cfg.GetBlock(am_cfg.last).phis;
  ASSERT_EQ(phis.size(), 1u);
  EXPECT_EQ(phis[0].dst, Reg(1, RegSize32));
  EXPECT_THAT(phis[0].srcs, ElementsEqual(Reg(2, RegSize32)));
}

// A function that never ends has no way to where it would, so it has no last
// block, and nothing after its loop is carried over.
TEST(GenerateAbstractMachineFunctionTest, FunctionThatNeverEndsHasNoLastBlock) {
  auto func = FuncDefStmt{
      .name = I("foo"),
      .result_type = T("Int32"),
      .stmts = StmtListOf({
          S(LoopStmt{
              .stmts = StmtListOf({
                  S(VarDeclStmt{
                      .name = I("n"),
                      .type_constraint = T("Int32"),
                      .init = E(IntLitExpr{.value = 1}),
                  }),
              }),
          }),
          S(ReturnStmt{
              .value = E(IntLitExpr{.value = 2}),
          }),
      }),
  };

  const AbstractMachineControlFlowGraph am_cfg = GenerateGraph(func);
  EXPECT_EQ(am_cfg.last, AbstractMachineControlFlowGraph::kNullBlockRef);
  for (const auto& block : am_cfg.Blocks()) {
    for (const auto& inst : block.instructions) {
      EXPECT_FALSE(std::holds_alternative<Return>(inst));
    }
  }
}

TEST(GenerateAbstractMachineFunctionTest, SingleLoopAndBreak) {
  auto func = FuncDefStmt{
      .name = I("foo"),
      .result_type = T("Int32"),
      .stmts = StmtListOf({
          S(VarDeclStmt{
              .name = I("n"),
              .type_constraint = T("Int32"),
              .init = E(IntLitExpr{
                  .value = 0,
              }),
          }),
          S(LoopStmt{
              .stmts = StmtListOf({
                  S(IfStmt{
                      .cond = E(BinaryOpExpr{
                          .op = BinaryOp::Gt,
                          .lhs = E(IdentExpr{.name = I("n")}),
                          .rhs = E(IntLitExpr{.value = 3}),
                      }),
                      .then_stmts = StmtListOf({S(BreakStmt{})}),
                  }),
                  S(VarAssignStmt{
                      .name = I("n"),
                      .expr = E(BinaryOpExpr{
                          .op = BinaryOp::Add,
                          .lhs = E(IdentExpr{.name = I("n")}),
                          .rhs = E(IntLitExpr{.value = 1}),
                      }),
                  }),
              }),
          }),
          S(ReturnStmt{
              .value = E(IdentExpr{.name = I("n")}),
          }),
      }),
  };

  EXPECT_THAT(Generate(func), ElementsEqual(
                                  SetReg{
                                      .src_val = 0,
                                      .dst_reg = Reg(2, RegSize32),
                                  },
                                  MoveReg{
                                      .src_reg = Reg(2, RegSize32),
                                      .dst_reg = Reg(3, RegSize32),
                                  },
                                  MoveReg{
                                      .src_reg = Reg(3, RegSize32),
                                      .dst_reg = Reg(5, RegSize32),
                                  },
                                  SetReg{
                                      .src_val = 3,
                                      .dst_reg = Reg(6, RegSize32),
                                  },
                                  GtReg{
                                      .res_reg = Reg(7, RegSize32),
                                      .lhs_reg = Reg(5, RegSize32),
                                      .rhs_reg = Reg(6, RegSize32),
                                  },
                                  MoveReg{
                                      .src_reg = Reg(3, RegSize32),
                                      .dst_reg = Reg(8, RegSize32),
                                  },
                                  SetReg{
                                      .src_val = 1,
                                      .dst_reg = Reg(9, RegSize32),
                                  },
                                  AddReg{
                                      .res_reg = Reg(10, RegSize32),
                                      .lhs_reg = Reg(8, RegSize32),
                                      .rhs_reg = Reg(9, RegSize32),
                                  },
                                  MoveReg{
                                      .src_reg = Reg(10, RegSize32),
                                      .dst_reg = Reg(3, RegSize32),
                                  },
                                  MoveReg{
                                      .src_reg = Reg(3, RegSize32),
                                      .dst_reg = Reg(4, RegSize32),
                                  },
                                  Return{
                                      .res_reg = Reg(1, RegSize32),
                                  }));
}

}  // namespace
}  // namespace lucid
