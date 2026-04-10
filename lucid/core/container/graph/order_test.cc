#include "lucid/core/container/graph/order.h"

#include <algorithm>
#include <cstddef>
#include <vector>

#include "gmock/gmock.h"
#include "gtest/gtest.h"
#include "lucid/core/container/graph/test_graph.h"

namespace lucid {
namespace {

using ::testing::ElementsAre;

TEST(ComputePostOrderTest, Simple) {
  TestGraph g;
  g.SetSource('A');
  g.SetSink('W');
  g.AddEdge('A', 'W');

  std::vector<char> vertices = g.Vertices();
  std::sort(vertices.begin(), vertices.end(),
            CompareVertexOrder(g, ComputePostOrder(g)));

  EXPECT_THAT(vertices, ElementsAre('W', 'A'));
}

TEST(ComputePostOrderTest, DiamondBranch) {
  TestGraph g;
  g.SetSource('A');
  g.SetSink('W');
  g.AddEdge('A', 'B');
  g.AddEdge('A', 'C');
  g.AddEdge('B', 'W');
  g.AddEdge('C', 'W');

  std::vector<char> vertices = g.Vertices();
  std::sort(vertices.begin(), vertices.end(),
            CompareVertexOrder(g, ComputePostOrder(g)));

  EXPECT_THAT(vertices, ElementsAre('W', 'C', 'B', 'A'));
}

TEST(ComputePostOrderTest, Complex) {
  TestGraph g;
  g.SetSource('A');
  g.SetSink('W');
  g.AddEdge('A', 'B');
  g.AddEdge('A', 'C');
  g.AddEdge('A', 'D');
  g.AddEdge('B', 'C');
  g.AddEdge('D', 'C');
  g.AddEdge('C', 'W');
  g.AddEdge('D', 'E');
  g.AddEdge('E', 'W');

  std::vector<char> vertices = g.Vertices();
  std::sort(vertices.begin(), vertices.end(),
            CompareVertexOrder(g, ComputePostOrder(g)));

  EXPECT_THAT(vertices, ElementsAre('W', 'C', 'B', 'E', 'D', 'A'));
}

TEST(ComputePostOrderTest, Loop) {
  TestGraph g;
  g.SetSource('A');
  g.SetSink('W');
  g.AddEdge('A', 'B');
  g.AddEdge('B', 'C');
  g.AddEdge('B', 'D');
  g.AddEdge('C', 'B');
  g.AddEdge('D', 'W');

  std::vector<char> vertices = g.Vertices();
  std::sort(vertices.begin(), vertices.end(),
            CompareVertexOrder(g, ComputePostOrder(g)));

  EXPECT_THAT(vertices, ElementsAre('C', 'W', 'D', 'B', 'A'));
}

TEST(ComputeReversePostOrderTest, Simple) {
  TestGraph g;
  g.SetSource('A');
  g.SetSink('W');
  g.AddEdge('A', 'W');

  std::vector<char> vertices = g.Vertices();
  std::sort(vertices.begin(), vertices.end(),
            CompareVertexOrder(g, ComputeReversePostOrder(g)));

  EXPECT_THAT(vertices, ElementsAre('A', 'W'));
}

TEST(ComputeReversePostOrderTest, DiamondBranch) {
  TestGraph g;
  g.SetSource('A');
  g.SetSink('W');
  g.AddEdge('A', 'B');
  g.AddEdge('A', 'C');
  g.AddEdge('B', 'W');
  g.AddEdge('C', 'W');

  std::vector<char> vertices = g.Vertices();
  std::sort(vertices.begin(), vertices.end(),
            CompareVertexOrder(g, ComputeReversePostOrder(g)));

  EXPECT_THAT(vertices, ElementsAre('A', 'B', 'C', 'W'));
}

TEST(ComputeReversePostOrderTest, Complex) {
  TestGraph g;
  g.SetSource('A');
  g.SetSink('W');
  g.AddEdge('A', 'B');
  g.AddEdge('A', 'C');
  g.AddEdge('A', 'D');
  g.AddEdge('B', 'C');
  g.AddEdge('D', 'C');
  g.AddEdge('C', 'W');
  g.AddEdge('D', 'E');
  g.AddEdge('E', 'W');

  std::vector<char> vertices = g.Vertices();
  std::sort(vertices.begin(), vertices.end(),
            CompareVertexOrder(g, ComputeReversePostOrder(g)));

  EXPECT_THAT(vertices, ElementsAre('A', 'D', 'E', 'B', 'C', 'W'));
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

  std::vector<char> vertices = g.Vertices();
  std::sort(vertices.begin(), vertices.end(),
            CompareVertexOrder(g, ComputeReversePostOrder(g)));

  EXPECT_THAT(vertices, ElementsAre('A', 'B', 'D', 'W', 'C'));
}

}  // namespace
}  // namespace lucid
