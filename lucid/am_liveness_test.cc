#include "lucid/am_liveness.h"

#include <optional>
#include <utility>
#include <vector>

#include "gmock/gmock.h"
#include "gtest/gtest.h"
#include "lucid/am/instructions.h"
#include "lucid/am_cfg.h"
#include "lucid/core/dataflow/dataflow.h"

namespace lucid {
namespace {

using ::testing::IsEmpty;
using ::testing::SizeIs;
using ::testing::UnorderedElementsAre;

class GraphBuilder {
 public:
  AbstractMachineControlFlowGraph::BlockRef block() { return am_cfg.add().ref; }

  void edge(AbstractMachineControlFlowGraph::BlockRef from,
            AbstractMachineControlFlowGraph::BlockRef to) {
    am_cfg.get(from).next.Insert(to);
    am_cfg.get(to).preds.Insert(from);
  }

  void inst(AbstractMachineControlFlowGraph::BlockRef ref, Instruction inst) {
    am_cfg.get(ref).instructions.push_back(std::move(inst));
  }

  void first(AbstractMachineControlFlowGraph::BlockRef ref) {
    am_cfg.first = ref;
  }

  void last(AbstractMachineControlFlowGraph::BlockRef ref) {
    am_cfg.last = ref;
  }

  std::vector<std::optional<AbstractMachineLivenessAnalysis::State>>
  build() && {
    AbstractMachineLivenessAnalysis analysis(am_cfg);
    return RunBackwardDataflow(am_cfg, analysis);
  }

