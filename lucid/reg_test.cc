#include "lucid/reg.h"

#include "gmock/gmock.h"
#include "gtest/gtest.h"
#include "lucid/am.h"
#include "lucid/am_cfg.h"

namespace lucid {
namespace {

using ::testing::IsEmpty;
using ::testing::Pair;

TEST(BuildInterferenceGraphTest, Simple) {
  std::vector<Instruction> instructions;
  auto am_cfg = BuildAbstractMachineControlFlowGraph(instructions);

  auto am_ig = BuildInterferenceGraph(am_cfg);

  EXPECT_THAT(am_ig, IsEmpty());
}

}  // namespace
}  // namespace lucid
