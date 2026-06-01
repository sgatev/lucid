#include "lucid/vm/interpreter.h"

#include <utility>

#include "gtest/gtest.h"
#include "lucid/am/cfg.h"
#include "lucid/am/cfg_builder.h"
#include "lucid/am/instructions.h"
#include "lucid/am/state.h"

namespace lucid {
namespace {

TEST(InterpretAbstractMachineFunctionTest, SetReg) {
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

TEST(InterpretAbstractMachineFunctionTest, MoveReg) {
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

TEST(InterpretAbstractMachineFunctionTest, AddReg) {
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

TEST(InterpretAbstractMachineFunctionTest, SubReg) {
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

TEST(InterpretAbstractMachineFunctionTest, MulReg) {
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

TEST(InterpretAbstractMachineFunctionTest, DivReg) {
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

TEST(InterpretAbstractMachineFunctionTest, GtReg) {
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

TEST(InterpretAbstractMachineFunctionTest, LtReg) {
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

TEST(InterpretAbstractMachineFunctionTest, EqReg) {
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

TEST(InterpretAbstractMachineFunctionTest, NotEqReg) {
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

TEST(InterpretAbstractMachineFunctionTest, Sequence) {
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