 private:
  AbstractMachineControlFlowGraph am_cfg;
};

TEST(AbstractMachineLivenessAnalysisTest, TwoBlocks) {
  GraphBuilder g;

  auto a = g.block();
  auto z = g.block();

  g.first(a);
  g.inst(a, SetReg32{
                .src_val = "1",
                .dst_reg = RegId(1),
            });
  g.edge(a, z);

  g.last(z);

  std::vector<std::optional<AbstractMachineLivenessAnalysis::State>>
      block_states = std::move(g).build();
  ASSERT_THAT(block_states, SizeIs(2));

  EXPECT_THAT(block_states[a.id()]->live_in, IsEmpty());
  EXPECT_THAT(block_states[a.id()]->live_out, IsEmpty());

  EXPECT_THAT(block_states[z.id()]->live_in, IsEmpty());
  EXPECT_THAT(block_states[z.id()]->live_out, IsEmpty());
}

TEST(AbstractMachineLivenessAnalysisTest, UseInMiddleBlock) {
  GraphBuilder g;

  auto a = g.block();
  auto b = g.block();
  auto z = g.block();

  g.first(a);
  g.inst(a, SetReg32{
                .src_val = "1",
                .dst_reg = RegId(1),
            });
  g.edge(a, b);

  g.inst(b, MoveReg32{
                .src_reg = RegId(1),
                .dst_reg = RegId(2),
            });
  g.edge(b, z);

  g.last(z);

  std::vector<std::optional<AbstractMachineLivenessAnalysis::State>>
      block_states = std::move(g).build();
  ASSERT_THAT(block_states, SizeIs(3));

  EXPECT_THAT(block_states[a.id()]->live_in, IsEmpty());
  EXPECT_THAT(block_states[a.id()]->live_out, UnorderedElementsAre(RegId(1)));

  EXPECT_THAT(block_states[b.id()]->live_in, UnorderedElementsAre(RegId(1)));
  EXPECT_THAT(block_states[b.id()]->live_out, IsEmpty());

  EXPECT_THAT(block_states[z.id()]->live_in, IsEmpty());
  EXPECT_THAT(block_states[z.id()]->live_out, IsEmpty());
}

TEST(AbstractMachineLivenessAnalysisTest, DiamondWithFollowUse) {
  GraphBuilder g;

  auto a = g.block();
  auto b = g.block();
  auto c = g.block();
  auto d = g.block();
  auto z = g.block();

  g.first(a);
  g.inst(a, SetReg32{
                .src_val = "1",
                .dst_reg = RegId(1),
            });
  g.inst(a, SetReg32{
                .src_val = "2",
                .dst_reg = RegId(2),
            });
  g.edge(a, b);
  g.edge(a, c);

  g.inst(b, MoveReg32{
                .src_reg = RegId(1),
                .dst_reg = RegId(3),
            });
  g.edge(b, d);

  g.inst(c, MoveReg32{
                .src_reg = RegId(2),
                .dst_reg = RegId(3),
            });
  g.edge(c, d);

  g.inst(d, MoveReg32{
                .src_reg = RegId(3),
                .dst_reg = RegId(4),
            });
  g.edge(d, z);

  g.last(z);

  std::vector<std::optional<AbstractMachineLivenessAnalysis::State>>
      block_states = std::move(g).build();
  ASSERT_THAT(block_states, SizeIs(5));

  EXPECT_THAT(block_states[a.id()]->live_in, IsEmpty());
  EXPECT_THAT(block_states[a.id()]->live_out,
              UnorderedElementsAre(RegId(1), RegId(2)));

  EXPECT_THAT(block_states[b.id()]->live_in, UnorderedElementsAre(RegId(1)));
  EXPECT_THAT(block_states[b.id()]->live_out, UnorderedElementsAre(RegId(3)));

  EXPECT_THAT(block_states[c.id()]->live_in, UnorderedElementsAre(RegId(2)));
  EXPECT_THAT(block_states[c.id()]->live_out, UnorderedElementsAre(RegId(3)));

  EXPECT_THAT(block_states[d.id()]->live_in, UnorderedElementsAre(RegId(3)));
  EXPECT_THAT(block_states[d.id()]->live_out, IsEmpty());

  EXPECT_THAT(block_states[z.id()]->live_in, IsEmpty());
  EXPECT_THAT(block_states[z.id()]->live_out, IsEmpty());
}

TEST(AbstractMachineLivenessAnalysisTest, IntraBlockUse) {
  GraphBuilder g;

  auto a = g.block();
  auto b = g.block();
  auto z = g.block();

  g.first(a);
  g.inst(a, SetReg32{
                .src_val = "1",
                .dst_reg = RegId(1),
            });
  g.inst(a, MoveReg32{
                .src_reg = RegId(1),
                .dst_reg = RegId(2),
            });
  g.edge(a, b);

  g.inst(b, MoveReg32{
                .src_reg = RegId(2),
                .dst_reg = RegId(3),
            });
  g.edge(b, z);

  g.last(z);

  std::vector<std::optional<AbstractMachineLivenessAnalysis::State>>
      block_states = std::move(g).build();
  ASSERT_THAT(block_states, SizeIs(3));

  EXPECT_THAT(block_states[a.id()]->live_in, IsEmpty());
  EXPECT_THAT(block_states[a.id()]->live_out, UnorderedElementsAre(RegId(2)));

  EXPECT_THAT(block_states[b.id()]->live_in, UnorderedElementsAre(RegId(2)));
  EXPECT_THAT(block_states[b.id()]->live_out, IsEmpty());

  EXPECT_THAT(block_states[z.id()]->live_in, IsEmpty());
  EXPECT_THAT(block_states[z.id()]->live_out, IsEmpty());
}

TEST(AbstractMachineLivenessAnalysisTest, SkipBlockUse) {
  GraphBuilder g;

  auto a = g.block();
  auto b = g.block();
  auto c = g.block();
  auto z = g.block();

  g.first(a);
  g.inst(a, SetReg32{
                .src_val = "1",
                .dst_reg = RegId(1),
            });
  g.edge(a, b);

  g.inst(b, SetReg32{
                .src_val = "2",
                .dst_reg = RegId(2),
            });
  g.edge(b, c);

  g.inst(c, AddReg32{
                .res_reg = RegId(3),
                .lhs_reg = RegId(1),
                .rhs_reg = RegId(2),
            });
  g.edge(c, z);

  g.last(z);

  std::vector<std::optional<AbstractMachineLivenessAnalysis::State>>
      block_states = std::move(g).build();
  ASSERT_THAT(block_states, SizeIs(4));

  EXPECT_THAT(block_states[a.id()]->live_in, IsEmpty());
  EXPECT_THAT(block_states[a.id()]->live_out, UnorderedElementsAre(RegId(1)));

  EXPECT_THAT(block_states[b.id()]->live_in, UnorderedElementsAre(RegId(1)));
  EXPECT_THAT(block_states[b.id()]->live_out,
              UnorderedElementsAre(RegId(1), RegId(2)));

  EXPECT_THAT(block_states[c.id()]->live_in,
              UnorderedElementsAre(RegId(1), RegId(2)));
  EXPECT_THAT(block_states[c.id()]->live_out, IsEmpty());

  EXPECT_THAT(block_states[z.id()]->live_in, IsEmpty());
  EXPECT_THAT(block_states[z.id()]->live_out, IsEmpty());
}

}  // namespace
}  // namespace lucid
