#include "lucid/core/container/graph/order.h"

#include <algorithm>
#include <cstddef>
#include <vector>

#include "lucid/core/container/graph/test_graph.h"
#include "lucid/core/testing/testing.h"

namespace lucid {
namespace {

TEST(Test, ComparePostOrderSimple) {
  TestGraph g;
  g.SetSource('A');
  g.SetSink('W');
  g.AddEdge('A', 'W');

  std::vector<char> vertices = g.Vertices();
  std::sort(vertices.begin(), vertices.end(), ComparePostOrder(g));

  EXPECT_THAT(vertices, ElementsEqual('W', 'A'));
}

TEST(Test, ComparePostOrderDiamondBranch) {
  TestGraph g;
  g.SetSource('A');
  g.SetSink('W');
  g.AddEdge('A', 'B');
  g.AddEdge('A', 'C');
  g.AddEdge('B', 'W');
  g.AddEdge('C', 'W');

  std::vector<char> vertices = g.Vertices();
  std::sort(vertices.begin(), vertices.end(), ComparePostOrder(g));

  EXPECT_THAT(vertices, ElementsEqual('W', 'C', 'B', 'A'));
}

TEST(Test, ComparePostOrderComplex) {
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

  EXPECT_THAT(vertices, ElementsEqual('W', 'C', 'B', 'E', 'D', 'A'));
}

TEST(Test, ComparePostOrderLoop) {
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

  EXPECT_THAT(vertices, ElementsEqual('C', 'W', 'D', 'B', 'A'));
}

TEST(Test, CompareReversePostOrderSimple) {
  TestGraph g;
  g.SetSource('A');
  g.SetSink('W');
  g.AddEdge('A', 'W');

  std::vector<char> vertices = g.Vertices();
  std::sort(vertices.begin(), vertices.end(), CompareReversePostOrder(g));

  EXPECT_THAT(vertices, ElementsEqual('A', 'W'));
}

TEST(Test, CompareReversePostOrderDiamondBranch) {
  TestGraph g;
  g.SetSource('A');
  g.SetSink('W');
  g.AddEdge('A', 'B');
  g.AddEdge('A', 'C');
  g.AddEdge('B', 'W');
  g.AddEdge('C', 'W');

  std::vector<char> vertices = g.Vertices();
  std::sort(vertices.begin(), vertices.end(), CompareReversePostOrder(g));

  EXPECT_THAT(vertices, ElementsEqual('A', 'B', 'C', 'W'));
}

TEST(Test, CompareReversePostOrderComplex) {
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

  EXPECT_THAT(vertices, ElementsEqual('A', 'D', 'E', 'B', 'C', 'W'));
}

TEST(Test, CompareReversePostOrderLoop) {
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

  EXPECT_THAT(vertices, ElementsEqual('A', 'B', 'D', 'W', 'C'));
}

}  // namespace
}  // namespace lucid
