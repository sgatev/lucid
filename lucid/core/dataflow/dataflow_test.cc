#include "lucid/core/dataflow/dataflow.h"

#include <optional>
#include <utility>
#include <vector>

#include "lucid/core/container/graph/test_graph.h"
#include "lucid/core/container/hash_set.h"
#include "lucid/core/testing/testing.h"

namespace lucid {
namespace {

using namespace std::string_literals;

class TestResultUnionAnalysis {
 public:
  struct State {
    bool operator==(const State&) const = default;

    HashSet<char> results;
  };

  explicit TestResultUnionAnalysis() {}

  State Transfer(std::optional<State>&& prior_state, TestGraph::vertex_type v) {
    State state;
    if (prior_state.has_value()) state = *std::move(prior_state);
    state.results.Insert(v);
    return state;
  }

  void Join(State& left, const State& right) {
    for (auto& result : right.results) left.results.Insert(result);
  }
};

TEST(Test, RunForwardDataflowSimple) {
  TestGraph g;
  g.SetSource('A');
  g.SetSink('W');
  g.AddEdge('A', 'W');

  TestResultUnionAnalysis a;
  std::vector<std::optional<TestResultUnionAnalysis::State>> vertex_states =
      RunDataflow(Forward(g), a);

  ASSERT_THAT(vertex_states, SizeIs(2));

  EXPECT_THAT(vertex_states[/* A */ 0],
              Optional(Field(&TestResultUnionAnalysis::State::results,
                             UnorderedElementsEqual('A'))));

  EXPECT_THAT(vertex_states[/* W */ 1],
              Optional(Field(&TestResultUnionAnalysis::State::results,
                             UnorderedElementsEqual('A', 'W'))));
}

TEST(Test, RunForwardDataflowDiamondBranch) {
  TestGraph g;
  g.SetSource('A');
  g.SetSink('W');
  g.AddEdge('A', 'B');
  g.AddEdge('A', 'C');
  g.AddEdge('B', 'W');
  g.AddEdge('C', 'W');

  TestResultUnionAnalysis a;
  std::vector<std::optional<TestResultUnionAnalysis::State>> vertex_states =
      RunDataflow(Forward(g), a);

  ASSERT_THAT(vertex_states, SizeIs(4));

  EXPECT_THAT(vertex_states[/* A */ 0],
              Optional(Field(&TestResultUnionAnalysis::State::results,
                             UnorderedElementsEqual('A'))));

  EXPECT_THAT(vertex_states[/* W */ 1],
              Optional(Field(&TestResultUnionAnalysis::State::results,
                             UnorderedElementsEqual('A', 'B', 'C', 'W'))));

  EXPECT_THAT(vertex_states[/* B */ 2],
              Optional(Field(&TestResultUnionAnalysis::State::results,
                             UnorderedElementsEqual('A', 'B'))));

  EXPECT_THAT(vertex_states[/* C */ 3],
              Optional(Field(&TestResultUnionAnalysis::State::results,
                             UnorderedElementsEqual('A', 'C'))));
}

TEST(Test, RunForwardDataflowLoop) {
  TestGraph g;
  g.SetSource('A');
  g.SetSink('W');
  g.AddEdge('A', 'B');
  g.AddEdge('B', 'C');
  g.AddEdge('B', 'D');
  g.AddEdge('C', 'B');
  g.AddEdge('D', 'W');

  TestResultUnionAnalysis a;
  std::vector<std::optional<TestResultUnionAnalysis::State>> vertex_states =
      RunDataflow(Forward(g), a);

  ASSERT_THAT(vertex_states, SizeIs(5));

  EXPECT_THAT(vertex_states[/* A */ 0],
              Optional(Field(&TestResultUnionAnalysis::State::results,
                             UnorderedElementsEqual('A'))));

  EXPECT_THAT(vertex_states[/* W */ 1],
              Optional(Field(&TestResultUnionAnalysis::State::results,
                             UnorderedElementsEqual('A', 'B', 'C', 'D', 'W'))));

  EXPECT_THAT(vertex_states[/* B */ 2],
              Optional(Field(&TestResultUnionAnalysis::State::results,
                             UnorderedElementsEqual('A', 'B', 'C'))));

  EXPECT_THAT(vertex_states[/* C */ 3],
              Optional(Field(&TestResultUnionAnalysis::State::results,
                             UnorderedElementsEqual('A', 'B', 'C'))));

  EXPECT_THAT(vertex_states[/* D */ 4],
              Optional(Field(&TestResultUnionAnalysis::State::results,
                             UnorderedElementsEqual('A', 'B', 'C', 'D'))));
}

TEST(Test, RunBackwardDataflowSimple) {
  TestGraph g;
  g.SetSource('A');
  g.SetSink('W');
  g.AddEdge('A', 'W');

  TestResultUnionAnalysis a;
  std::vector<std::optional<TestResultUnionAnalysis::State>> vertex_states =
      RunDataflow(Backward(g), a);

  ASSERT_THAT(vertex_states, SizeIs(2));

  EXPECT_THAT(vertex_states[/* A */ 0],
              Optional(Field(&TestResultUnionAnalysis::State::results,
                             UnorderedElementsEqual('A', 'W'))));

  EXPECT_THAT(vertex_states[/* W */ 1],
              Optional(Field(&TestResultUnionAnalysis::State::results,
                             UnorderedElementsEqual('W'))));
}

TEST(Test, RunBackwardDataflowDiamondBranch) {
  TestGraph g;
  g.SetSource('A');
  g.SetSink('W');
  g.AddEdge('A', 'B');
  g.AddEdge('A', 'C');
  g.AddEdge('B', 'W');
  g.AddEdge('C', 'W');

  TestResultUnionAnalysis a;
  std::vector<std::optional<TestResultUnionAnalysis::State>> vertex_states =
      RunDataflow(Backward(g), a);

  ASSERT_THAT(vertex_states, SizeIs(4));

  EXPECT_THAT(vertex_states[/* A */ 0],
              Optional(Field(&TestResultUnionAnalysis::State::results,
                             UnorderedElementsEqual('A', 'B', 'C', 'W'))));

  EXPECT_THAT(vertex_states[/* W */ 1],
              Optional(Field(&TestResultUnionAnalysis::State::results,
                             UnorderedElementsEqual('W'))));

  EXPECT_THAT(vertex_states[/* B */ 2],
              Optional(Field(&TestResultUnionAnalysis::State::results,
                             UnorderedElementsEqual('B', 'W'))));

  EXPECT_THAT(vertex_states[/* C */ 3],
              Optional(Field(&TestResultUnionAnalysis::State::results,
                             UnorderedElementsEqual('C', 'W'))));
}

TEST(Test, RunBackwardDataflowLoop) {
  TestGraph g;
  g.SetSource('A');
  g.SetSink('W');
  g.AddEdge('A', 'B');
  g.AddEdge('B', 'C');
  g.AddEdge('B', 'D');
  g.AddEdge('C', 'B');
  g.AddEdge('D', 'W');

  TestResultUnionAnalysis a;
  std::vector<std::optional<TestResultUnionAnalysis::State>> vertex_states =
      RunDataflow(Backward(g), a);

  ASSERT_THAT(vertex_states, SizeIs(5));

  EXPECT_THAT(vertex_states[/* A */ 0],
              Optional(Field(&TestResultUnionAnalysis::State::results,
                             UnorderedElementsEqual('A', 'B', 'C', 'D', 'W'))));

  EXPECT_THAT(vertex_states[/* W */ 1],
              Optional(Field(&TestResultUnionAnalysis::State::results,
                             UnorderedElementsEqual('W'))));

  EXPECT_THAT(vertex_states[/* B */ 2],
              Optional(Field(&TestResultUnionAnalysis::State::results,
                             UnorderedElementsEqual('B', 'C', 'D', 'W'))));

  EXPECT_THAT(vertex_states[/* C */ 3],
              Optional(Field(&TestResultUnionAnalysis::State::results,
                             UnorderedElementsEqual('B', 'C', 'D', 'W'))));

  EXPECT_THAT(vertex_states[/* D */ 4],
              Optional(Field(&TestResultUnionAnalysis::State::results,
                             UnorderedElementsEqual('D', 'W'))));
}

// A loop with no way out is never reached walking back from the sink, and
// yet what holds on it is as well defined as on a loop that ends.
TEST(Test, RunBackwardDataflowLoopWithNoExit) {
  TestGraph g;
  g.SetSource('A');
  g.SetSink('W');
  g.AddEdge('A', 'B');
  g.AddEdge('B', 'C');
  g.AddEdge('C', 'B');

  TestResultUnionAnalysis a;
  std::vector<std::optional<TestResultUnionAnalysis::State>> vertex_states =
      RunDataflow(Backward(g), a);

  ASSERT_THAT(vertex_states, SizeIs(4));

  EXPECT_THAT(vertex_states[/* A */ 0],
              Optional(Field(&TestResultUnionAnalysis::State::results,
                             UnorderedElementsEqual('A', 'B', 'C'))));

  EXPECT_THAT(vertex_states[/* W */ 1],
              Optional(Field(&TestResultUnionAnalysis::State::results,
                             UnorderedElementsEqual('W'))));

  EXPECT_THAT(vertex_states[/* B */ 2],
              Optional(Field(&TestResultUnionAnalysis::State::results,
                             UnorderedElementsEqual('B', 'C'))));

  EXPECT_THAT(vertex_states[/* C */ 3],
              Optional(Field(&TestResultUnionAnalysis::State::results,
                             UnorderedElementsEqual('B', 'C'))));
}

// A vertex that can reach the sink and can also enter a loop with no exit is
// worked out once before the loop has a state and once after it has one. The
// second time is what tells it what the loop holds.
TEST(Test, RunBackwardDataflowBranchIntoLoopWithNoExit) {
  TestGraph g;
  g.SetSource('A');
  g.SetSink('W');
  g.AddEdge('A', 'W');
  g.AddEdge('A', 'B');
  g.AddEdge('B', 'C');
  g.AddEdge('C', 'B');

  TestResultUnionAnalysis a;
  std::vector<std::optional<TestResultUnionAnalysis::State>> vertex_states =
      RunDataflow(Backward(g), a);

  ASSERT_THAT(vertex_states, SizeIs(4));

  EXPECT_THAT(vertex_states[/* A */ 0],
              Optional(Field(&TestResultUnionAnalysis::State::results,
                             UnorderedElementsEqual('A', 'B', 'C', 'W'))));
}

// Every vertex gets a state whichever way the analysis runs, including one
// the source never reaches.
TEST(Test, RunForwardDataflowUnreachedLoop) {
  TestGraph g;
  g.SetSource('A');
  g.SetSink('W');
  g.AddEdge('A', 'W');
  g.AddEdge('X', 'Y');
  g.AddEdge('Y', 'X');

  TestResultUnionAnalysis a;
  std::vector<std::optional<TestResultUnionAnalysis::State>> vertex_states =
      RunDataflow(Forward(g), a);

  ASSERT_THAT(vertex_states, SizeIs(4));

  EXPECT_THAT(vertex_states[/* A */ 0],
              Optional(Field(&TestResultUnionAnalysis::State::results,
                             UnorderedElementsEqual('A'))));

  EXPECT_THAT(vertex_states[/* W */ 1],
              Optional(Field(&TestResultUnionAnalysis::State::results,
                             UnorderedElementsEqual('A', 'W'))));

  EXPECT_THAT(vertex_states[/* X */ 2],
              Optional(Field(&TestResultUnionAnalysis::State::results,
                             UnorderedElementsEqual('X', 'Y'))));

  EXPECT_THAT(vertex_states[/* Y */ 3],
              Optional(Field(&TestResultUnionAnalysis::State::results,
                             UnorderedElementsEqual('X', 'Y'))));
}

}  // namespace
}  // namespace lucid
