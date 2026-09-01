#include "lucid/vm/interpreter.h"

#include <utility>

#include "lucid/am/cfg.h"
#include "lucid/am/cfg_builder.h"
#include "lucid/am/instructions.h"
#include "lucid/am/state.h"
#include "lucid/core/testing/testing.h"

namespace lucid {
namespace {

TEST(Test, InterpretAbstractMachineFunctionSetReg) {
  AbstractMachineControlFlowGraphBuilder g;

  auto a = g.AddBlock();
  auto z = g.AddBlock();

  g.SetFirst(a);
  g.AddInstruction(a, SetReg{
                          .src_val = 21,
                          .dst_reg = Reg(1),
                      });
  g.AddEdge(a, z);

  g.SetLast(z);
  g.AddInstruction(z, Return{
                          .res_reg = Reg(1),
                      });

  AbstractMachineState am_state;
  int result = InterpretAbstractMachineFunction(
      /*am_cfgs=*/{}, std::move(g).Build(), /*args=*/{}, am_state);
  EXPECT_EQ(result, 21);
}

TEST(Test, InterpretAbstractMachineFunctionMoveReg) {
  AbstractMachineControlFlowGraphBuilder g;

  auto a = g.AddBlock();
  auto z = g.AddBlock();

  g.SetFirst(a);
  g.AddInstruction(a, SetReg{
                          .src_val = 21,
                          .dst_reg = Reg(1),
                      });
  g.AddInstruction(a, MoveReg{
                          .src_reg = Reg(1),
                          .dst_reg = Reg(2),
                      });
  g.AddEdge(a, z);

  g.SetLast(z);
  g.AddInstruction(z, Return{
                          .res_reg = Reg(2),
                      });

  AbstractMachineState am_state;
  int result = InterpretAbstractMachineFunction(
      /*am_cfgs=*/{}, std::move(g).Build(), /*args=*/{}, am_state);
  EXPECT_EQ(result, 21);
}

TEST(Test, InterpretAbstractMachineFunctionAddReg) {
  AbstractMachineControlFlowGraphBuilder g;

  auto a = g.AddBlock();
  auto z = g.AddBlock();

  g.SetFirst(a);
  g.AddInstruction(a, SetReg{
                          .src_val = 21,
                          .dst_reg = Reg(1),
                      });
  g.AddInstruction(a, SetReg{
                          .src_val = 42,
                          .dst_reg = Reg(2),
                      });
  g.AddInstruction(a, AddReg{
                          .res_reg = Reg(3),
                          .lhs_reg = Reg(1),
                          .rhs_reg = Reg(2),
                      });
  g.AddEdge(a, z);

  g.SetLast(z);
  g.AddInstruction(z, Return{
                          .res_reg = Reg(3),
                      });

  AbstractMachineState am_state;
  int result = InterpretAbstractMachineFunction(
      /*am_cfgs=*/{}, std::move(g).Build(), /*args=*/{}, am_state);
  EXPECT_EQ(result, 63);
}

TEST(Test, InterpretAbstractMachineFunctionSubReg) {
  AbstractMachineControlFlowGraphBuilder g;

  auto a = g.AddBlock();
  auto z = g.AddBlock();

  g.SetFirst(a);
  g.AddInstruction(a, SetReg{
                          .src_val = 60,
                          .dst_reg = Reg(1),
                      });
  g.AddInstruction(a, SetReg{
                          .src_val = 42,
                          .dst_reg = Reg(2),
                      });
  g.AddInstruction(a, SubReg{
                          .res_reg = Reg(3),
                          .lhs_reg = Reg(1),
                          .rhs_reg = Reg(2),
                      });
  g.AddEdge(a, z);

  g.SetLast(z);
  g.AddInstruction(z, Return{
                          .res_reg = Reg(3),
                      });

  AbstractMachineState am_state;
  int result = InterpretAbstractMachineFunction(
      /*am_cfgs=*/{}, std::move(g).Build(), /*args=*/{}, am_state);
  EXPECT_EQ(result, 18);
}

TEST(Test, InterpretAbstractMachineFunctionMulReg) {
  AbstractMachineControlFlowGraphBuilder g;

  auto a = g.AddBlock();
  auto z = g.AddBlock();

  g.SetFirst(a);
  g.AddInstruction(a, SetReg{
                          .src_val = 21,
                          .dst_reg = Reg(1),
                      });
  g.AddInstruction(a, SetReg{
                          .src_val = 2,
                          .dst_reg = Reg(2),
                      });
  g.AddInstruction(a, MulReg{
                          .res_reg = Reg(3),
                          .lhs_reg = Reg(1),
                          .rhs_reg = Reg(2),
                      });
  g.AddEdge(a, z);

  g.SetLast(z);
  g.AddInstruction(z, Return{
                          .res_reg = Reg(3),
                      });

  AbstractMachineState am_state;
  int result = InterpretAbstractMachineFunction(
      /*am_cfgs=*/{}, std::move(g).Build(), /*args=*/{}, am_state);
  EXPECT_EQ(result, 42);
}

TEST(Test, InterpretAbstractMachineFunctionDivReg) {
  AbstractMachineControlFlowGraphBuilder g;

  auto a = g.AddBlock();
  auto z = g.AddBlock();

  g.SetFirst(a);
  g.AddInstruction(a, SetReg{
                          .src_val = 30,
                          .dst_reg = Reg(1),
                      });
  g.AddInstruction(a, SetReg{
                          .src_val = 3,
                          .dst_reg = Reg(2),
                      });
  g.AddInstruction(a, DivReg{
                          .res_reg = Reg(3),
                          .lhs_reg = Reg(1),
                          .rhs_reg = Reg(2),
                      });
  g.AddEdge(a, z);

  g.SetLast(z);
  g.AddInstruction(z, Return{
                          .res_reg = Reg(3),
                      });

  AbstractMachineState am_state;
  int result = InterpretAbstractMachineFunction(
      /*am_cfgs=*/{}, std::move(g).Build(), /*args=*/{}, am_state);
  EXPECT_EQ(result, 10);
}

TEST(Test, InterpretAbstractMachineFunctionGtReg) {
  AbstractMachineControlFlowGraphBuilder g;

  auto a = g.AddBlock();
  auto z = g.AddBlock();

  g.SetFirst(a);
  g.AddInstruction(a, SetReg{
                          .src_val = 21,
                          .dst_reg = Reg(1),
                      });
  g.AddInstruction(a, SetReg{
                          .src_val = 42,
                          .dst_reg = Reg(2),
                      });
  g.AddInstruction(a, GtReg{
                          .res_reg = Reg(3),
                          .lhs_reg = Reg(1),
                          .rhs_reg = Reg(2),
                      });
  g.AddEdge(a, z);

  g.SetLast(z);
  g.AddInstruction(z, Return{
                          .res_reg = Reg(3),
                      });

  AbstractMachineState am_state;
  int result = InterpretAbstractMachineFunction(
      /*am_cfgs=*/{}, std::move(g).Build(), /*args=*/{}, am_state);
  EXPECT_EQ(result, 0);
}

TEST(Test, InterpretAbstractMachineFunctionLtReg) {
  AbstractMachineControlFlowGraphBuilder g;

  auto a = g.AddBlock();
  auto z = g.AddBlock();

  g.SetFirst(a);
  g.AddInstruction(a, SetReg{
                          .src_val = 21,
                          .dst_reg = Reg(1),
                      });
  g.AddInstruction(a, SetReg{
                          .src_val = 42,
                          .dst_reg = Reg(2),
                      });
  g.AddInstruction(a, LtReg{
                          .res_reg = Reg(3),
                          .lhs_reg = Reg(1),
                          .rhs_reg = Reg(2),
                      });
  g.AddEdge(a, z);

  g.SetLast(z);
  g.AddInstruction(z, Return{
                          .res_reg = Reg(3),
                      });

  AbstractMachineState am_state;
  int result = InterpretAbstractMachineFunction(
      /*am_cfgs=*/{}, std::move(g).Build(), /*args=*/{}, am_state);
  EXPECT_EQ(result, 1);
}

TEST(Test, InterpretAbstractMachineFunctionEqReg) {
  AbstractMachineControlFlowGraphBuilder g;

  auto a = g.AddBlock();
  auto z = g.AddBlock();

  g.SetFirst(a);
  g.AddInstruction(a, SetReg{
                          .src_val = 21,
                          .dst_reg = Reg(1),
                      });
  g.AddInstruction(a, SetReg{
                          .src_val = 42,
                          .dst_reg = Reg(2),
                      });
  g.AddInstruction(a, EqReg{
                          .res_reg = Reg(3),
                          .lhs_reg = Reg(1),
                          .rhs_reg = Reg(2),
                      });
  g.AddEdge(a, z);

  g.SetLast(z);
  g.AddInstruction(z, Return{
                          .res_reg = Reg(3),
                      });

  AbstractMachineState am_state;
  int result = InterpretAbstractMachineFunction(
      /*am_cfgs=*/{}, std::move(g).Build(), /*args=*/{}, am_state);
  EXPECT_EQ(result, 0);
}

TEST(Test, InterpretAbstractMachineFunctionNotEqReg) {
  AbstractMachineControlFlowGraphBuilder g;

  auto a = g.AddBlock();
  auto z = g.AddBlock();

  g.SetFirst(a);
  g.AddInstruction(a, SetReg{
                          .src_val = 21,
                          .dst_reg = Reg(1),
                      });
  g.AddInstruction(a, SetReg{
                          .src_val = 42,
                          .dst_reg = Reg(2),
                      });
  g.AddInstruction(a, NotEqReg{
                          .res_reg = Reg(3),
                          .lhs_reg = Reg(1),
                          .rhs_reg = Reg(2),
                      });
  g.AddEdge(a, z);

  g.SetLast(z);
  g.AddInstruction(z, Return{
                          .res_reg = Reg(3),
                      });

  AbstractMachineState am_state;
  int result = InterpretAbstractMachineFunction(
      /*am_cfgs=*/{}, std::move(g).Build(), /*args=*/{}, am_state);
  EXPECT_EQ(result, 1);
}

TEST(Test, InterpretAbstractMachineFunctionSequence) {
  AbstractMachineControlFlowGraphBuilder g;

  auto a = g.AddBlock();
  auto b = g.AddBlock();
  auto c = g.AddBlock();
  auto z = g.AddBlock();

  g.SetFirst(a);
  g.AddInstruction(a, SetReg{
                          .src_val = 21,
                          .dst_reg = Reg(1),
                      });
  g.AddEdge(a, b);

  g.AddInstruction(b, SetReg{
                          .src_val = 42,
                          .dst_reg = Reg(2),
                      });
  g.AddEdge(b, c);

  g.AddInstruction(c, AddReg{
                          .res_reg = Reg(3),
                          .lhs_reg = Reg(1),
                          .rhs_reg = Reg(2),
                      });
  g.AddEdge(c, z);

  g.SetLast(z);
  g.AddInstruction(z, Return{
                          .res_reg = Reg(3),
                      });

  AbstractMachineState am_state;
  int result = InterpretAbstractMachineFunction(
      /*am_cfgs=*/{}, std::move(g).Build(), /*args=*/{}, am_state);
  EXPECT_EQ(result, 63);
}

}  // namespace
}  // namespace lucid
