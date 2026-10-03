#include "lucid/am/strict_ssa.h"

#include <string>
#include <utility>

#include "lucid/am/cfg.h"
#include "lucid/am/cfg_builder.h"
#include "lucid/am/instructions.h"
#include "lucid/core/testing/testing.h"

namespace lucid {
namespace {

// Returns what `CheckStrictSsa` finds wrong with the graph `g` builds, or
// nothing.
std::string Violation(AbstractMachineControlFlowGraphBuilder g) {
  const auto result = CheckStrictSsa(std::move(g).Build());
  return result.has_value() ? "" : result.error();
}

// A loop counting its parameter down: the phi at its head takes the
// parameter in from the entry and the decremented count back from its own
// body.
TEST(Test, CheckStrictSsaAcceptsALoop) {
  AbstractMachineControlFlowGraphBuilder g;
  auto entry = g.AddBlock();
  auto head = g.AddBlock();
  auto exit = g.AddBlock();
  g.SetFirst(entry);
  g.SetLast(exit);
  g.AddEdge(entry, head);
  g.AddEdge(head, head);
  g.AddEdge(head, exit);

  g.AddParam(Reg(1));
  g.AddPhi(head, {.dst = Reg(2), .srcs = {Reg(1), Reg(4)}});
  g.AddInstruction(head, SetReg{.src_val = 1, .dst_reg = Reg(3)});
  g.AddInstruction(
      head, SubReg{.res_reg = Reg(4), .lhs_reg = Reg(2), .rhs_reg = Reg(3)});
  g.AddInstruction(exit, Return{.res_reg = Reg(4)});

  EXPECT_THAT(Violation(std::move(g)), Equals(""));
}

TEST(Test, CheckStrictSsaReportsARegisterWrittenTwice) {
  AbstractMachineControlFlowGraphBuilder g;
  auto entry = g.AddBlock();
  g.SetFirst(entry);
  g.SetLast(entry);
  g.AddInstruction(entry, SetReg{.src_val = 1, .dst_reg = Reg(1)});
  g.AddInstruction(
      entry, AddReg{.res_reg = Reg(1), .lhs_reg = Reg(1), .rhs_reg = Reg(1)});

  EXPECT_THAT(Violation(std::move(g)),
              Equals("register 1 is written more than once"));
}

TEST(Test, CheckStrictSsaReportsARegisterNeverWritten) {
  AbstractMachineControlFlowGraphBuilder g;
  auto entry = g.AddBlock();
  g.SetFirst(entry);
  g.SetLast(entry);
  g.AddInstruction(entry, Return{.res_reg = Reg(1)});

  EXPECT_THAT(Violation(std::move(g)),
              Equals("register 1 is read in block 0 but never written"));
}

// Written on one side of a branch and read where the sides meet, which the
// other side reaches without it.
TEST(Test, CheckStrictSsaReportsAWriteOnOneSideOfABranch) {
  AbstractMachineControlFlowGraphBuilder g;
  auto entry = g.AddBlock();
  auto then = g.AddBlock();
  auto join = g.AddBlock();
  g.SetFirst(entry);
  g.SetLast(join);
  g.AddEdge(entry, then);
  g.AddEdge(entry, join);
  g.AddEdge(then, join);

  g.AddInstruction(then, SetReg{.src_val = 1, .dst_reg = Reg(1)});
  g.AddInstruction(join, Return{.res_reg = Reg(1)});

  EXPECT_THAT(Violation(std::move(g)),
              Equals("register 1 is read in block 2 where its write in block 1 "
                     "does not come first on every way there"));
}

// A phi function reads each argument where control leaves the block it comes
// from, so an argument written on the other side does not reach it.
TEST(Test, CheckStrictSsaReportsAPhiArgumentFromTheOtherSide) {
  AbstractMachineControlFlowGraphBuilder g;
  auto entry = g.AddBlock();
  auto left = g.AddBlock();
  auto right = g.AddBlock();
  auto join = g.AddBlock();
  g.SetFirst(entry);
  g.SetLast(join);
  g.AddEdge(entry, left);
  g.AddEdge(entry, right);
  g.AddEdge(left, join);
  g.AddEdge(right, join);

  g.AddInstruction(left, SetReg{.src_val = 1, .dst_reg = Reg(1)});
  g.AddInstruction(right, SetReg{.src_val = 2, .dst_reg = Reg(2)});
  g.AddPhi(join, {.dst = Reg(3), .srcs = {Reg(2), Reg(1)}});
  g.AddInstruction(join, Return{.res_reg = Reg(3)});

  EXPECT_THAT(Violation(std::move(g)),
              Equals("register 2 is read in block 1 where its write in block 2 "
                     "does not come first on every way there"));
}

}  // namespace
}  // namespace lucid
