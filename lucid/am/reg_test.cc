#include "lucid/am/reg.h"

#include <format>
#include <optional>
#include <string>
#include <string_view>
#include <utility>
#include <variant>

#include "lucid/am/cfg.h"
#include "lucid/am/ig.h"
#include "lucid/am/instructions.h"
#include "lucid/am/opt.h"
#include "lucid/am/translator.h"
#include "lucid/core/container/hash_map.h"
#include "lucid/core/container/hash_set.h"
#include "lucid/core/testing/testing.h"
#include "lucid/syntax/ast.h"
#include "lucid/syntax/cfg.h"
#include "lucid/syntax/comp.h"
#include "lucid/syntax/lexer.h"
#include "lucid/syntax/parser.h"
#include "lucid/syntax/ssa.h"
#include "lucid/syntax/type.h"

namespace lucid {
namespace {

using namespace std::string_literals;

// The registers the ARM64 backend hands the allocator.
constexpr int kRegistersCount = 10;

// A chain of values, each dying as the next is born.
std::string ChainedValues(int count) {
  std::string code = "fun main(): Int32 {\n  val v0: Int32 = 1\n";
  for (int i = 1; i < count; ++i) {
    code += std::format("  val v{}: Int32 = v{} + {}\n", i, i - 1, i);
  }
  code += std::format("  return v{}\n}}\n", count - 1);
  return code;
}

// Values that are all live at once, so that the allocator has to spill.
std::string LiveValues(int count) {
  std::string code = "fun main(): Int32 {\n";
  for (int i = 0; i < count; ++i) {
    code += std::format("  val v{}: Int32 = {}\n", i, i);
  }
  code += "  val sum: Int32 = v0\n";
  for (int i = 1; i < count; ++i) {
    code += std::format("  val sum{}: Int32 = sum + v{}\n", i, i);
  }
  code += std::format("  return sum{}\n}}\n", count - 1);
  return code;
}

class ColoringTest : public Test {
 protected:
  // What a colouring has to hold for, whatever order it takes the registers
  // in: every register in the graph gets a colour, from the colours it was
  // allowed, and no two registers that interfere share one.
  void ExpectValidColoring(std::string_view source) {
    std::string code(source);
    code.append("\0"s);

    SyntaxContext syn_ctx;
    auto def_or_error = Parser(syn_ctx, code, Lexer(code)).Parse();
    ASSERT_TRUE(def_or_error.has_value());
    std::optional<Def> maybe_def = std::move(def_or_error).value();
    ASSERT_TRUE(maybe_def.has_value());

    Def def = std::move(maybe_def).value();
    ASSERT_TRUE(std::holds_alternative<FuncDefStmt>(def));
    auto& func_def = std::get<FuncDefStmt>(def);

    ASSERT_TRUE(InferExprTypes(syn_ctx, func_def).has_value());
    ASSERT_TRUE(CheckComp(syn_ctx, func_def).has_value());

    SyntaxControlFlowGraph syn_cfg = BuildControlFlowGraph(syn_ctx, func_def);
    ConvertToStaticSingleAssignment(syn_ctx, syn_cfg);

    AbstractMachineState am_state;
    AbstractMachineControlFlowGraph am_cfg = GenerateAbstractMachineFunction(
        /*am_cfgs=*/{}, syn_ctx, syn_cfg, am_state);
    OptimizeAbstractMachineFunction(am_cfg);
    SpillRegisters(am_cfg, am_state, kRegistersCount);

    const HashMap<Reg, HashSet<Reg>> am_ig = BuildInterferenceGraph(am_cfg);
    const HashMap<Reg, int> colors =
        ColorInterferenceGraph(am_cfg, am_ig, kRegistersCount);

    for (const auto& [reg, neighbours] : am_ig) {
      const std::optional<const int&> color = colors.Get(reg);
      EXPECT_TRUE(color.has_value());
      if (!color.has_value()) continue;

      EXPECT_TRUE(*color >= 19);
      EXPECT_TRUE(*color < 19 + kRegistersCount);

      for (const Reg& neighbour : neighbours) {
        const std::optional<const int&> neighbour_color = colors.Get(neighbour);
        if (!neighbour_color.has_value()) continue;
        EXPECT_NE(*color, *neighbour_color);
      }
    }
  }
};

TEST(ColoringTest, ColorsASingleReturn) {
  ExpectValidColoring(R"(
    fun main(): Int32 {
      return 2 + 3
    }
  )");
}

TEST(ColoringTest, ColorsBranches) {
  ExpectValidColoring(R"(
    fun main(): Int32 {
      val a: Int32 = 12
      val b: Int32 = 8
      loop {
        if a == b {
          break
        }
        if a > b {
          &a = a - b
        } else {
          &b = b - a
        }
      }
      return a
    }
  )");
}

// Branches nested this deep leave blocks holding phi functions and no
// instructions at all, whose registers need colours like any other.
TEST(ColoringTest, ColorsNestedBranches) {
  ExpectValidColoring(R"(
    fun main(): Int32 {
      val a: Int32 = 3
      val v: Int32 = 0
      if a == 3 {
        if a == 2 {
          if a == 1 { &v = 1 } else { &v = 2 }
        } else {
          if a == 1 { &v = 3 } else { &v = 4 }
        }
      } else {
        if a == 2 {
          if a == 1 { &v = 5 } else { &v = 6 }
        } else {
          if a == 1 { &v = 7 } else { &v = 8 }
        }
      }
      return v
    }
  )");
}

TEST(ColoringTest, ColorsAChainOfValues) {
  ExpectValidColoring(ChainedValues(128));
}

TEST(ColoringTest, ColorsValuesThatAreAllLiveAtOnce) {
  ExpectValidColoring(LiveValues(64));
}

}  // namespace
}  // namespace lucid
