#include "lucid/am/ig.h"

#include <utility>

#include "gmock/gmock.h"
#include "gtest/gtest.h"
#include "lucid/am/cfg.h"
#include "lucid/am/cfg_builder.h"
#include "lucid/am/instructions.h"

namespace lucid {
namespace {

using ::testing::IsEmpty;
using ::testing::Pair;
using ::testing::UnorderedElementsAre;

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

TEST(BuildInterferenceGraphTest, Empty) {
  AbstractMachineControlFlowGraphBuilder g;

  auto a = g.AddBlock();
  auto z = g.AddBlock();

  // Block a:
  g.SetFirst(a);
  g.AddEdge(a, z);

  // Block z:
  g.SetLast(z);

  EXPECT_THAT(BuildInterferenceGraph(std::move(g).Build()), IsEmpty());
}

TEST(BuildInterferenceGraphTest, Simple) {
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

  EXPECT_THAT(BuildInterferenceGraph(std::move(g).Build()),
              UnorderedElementsAre(Pair(kReg1, IsEmpty())));
}

TEST(BuildInterferenceGraphTest, NonOverlapping) {
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

  EXPECT_THAT(
      BuildInterferenceGraph(std::move(g).Build()),
      UnorderedElementsAre(Pair(kReg1, IsEmpty()), Pair(kReg2, IsEmpty())));
}

TEST(BuildInterferenceGraphTest, Overlapping) {
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

  EXPECT_THAT(BuildInterferenceGraph(std::move(g).Build()),
              UnorderedElementsAre(Pair(kReg1, UnorderedElementsAre(kReg2)),
                                   Pair(kReg2, UnorderedElementsAre(kReg1)),
                                   Pair(kReg3, IsEmpty())));
}

TEST(BuildInterferenceGraphTest, Branching) {
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

  EXPECT_THAT(BuildInterferenceGraph(std::move(g).Build()),
              UnorderedElementsAre(Pair(kReg1, UnorderedElementsAre(kReg2)),
                                   Pair(kReg2, UnorderedElementsAre(kReg1)),
                                   Pair(kReg3, IsEmpty())));
}

TEST(BuildInterferenceGraphTest, Merging) {
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

  EXPECT_THAT(
      BuildInterferenceGraph(std::move(g).Build()),
      UnorderedElementsAre(Pair(kReg1, UnorderedElementsAre(kReg3)),
                           Pair(kReg2, UnorderedElementsAre(kReg3)),
                           Pair(kReg3, UnorderedElementsAre(kReg1, kReg2))));
}

}  // namespace
}  // namespace lucid
