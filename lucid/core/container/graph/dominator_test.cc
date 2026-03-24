#include "lucid/core/container/graph/dominator.h"

#include "gmock/gmock.h"
#include "gtest/gtest.h"
#include "lucid/core/container/graph/test_graph.h"

namespace lucid {
namespace {

using ::testing::ElementsAre;
using ::testing::IsEmpty;
using ::testing::Pair;
using ::testing::UnorderedElementsAre;

TEST(ComputeImmediateDominatorsTest, Simple) {
  TestGraph g;
  g.SetSource('A');
  g.SetSink('W');
  g.AddEdge('A', 'W');

  EXPECT_THAT(ComputeImmediateDominators(g), ElementsAre(/* A, W */ 'A', 'A'));
}

TEST(ComputeImmediateDominatorsTest, DiamondBranch) {
  TestGraph g;
  g.SetSource('A');
  g.SetSink('W');
  g.AddEdge('A', 'B');
  g.AddEdge('A', 'C');
  g.AddEdge('B', 'W');
  g.AddEdge('C', 'W');

  EXPECT_THAT(ComputeImmediateDominators(g),
              ElementsAre(/* A, W, B, C */ 'A', 'A', 'A', 'A'));
}

TEST(ComputeImmediateDominatorsTest, Loop) {
  TestGraph g;
  g.SetSource('A');
  g.SetSink('W');
  g.AddEdge('A', 'B');
  g.AddEdge('B', 'C');
  g.AddEdge('B', 'D');
  g.AddEdge('C', 'B');
  g.AddEdge('D', 'W');

  EXPECT_THAT(ComputeImmediateDominators(g),
              ElementsAre(/* A, W, B, C, D */ 'A', 'D', 'A', 'B', 'B'));
}

TEST(ComputeDominanceFrontiersTest, Simple) {
  TestGraph g;
  g.SetSource('A');
  g.SetSink('W');
  g.AddEdge('A', 'W');

  EXPECT_THAT(ComputeDominanceFrontiers(g, ComputeImmediateDominators(g)),
              IsEmpty());
}

TEST(ComputeDominanceFrontiersTest, DiamondBranch) {
  TestGraph g;
  g.SetSource('A');
  g.SetSink('W');
  g.AddEdge('A', 'B');
  g.AddEdge('A', 'C');
  g.AddEdge('B', 'W');
  g.AddEdge('C', 'W');

  EXPECT_THAT(ComputeDominanceFrontiers(g, ComputeImmediateDominators(g)),
              UnorderedElementsAre(Pair('B', UnorderedElementsAre('W')),
                                   Pair('C', UnorderedElementsAre('W'))));
}

TEST(ComputeDominanceFrontiersTest, Loop) {
  TestGraph g;
  g.SetSource('A');
  g.SetSink('W');
  g.AddEdge('A', 'B');
  g.AddEdge('B', 'C');
  g.AddEdge('B', 'D');
  g.AddEdge('C', 'B');
  g.AddEdge('D', 'W');

  EXPECT_THAT(ComputeDominanceFrontiers(g, ComputeImmediateDominators(g)),
              UnorderedElementsAre(Pair('B', UnorderedElementsAre('B')),
                                   Pair('C', UnorderedElementsAre('B'))));
}

TEST(BuildDominatorTreeTest, Simple) {
  TestGraph g;
  g.SetSource('A');
  g.SetSink('W');
  g.AddEdge('A', 'W');

  EXPECT_THAT(BuildDominatorTree(g),
              UnorderedElementsAre(Pair('A', UnorderedElementsAre('A', 'W'))));
}

TEST(BuildDominatorTreeTest, DiamondBranch) {
  TestGraph g;
  g.SetSource('A');
  g.SetSink('W');
  g.AddEdge('A', 'B');
  g.AddEdge('A', 'C');
  g.AddEdge('B', 'W');
  g.AddEdge('C', 'W');

  EXPECT_THAT(BuildDominatorTree(g),
              UnorderedElementsAre(
                  Pair('A', UnorderedElementsAre('A', 'B', 'C', 'W'))));
}

TEST(BuildDominatorTreeTest, Loop) {
  TestGraph g;
  g.SetSource('A');
  g.SetSink('W');
  g.AddEdge('A', 'B');
  g.AddEdge('B', 'C');
  g.AddEdge('B', 'D');
  g.AddEdge('C', 'B');
  g.AddEdge('D', 'W');

  EXPECT_THAT(BuildDominatorTree(g),
              UnorderedElementsAre(Pair('A', UnorderedElementsAre('A', 'B')),
                                   Pair('B', UnorderedElementsAre('C', 'D')),
                                   Pair('D', UnorderedElementsAre('W'))));
}

}  // namespace
}  // namespace lucid
