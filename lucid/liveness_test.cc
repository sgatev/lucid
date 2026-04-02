#include "lucid/liveness.h"

#include <optional>
#include <string>
#include <string_view>
#include <vector>

#include "gmock/gmock.h"
#include "gtest/gtest.h"
#include "lucid/ast.h"
#include "lucid/cfg.h"
#include "lucid/core/dataflow/dataflow.h"
#include "lucid/parser.h"
#include "lucid/syntax/lexer.h"

namespace lucid {
namespace {

using namespace std::string_literals;

using ::testing::AllOf;
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

TEST(LivenessAnalysisTest, Conditional) {
  std::string code = StringWithNull(R"(
    let foo = (a: Int32, b: Int32, c: Bool) -> Int32 {
      let m: Int32 = 0
      if c {
        m = a
      }
      return b + m
    }
  )");

  SyntaxContext ctx;
  Lexer lexer(code);
  Parser parser(ctx, code, lexer);
  auto func_def = std::get<std::optional<FuncDefStmt>>(parser.ParseFuncDef());
  auto cfg = BuildControlFlowGraph(ctx, *func_def);

  ControlFlowGraphAnalysis<LivenessAnalysis> analysis(cfg, ctx);
  std::vector<std::optional<LivenessAnalysis::State>> block_states =
      RunBackwardDataflow(cfg, analysis);

  ASSERT_THAT(block_states, SizeIs(4));

  auto a = ctx.AddIdent("a");
  auto b = ctx.AddIdent("b");
  auto c = ctx.AddIdent("c");
  auto m = ctx.AddIdent("m");

  auto first = cfg.first;
  EXPECT_THAT(block_states[first.id()],
              AllOf(Optional(Field(&LivenessAnalysis::State::live_in,
                                   UnorderedElementsAre(a, b, c))),
                    Optional(Field(&LivenessAnalysis::State::live_out,
                                   UnorderedElementsAre(a, b, m)))));

  auto then_branch = cfg.get(first).next[0];
  EXPECT_THAT(block_states[then_branch.id()],
              AllOf(Optional(Field(&LivenessAnalysis::State::live_in,
                                   UnorderedElementsAre(a, b))),
                    Optional(Field(&LivenessAnalysis::State::live_out,
                                   UnorderedElementsAre(b, m)))));

  auto post_if = cfg.get(then_branch).next[0];
  EXPECT_THAT(
      block_states[post_if.id()],
      AllOf(Optional(Field(&LivenessAnalysis::State::live_in,
                           UnorderedElementsAre(b, m))),
            Optional(Field(&LivenessAnalysis::State::live_out, IsEmpty()))));

  auto last = cfg.last;
  EXPECT_THAT(
      block_states[last.id()],
      AllOf(Optional(Field(&LivenessAnalysis::State::live_in, IsEmpty())),
            Optional(Field(&LivenessAnalysis::State::live_out, IsEmpty()))));
}

}  // namespace
}  // namespace lucid
