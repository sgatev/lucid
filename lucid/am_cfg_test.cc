#include "lucid/am_cfg.h"

#include <vector>

#include "gmock/gmock.h"
#include "gtest/gtest.h"
#include "lucid/am.h"

namespace lucid {
namespace {

using ::testing::IsEmpty;
using ::testing::UnorderedElementsAre;

using BlockRef = AbstractMachineControlFlowGraph::BlockRef;

TEST(BuildAbstractMachineControlFlowGraphTest, NoInstructions) {
  std::vector<Instruction> instructions;

  auto cfg = BuildAbstractMachineControlFlowGraph(instructions);

  EXPECT_EQ(VertexCount(cfg), 2);

  BlockRef source = SourceVertex(cfg);
  BlockRef sink = SinkVertex(cfg);

  EXPECT_THAT(Vertices(cfg), UnorderedElementsAre(source, sink));

  EXPECT_THAT(PrevVertices(cfg, source), IsEmpty());
  EXPECT_THAT(NextVertices(cfg, source), UnorderedElementsAre(sink));
  ASSERT_TRUE(cfg.Get(source).has_value());
  EXPECT_THAT(cfg.Get(source)->Instructions(), IsEmpty());

  EXPECT_THAT(NextVertices(cfg, sink), IsEmpty());
  EXPECT_THAT(PrevVertices(cfg, sink), UnorderedElementsAre(source));
  ASSERT_TRUE(cfg.Get(sink).has_value());
  EXPECT_THAT(cfg.Get(sink)->Instructions(), IsEmpty());
}

TEST(BuildAbstractMachineControlFlowGraphTest, LabelWithoutJumps) {
  std::vector<Instruction> instructions = {
      PushStack{},
      Label{
          .id = 0,
      },
      SetReg32{
          .src_val = "21",
          .dst_reg = 1,
      },
      MoveReg32{
          .src_reg = 1,
          .dst_reg = 0,
      },
  };

  auto cfg = BuildAbstractMachineControlFlowGraph(instructions);

  EXPECT_EQ(VertexCount(cfg), 3);

  BlockRef source = SourceVertex(cfg);
  BlockRef foo = BlockRef(2);
  BlockRef sink = SinkVertex(cfg);

  EXPECT_THAT(Vertices(cfg), UnorderedElementsAre(source, foo, sink));

  EXPECT_THAT(PrevVertices(cfg, source), IsEmpty());
  EXPECT_THAT(NextVertices(cfg, source), UnorderedElementsAre(foo));
  ASSERT_TRUE(cfg.Get(source).has_value());
  EXPECT_THAT(cfg.Get(source)->Instructions(),
              UnorderedElementsAre(PushStack{}));

  EXPECT_THAT(PrevVertices(cfg, foo), UnorderedElementsAre(source));
  EXPECT_THAT(NextVertices(cfg, foo), UnorderedElementsAre(sink));
  ASSERT_TRUE(cfg.Get(foo).has_value());
  EXPECT_THAT(cfg.Get(foo)->Instructions(), UnorderedElementsAre(
                                                SetReg32{
                                                    .src_val = "21",
                                                    .dst_reg = 1,
                                                },
                                                MoveReg32{
                                                    .src_reg = 1,
                                                    .dst_reg = 0,
                                                }));

  EXPECT_THAT(NextVertices(cfg, sink), IsEmpty());
  EXPECT_THAT(PrevVertices(cfg, sink), UnorderedElementsAre(foo));
  ASSERT_TRUE(cfg.Get(sink).has_value());
  EXPECT_THAT(cfg.Get(sink)->Instructions(), IsEmpty());
}

TEST(BuildAbstractMachineControlFlowGraphTest, LabelWithJump) {
  std::vector<Instruction> instructions = {
      PushStack{},
      SetReg32{
          .src_val = "5",
          .dst_reg = 1,
      },
      Label{
          .id = 0,
      },
      SetReg32{
          .src_val = "1",
          .dst_reg = 2,
      },
      SubReg32{
          .res_reg = 1,
          .lhs_reg = 1,
          .rhs_reg = 2,
      },
      SetReg32{
          .src_val = "0",
          .dst_reg = 3,
      },
      EqReg32{
          .res_reg = 4,
          .lhs_reg = 1,
          .rhs_reg = 3,
      },
      CondJump{
          .cond_reg = 4,
          .then_label = 1,
          .else_label = 0,
      },
      Label{
          .id = 1,
      },
      SetReg32{
          .src_val = "21",
          .dst_reg = 0,
      },
  };

  auto cfg = BuildAbstractMachineControlFlowGraph(instructions);

  EXPECT_EQ(VertexCount(cfg), 4);

  BlockRef source = SourceVertex(cfg);
  BlockRef foo = BlockRef(2);
  BlockRef bar = BlockRef(3);
  BlockRef sink = SinkVertex(cfg);

  EXPECT_THAT(Vertices(cfg), UnorderedElementsAre(source, foo, bar, sink));

  EXPECT_THAT(PrevVertices(cfg, source), IsEmpty());
  EXPECT_THAT(NextVertices(cfg, source), UnorderedElementsAre(foo));
  ASSERT_TRUE(cfg.Get(source).has_value());
  EXPECT_THAT(cfg.Get(source)->Instructions(),
              UnorderedElementsAre(PushStack{}, SetReg32{
                                                    .src_val = "5",
                                                    .dst_reg = 1,
                                                }));

  EXPECT_THAT(PrevVertices(cfg, foo), UnorderedElementsAre(source, foo));
  EXPECT_THAT(NextVertices(cfg, foo), UnorderedElementsAre(foo, bar));
  ASSERT_TRUE(cfg.Get(foo).has_value());
  EXPECT_THAT(cfg.Get(foo)->Instructions(), UnorderedElementsAre(
                                                SetReg32{
                                                    .src_val = "1",
                                                    .dst_reg = 2,
                                                },
                                                SubReg32{
                                                    .res_reg = 1,
                                                    .lhs_reg = 1,
                                                    .rhs_reg = 2,
                                                },
                                                SetReg32{
                                                    .src_val = "0",
                                                    .dst_reg = 3,
                                                },
                                                EqReg32{
                                                    .res_reg = 4,
                                                    .lhs_reg = 1,
                                                    .rhs_reg = 3,
                                                },
                                                CondJump{
                                                    .cond_reg = 4,
                                                    .then_label = 1,
                                                    .else_label = 0,
                                                }));

  EXPECT_THAT(PrevVertices(cfg, bar), UnorderedElementsAre(foo));
  EXPECT_THAT(NextVertices(cfg, bar), UnorderedElementsAre(sink));
  ASSERT_TRUE(cfg.Get(bar).has_value());
  EXPECT_THAT(cfg.Get(bar)->Instructions(), UnorderedElementsAre(SetReg32{
                                                .src_val = "21",
                                                .dst_reg = 0,
                                            }));

  EXPECT_THAT(NextVertices(cfg, sink), IsEmpty());
  EXPECT_THAT(PrevVertices(cfg, sink), UnorderedElementsAre(bar));
  ASSERT_TRUE(cfg.Get(sink).has_value());
  EXPECT_THAT(cfg.Get(sink)->Instructions(), IsEmpty());
}

}  // namespace
}  // namespace lucid
