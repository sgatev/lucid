#include "lucid/dataflow.h"

#include <optional>
#include <string>
#include <string_view>
#include <unordered_set>
#include <vector>

#include "gmock/gmock.h"
#include "gtest/gtest.h"
#include "lucid/ast.h"
#include "lucid/cfg.h"
#include "lucid/lexer.h"
#include "lucid/parser.h"

namespace lucid {
namespace {

using namespace std::string_literals;

using ::testing::Field;
using ::testing::IsEmpty;
using ::testing::Optional;
using ::testing::SizeIs;
using ::testing::UnorderedElementsAre;

std::string StringWithNull(std::string_view s) {
  std::string s_with_null(s);
  s_with_null.append("\0"s);
  return s_with_null;
}

class TestResultUnionAnalysis {
 public:
  struct State {
    bool operator==(const State&) const = default;

    std::unordered_set<std::string_view> results;
  };

  explicit TestResultUnionAnalysis(const SyntaxContext& ctx) : ctx_(ctx) {}

  State MakeInitial() { return {}; }

  State Transfer(State state, const ControlFlowGraph::Sequence& seq) {
    if (!seq.stmt.has_value()) return state;

    auto* return_stmt =
        std::get_if<ReturnStmt>(&ctx_.DerefStmt(seq.stmt.value()));
    if (return_stmt == nullptr) return state;

    auto* return_val_expr =
        std::get_if<IntLitExpr>(&ctx_.DerefExpr(return_stmt->value));
    if (return_val_expr == nullptr) return state;

    state.results.insert(ctx_.DerefIdent(return_val_expr->value));

    return state;
  }

  State Join(State left, State right) {
    State state = std::move(left);
    state.results.insert(right.results.begin(), right.results.end());
    return state;
  }

 private:
  const SyntaxContext& ctx_;
};

TEST(RunBackwardDataflowTest, Simple) {
  std::string code = StringWithNull(R"(
    let foo = (c: Bool) -> Int32 {
      return 21
    }
  )");

  SyntaxContext ctx;
  Lexer lexer(code);
  Parser parser(ctx, code, lexer);
  auto func_def = std::get<std::optional<FuncDefStmt>>(parser.ParseFuncDef());
  auto cfg = BuildControlFlowGraph(ctx, *func_def);

  TestResultUnionAnalysis analysis(ctx);
  std::vector<std::optional<TestResultUnionAnalysis::State>> block_states =
      RunBackwardDataflow(cfg, analysis);

  ASSERT_THAT(block_states, SizeIs(2));

  auto first = cfg.first;
  EXPECT_THAT(block_states[first.id()],
              Optional(Field(&TestResultUnionAnalysis::State::results,
                             UnorderedElementsAre("21"))));

  auto last = cfg.last;
  EXPECT_THAT(
      block_states[last.id()],
      Optional(Field(&TestResultUnionAnalysis::State::results, IsEmpty())));
}

TEST(RunBackwardDataflowTest, Conditional) {
  std::string code = StringWithNull(R"(
    let foo = (c: Bool) -> Int32 {
      if c {
        return 1
      } else {
        return 2
      }
    }
  )");

  SyntaxContext ctx;
  Lexer lexer(code);
  Parser parser(ctx, code, lexer);
  auto func_def = std::get<std::optional<FuncDefStmt>>(parser.ParseFuncDef());
  auto cfg = BuildControlFlowGraph(ctx, *func_def);

  TestResultUnionAnalysis analysis(ctx);
  std::vector<std::optional<TestResultUnionAnalysis::State>> block_states =
      RunBackwardDataflow(cfg, analysis);

  ASSERT_THAT(block_states, SizeIs(5));

  auto first = cfg.first;
  EXPECT_THAT(block_states[first.id()],
              Optional(Field(&TestResultUnionAnalysis::State::results,
                             UnorderedElementsAre("1", "2"))));

  auto then_branch = cfg.get(first).next[0];
  EXPECT_THAT(block_states[then_branch.id()],
              Optional(Field(&TestResultUnionAnalysis::State::results,
                             UnorderedElementsAre("1"))));

  auto else_branch = cfg.get(first).next[1];
  EXPECT_THAT(block_states[else_branch.id()],
              Optional(Field(&TestResultUnionAnalysis::State::results,
                             UnorderedElementsAre("2"))));

  auto post_if = cfg.get(then_branch).next[0];
  EXPECT_THAT(
      block_states[post_if.id()],
      Optional(Field(&TestResultUnionAnalysis::State::results, IsEmpty())));

  auto last = cfg.last;
  EXPECT_THAT(
      block_states[last.id()],
      Optional(Field(&TestResultUnionAnalysis::State::results, IsEmpty())));
}

TEST(RunBackwardDataflowTest, Loop) {
  std::string code = StringWithNull(R"(
    let factors = (n: Int32) -> Int32 {
      let i: Int32 = 0
      loop {
        if i == n {
          break
        }

        i = i + 1
      }
      return 0
    }
  )");

  SyntaxContext ctx;
  Lexer lexer(code);
  Parser parser(ctx, code, lexer);
  auto func_def = std::get<std::optional<FuncDefStmt>>(parser.ParseFuncDef());
  auto cfg = BuildControlFlowGraph(ctx, *func_def);

  TestResultUnionAnalysis analysis(ctx);
  std::vector<std::optional<TestResultUnionAnalysis::State>> block_states =
      RunBackwardDataflow(cfg, analysis);

  ASSERT_THAT(block_states, SizeIs(6));

  auto first = cfg.first;
  EXPECT_THAT(block_states[first.id()],
              Optional(Field(&TestResultUnionAnalysis::State::results,
                             UnorderedElementsAre("0"))));

  auto loop_branch = cfg.get(first).next[0];
  EXPECT_THAT(block_states[loop_branch.id()],
              Optional(Field(&TestResultUnionAnalysis::State::results,
                             UnorderedElementsAre("0"))));

  auto then_branch = cfg.get(loop_branch).next[0];
  EXPECT_THAT(block_states[then_branch.id()],
              Optional(Field(&TestResultUnionAnalysis::State::results,
                             UnorderedElementsAre("0"))));

  auto else_branch = cfg.get(loop_branch).next[1];
  EXPECT_THAT(block_states[else_branch.id()],
              Optional(Field(&TestResultUnionAnalysis::State::results,
                             UnorderedElementsAre("0"))));

  auto post_loop = cfg.get(then_branch).next[0];
  EXPECT_THAT(block_states[post_loop.id()],
              Optional(Field(&TestResultUnionAnalysis::State::results,
                             UnorderedElementsAre("0"))));

  auto last = cfg.last;
  EXPECT_THAT(
      block_states[last.id()],
      Optional(Field(&TestResultUnionAnalysis::State::results, IsEmpty())));
}

}  // namespace
}  // namespace lucid
