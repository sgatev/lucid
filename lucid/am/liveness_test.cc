#include "lucid/am/liveness.h"

#include <optional>
#include <utility>
#include <vector>

#include "gmock/gmock.h"
#include "gtest/gtest.h"
#include "lucid/am/cfg.h"
#include "lucid/am/cfg_builder.h"
#include "lucid/am/instructions.h"
#include "lucid/core/dataflow/dataflow.h"

namespace lucid {
namespace {

using ::testing::IsEmpty;
using ::testing::SizeIs;
using ::testing::UnorderedElementsAre;

class LivenessAnalysisGraphBuilder
    : public AbstractMachineControlFlowGraphBuilder {
 public:
  std::vector<std::optional<AbstractMachineLivenessAnalysis::State>> run() && {
    AbstractMachineControlFlowGraph am_cfg = std::move(*this).build();
    AbstractMachineLivenessAnalysis analysis(am_cfg);
    return RunDataflow(Backward(am_cfg), analysis);
  }
};

TEST(AbstractMachineLivenessAnalysisTest, TwoBlocks) {
  LivenessAnalysisGraphBuilder g;

  auto a = g.block();
  auto z = g.block();

  g.first(a);
  g.inst(a, SetReg{
                .src_val = "1",
                .dst_reg = Reg(1),
            });
  g.edge(a, z);

  g.last(z);

  std::vector<std::optional<AbstractMachineLivenessAnalysis::State>>
      block_states = std::move(g).run();
  ASSERT_THAT(block_states, SizeIs(2));

  EXPECT_THAT(block_states[a.id()]->live_in, IsEmpty());
  EXPECT_THAT(block_states[a.id()]->live_out, IsEmpty());

  EXPECT_THAT(block_states[z.id()]->live_in, IsEmpty());
  EXPECT_THAT(block_states[z.id()]->live_out, IsEmpty());
}

TEST(AbstractMachineLivenessAnalysisTest, UseInMiddleBlock) {
  LivenessAnalysisGraphBuilder g;

  auto a = g.block();
  auto b = g.block();
  auto z = g.block();

  g.first(a);
  g.inst(a, SetReg{
                .src_val = "1",
                .dst_reg = Reg(1, RegSize32),
            });
  g.edge(a, b);

  g.inst(b, MoveReg{
                .src_reg = Reg(1, RegSize32),
                .dst_reg = Reg(2, RegSize32),
            });
  g.edge(b, z);

  g.last(z);

  std::vector<std::optional<AbstractMachineLivenessAnalysis::State>>
      block_states = std::move(g).run();
  ASSERT_THAT(block_states, SizeIs(3));

  EXPECT_THAT(block_states[a.id()]->live_in, IsEmpty());
  EXPECT_THAT(block_states[a.id()]->live_out, UnorderedElementsAre(Reg(1)));

  EXPECT_THAT(block_states[b.id()]->live_in, UnorderedElementsAre(Reg(1)));
  EXPECT_THAT(block_states[b.id()]->live_out, IsEmpty());

  EXPECT_THAT(block_states[z.id()]->live_in, IsEmpty());
  EXPECT_THAT(block_states[z.id()]->live_out, IsEmpty());
}

TEST(AbstractMachineLivenessAnalysisTest, DiamondWithFollowUse) {
  LivenessAnalysisGraphBuilder g;

  auto a = g.block();
  auto b = g.block();
  auto c = g.block();
  auto d = g.block();
  auto z = g.block();

  g.first(a);
  g.inst(a, SetReg{
                .src_val = "1",
                .dst_reg = Reg(1),
            });
  g.inst(a, SetReg{
                .src_val = "2",
                .dst_reg = Reg(2),
            });
  g.edge(a, b);
  g.edge(a, c);

  g.inst(b, MoveReg{
                .src_reg = Reg(1, RegSize32),
                .dst_reg = Reg(3, RegSize32),
            });
  g.edge(b, d);

  g.inst(c, MoveReg{
                .src_reg = Reg(2, RegSize32),
                .dst_reg = Reg(3, RegSize32),
            });
  g.edge(c, d);

  g.inst(d, MoveReg{
                .src_reg = Reg(3, RegSize32),
                .dst_reg = Reg(4, RegSize32),
            });
  g.edge(d, z);

  g.last(z);

  std::vector<std::optional<AbstractMachineLivenessAnalysis::State>>
      block_states = std::move(g).run();
  ASSERT_THAT(block_states, SizeIs(5));

  EXPECT_THAT(block_states[a.id()]->live_in, IsEmpty());
  EXPECT_THAT(block_states[a.id()]->live_out,
              UnorderedElementsAre(Reg(1), Reg(2)));

  EXPECT_THAT(block_states[b.id()]->live_in, UnorderedElementsAre(Reg(1)));
  EXPECT_THAT(block_states[b.id()]->live_out, UnorderedElementsAre(Reg(3)));

  EXPECT_THAT(block_states[c.id()]->live_in, UnorderedElementsAre(Reg(2)));
  EXPECT_THAT(block_states[c.id()]->live_out, UnorderedElementsAre(Reg(3)));

  EXPECT_THAT(block_states[d.id()]->live_in, UnorderedElementsAre(Reg(3)));
  EXPECT_THAT(block_states[d.id()]->live_out, IsEmpty());

  EXPECT_THAT(block_states[z.id()]->live_in, IsEmpty());
  EXPECT_THAT(block_states[z.id()]->live_out, IsEmpty());
}

TEST(AbstractMachineLivenessAnalysisTest, IntraBlockUse) {
  LivenessAnalysisGraphBuilder g;

  auto a = g.block();
  auto b = g.block();
  auto z = g.block();

  g.first(a);
  g.inst(a, SetReg{
                .src_val = "1",
                .dst_reg = Reg(1),
            });
  g.inst(a, MoveReg{
                .src_reg = Reg(1, RegSize32),
                .dst_reg = Reg(2, RegSize32),
            });
  g.edge(a, b);

  g.inst(b, MoveReg{
                .src_reg = Reg(2, RegSize32),
                .dst_reg = Reg(3, RegSize32),
            });
  g.edge(b, z);

  g.last(z);

  std::vector<std::optional<AbstractMachineLivenessAnalysis::State>>
      block_states = std::move(g).run();
  ASSERT_THAT(block_states, SizeIs(3));

  EXPECT_THAT(block_states[a.id()]->live_in, IsEmpty());
  EXPECT_THAT(block_states[a.id()]->live_out, UnorderedElementsAre(Reg(2)));

  EXPECT_THAT(block_states[b.id()]->live_in, UnorderedElementsAre(Reg(2)));
  EXPECT_THAT(block_states[b.id()]->live_out, IsEmpty());

  EXPECT_THAT(block_states[z.id()]->live_in, IsEmpty());
  EXPECT_THAT(block_states[z.id()]->live_out, IsEmpty());
}

TEST(AbstractMachineLivenessAnalysisTest, SkipBlockUse) {
  LivenessAnalysisGraphBuilder g;

  auto a = g.block();
  auto b = g.block();
  auto c = g.block();
  auto z = g.block();

  g.first(a);
  g.inst(a, SetReg{
                .src_val = "1",
                .dst_reg = Reg(1),
            });
  g.edge(a, b);

  g.inst(b, SetReg{
                .src_val = "2",
                .dst_reg = Reg(2),
            });
  g.edge(b, c);

  g.inst(c, AddReg{
                .res_reg = Reg(3),
                .lhs_reg = Reg(1),
                .rhs_reg = Reg(2),
            });
  g.edge(c, z);

  g.last(z);

  std::vector<std::optional<AbstractMachineLivenessAnalysis::State>>
      block_states = std::move(g).run();
  ASSERT_THAT(block_states, SizeIs(4));

  EXPECT_THAT(block_states[a.id()]->live_in, IsEmpty());
  EXPECT_THAT(block_states[a.id()]->live_out, UnorderedElementsAre(Reg(1)));

  EXPECT_THAT(block_states[b.id()]->live_in, UnorderedElementsAre(Reg(1)));
  EXPECT_THAT(block_states[b.id()]->live_out,
              UnorderedElementsAre(Reg(1), Reg(2)));

  EXPECT_THAT(block_states[c.id()]->live_in,
              UnorderedElementsAre(Reg(1), Reg(2)));
  EXPECT_THAT(block_states[c.id()]->live_out, IsEmpty());

  EXPECT_THAT(block_states[z.id()]->live_in, IsEmpty());
  EXPECT_THAT(block_states[z.id()]->live_out, IsEmpty());
}

}  // namespace
}  // namespace lucid
