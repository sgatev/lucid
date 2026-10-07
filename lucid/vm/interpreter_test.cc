#include "lucid/vm/interpreter.h"

#include <cstddef>
#include <cstdint>
#include <limits>
#include <optional>
#include <utility>

#include "lucid/am/cfg.h"
#include "lucid/am/cfg_builder.h"
#include "lucid/am/instructions.h"
#include "lucid/am/state.h"
#include "lucid/core/testing/testing.h"

namespace lucid {
namespace {

constexpr int kLeast = std::numeric_limits<std::int32_t>::min();
constexpr int kGreatest = std::numeric_limits<std::int32_t>::max();

// Returns what an instruction of type `Op` comes to over `lhs` and `rhs`, set
// into registers of `size`.
template <typename Op>
std::optional<std::int64_t> InterpretBinary(int lhs, int rhs,
                                            RegSize size = RegSize32) {
  AbstractMachineControlFlowGraphBuilder g;

  auto a = g.AddBlock();
  auto z = g.AddBlock();

  const Reg l{.id = 1, .size = size};
  const Reg r{.id = 2, .size = size};
  const Reg res{.id = 3, .size = size};

  g.SetFirst(a);
  g.AddInstruction(a, SetReg{.src_val = lhs, .dst_reg = l});
  g.AddInstruction(a, SetReg{.src_val = rhs, .dst_reg = r});
  g.AddInstruction(a, Op{.res_reg = res, .lhs_reg = l, .rhs_reg = r});
  g.AddEdge(a, z);

  g.SetLast(z);
  g.AddInstruction(z, Return{.res_reg = res});

  AbstractMachineState am_state;
  auto result = InterpretAbstractMachineFunction(
      /*am_cfgs=*/{}, std::move(g).Build(), /*args=*/{}, am_state);
  if (!result.has_value()) return std::nullopt;
  return *result;
}

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
  auto result = InterpretAbstractMachineFunction(
      /*am_cfgs=*/{}, std::move(g).Build(), /*args=*/{}, am_state);
  ASSERT_TRUE(result.has_value());
  EXPECT_EQ(*result, 21);
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
  auto result = InterpretAbstractMachineFunction(
      /*am_cfgs=*/{}, std::move(g).Build(), /*args=*/{}, am_state);
  ASSERT_TRUE(result.has_value());
  EXPECT_EQ(*result, 21);
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
  auto result = InterpretAbstractMachineFunction(
      /*am_cfgs=*/{}, std::move(g).Build(), /*args=*/{}, am_state);
  ASSERT_TRUE(result.has_value());
  EXPECT_EQ(*result, 63);
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
  auto result = InterpretAbstractMachineFunction(
      /*am_cfgs=*/{}, std::move(g).Build(), /*args=*/{}, am_state);
  ASSERT_TRUE(result.has_value());
  EXPECT_EQ(*result, 18);
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
  auto result = InterpretAbstractMachineFunction(
      /*am_cfgs=*/{}, std::move(g).Build(), /*args=*/{}, am_state);
  ASSERT_TRUE(result.has_value());
  EXPECT_EQ(*result, 42);
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
  auto result = InterpretAbstractMachineFunction(
      /*am_cfgs=*/{}, std::move(g).Build(), /*args=*/{}, am_state);
  ASSERT_TRUE(result.has_value());
  EXPECT_EQ(*result, 10);
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
  auto result = InterpretAbstractMachineFunction(
      /*am_cfgs=*/{}, std::move(g).Build(), /*args=*/{}, am_state);
  ASSERT_TRUE(result.has_value());
  EXPECT_EQ(*result, 0);
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
  auto result = InterpretAbstractMachineFunction(
      /*am_cfgs=*/{}, std::move(g).Build(), /*args=*/{}, am_state);
  ASSERT_TRUE(result.has_value());
  EXPECT_EQ(*result, 1);
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
  auto result = InterpretAbstractMachineFunction(
      /*am_cfgs=*/{}, std::move(g).Build(), /*args=*/{}, am_state);
  ASSERT_TRUE(result.has_value());
  EXPECT_EQ(*result, 0);
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
  auto result = InterpretAbstractMachineFunction(
      /*am_cfgs=*/{}, std::move(g).Build(), /*args=*/{}, am_state);
  ASSERT_TRUE(result.has_value());
  EXPECT_EQ(*result, 1);
}

// Both sides of each comparison, and the point where they meet, which is the
// whole of what `>=` and `<=` add over `>` and `<`.
TEST(Test, InterpretAbstractMachineFunctionGeRegAndLeReg) {
  EXPECT_EQ(InterpretBinary<GeReg>(1, 2), 0);
  EXPECT_EQ(InterpretBinary<GeReg>(2, 1), 1);
  EXPECT_EQ(InterpretBinary<GeReg>(2, 2), 1);
  EXPECT_EQ(InterpretBinary<LeReg>(1, 2), 1);
  EXPECT_EQ(InterpretBinary<LeReg>(2, 1), 0);
  EXPECT_EQ(InterpretBinary<LeReg>(2, 2), 1);
}

// 2^40 + 3, which needs more than 32 bits.
constexpr std::int64_t kWide = 0x100'0000'0003;

// Returns what the slot with index `slot` comes to when read back, in a frame
// of slots 4, 8 and 4 bytes wide that have had -5, `kWide` and 7 stored in
// them, in that order.
std::optional<std::int64_t> InterpretStoredSlot(std::size_t slot) {
  AbstractMachineControlFlowGraphBuilder g;

  auto a = g.AddBlock();
  auto z = g.AddBlock();

  const Reg first{.id = 1, .size = RegSize32};
  const Reg second{.id = 2, .size = RegSize64};
  const Reg third{.id = 3, .size = RegSize32};
  const Reg read{.id = 4, .size = slot == 1 ? RegSize64 : RegSize32};

  g.SetFirst(a);
  g.AddInstruction(a, SetReg{.src_val = -5, .dst_reg = first});
  g.AddInstruction(a, SetInt{.src_val = kWide, .dst_reg = second});
  g.AddInstruction(a, SetReg{.src_val = 7, .dst_reg = third});
  g.AddInstruction(a, StoreStack{.offset = 0, .src_reg = first});
  g.AddInstruction(a, StoreStack{.offset = 1, .src_reg = second});
  g.AddInstruction(a, StoreStack{.offset = 2, .src_reg = third});
  g.AddInstruction(a, LoadStack{.offset = slot, .dst_reg = read});
  g.AddEdge(a, z);

  g.SetLast(z);
  g.AddInstruction(z, Return{.res_reg = read});

  AbstractMachineControlFlowGraph am_cfg = std::move(g).Build();
  am_cfg.stack_slots = {4, 8, 4};

  AbstractMachineState am_state;
  auto result = InterpretAbstractMachineFunction(
      /*am_cfgs=*/{}, am_cfg, /*args=*/{}, am_state);
  if (!result.has_value()) return std::nullopt;
  return *result;
}

// Each slot begins where the ones before it end, so storing into one leaves
// the others as they were. A value comes back whole at the width it was
// stored at, and one narrower than a register comes back with its sign.
TEST(Test, InterpretAbstractMachineFunctionKeepsStackSlotsApart) {
  EXPECT_EQ(InterpretStoredSlot(0), -5);
  EXPECT_EQ(InterpretStoredSlot(1), kWide);
  EXPECT_EQ(InterpretStoredSlot(2), 7);
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
  auto result = InterpretAbstractMachineFunction(
      /*am_cfgs=*/{}, std::move(g).Build(), /*args=*/{}, am_state);
  ASSERT_TRUE(result.has_value());
  EXPECT_EQ(*result, 63);
}

// A number that runs past the width of its register comes back round, as it
// does in the machine, rather than being held wider than the register could.
TEST(Test, InterpretAbstractMachineFunctionWrapsAtTheRegisterWidth) {
  EXPECT_EQ(InterpretBinary<AddReg>(kGreatest, 1), kLeast);
  EXPECT_EQ(InterpretBinary<SubReg>(kLeast, 1), kGreatest);
  EXPECT_EQ(InterpretBinary<MulReg>(65536, 65536), 0);
  EXPECT_EQ(InterpretBinary<AddReg>(kGreatest, 1, RegSize64),
            std::int64_t{kGreatest} + 1);
}

// Dividing the least number by minus one has no answer in C++ and has one in
// the machine: it runs past the width and comes back round to where it began.
TEST(Test, InterpretAbstractMachineFunctionDividesAsTheMachineDoes) {
  EXPECT_EQ(InterpretBinary<DivReg>(kLeast, -1), kLeast);
  EXPECT_EQ(InterpretBinary<ModReg>(kLeast, -1), 0);
  EXPECT_EQ(InterpretBinary<DivReg>(-7, 2), -3);
  EXPECT_EQ(InterpretBinary<ModReg>(-7, 2), -1);
}

// A division by nothing has no answer, so working one out is reported rather
// than given one.
TEST(Test, InterpretAbstractMachineFunctionReportsDivisionByZero) {
  EXPECT_EQ(InterpretBinary<DivReg>(7, 0), std::nullopt);
  EXPECT_EQ(InterpretBinary<ModReg>(7, 0), std::nullopt);
}

}  // namespace
}  // namespace lucid
