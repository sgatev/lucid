#include "lucid/am/reg.h"

#include "gmock/gmock.h"
#include "gtest/gtest.h"
#include "lucid/am/cfg.h"

namespace lucid {
namespace {

using ::testing::IsEmpty;
using ::testing::Pair;

TEST(BuildInterferenceGraphTest, Simple) {
  AbstractMachineControlFlowGraph am_cfg;
  am_cfg.first = am_cfg.add().ref;
  am_cfg.last = am_cfg.add().ref;

  auto am_ig = BuildInterferenceGraph(am_cfg);

  EXPECT_THAT(am_ig, IsEmpty());
}

}  // namespace
}  // namespace lucid
