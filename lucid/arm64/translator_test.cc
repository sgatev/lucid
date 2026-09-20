#include "lucid/arm64/translator.h"

#include <algorithm>
#include <cstdint>
#include <cstring>
#include <sstream>
#include <string>
#include <vector>

#include "lucid/am/cfg_builder.h"
#include "lucid/am/instructions.h"
#include "lucid/am/opt.h"
#include "lucid/am/state.h"
#include "lucid/arm64/assembler.h"
#include "lucid/core/testing/testing.h"
#include "lucid/syntax/context.h"

namespace lucid {
namespace {

// Returns the machine code that `assembler` produced.
std::string Bytes(const arm64::Assembler& assembler) {
  std::ostringstream out;
  assembler.WriteBytes(out);
  return out.str();
}

// Returns the instruction words that `assembler` produced.
std::vector<std::uint32_t> Words(const arm64::Assembler& assembler) {
  const std::string bytes = Bytes(assembler);
  std::vector<std::uint32_t> words(bytes.size() / sizeof(std::uint32_t));
  std::memcpy(words.data(), bytes.data(), words.size() * sizeof(std::uint32_t));
  return words;
}

// Returns whether `assembler` emitted `instruction` anywhere.
bool Emitted(const arm64::Assembler& assembler, std::uint32_t instruction) {
  return std::ranges::contains(Words(assembler), instruction);
}

// Returns the one instruction that `emit` assembles to.
template <typename F>
std::uint32_t Instruction(F emit) {
  arm64::Assembler assembler;
  emit(assembler);
  return Words(assembler).front();
}

// Returns a graph that compares two registers and branches on the result,
// with `use_result_later` deciding whether anything but the branch reads it.
//
// Both arms meet again at the block the function returns from, as the arms of
// a branch in a compiled program do.
//
// The graph is optimized before it is returned, as one reaching the backend
// has been: whether the branch is the only reader of what it branches on is
// recorded there, and a graph that never went through it says no.
AbstractMachineControlFlowGraph ComparingGraph(bool use_result_later) {
  const Reg lhs{1, RegSize32};
  const Reg rhs{2, RegSize32};
  const Reg result{3, RegSize32};

  AbstractMachineControlFlowGraphBuilder builder;
  const auto head = builder.AddBlock();
  const auto then_block = builder.AddBlock();
  const auto else_block = builder.AddBlock();
  const auto exit_block = builder.AddBlock();
  builder.SetFirst(head);
  builder.SetLast(exit_block);
  builder.AddEdge(head, then_block);
  builder.AddEdge(head, else_block);
  builder.AddEdge(then_block, exit_block);
  builder.AddEdge(else_block, exit_block);

  builder.AddInstruction(
      head, GtReg{.res_reg = result, .lhs_reg = lhs, .rhs_reg = rhs});
  if (use_result_later) {
    builder.AddInstruction(
        then_block, MoveReg{.src_reg = result, .dst_reg = Reg{4, RegSize32}});
  }
  builder.AddInstruction(exit_block, Return{.res_reg = lhs});

  AbstractMachineControlFlowGraph am_cfg = std::move(builder).Build();
  am_cfg.GetBlock(head).branch_cond = result;
  OptimizeAbstractMachineFunction(am_cfg);
  return am_cfg;
}

TEST(Test, BranchDecidesTheComparisonItBranchesOn) {
  // Nothing but the branch reads the comparison, so the branch carries it.
  AbstractMachineControlFlowGraph am_cfg =
      ComparingGraph(/*use_result_later=*/false);

  arm64::Assembler assembler;
  GenerateArmAssemblyBinary("f", {}, am_cfg, assembler);

  EXPECT_TRUE(Emitted(assembler, Instruction([](arm64::Assembler& a) {
                        a.Cmp(arm64::W(1), arm64::W(2));
                      })));
  EXPECT_FALSE(Emitted(assembler, Instruction([](arm64::Assembler& a) {
                         a.Cset(arm64::W(3), arm64::InvCond::Gt);
                       })));
  // Comparing the result against zero is what the branch no longer needs.
  EXPECT_FALSE(Emitted(assembler, Instruction([](arm64::Assembler& a) {
                         a.Cmp(arm64::W(3), arm64::Imm(0));
                       })));
}

TEST(Test, ComparisonThatOutlivesItsBlockIsStillComputed) {
  // The branch is not the only reader: a block it branches to moves the
  // result elsewhere, so the result has to be a value in a register.
  AbstractMachineControlFlowGraph am_cfg =
      ComparingGraph(/*use_result_later=*/true);

  arm64::Assembler assembler;
  GenerateArmAssemblyBinary("f", {}, am_cfg, assembler);

  EXPECT_TRUE(Emitted(assembler, Instruction([](arm64::Assembler& a) {
                        a.Cset(arm64::W(3), arm64::InvCond::Gt);
                      })));
  EXPECT_TRUE(Emitted(assembler, Instruction([](arm64::Assembler& a) {
                        a.Cmp(arm64::W(3), arm64::Imm(0));
                      })));
}

TEST(Test, GenerateArmStartBinaryWorks) {
  arm64::Assembler assembler;

  GenerateArmStartBinary(assembler);

  // The entry point is the one symbol the linker has to resolve, and the
  // sequence calls into libc, which the linker resolves for it.
  EXPECT_THAT(assembler.GlobalLabels().Get("_start"), Optional(Equals(0)));
  EXPECT_TRUE(assembler.ExternalLabels().contains("_printf"));
  EXPECT_TRUE(assembler.ExternalLabels().contains("_nanosleep"));

  // The sequence calls the program's entry function, so it cannot be written
  // until something defines it.
  assembler.Label("main");
  EXPECT_EQ(Bytes(assembler).size(), assembler.OutputBytesCount());
  EXPECT_EQ(assembler.OutputBytesCount() % 4, 0u);
}

TEST(Test, GenerateArmEndBinaryWritesStringsInPoolOrder) {
  // A string literal is interned as it was written, quote marks included.
  SyntaxContext syn_ctx;
  StringIndex::Ref ab = syn_ctx.AddIdent(R"("ab")");
  StringIndex::Ref cd = syn_ctx.AddIdent(R"("cd")");

  // The pool order is the order the strings were first referenced, which is
  // what their `strN` labels are numbered by, so it is the order they must be
  // written in -- not the order they were interned in.
  AbstractMachineState am_state;
  am_state.strings = {cd, ab};

  arm64::Assembler assembler;
  GenerateArmEndBinary(syn_ctx, am_state, assembler);

  // Each string is terminated and padded to a four byte boundary.
  EXPECT_EQ(Bytes(assembler), std::string("cd\0\0ab\0\0", 8));
}

TEST(Test, GenerateArmEndBinaryWritesIntegers) {
  SyntaxContext syn_ctx;
  AbstractMachineState am_state;
  am_state.ints.Insert(7);

  arm64::Assembler assembler;
  GenerateArmEndBinary(syn_ctx, am_state, assembler);

  EXPECT_EQ(Bytes(assembler), std::string("\x07\0\0\0\0\0\0\0", 8));
}

TEST(Test, GenerateArmEndBinaryWritesNothingWithoutConstants) {
  SyntaxContext syn_ctx;
  AbstractMachineState am_state;

  arm64::Assembler assembler;
  GenerateArmEndBinary(syn_ctx, am_state, assembler);

  EXPECT_EQ(assembler.OutputBytesCount(), 0u);
}

TEST(Test, GenerateArmAssemblyBinaryWorks) {
  AbstractMachineControlFlowGraphBuilder builder;
  auto block = builder.AddBlock();
  builder.SetFirst(block);
  builder.SetLast(block);
  builder.AddInstruction(block, Return{.res_reg = {1, RegSize32}});
  AbstractMachineControlFlowGraph am_cfg = std::move(builder).Build();

  arm64::Assembler assembler;
  GenerateArmAssemblyBinary("main", {}, am_cfg, assembler);

  const std::string bytes = Bytes(assembler);
  EXPECT_EQ(bytes.size() % 4, 0u);
  // A function ends by returning to its caller.
  EXPECT_EQ(bytes.substr(bytes.size() - 4), std::string("\xc0\x03\x5f\xd6", 4));
}

}  // namespace
}  // namespace lucid
