#include "lucid/am/liveness.h"

#include <optional>
#include <utility>
#include <vector>

#include "lucid/am/cfg.h"
#include "lucid/am/cfg_builder.h"
#include "lucid/am/instructions.h"
#include "lucid/core/container/hash_set.h"
#include "lucid/core/dataflow/dataflow.h"
#include "lucid/core/testing/testing.h"

namespace lucid {
namespace {

class LivenessAnalysisGraphBuilder
    : public AbstractMachineControlFlowGraphBuilder {
 public:
  // What the analysis settled on, along with the graph it ran over: what is
  // live where a block exits is read back off the blocks it runs into, so the
  // graph has to outlive the run.
  struct Result {
    AbstractMachineControlFlowGraph am_cfg;
    std::vector<std::optional<AbstractMachineLivenessAnalysis::State>> states;

    const HashSet<Reg>& live_in(
        AbstractMachineControlFlowGraph::BlockRef ref) const {
      return states[ref.id()]->live_in;
    }

    HashSet<Reg> live_out(AbstractMachineControlFlowGraph::BlockRef ref) const {
      return LiveOut(am_cfg, states, am_cfg.GetBlock(ref));
    }
  };

  Result run() && {
    AbstractMachineControlFlowGraph am_cfg = std::move(*this).Build();
    AbstractMachineLivenessAnalysis analysis(am_cfg);
    auto states = RunDataflow(Backward(am_cfg), analysis);
    return Result{std::move(am_cfg), std::move(states)};
  }
};

TEST(Test, AbstractMachineLivenessAnalysisTwoBlocks) {
  LivenessAnalysisGraphBuilder g;

  auto a = g.AddBlock();
  auto z = g.AddBlock();

  g.SetFirst(a);
  g.AddInstruction(a, SetReg{
                          .src_val = 1,
                          .dst_reg = Reg(1),
                      });
  g.AddEdge(a, z);

  g.SetLast(z);

  LivenessAnalysisGraphBuilder::Result result = std::move(g).run();
  ASSERT_THAT(result.states, SizeIs(2));

  EXPECT_THAT(result.live_in(a), IsEmpty());
  EXPECT_THAT(result.live_out(a), IsEmpty());

  EXPECT_THAT(result.live_in(z), IsEmpty());
  EXPECT_THAT(result.live_out(z), IsEmpty());
}

TEST(Test, AbstractMachineLivenessAnalysisUseInMiddleBlock) {
  LivenessAnalysisGraphBuilder g;

  auto a = g.AddBlock();
  auto b = g.AddBlock();
  auto z = g.AddBlock();

  g.SetFirst(a);
  g.AddInstruction(a, SetReg{
                          .src_val = 1,
                          .dst_reg = Reg(1, RegSize32),
                      });
  g.AddEdge(a, b);

  g.AddInstruction(b, MoveReg{
                          .src_reg = Reg(1, RegSize32),
                          .dst_reg = Reg(2, RegSize32),
                      });
  g.AddEdge(b, z);

  g.SetLast(z);

  LivenessAnalysisGraphBuilder::Result result = std::move(g).run();
  ASSERT_THAT(result.states, SizeIs(3));

  EXPECT_THAT(result.live_in(a), IsEmpty());
  EXPECT_THAT(result.live_out(a), UnorderedElementsEqual(Reg(1)));

  EXPECT_THAT(result.live_in(b), UnorderedElementsEqual(Reg(1)));
  EXPECT_THAT(result.live_out(b), IsEmpty());

  EXPECT_THAT(result.live_in(z), IsEmpty());
  EXPECT_THAT(result.live_out(z), IsEmpty());
}

TEST(Test, AbstractMachineLivenessAnalysisDiamondWithFollowUse) {
  LivenessAnalysisGraphBuilder g;

  auto a = g.AddBlock();
  auto b = g.AddBlock();
  auto c = g.AddBlock();
  auto d = g.AddBlock();
  auto z = g.AddBlock();

  g.SetFirst(a);
  g.AddInstruction(a, SetReg{
                          .src_val = 1,
                          .dst_reg = Reg(1),
                      });
  g.AddInstruction(a, SetReg{
                          .src_val = 2,
                          .dst_reg = Reg(2),
                      });
  g.AddEdge(a, b);
  g.AddEdge(a, c);

  g.AddInstruction(b, MoveReg{
                          .src_reg = Reg(1, RegSize32),
                          .dst_reg = Reg(3, RegSize32),
                      });
  g.AddEdge(b, d);

  g.AddInstruction(c, MoveReg{
                          .src_reg = Reg(2, RegSize32),
                          .dst_reg = Reg(3, RegSize32),
                      });
  g.AddEdge(c, d);

  g.AddInstruction(d, MoveReg{
                          .src_reg = Reg(3, RegSize32),
                          .dst_reg = Reg(4, RegSize32),
                      });
  g.AddEdge(d, z);

  g.SetLast(z);

  LivenessAnalysisGraphBuilder::Result result = std::move(g).run();
  ASSERT_THAT(result.states, SizeIs(5));

  EXPECT_THAT(result.live_in(a), IsEmpty());
  EXPECT_THAT(result.live_out(a), UnorderedElementsEqual(Reg(1), Reg(2)));

  EXPECT_THAT(result.live_in(b), UnorderedElementsEqual(Reg(1)));
  EXPECT_THAT(result.live_out(b), UnorderedElementsEqual(Reg(3)));

  EXPECT_THAT(result.live_in(c), UnorderedElementsEqual(Reg(2)));
  EXPECT_THAT(result.live_out(c), UnorderedElementsEqual(Reg(3)));

  EXPECT_THAT(result.live_in(d), UnorderedElementsEqual(Reg(3)));
  EXPECT_THAT(result.live_out(d), IsEmpty());

  EXPECT_THAT(result.live_in(z), IsEmpty());
  EXPECT_THAT(result.live_out(z), IsEmpty());
}

TEST(Test, AbstractMachineLivenessAnalysisIntraBlockUse) {
  LivenessAnalysisGraphBuilder g;

  auto a = g.AddBlock();
  auto b = g.AddBlock();
  auto z = g.AddBlock();

  g.SetFirst(a);
  g.AddInstruction(a, SetReg{
                          .src_val = 1,
                          .dst_reg = Reg(1),
                      });
  g.AddInstruction(a, MoveReg{
                          .src_reg = Reg(1, RegSize32),
                          .dst_reg = Reg(2, RegSize32),
                      });
  g.AddEdge(a, b);

  g.AddInstruction(b, MoveReg{
                          .src_reg = Reg(2, RegSize32),
                          .dst_reg = Reg(3, RegSize32),
                      });
  g.AddEdge(b, z);

  g.SetLast(z);

  LivenessAnalysisGraphBuilder::Result result = std::move(g).run();
  ASSERT_THAT(result.states, SizeIs(3));

  EXPECT_THAT(result.live_in(a), IsEmpty());
  EXPECT_THAT(result.live_out(a), UnorderedElementsEqual(Reg(2)));

  EXPECT_THAT(result.live_in(b), UnorderedElementsEqual(Reg(2)));
  EXPECT_THAT(result.live_out(b), IsEmpty());

  EXPECT_THAT(result.live_in(z), IsEmpty());
  EXPECT_THAT(result.live_out(z), IsEmpty());
}

TEST(Test, AbstractMachineLivenessAnalysisSkipBlockUse) {
  LivenessAnalysisGraphBuilder g;

  auto a = g.AddBlock();
  auto b = g.AddBlock();
  auto c = g.AddBlock();
  auto z = g.AddBlock();

  g.SetFirst(a);
  g.AddInstruction(a, SetReg{
                          .src_val = 1,
                          .dst_reg = Reg(1),
                      });
  g.AddEdge(a, b);

  g.AddInstruction(b, SetReg{
                          .src_val = 2,
                          .dst_reg = Reg(2),
                      });
  g.AddEdge(b, c);

  g.AddInstruction(c, AddReg{
                          .res_reg = Reg(3),
                          .lhs_reg = Reg(1),
                          .rhs_reg = Reg(2),
                      });
  g.AddEdge(c, z);

  g.SetLast(z);

  LivenessAnalysisGraphBuilder::Result result = std::move(g).run();
  ASSERT_THAT(result.states, SizeIs(4));

  EXPECT_THAT(result.live_in(a), IsEmpty());
  EXPECT_THAT(result.live_out(a), UnorderedElementsEqual(Reg(1)));

  EXPECT_THAT(result.live_in(b), UnorderedElementsEqual(Reg(1)));
  EXPECT_THAT(result.live_out(b), UnorderedElementsEqual(Reg(1), Reg(2)));

  EXPECT_THAT(result.live_in(c), UnorderedElementsEqual(Reg(1), Reg(2)));
  EXPECT_THAT(result.live_out(c), IsEmpty());

  EXPECT_THAT(result.live_in(z), IsEmpty());
  EXPECT_THAT(result.live_out(z), IsEmpty());
}

}  // namespace
}  // namespace lucid
