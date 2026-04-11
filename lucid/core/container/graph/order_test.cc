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

TEST(ComparePostOrderTest, Simple) {
  TestGraph g;
  g.SetSource('A');
  g.SetSink('W');
  g.AddEdge('A', 'W');

  std::vector<char> vertices = g.Vertices();
  std::sort(vertices.begin(), vertices.end(), ComparePostOrder(g));

  EXPECT_THAT(vertices, ElementsAre('W', 'A'));
}

TEST(ComparePostOrderTest, DiamondBranch) {
  TestGraph g;
  g.SetSource('A');
  g.SetSink('W');
  g.AddEdge('A', 'B');
  g.AddEdge('A', 'C');
  g.AddEdge('B', 'W');
  g.AddEdge('C', 'W');

  std::vector<char> vertices = g.Vertices();
  std::sort(vertices.begin(), vertices.end(), ComparePostOrder(g));

  EXPECT_THAT(vertices, ElementsAre('W', 'C', 'B', 'A'));
}

TEST(ComparePostOrderTest, Complex) {
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
  std::sort(vertices.begin(), vertices.end(), ComparePostOrder(g));

  EXPECT_THAT(vertices, ElementsAre('W', 'C', 'B', 'E', 'D', 'A'));
}

TEST(ComparePostOrderTest, Loop) {
  TestGraph g;
  g.SetSource('A');
  g.SetSink('W');
  g.AddEdge('A', 'B');
  g.AddEdge('B', 'C');
  g.AddEdge('B', 'D');
  g.AddEdge('C', 'B');
  g.AddEdge('D', 'W');

  std::vector<char> vertices = g.Vertices();
  std::sort(vertices.begin(), vertices.end(), ComparePostOrder(g));

  EXPECT_THAT(vertices, ElementsAre('C', 'W', 'D', 'B', 'A'));
}

TEST(CompareReversePostOrderTest, Simple) {
  TestGraph g;
  g.SetSource('A');
  g.SetSink('W');
  g.AddEdge('A', 'W');

  std::vector<char> vertices = g.Vertices();
  std::sort(vertices.begin(), vertices.end(), CompareReversePostOrder(g));

  EXPECT_THAT(vertices, ElementsAre('A', 'W'));
}

TEST(CompareReversePostOrderTest, DiamondBranch) {
  TestGraph g;
  g.SetSource('A');
  g.SetSink('W');
  g.AddEdge('A', 'B');
  g.AddEdge('A', 'C');
  g.AddEdge('B', 'W');
  g.AddEdge('C', 'W');

  std::vector<char> vertices = g.Vertices();
  std::sort(vertices.begin(), vertices.end(), CompareReversePostOrder(g));

  EXPECT_THAT(vertices, ElementsAre('A', 'B', 'C', 'W'));
}

TEST(CompareReversePostOrderTest, Complex) {
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
  std::sort(vertices.begin(), vertices.end(), CompareReversePostOrder(g));

  EXPECT_THAT(vertices, ElementsAre('A', 'D', 'E', 'B', 'C', 'W'));
}

TEST(CompareReversePostOrderTest, Loop) {
  TestGraph g;
  g.SetSource('A');
  g.SetSink('W');
  g.AddEdge('A', 'B');
  g.AddEdge('B', 'C');
  g.AddEdge('B', 'D');
  g.AddEdge('C', 'B');
  g.AddEdge('D', 'W');

  std::vector<char> vertices = g.Vertices();
  std::sort(vertices.begin(), vertices.end(), CompareReversePostOrder(g));

  EXPECT_THAT(vertices, ElementsAre('A', 'B', 'D', 'W', 'C'));
}

}  // namespace
}  // namespace lucid
