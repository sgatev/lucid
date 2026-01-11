#include "lucid/reg.h"

#include <iostream>
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
      let k: Int32 = 1
      if b {
        let i: Int32 = 0
        loop {
          if i == n {
            break
          }

          i = i + k
        }
      } else {
        let j: Int32 = 0
        loop {
          if j == m {
            break
          }

          j = j + k
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
  StringIndex::Ref k_var = ctx.AddIdent("k");

auto x = BuildInterferenceGraph(ctx, cfg);

for (const auto& [k, vs] : x) {
  for (const auto& v : vs) {
    std::cout << ctx.DerefIdent(k) << " -> " << ctx.DerefIdent(v) << std::endl;
  }
}

  EXPECT_THAT(x,
              UnorderedElementsAre(
                  Pair(b_var, UnorderedElementsAre(n_var, m_var, k_var)),
                  Pair(n_var, UnorderedElementsAre(b_var, m_var, i_var, k_var)),
                  Pair(m_var, UnorderedElementsAre(b_var, n_var, j_var, k_var)),
                  Pair(i_var, UnorderedElementsAre(n_var, k_var)),
                  Pair(j_var, UnorderedElementsAre(m_var, k_var)),
                  Pair(k_var, UnorderedElementsAre(b_var, m_var, n_var, i_var, j_var))));
}

TEST(ColorInterferenceGraphTest, Branching) {
  std::string code = StringWithNull(R"(
    let factors = (b: Bool, n: Int32, m: Int32) -> Int32 {
      let k: Int32 = 1
      if b {
        let i: Int32 = 0
        loop {
          if i == n {
            break
          }

          i = i + k
        }
      } else {
        let j: Int32 = 0
        loop {
          if j == m {
            break
          }

          j = j + k
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
  auto ig = BuildInterferenceGraph(ctx, cfg);

  StringIndex::Ref i_var = ctx.AddIdent("i");
  StringIndex::Ref j_var = ctx.AddIdent("j");
  StringIndex::Ref k_var = ctx.AddIdent("k");

  EXPECT_THAT(
      ColorInterferenceGraph(ctx, cfg, ig, 10),
      UnorderedElementsAre(Pair(i_var, 0), Pair(j_var, 0), Pair(k_var, 1)));
}

}  // namespace
}  // namespace lucid
