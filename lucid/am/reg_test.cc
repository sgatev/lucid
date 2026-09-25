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

// `crossing` values written on one side of a branch and all read after it,
// so that a phi function waits for each of them where the sides meet, and
// `inside` more live only within that side, to crowd the registers there.
//
// What crosses the branch is what is read furthest ahead, so it is what the
// spilling takes, which is to say a register a phi function reads.
std::string BranchingValues(int crossing, int inside) {
  std::string code = "fun main(): Int32 {\n  val a: Int32 = 1\n";
  for (int i = 0; i < crossing; ++i) {
    code += std::format("  val v{}: Int32 = {}\n", i, i + 1);
  }

  code += "  if a == 1 {\n";
  for (int i = 0; i < inside; ++i) {
    code += std::format("    val w{}: Int32 = {}\n", i, i + 1);
  }
  if (inside > 0) {
    code += "    val t0: Int32 = w0\n";
    for (int i = 1; i < inside; ++i) {
      code += std::format("    val t{}: Int32 = t{} + w{}\n", i, i - 1, i);
    }
    for (int i = 0; i < crossing; ++i) {
      code += std::format("    &v{} = v{} + t{}\n", i, i, inside - 1);
    }
  }
  code += "  } else {\n";
  for (int i = 0; i < crossing; ++i) {
    code += std::format("    &v{} = v{} - 1\n", i, i);
  }
  code += "  }\n";

  code += "  val sum0: Int32 = v0\n";
  for (int i = 1; i < crossing; ++i) {
    code += std::format("  val sum{}: Int32 = sum{} + v{}\n", i, i - 1, i);
  }
  code += std::format("  return sum{}\n}}\n", crossing - 1);
  return code;
}

// `carried` values written on every turn of a loop and read after it, so
// that each has a phi function where the loop is entered, taking one value
// from before it and one from the turn before.
std::string LoopCarriedValues(int carried) {
  std::string code = "fun main(): Int32 {\n  val i: Int32 = 0\n";
  for (int k = 0; k < carried; ++k) {
    code += std::format("  val v{}: Int32 = {}\n", k, k);
  }

  code += "  loop {\n    if i == 3 {\n      break\n    }\n";
  for (int k = 0; k < carried; ++k) {
    code += std::format("    &v{} = v{} + 1\n", k, k);
  }
  code += "    &i = i + 1\n  }\n";

  code += "  val s0: Int32 = v0\n";
  for (int k = 1; k < carried; ++k) {
    code += std::format("  val s{}: Int32 = s{} + v{}\n", k, k - 1, k);
  }
  code += std::format("  return s{}\n}}\n", carried - 1);
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
    const AbstractMachineLiveness liveness =
        SpillRegisters(am_cfg, am_state, kRegistersCount);

    const InterferenceGraph am_ig = BuildInterferenceGraph(am_cfg, liveness);
    const HashMap<Reg, int> colors =
        ColorInterferenceGraph(am_cfg, am_ig, kRegistersCount);

    for (Reg reg : am_ig.Regs()) {
      const std::optional<const int&> color = colors.Get(reg);
      EXPECT_TRUE(color.has_value());
      if (!color.has_value()) continue;

      EXPECT_TRUE(*color >= 19);
      EXPECT_TRUE(*color < 19 + kRegistersCount);

      for (Reg neighbour : am_ig.Neighbours(reg)) {
        const std::optional<const int&> neighbour_color = colors.Get(neighbour);
        if (!neighbour_color.has_value()) continue;
        EXPECT_NE(*color, *neighbour_color);
      }
    }
  }
};

TEST(ColoringTest, ColorsBranchingValuesThatSpill) {
  ExpectValidColoring(BranchingValues(/*crossing=*/9, /*inside=*/0));
  ExpectValidColoring(BranchingValues(/*crossing=*/10, /*inside=*/12));
}

TEST(ColoringTest, ColorsLoopCarriedValuesThatSpill) {
  ExpectValidColoring(LoopCarriedValues(/*carried=*/8));
}

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
