#include "lucid/am/ig.h"

#include "gmock/gmock.h"
#include "gtest/gtest.h"
#include "lucid/am/cfg.h"
#include "lucid/am/instructions.h"

namespace lucid {
namespace {

using ::testing::IsEmpty;
using ::testing::Pair;
using ::testing::UnorderedElementsAre;

constexpr static RegId kReg1 = {
    .id = 1,
    .size = RegSize32,
};

constexpr static RegId kReg2 = {
    .id = 2,
    .size = RegSize32,
};

constexpr static RegId kReg3 = {
    .id = 3,
    .size = RegSize32,
};

TEST(BuildInterferenceGraphTest, Empty) {
  AbstractMachineControlFlowGraph am_cfg;

  auto a = am_cfg.add().ref;
  auto z = am_cfg.add().ref;

  // Block a:
  am_cfg.first = a;
  am_cfg.edge(a, z);

  // Block z:
  am_cfg.last = z;

  auto am_ig = BuildInterferenceGraph(am_cfg);

  EXPECT_THAT(am_ig, IsEmpty());
}

TEST(BuildInterferenceGraphTest, Simple) {
  AbstractMachineControlFlowGraph am_cfg;

  auto a = am_cfg.add().ref;
  auto z = am_cfg.add().ref;

  // Block a:
  am_cfg.first = a;
  am_cfg.get(a).instructions.push_back(SetReg{
      .src_val = "1",
      .dst_reg = kReg1,
  });
  am_cfg.edge(a, z);

  // Block z:
  am_cfg.get(z).instructions.push_back(Return{
      .res_reg = kReg1,
  });
  am_cfg.last = z;

  auto am_ig = BuildInterferenceGraph(am_cfg);

  EXPECT_THAT(am_ig, UnorderedElementsAre(Pair(kReg1, IsEmpty())));
}

TEST(BuildInterferenceGraphTest, NonOverlapping) {
  AbstractMachineControlFlowGraph am_cfg;

  auto a = am_cfg.add().ref;
  auto z = am_cfg.add().ref;

  // Block a:
  am_cfg.first = a;
  am_cfg.get(a).instructions.push_back(SetReg{
      .src_val = "1",
      .dst_reg = kReg1,
  });
  am_cfg.get(a).instructions.push_back(MoveReg{
      .src_reg = kReg1,
      .dst_reg = kReg2,
  });
  am_cfg.edge(a, z);

  // Block z:
  am_cfg.get(z).instructions.push_back(Return{
      .res_reg = kReg2,
  });
  am_cfg.last = z;

  auto am_ig = BuildInterferenceGraph(am_cfg);

  EXPECT_THAT(am_ig, UnorderedElementsAre(Pair(kReg1, IsEmpty()),
                                          Pair(kReg2, IsEmpty())));
}

TEST(BuildInterferenceGraphTest, Overlapping) {
  AbstractMachineControlFlowGraph am_cfg;

  auto a = am_cfg.add().ref;
  auto z = am_cfg.add().ref;

  // Block a:
  am_cfg.first = a;
  am_cfg.get(a).instructions.push_back(SetReg{
      .src_val = "1",
      .dst_reg = kReg1,
  });
  am_cfg.get(a).instructions.push_back(SetReg{
      .src_val = "2",
      .dst_reg = kReg2,
  });
  am_cfg.get(a).instructions.push_back(AddReg{
      .res_reg = kReg3,
      .lhs_reg = kReg1,
      .rhs_reg = kReg2,
  });
  am_cfg.edge(a, z);

  // Block z:
  am_cfg.get(z).instructions.push_back(Return{
      .res_reg = kReg3,
  });
  am_cfg.last = z;

  auto am_ig = BuildInterferenceGraph(am_cfg);

  EXPECT_THAT(am_ig,
              UnorderedElementsAre(Pair(kReg1, UnorderedElementsAre(kReg2)),
                                   Pair(kReg2, UnorderedElementsAre(kReg1)),
                                   Pair(kReg3, IsEmpty())));
}

TEST(BuildInterferenceGraphTest, BRanching) {
  AbstractMachineControlFlowGraph am_cfg;

  auto a = am_cfg.add().ref;
  auto b = am_cfg.add().ref;
  auto c = am_cfg.add().ref;
  auto z = am_cfg.add().ref;

  // Block a:
  am_cfg.first = a;
  am_cfg.get(a).instructions.push_back(SetReg{
      .src_val = "1",
      .dst_reg = kReg1,
  });
  am_cfg.get(a).instructions.push_back(SetReg{
      .src_val = "2",
      .dst_reg = kReg2,
  });
  am_cfg.edge(a, b);
  am_cfg.edge(a, c);

  // Block b:
  am_cfg.get(b).instructions.push_back(MoveReg{
      .src_reg = kReg1,
      .dst_reg = kReg3,
  });
  am_cfg.edge(b, z);

  // Block c:
  am_cfg.get(c).instructions.push_back(MoveReg{
      .src_reg = kReg2,
      .dst_reg = kReg3,
  });
  am_cfg.edge(b, z);

  // Block z:
  am_cfg.get(z).instructions.push_back(Return{
      .res_reg = kReg3,
  });
  am_cfg.last = z;

  auto am_ig = BuildInterferenceGraph(am_cfg);

  EXPECT_THAT(am_ig,
              UnorderedElementsAre(Pair(kReg1, UnorderedElementsAre(kReg2)),
                                   Pair(kReg2, UnorderedElementsAre(kReg1)),
                                   Pair(kReg3, IsEmpty())));
}

}  // namespace
}  // namespace lucid
