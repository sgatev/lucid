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

  auto a = g.block();
  auto z = g.block();

  g.first(a);
  g.inst(a, SetReg{
                .src_val = "21",
                .dst_reg = RegId(1),
            });
  g.edge(a, z);

  g.last(z);
  g.inst(z, Return{
                .res_reg = RegId(1),
            });

  AbstractMachineState am_state;
  int result = InterpretAbstractMachineFunction(
      /*am_cfgs=*/{}, std::move(g).build(), /*args=*/{}, am_state);
  EXPECT_EQ(result, 21);
}

TEST(InterpretAbstractMachineFunctionTest, MoveReg) {
  AbstractMachineControlFlowGraphBuilder g;

  auto a = g.block();
  auto z = g.block();

  g.first(a);
  g.inst(a, SetReg{
                .src_val = "21",
                .dst_reg = RegId(1),
            });
  g.inst(a, MoveReg{
                .src_reg = RegId(1),
                .dst_reg = RegId(2),
            });
  g.edge(a, z);

  g.last(z);
  g.inst(z, Return{
                .res_reg = RegId(2),
            });

  AbstractMachineState am_state;
  int result = InterpretAbstractMachineFunction(
      /*am_cfgs=*/{}, std::move(g).build(), /*args=*/{}, am_state);
  EXPECT_EQ(result, 21);
}

TEST(InterpretAbstractMachineFunctionTest, AddReg) {
  AbstractMachineControlFlowGraphBuilder g;

  auto a = g.block();
  auto z = g.block();

  g.first(a);
  g.inst(a, SetReg{
                .src_val = "21",
                .dst_reg = RegId(1),
            });
  g.inst(a, SetReg{
                .src_val = "42",
                .dst_reg = RegId(2),
            });
  g.inst(a, AddReg{
                .res_reg = RegId(3),
                .lhs_reg = RegId(1),
                .rhs_reg = RegId(2),
            });
  g.edge(a, z);

  g.last(z);
  g.inst(z, Return{
                .res_reg = RegId(3),
            });

  AbstractMachineState am_state;
  int result = InterpretAbstractMachineFunction(
      /*am_cfgs=*/{}, std::move(g).build(), /*args=*/{}, am_state);
  EXPECT_EQ(result, 63);
}

TEST(InterpretAbstractMachineFunctionTest, GtReg) {
  AbstractMachineControlFlowGraphBuilder g;

  auto a = g.block();
  auto z = g.block();

  g.first(a);
  g.inst(a, SetReg{
                .src_val = "21",
                .dst_reg = RegId(1),
            });
  g.inst(a, SetReg{
                .src_val = "42",
                .dst_reg = RegId(2),
            });
  g.inst(a, GtReg{
                .res_reg = RegId(3),
                .lhs_reg = RegId(1),
                .rhs_reg = RegId(2),
            });
  g.edge(a, z);

  g.last(z);
  g.inst(z, Return{
                .res_reg = RegId(3),
            });

  AbstractMachineState am_state;
  int result = InterpretAbstractMachineFunction(
      /*am_cfgs=*/{}, std::move(g).build(), /*args=*/{}, am_state);
  EXPECT_EQ(result, 0);
}

TEST(InterpretAbstractMachineFunctionTest, Sequence) {
  AbstractMachineControlFlowGraphBuilder g;

  auto a = g.block();
  auto b = g.block();
  auto c = g.block();
  auto z = g.block();

  g.first(a);
  g.inst(a, SetReg{
                .src_val = "21",
                .dst_reg = RegId(1),
            });
  g.edge(a, b);

  g.inst(b, SetReg{
                .src_val = "42",
                .dst_reg = RegId(2),
            });
  g.edge(b, c);

  g.inst(c, AddReg{
                .res_reg = RegId(3),
                .lhs_reg = RegId(1),
                .rhs_reg = RegId(2),
            });
  g.edge(c, z);

  g.last(z);
  g.inst(z, Return{
                .res_reg = RegId(3),
            });

  AbstractMachineState am_state;
  int result = InterpretAbstractMachineFunction(
      /*am_cfgs=*/{}, std::move(g).build(), /*args=*/{}, am_state);
  EXPECT_EQ(result, 63);
}

}  // namespace
}  // namespace lucid
