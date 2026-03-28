#include "lucid/am_liveness.h"

#include <optional>
#include <vector>

#include "gmock/gmock.h"
#include "gtest/gtest.h"
#include "lucid/am_cfg.h"
#include "lucid/core/dataflow/dataflow.h"

namespace lucid {
namespace {

using ::testing::SizeIs;

TEST(AbstractMachineLivenessAnalysisTest, Simple) {
  AbstractMachineControlFlowGraph am_cfg;
  am_cfg.first = am_cfg.add().ref;
  am_cfg.last = am_cfg.add().ref;

  AbstractMachineLivenessAnalysis analysis(am_cfg);
  std::vector<std::optional<AbstractMachineLivenessAnalysis::State>>
      block_states = RunBackwardDataflow(am_cfg, analysis);

  ASSERT_THAT(block_states, SizeIs(2));
}

}  // namespace
}  // namespace lucid
