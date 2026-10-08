#include "lucid/arm64/translator.h"

#include <algorithm>
#include <cstdint>
#include <cstring>
#include <initializer_list>
#include <string>
#include <utility>
#include <vector>

#include "lucid/am/cfg.h"
#include "lucid/am/cfg_builder.h"
#include "lucid/am/instructions.h"
#include "lucid/am/opt.h"
#include "lucid/am/state.h"
#include "lucid/arm64/assembler.h"
#include "lucid/core/string/index.h"
#include "lucid/core/testing/testing.h"
#include "lucid/syntax/context.h"

namespace lucid {
namespace {

// Returns the machine code that `assembler` produced.
std::vector<std::uint8_t> Bytes(const arm64::Assembler& assembler) {
  std::vector<std::uint8_t> out;
  assembler.WriteBytes(out);
  return out;
}

// Returns the instruction words that `assembler` produced.
std::vector<std::uint32_t> Words(const arm64::Assembler& assembler) {
  const std::vector<std::uint8_t> bytes = Bytes(assembler);
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
  GenerateArmAssemblyBinary("f", {}, {}, am_cfg, assembler);

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
  GenerateArmAssemblyBinary("f", {}, {}, am_cfg, assembler);

  EXPECT_TRUE(Emitted(assembler, Instruction([](arm64::Assembler& a) {
                        a.Cset(arm64::W(3), arm64::InvCond::Gt);
                      })));
  EXPECT_TRUE(Emitted(assembler, Instruction([](arm64::Assembler& a) {
                        a.Cmp(arm64::W(3), arm64::Imm(0));
                      })));
}

// Generates code for a function that compares the 64-bit registers 1 and 2
// with `Comparison` and returns the result, so that the result is kept in a
// register rather than branched on.
//
// The result is a `Bool`, which is held in 32 bits whatever the width of what
// was compared.
template <typename Comparison>
void GenerateWideComparison(arm64::Assembler& assembler) {
  const Reg result{3, RegSize32};

  AbstractMachineControlFlowGraphBuilder builder;
  const auto block = builder.AddBlock();
  builder.SetFirst(block);
  builder.SetLast(block);
  builder.AddInstruction(block, Comparison{.res_reg = result,
                                           .lhs_reg = Reg{1, RegSize64},
                                           .rhs_reg = Reg{2, RegSize64}});
  builder.AddInstruction(block, Return{.res_reg = result});
  AbstractMachineControlFlowGraph am_cfg = std::move(builder).Build();

  GenerateArmAssemblyBinary("f", {}, {}, am_cfg, assembler);
}

// Returns whether the comparison `Comparison` of two 64-bit registers is
// computed from all 64 bits of each and sets its result on `cond`.
template <typename Comparison>
bool ComparesWideRegisters(arm64::InvCond cond) {
  arm64::Assembler assembler;
  GenerateWideComparison<Comparison>(assembler);

  return Emitted(assembler, Instruction([](arm64::Assembler& a) {
                   a.Cmp(arm64::X(1), arm64::X(2));
                 })) &&
         Emitted(assembler, Instruction([&](arm64::Assembler& a) {
                   a.Cset(arm64::X(3), cond);
                 }));
}

TEST(Test, ComparisonOfWideRegistersComparesAllOfThem) {
  EXPECT_TRUE(ComparesWideRegisters<GtReg>(arm64::InvCond::Gt));
  EXPECT_TRUE(ComparesWideRegisters<LtReg>(arm64::InvCond::Lt));
  EXPECT_TRUE(ComparesWideRegisters<GeReg>(arm64::InvCond::Ge));
  EXPECT_TRUE(ComparesWideRegisters<LeReg>(arm64::InvCond::Le));
  EXPECT_TRUE(ComparesWideRegisters<EqReg>(arm64::InvCond::Eq));
  EXPECT_TRUE(ComparesWideRegisters<NotEqReg>(arm64::InvCond::Ne));
}

// Generates code for a function whose first block falls through to one that
// settles `phis`, each taking its one argument from the first block.
void GeneratePhiMoves(std::vector<AbstractMachineControlFlowGraph::Phi> phis,
                      arm64::Assembler& assembler) {
  AbstractMachineControlFlowGraphBuilder builder;
  const auto entry = builder.AddBlock();
  const auto exit = builder.AddBlock();
  builder.SetFirst(entry);
  builder.SetLast(exit);
  builder.AddEdge(entry, exit);
  for (auto& phi : phis) builder.AddPhi(exit, std::move(phi));
  builder.AddInstruction(exit, Return{.res_reg = {1, RegSize64}});
  AbstractMachineControlFlowGraph am_cfg = std::move(builder).Build();

  GenerateArmAssemblyBinary("f", {}, {}, am_cfg, assembler);
}

// Returns the instructions that the moves in `dst_src_pairs` assemble to, in
// order, each one moving the 64-bit register `src` into `dst`.
std::vector<std::uint32_t> Moves(
    std::initializer_list<std::pair<int, int>> dst_src_pairs) {
  arm64::Assembler assembler;
  for (const auto [dst, src] : dst_src_pairs) {
    assembler.Mov(arm64::X(dst), arm64::X(src));
  }
  return Words(assembler);
}

TEST(Test, PhiOfWideRegistersMovesAllOfIt) {
  arm64::Assembler assembler;
  GeneratePhiMoves({{.dst = {4, RegSize64}, .srcs = {{1, RegSize64}}}},
                   assembler);

  EXPECT_TRUE(Emitted(assembler, Moves({{4, 1}}).front()));
}

TEST(Test, PhisSwappingWideRegistersGoThroughTheScratchRegister) {
  // Each phi reads what the other writes, so neither move can go first
  // without overwriting what the other still has to read. One of the two is
  // set aside in the scratch register to break the cycle, and which one that
  // is depends on the order the moves are kept in.
  arm64::Assembler assembler;
  GeneratePhiMoves({{.dst = {1, RegSize64}, .srcs = {{2, RegSize64}}},
                    {.dst = {2, RegSize64}, .srcs = {{1, RegSize64}}}},
                   assembler);

  const std::vector<std::uint32_t> words = Words(assembler);
  EXPECT_TRUE(
      std::ranges::contains_subrange(words,
                                     Moves({{15, 1}, {1, 2}, {2, 15}})) ||
      std::ranges::contains_subrange(words, Moves({{15, 2}, {2, 1}, {1, 15}})));
}

// Returns the instructions that `emit` assembles to.
template <typename F>
std::vector<std::uint32_t> Assembled(F emit) {
  arm64::Assembler assembler;
  emit(assembler);
  return Words(assembler);
}

TEST(Test, PhisSwappingRegistersOfEitherWidthGoThroughTheScratchRegister) {
  // `w19` is the lower half of `x19`, so writing it overwrites what the move
  // into `x20` still has to read, and the two moves wait on each other as
  // much as two of the same width would. Whichever register is set aside is
  // set aside whole.
  arm64::Assembler assembler;
  GeneratePhiMoves({{.dst = {20, RegSize64}, .srcs = {{19, RegSize64}}},
                    {.dst = {19, RegSize32}, .srcs = {{20, RegSize32}}}},
                   assembler);

  const std::vector<std::uint32_t> words = Words(assembler);
  EXPECT_TRUE(
      std::ranges::contains_subrange(words, Assembled([](arm64::Assembler& a) {
                                       a.Mov(arm64::X(15), arm64::X(20));
                                       a.Mov(arm64::X(20), arm64::X(19));
                                       a.Mov(arm64::W(19), arm64::W(15));
                                     })) ||
      std::ranges::contains_subrange(words, Assembled([](arm64::Assembler& a) {
                                       a.Mov(arm64::X(15), arm64::X(19));
                                       a.Mov(arm64::W(19), arm64::W(20));
                                       a.Mov(arm64::X(20), arm64::X(15));
                                     })));
}

TEST(Test, PhisReadingOneRegisterBothReadItBeforeItIsWritten) {
  // Two phis take what `x1` holds, and a third writes `x1`. It can only be
  // written once both of the others have read it.
  arm64::Assembler assembler;
  GeneratePhiMoves({{.dst = {3, RegSize64}, .srcs = {{1, RegSize64}}},
                    {.dst = {4, RegSize64}, .srcs = {{1, RegSize64}}},
                    {.dst = {1, RegSize64}, .srcs = {{2, RegSize64}}}},
                   assembler);

  const std::vector<std::uint32_t> words = Words(assembler);
  const auto at = [&](int dst, int src) {
    return std::ranges::find(words, Moves({{dst, src}}).front()) -
           words.begin();
  };
  ASSERT_TRUE(at(1, 2) < std::ssize(words));
  EXPECT_TRUE(at(3, 1) < at(1, 2));
  EXPECT_TRUE(at(4, 1) < at(1, 2));
}

// Returns how many unconditional branches `assembler` emitted.
std::size_t Jumps(const arm64::Assembler& assembler) {
  return std::ranges::count_if(Words(assembler), [](std::uint32_t word) {
    return (word & 0xfc000000u) == 0x14000000u;
  });
}

// Returns how many conditional branches `assembler` emitted.
std::size_t ConditionalBranches(const arm64::Assembler& assembler) {
  return std::ranges::count_if(Words(assembler), [](std::uint32_t word) {
    return (word & 0xff000010u) == 0x54000000u;
  });
}

TEST(Test, BranchFallsThroughToTheBlockWrittenNext) {
  // Both sides of the branch hold nothing and meet where the function
  // returns, so both land there, and that is the block written next. One
  // conditional branch is all it takes, and nothing jumps.
  AbstractMachineControlFlowGraph am_cfg =
      ComparingGraph(/*use_result_later=*/false);

  arm64::Assembler assembler;
  GenerateArmAssemblyBinary("f", {}, {}, am_cfg, assembler);

  EXPECT_EQ(ConditionalBranches(assembler), 1u);
  EXPECT_EQ(Jumps(assembler), 0u);
}

TEST(Test, EdgeGoesStraightPastBlocksThatOnlyPassControlOn) {
  // The two blocks in the middle hold nothing, so control goes from the
  // first block to the last without a jump through either.
  AbstractMachineControlFlowGraphBuilder builder;
  const auto entry = builder.AddBlock();
  const auto first_empty = builder.AddBlock();
  const auto second_empty = builder.AddBlock();
  const auto exit = builder.AddBlock();
  builder.SetFirst(entry);
  builder.SetLast(exit);
  builder.AddEdge(entry, first_empty);
  builder.AddEdge(first_empty, second_empty);
  builder.AddEdge(second_empty, exit);
  builder.AddInstruction(entry,
                         SetReg{.src_val = 7, .dst_reg = {1, RegSize32}});
  builder.AddInstruction(exit, Return{.res_reg = {1, RegSize32}});
  AbstractMachineControlFlowGraph am_cfg = std::move(builder).Build();

  arm64::Assembler assembler;
  GenerateArmAssemblyBinary("f", {}, {}, am_cfg, assembler);

  EXPECT_EQ(Jumps(assembler), 0u);
}

TEST(Test, GenerateArmStartBinaryWorks) {
  arm64::Assembler assembler;

  GenerateArmStartBinary(assembler);

  // The entry point is the one symbol the linker has to resolve, and the
  // sequence calls into libc, which the linker resolves for it.
  EXPECT_THAT(assembler.GlobalLabels().Get("_start"), Optional(Equals(0)));
  EXPECT_TRUE(assembler.ExternalLabels().contains("_printf"));
  EXPECT_TRUE(assembler.ExternalLabels().contains("_nanosleep"));

  // A division by nothing flushes what was printed, writes its message and
  // leaves, each of which libc does for it.
  EXPECT_TRUE(assembler.ExternalLabels().contains("_fflush"));
  EXPECT_TRUE(assembler.ExternalLabels().contains("_write"));
  EXPECT_TRUE(assembler.ExternalLabels().contains("_exit"));

  // The sequence calls the program's entry function, so it cannot be written
  // until something defines it.
  assembler.Bind(assembler.Named("main"));
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
  EXPECT_EQ(Bytes(assembler),
            (std::vector<std::uint8_t>{'c', 'd', 0, 0, 'a', 'b', 0, 0}));
}

TEST(Test, GenerateArmEndBinaryWritesIntegers) {
  SyntaxContext syn_ctx;
  AbstractMachineState am_state;
  am_state.ints.Insert(7);

  arm64::Assembler assembler;
  GenerateArmEndBinary(syn_ctx, am_state, assembler);

  EXPECT_EQ(Bytes(assembler),
            (std::vector<std::uint8_t>{7, 0, 0, 0, 0, 0, 0, 0}));
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
  GenerateArmAssemblyBinary("main", {}, {}, am_cfg, assembler);

  const std::vector<std::uint8_t> bytes = Bytes(assembler);
  EXPECT_EQ(bytes.size() % 4, 0u);
  // A function ends by returning to its caller.
  EXPECT_EQ(std::vector<std::uint8_t>(bytes.end() - 4, bytes.end()),
            (std::vector<std::uint8_t>{0xc0, 0x03, 0x5f, 0xd6}));
}

}  // namespace
}  // namespace lucid
