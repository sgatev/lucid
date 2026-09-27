#include "lucid/am/ig.h"

#include <utility>

#include "lucid/am/cfg.h"
#include "lucid/am/cfg_builder.h"
#include "lucid/am/instructions.h"
#include "lucid/core/testing/testing.h"

namespace lucid {
namespace {

constexpr static Reg kReg1 = {
    .id = 1,
    .size = RegSize32,
};

constexpr static Reg kReg2 = {
    .id = 2,
    .size = RegSize32,
};

constexpr static Reg kReg3 = {
    .id = 3,
    .size = RegSize32,
};

constexpr static Reg kReg4 = {
    .id = 4,
    .size = RegSize32,
};

TEST(Test, BuildInterferenceGraphEmpty) {
  AbstractMachineControlFlowGraphBuilder g;

  auto a = g.AddBlock();
  auto z = g.AddBlock();

  // Block a:
  g.SetFirst(a);
  g.AddEdge(a, z);

  // Block z:
  g.SetLast(z);

  EXPECT_THAT(BuildInterferenceGraph(std::move(g).Build()).Regs(), IsEmpty());
}

TEST(Test, BuildInterferenceGraphSimple) {
  AbstractMachineControlFlowGraphBuilder g;

  auto a = g.AddBlock();
  auto z = g.AddBlock();

  // Block a:
  g.SetFirst(a);
  g.AddInstruction(a, SetReg{
                          .src_val = 1,
                          .dst_reg = kReg1,
                      });
  g.AddEdge(a, z);

  // Block z:
  g.AddInstruction(z, Return{
                          .res_reg = kReg1,
                      });
  g.SetLast(z);

  const InterferenceGraph ig = BuildInterferenceGraph(std::move(g).Build());

  EXPECT_THAT(ig.Regs(), UnorderedElementsEqual(kReg1));
  EXPECT_THAT(ig.Neighbours(kReg1), IsEmpty());
}

TEST(Test, BuildInterferenceGraphNonOverlapping) {
  AbstractMachineControlFlowGraphBuilder g;

  auto a = g.AddBlock();
  auto z = g.AddBlock();

  // Block a:
  g.SetFirst(a);
  g.AddInstruction(a, SetReg{
                          .src_val = 1,
                          .dst_reg = kReg1,
                      });
  g.AddInstruction(a, MoveReg{
                          .src_reg = kReg1,
                          .dst_reg = kReg2,
                      });
  g.AddEdge(a, z);

  // Block z:
  g.AddInstruction(z, Return{
                          .res_reg = kReg2,
                      });
  g.SetLast(z);

  const InterferenceGraph ig = BuildInterferenceGraph(std::move(g).Build());

  EXPECT_THAT(ig.Regs(), UnorderedElementsEqual(kReg1, kReg2));
  EXPECT_THAT(ig.Neighbours(kReg1), IsEmpty());
  EXPECT_THAT(ig.Neighbours(kReg2), IsEmpty());
}

TEST(Test, BuildInterferenceGraphOverlapping) {
  AbstractMachineControlFlowGraphBuilder g;

  auto a = g.AddBlock();
  auto z = g.AddBlock();

  // Block a:
  g.SetFirst(a);
  g.AddInstruction(a, SetReg{
                          .src_val = 1,
                          .dst_reg = kReg1,
                      });
  g.AddInstruction(a, SetReg{
                          .src_val = 2,
                          .dst_reg = kReg2,
                      });
  g.AddInstruction(a, AddReg{
                          .res_reg = kReg3,
                          .lhs_reg = kReg1,
                          .rhs_reg = kReg2,
                      });
  g.AddEdge(a, z);

  // Block z:
  g.AddInstruction(z, Return{
                          .res_reg = kReg3,
                      });
  g.SetLast(z);

  const InterferenceGraph ig = BuildInterferenceGraph(std::move(g).Build());

  EXPECT_THAT(ig.Regs(), UnorderedElementsEqual(kReg1, kReg2, kReg3));
  EXPECT_THAT(ig.Neighbours(kReg1), UnorderedElementsEqual(kReg2));
  EXPECT_THAT(ig.Neighbours(kReg2), UnorderedElementsEqual(kReg1));
  EXPECT_THAT(ig.Neighbours(kReg3), IsEmpty());
}

TEST(Test, BuildInterferenceGraphBranching) {
  AbstractMachineControlFlowGraphBuilder g;

  auto a = g.AddBlock();
  auto b = g.AddBlock();
  auto c = g.AddBlock();
  auto z = g.AddBlock();

  // Block a:
  g.SetFirst(a);
  g.AddInstruction(a, SetReg{
                          .src_val = 1,
                          .dst_reg = kReg1,
                      });
  g.AddInstruction(a, SetReg{
                          .src_val = 2,
                          .dst_reg = kReg2,
                      });
  g.AddEdge(a, b);
  g.AddEdge(a, c);

  // Block b:
  g.AddInstruction(b, MoveReg{
                          .src_reg = kReg1,
                          .dst_reg = kReg3,
                      });
  g.AddEdge(b, z);

  // Block c:
  g.AddInstruction(c, MoveReg{
                          .src_reg = kReg2,
                          .dst_reg = kReg3,
                      });
  g.AddEdge(c, z);

  // Block z:
  g.AddInstruction(z, Return{
                          .res_reg = kReg3,
                      });
  g.SetLast(z);

  const InterferenceGraph ig = BuildInterferenceGraph(std::move(g).Build());

  EXPECT_THAT(ig.Regs(), UnorderedElementsEqual(kReg1, kReg2, kReg3));
  EXPECT_THAT(ig.Neighbours(kReg1), UnorderedElementsEqual(kReg2));
  EXPECT_THAT(ig.Neighbours(kReg2), UnorderedElementsEqual(kReg1));
  EXPECT_THAT(ig.Neighbours(kReg3), IsEmpty());
}

TEST(Test, BuildInterferenceGraphMerging) {
  AbstractMachineControlFlowGraphBuilder g;

  auto a = g.AddBlock();
  auto b = g.AddBlock();
  auto c = g.AddBlock();
  auto z = g.AddBlock();

  // Block a:
  g.SetFirst(a);
  g.AddInstruction(a, SetReg{
                          .src_val = 1,
                          .dst_reg = kReg1,
                      });
  g.AddEdge(a, b);

  // Block b:
  g.AddPhi(b, {
                  .dst = kReg3,
                  .srcs = {kReg1, kReg2},
              });
  g.AddEdge(b, c);
  g.AddEdge(b, z);

  // Block c:
  g.AddInstruction(c, SetReg{
                          .src_val = 2,
                          .dst_reg = kReg2,
                      });
  g.AddEdge(c, b);

  // Block z:
  g.AddInstruction(z, Return{
                          .res_reg = kReg3,
                      });
  g.SetLast(z);

  const InterferenceGraph ig = BuildInterferenceGraph(std::move(g).Build());

  // A phi function's result is in the graph, and is held apart from nothing:
  // neither argument is live where it is, and the one value they stand for
  // is as well off in the register an argument already has.
  EXPECT_THAT(ig.Regs(), UnorderedElementsEqual(kReg1, kReg2, kReg3));
  EXPECT_THAT(ig.Neighbours(kReg1), IsEmpty());
  EXPECT_THAT(ig.Neighbours(kReg2), IsEmpty());
  EXPECT_THAT(ig.Neighbours(kReg3), IsEmpty());
}

// A phi function's result is held apart from what is live where the sides
// meet, as any other value written there would be. Only its own arguments
// are no reason to hold it apart.
TEST(Test, BuildInterferenceGraphMergingBesideALiveValue) {
  AbstractMachineControlFlowGraphBuilder g;

  auto a = g.AddBlock();
  auto b = g.AddBlock();
  auto z = g.AddBlock();

  // Block a: writes what the phi takes, and a value that outlives the merge.
  g.SetFirst(a);
  g.AddInstruction(a, SetReg{
                          .src_val = 1,
                          .dst_reg = kReg1,
                      });
  g.AddInstruction(a, SetReg{
                          .src_val = 2,
                          .dst_reg = kReg2,
                      });
  g.AddEdge(a, b);

  // Block b: the merge, where kReg2 is still live.
  g.AddPhi(b, {
                  .dst = kReg3,
                  .srcs = {kReg1},
              });
  g.AddInstruction(b, AddReg{
                          .res_reg = kReg4,
                          .lhs_reg = kReg3,
                          .rhs_reg = kReg2,
                      });
  g.AddEdge(b, z);

  // Block z:
  g.AddInstruction(z, Return{
                          .res_reg = kReg4,
                      });
  g.SetLast(z);

  const InterferenceGraph ig = BuildInterferenceGraph(std::move(g).Build());

  // The result is held apart from the value that outlives the merge, and the
  // argument from that same value, because each of those two pairs is live at
  // once. The result and the argument are not in either list.
  EXPECT_THAT(ig.Neighbours(kReg3), UnorderedElementsEqual(kReg2));
  EXPECT_THAT(ig.Neighbours(kReg1), UnorderedElementsEqual(kReg2));
}

}  // namespace
}  // namespace lucid
