#include "lucid/reg.h"

#include <optional>
#include <string>
#include <string_view>

#include "gmock/gmock.h"
#include "gtest/gtest.h"
#include "lucid/ast.h"
#include "lucid/cfg.h"
#include "lucid/lexer.h"
#include "lucid/parser.h"
#include "lucid/string_index.h"

namespace lucid {
namespace {

using namespace std::string_literals;

using ::testing::Pair;
using ::testing::UnorderedElementsAre;

std::string StringWithNull(std::string_view s) {
  std::string s_with_null(s);
  s_with_null.append("\0"s);
  return s_with_null;
}

TEST(BuildInterferenceGraphTest, Branching) {
  std::string code = StringWithNull(R"(
    let factors = (b: Bool, n: Int32, m: Int32) -> Int32 {
      if b {
        let i: Int32 = 0
        loop {
          if i == n {
            break
          }

          i = i + 1
        }
      } else {
        let j: Int32 = 0
        loop {
          if j == m {
            break
          }

          j = j + 1
        }
      }
      return 0
    }
  )");

  SyntaxContext ctx;
  Lexer lexer(code);
  Parser parser(ctx, code, lexer);
  auto func_def = std::get<std::optional<FuncDefStmt>>(parser.ParseFuncDef());
  auto cfg = BuildControlFlowGraph(ctx, *func_def);

  StringIndex::Ref b_var = ctx.AddIdent("b");
  StringIndex::Ref n_var = ctx.AddIdent("n");
  StringIndex::Ref m_var = ctx.AddIdent("m");
  StringIndex::Ref i_var = ctx.AddIdent("i");
  StringIndex::Ref j_var = ctx.AddIdent("j");

  EXPECT_THAT(BuildInterferenceGraph(ctx, cfg),
              UnorderedElementsAre(
                  Pair(b_var, UnorderedElementsAre(b_var, n_var, m_var)),
                  Pair(n_var, UnorderedElementsAre(b_var, n_var, m_var, i_var)),
                  Pair(m_var, UnorderedElementsAre(b_var, n_var, m_var, j_var)),
                  Pair(i_var, UnorderedElementsAre(n_var, i_var)),
                  Pair(j_var, UnorderedElementsAre(m_var, j_var))));
}

}  // namespace
}  // namespace lucid
