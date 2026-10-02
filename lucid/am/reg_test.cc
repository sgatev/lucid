#include "lucid/am/reg.h"

#include <cstddef>
#include <format>
#include <optional>
#include <string>
#include <string_view>
#include <utility>
#include <variant>

#include "lucid/am/abi.h"
#include "lucid/am/cfg.h"
#include "lucid/am/ig.h"
#include "lucid/am/instructions.h"
#include "lucid/am/opt.h"
#include "lucid/am/reg_programs.h"
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

// What the ARM64 backend hands the allocator: the registers it has to colour
// with, and the call it has to pass arguments by.
constexpr int kRegistersCount = 10;
constexpr CallingConvention kCallingConvention = {
    .max_register_args = kRegistersCount,
    .stack_arg_size = 8,
};

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
    auto am_cfg_or_error = GenerateAbstractMachineFunction(
        /*am_cfgs=*/{}, syn_ctx, syn_cfg, am_state);
    ASSERT_TRUE(am_cfg_or_error.has_value());

    AbstractMachineControlFlowGraph am_cfg = std::move(am_cfg_or_error).value();
    OptimizeAbstractMachineFunction(am_cfg);
    LowerCallingConvention(am_cfg, kCallingConvention);
    const AbstractMachineLiveness liveness =
        SpillRegisters(am_cfg, am_state, kRegistersCount);

    const InterferenceGraph am_ig = BuildInterferenceGraph(am_cfg, liveness);
    const HashMap<Reg, int> colors =
        ColorInterferenceGraph(am_cfg, am_ig, kRegistersCount);

    // Every parameter is live where the function is entered, because that is
    // where the caller leaves it, so no two of them can share a colour
    // whether or not the body reads them.
    const auto register_params = am_cfg.params;
    for (std::size_t i = 0; i < register_params.size(); ++i) {
      const std::optional<const int&> color = colors.Get(register_params[i]);
      ASSERT_TRUE(color.has_value());

      for (std::size_t j = i + 1; j < register_params.size(); ++j) {
        const std::optional<const int&> other = colors.Get(register_params[j]);
        ASSERT_TRUE(other.has_value());
        EXPECT_NE(*color, *other);
      }
    }

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

// As many values crossing a branch as there are registers to hold them.
//
// Each has a phi function where the sides meet, and each of those takes the
// register its argument already has, so what crosses needs no more registers
// than it is already in. Holding a result apart from its own argument is
// what once made these want one register more than there are, and what made
// whether they were coloured turn on how crowded the sides were.
TEST(ColoringTest, ColorsAsManyValuesCrossingABranchAsThereAreRegisters) {
  ExpectValidColoring(BranchingValues(/*crossing=*/kRegistersCount,
                                      /*inside=*/0));
}

TEST(ColoringTest, ColorsCrossingValuesBesideACrowdedSide) {
  ExpectValidColoring(BranchingValues(/*crossing=*/kRegistersCount,
                                      /*inside=*/8));
}

// More values crossing a branch than there are registers to hold them.
//
// A phi function held each of its arguments in a register to the end of
// every block it came from, which no amount of spilling could take back so
// long as the phi was there to read them. Putting the result and the
// arguments away in one slot between them leaves nothing for the phi to
// settle, and the room the spilling meant to buy is bought.
TEST(ColoringTest, ColorsMoreValuesCrossingABranchThanThereAreRegisters) {
  ExpectValidColoring(BranchingValues(/*crossing=*/kRegistersCount + 1,
                                      /*inside=*/0));
  ExpectValidColoring(BranchingValues(/*crossing=*/kRegistersCount + 10,
                                      /*inside=*/4));
  ExpectValidColoring(BranchingValues(/*crossing=*/40, /*inside=*/0));
}

// The same where the values are carried around a loop rather than across a
// branch, which is a phi function reading its own result from the turn
// before.
TEST(ColoringTest, ColorsMoreLoopCarriedValuesThanThereAreRegisters) {
  ExpectValidColoring(LoopCarriedValues(/*carried=*/kRegistersCount + 1));
  ExpectValidColoring(LoopCarriedValues(/*carried=*/30));
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
      mut val a: Int32 = 12
      mut val b: Int32 = 8
      loop {
        if a == b {
          break
        }
        if a > b {
          mut a = a - b
        } else {
          mut b = b - a
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
      mut val v: Int32 = 0
      if a == 3 {
        if a == 2 {
          if a == 1 { mut v = 1 } else { mut v = 2 }
        } else {
          if a == 1 { mut v = 3 } else { mut v = 4 }
        }
      } else {
        if a == 2 {
          if a == 1 { mut v = 5 } else { mut v = 6 }
        } else {
          if a == 1 { mut v = 7 } else { mut v = 8 }
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

TEST(ColoringTest, ColorsEveryParameterThereIsARegisterFor) {
  ExpectValidColoring(ManyParameters(kRegistersCount));
}

TEST(ColoringTest, ColorsParametersThatArriveOnTheStack) {
  ExpectValidColoring(ManyParameters(kRegistersCount + 1));
  ExpectValidColoring(ManyParameters(32));
}

// A parameter the body never reads is live where the function is entered all
// the same, so it cannot share a register with one that is read: the entry
// fills them one after another, and the fill would write over what it shared
// with.
TEST(ColoringTest, ColorsParametersThatAreNotRead) {
  ExpectValidColoring(R"(
    fun main(a: Int32, b: Int32, c: Int32): Int32 {
      return b
    }
  )");
}

}  // namespace
}  // namespace lucid
