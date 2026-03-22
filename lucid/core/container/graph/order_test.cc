#include "lucid/core/container/graph/order.h"

#include <cstddef>

#include "gmock/gmock.h"
#include "gtest/gtest.h"
#include "lucid/core/container/graph/test_graph.h"

namespace lucid {
namespace {

using ::testing::ElementsAre;

TEST(ComputeReversePostOrderTest, Simple) {
  TestGraph g;
  g.SetSource('A');
  g.SetSink('W');
  g.AddEdge('A', 'W');

  EXPECT_THAT(ComputeReversePostOrder(g), ElementsAre(/* A, W */ 0, 1));
}

TEST(ComputeReversePostOrderTest, DiamondBranch) {
  TestGraph g;
  g.SetSource('A');
  g.SetSink('W');
  g.AddEdge('A', 'B');
  g.AddEdge('A', 'C');
  g.AddEdge('B', 'W');
  g.AddEdge('C', 'W');

  EXPECT_THAT(ComputeReversePostOrder(g),
              ElementsAre(/* A, W, B, C */ 0, 3, 2, 1));
}

TEST(ComputeReversePostOrderTest, Loop) {
  TestGraph g;
  g.SetSource('A');
  g.SetSink('W');
  g.AddEdge('A', 'B');
  g.AddEdge('B', 'C');
  g.AddEdge('B', 'D');
  g.AddEdge('C', 'B');
  g.AddEdge('D', 'W');

  EXPECT_THAT(ComputeReversePostOrder(g),
              ElementsAre(/* A, W, B, C, D */ 0, 4, 1, 2, 3));
}

}  // namespace
}  // namespace lucid
