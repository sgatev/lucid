#include "lucid/core/container/graph/dominator.h"

#include "lucid/core/container/graph/test_graph.h"
#include "lucid/core/testing/testing.h"

namespace lucid {
namespace {

TEST(Test, ComputeImmediateDominatorsSimple) {
  TestGraph g;
  g.SetSource('A');
  g.SetSink('W');
  g.AddEdge('A', 'W');

  EXPECT_THAT(ComputeImmediateDominators(g),
              ElementsEqual(/* A, W */ 'A', 'A'));
}

TEST(Test, ComputeImmediateDominatorsDiamondBranch) {
  TestGraph g;
  g.SetSource('A');
  g.SetSink('W');
  g.AddEdge('A', 'B');
  g.AddEdge('A', 'C');
  g.AddEdge('B', 'W');
  g.AddEdge('C', 'W');

  EXPECT_THAT(ComputeImmediateDominators(g),
              ElementsEqual(/* A, W, B, C */ 'A', 'A', 'A', 'A'));
}

TEST(Test, ComputeImmediateDominatorsLoop) {
  TestGraph g;
  g.SetSource('A');
  g.SetSink('W');
  g.AddEdge('A', 'B');
  g.AddEdge('B', 'C');
  g.AddEdge('B', 'D');
  g.AddEdge('C', 'B');
  g.AddEdge('D', 'W');

  EXPECT_THAT(ComputeImmediateDominators(g),
              ElementsEqual(/* A, W, B, C, D */ 'A', 'D', 'A', 'B', 'B'));
}

TEST(Test, ComputeDominanceFrontiersSimple) {
  TestGraph g;
  g.SetSource('A');
  g.SetSink('W');
  g.AddEdge('A', 'W');

  EXPECT_THAT(ComputeDominanceFrontiers(g, ComputeImmediateDominators(g)),
              IsEmpty());
}

TEST(Test, ComputeDominanceFrontiersDiamondBranch) {
  TestGraph g;
  g.SetSource('A');
  g.SetSink('W');
  g.AddEdge('A', 'B');
  g.AddEdge('A', 'C');
  g.AddEdge('B', 'W');
  g.AddEdge('C', 'W');

  EXPECT_THAT(
      ComputeDominanceFrontiers(g, ComputeImmediateDominators(g)),
      UnorderedElements(Pair(Equals('B'), UnorderedElementsEqual('W')),
                        Pair(Equals('C'), UnorderedElementsEqual('W'))));
}

TEST(Test, ComputeDominanceFrontiersLoop) {
  TestGraph g;
  g.SetSource('A');
  g.SetSink('W');
  g.AddEdge('A', 'B');
  g.AddEdge('B', 'C');
  g.AddEdge('B', 'D');
  g.AddEdge('C', 'B');
  g.AddEdge('D', 'W');

  EXPECT_THAT(
      ComputeDominanceFrontiers(g, ComputeImmediateDominators(g)),
      UnorderedElements(Pair(Equals('B'), UnorderedElementsEqual('B')),
                        Pair(Equals('C'), UnorderedElementsEqual('B'))));
}

TEST(Test, BuildDominatorTreeSimple) {
  TestGraph g;
  g.SetSource('A');
  g.SetSink('W');
  g.AddEdge('A', 'W');

  EXPECT_THAT(
      BuildDominatorTree(g, ComputeImmediateDominators(g)),
      UnorderedElements(Pair(Equals('A'), UnorderedElementsEqual('A', 'W'))));
}

TEST(Test, BuildDominatorTreeDiamondBranch) {
  TestGraph g;
  g.SetSource('A');
  g.SetSink('W');
  g.AddEdge('A', 'B');
  g.AddEdge('A', 'C');
  g.AddEdge('B', 'W');
  g.AddEdge('C', 'W');

  EXPECT_THAT(BuildDominatorTree(g, ComputeImmediateDominators(g)),
              UnorderedElements(Pair(
                  Equals('A'), UnorderedElementsEqual('A', 'B', 'C', 'W'))));
}

TEST(Test, BuildDominatorTreeLoop) {
  TestGraph g;
  g.SetSource('A');
  g.SetSink('W');
  g.AddEdge('A', 'B');
  g.AddEdge('B', 'C');
  g.AddEdge('B', 'D');
  g.AddEdge('C', 'B');
  g.AddEdge('D', 'W');

  EXPECT_THAT(
      BuildDominatorTree(g, ComputeImmediateDominators(g)),
      UnorderedElements(Pair(Equals('A'), UnorderedElementsEqual('A', 'B')),
                        Pair(Equals('B'), UnorderedElementsEqual('C', 'D')),
                        Pair(Equals('D'), UnorderedElementsEqual('W'))));
}

}  // namespace
}  // namespace lucid
