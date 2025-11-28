#include "lucid/cfg_order.h"

#include <optional>
#include <string>
#include <string_view>

#include "gmock/gmock.h"
#include "gtest/gtest.h"
#include "lucid/ast.h"
#include "lucid/cfg.h"
#include "lucid/lexer.h"
#include "lucid/parser.h"

namespace lucid {
namespace {

using namespace std::string_literals;

using ::testing::ElementsAre;
using ::testing::Field;
using ::testing::IsEmpty;
using ::testing::Optional;
using ::testing::SizeIs;

std::string StringWithNull(std::string_view s) {
  std::string s_with_null(s);
  s_with_null.append("\0"s);
  return s_with_null;
}

TEST(RunBackwardDataflowTest, Simple) {
  std::string code = StringWithNull(R"(
    let foo = () -> Int32 {
      return 21
    }
  )");

  SyntaxContext ctx;
  Lexer lexer(code);
  Parser parser(ctx, code, lexer);
  auto func_def = std::get<std::optional<FuncDefStmt>>(parser.ParseFuncDef());
  auto cfg = BuildControlFlowGraph(ctx, *func_def);

  EXPECT_THAT(ComputeReversePostOrder(cfg), ElementsAre(0, 1));
}

TEST(RunBackwardDataflowTest, Conditional) {
  std::string code = StringWithNull(R"(
    let foo = (c: Bool) -> Int32 {
      let res: Int32 = 0
      if c {
        res = 1
      } else {
        res = 2
      }
      return res
    }
  )");

  SyntaxContext ctx;
  Lexer lexer(code);
  Parser parser(ctx, code, lexer);
  auto func_def = std::get<std::optional<FuncDefStmt>>(parser.ParseFuncDef());
  auto cfg = BuildControlFlowGraph(ctx, *func_def);

  EXPECT_THAT(ComputeReversePostOrder(cfg), ElementsAre(0, 4, 3, 1, 2));
}

TEST(RunBackwardDataflowTest, Loop) {
  std::string code = StringWithNull(R"(
    let foo = () -> Int32 {
      let res: Int32 = 0
      loop {
        if res > 5 {
          break
        }
        res = res + 1
      }
      return res
    }
  )");

  SyntaxContext ctx;
  Lexer lexer(code);
  Parser parser(ctx, code, lexer);
  auto func_def = std::get<std::optional<FuncDefStmt>>(parser.ParseFuncDef());
  auto cfg = BuildControlFlowGraph(ctx, *func_def);

  EXPECT_THAT(ComputeReversePostOrder(cfg), ElementsAre(0, 4, 3, 1, 5, 2));
}

}  // namespace
}  // namespace lucid
