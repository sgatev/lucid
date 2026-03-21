#include "lucid/am_liveness.h"

#include <optional>
#include <vector>

#include "gmock/gmock.h"
#include "gtest/gtest.h"
#include "lucid/am_cfg.h"
#include "lucid/ast.h"
#include "lucid/dataflow.h"

namespace lucid {
namespace {

using ::testing::SizeIs;

TEST(AbstractMachineLivenessAnalysisTest, Simple) {
  SyntaxContext ctx;
  std::vector<Instruction> instructions;
  auto am_cfg = BuildAbstractMachineControlFlowGraph(instructions);

  AbstractMachineLivenessAnalysis analysis(ctx);
  std::vector<std::optional<AbstractMachineLivenessAnalysis::State>>
      block_states = RunBackwardDataflow(am_cfg, analysis);

  ASSERT_THAT(block_states, SizeIs(2));
}

}  // namespace
}  // namespace lucid
