#include "lucid/core/dataflow/dataflow.h"

#include <optional>
#include <unordered_set>
#include <utility>
#include <vector>

#include "gmock/gmock.h"
#include "gtest/gtest.h"
#include "lucid/core/container/graph/test_graph.h"

namespace lucid {
namespace {

using namespace std::string_literals;

using ::testing::Field;
using ::testing::IsEmpty;
using ::testing::Optional;
using ::testing::SizeIs;
using ::testing::UnorderedElementsAre;

class TestResultUnionAnalysis {
 public:
  struct State {
    bool operator==(const State&) const = default;

    std::unordered_set<char> results;
  };

  explicit TestResultUnionAnalysis() {}

  State Transfer(std::optional<State> prior_state, TestGraph::vertex_type v) {
    State state;
    if (prior_state.has_value()) state = *std::move(prior_state);
    state.results.insert(v);
    return state;
  }

  State Join(State left, State right) {
    State state = std::move(left);
    state.results.insert(right.results.begin(), right.results.end());
    return state;
  }

 private:
};

TEST(RunForwardDataflowTest, Simple) {
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
                             UnorderedElementsAre('A'))));

  EXPECT_THAT(vertex_states[/* W */ 1],
              Optional(Field(&TestResultUnionAnalysis::State::results,
                             UnorderedElementsAre('A', 'W'))));
}

TEST(RunForwardDataflowTest, DiamondBranch) {
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
                             UnorderedElementsAre('A'))));

  EXPECT_THAT(vertex_states[/* W */ 1],
              Optional(Field(&TestResultUnionAnalysis::State::results,
                             UnorderedElementsAre('A', 'B', 'C', 'W'))));

  EXPECT_THAT(vertex_states[/* B */ 2],
              Optional(Field(&TestResultUnionAnalysis::State::results,
                             UnorderedElementsAre('A', 'B'))));

  EXPECT_THAT(vertex_states[/* C */ 3],
              Optional(Field(&TestResultUnionAnalysis::State::results,
                             UnorderedElementsAre('A', 'C'))));
}

TEST(RunForwardDataflowTest, Loop) {
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
                             UnorderedElementsAre('A'))));

  EXPECT_THAT(vertex_states[/* W */ 1],
              Optional(Field(&TestResultUnionAnalysis::State::results,
                             UnorderedElementsAre('A', 'B', 'C', 'D', 'W'))));

  EXPECT_THAT(vertex_states[/* B */ 2],
              Optional(Field(&TestResultUnionAnalysis::State::results,
                             UnorderedElementsAre('A', 'B', 'C'))));

  EXPECT_THAT(vertex_states[/* C */ 3],
              Optional(Field(&TestResultUnionAnalysis::State::results,
                             UnorderedElementsAre('A', 'B', 'C'))));

  EXPECT_THAT(vertex_states[/* D */ 4],
              Optional(Field(&TestResultUnionAnalysis::State::results,
                             UnorderedElementsAre('A', 'B', 'C', 'D'))));
}

TEST(RunBackwardDataflowTest, Simple) {
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
                             UnorderedElementsAre('A', 'W'))));

  EXPECT_THAT(vertex_states[/* W */ 1],
              Optional(Field(&TestResultUnionAnalysis::State::results,
                             UnorderedElementsAre('W'))));
}

TEST(RunBackwardDataflowTest, DiamondBranch) {
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
                             UnorderedElementsAre('A', 'B', 'C', 'W'))));

  EXPECT_THAT(vertex_states[/* W */ 1],
              Optional(Field(&TestResultUnionAnalysis::State::results,
                             UnorderedElementsAre('W'))));

  EXPECT_THAT(vertex_states[/* B */ 2],
              Optional(Field(&TestResultUnionAnalysis::State::results,
                             UnorderedElementsAre('B', 'W'))));

  EXPECT_THAT(vertex_states[/* C */ 3],
              Optional(Field(&TestResultUnionAnalysis::State::results,
                             UnorderedElementsAre('C', 'W'))));
}

TEST(RunBackwardDataflowTest, Loop) {
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
                             UnorderedElementsAre('A', 'B', 'C', 'D', 'W'))));

  EXPECT_THAT(vertex_states[/* W */ 1],
              Optional(Field(&TestResultUnionAnalysis::State::results,
                             UnorderedElementsAre('W'))));

  EXPECT_THAT(vertex_states[/* B */ 2],
              Optional(Field(&TestResultUnionAnalysis::State::results,
                             UnorderedElementsAre('B', 'C', 'D', 'W'))));

  EXPECT_THAT(vertex_states[/* C */ 3],
              Optional(Field(&TestResultUnionAnalysis::State::results,
                             UnorderedElementsAre('B', 'C', 'D', 'W'))));

  EXPECT_THAT(vertex_states[/* D */ 4],
              Optional(Field(&TestResultUnionAnalysis::State::results,
                             UnorderedElementsAre('D', 'W'))));
}

}  // namespace
}  // namespace lucid
