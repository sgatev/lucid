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

  State Join(State&& left, const State& right) {
    State state = std::move(left);
    for (auto& result : right.results) state.results.Insert(result);
    return state;
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

}  // namespace
}  // namespace lucid
