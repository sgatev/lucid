#include "lucid/am/reg.h"

#include <cstddef>
#include <optional>
#include <ranges>
#include <string>
#include <string_view>
#include <utility>
#include <variant>
#include <vector>

#include "lucid/am/abi.h"
#include "lucid/am/cfg.h"
#include "lucid/am/cfg_builder.h"
#include "lucid/am/instructions.h"
#include "lucid/am/liveness.h"
#include "lucid/am/opt.h"
#include "lucid/am/reg_programs.h"
#include "lucid/am/reg_set.h"
#include "lucid/am/state.h"
#include "lucid/am/translator.h"
#include "lucid/core/container/hash_map.h"
#include "lucid/core/testing/testing.h"
#include "lucid/syntax/ast.h"
#include "lucid/syntax/cfg.h"
#include "lucid/syntax/comp.h"
#include "lucid/syntax/context.h"
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
  // in: every register gets a colour, from the colours it was allowed, and no
  // two registers live at the same point share one.
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

    const RegisterColors colors =
        ColorRegisters(am_cfg, liveness, kRegistersCount);
    const auto color = [&](Reg reg) {
      const std::optional<int> color = colors.Get(reg);
      EXPECT_TRUE(color.has_value());
      if (color.has_value()) {
        EXPECT_TRUE(*color >= 19);
        EXPECT_TRUE(*color < 19 + kRegistersCount);
      }
      return color.value_or(-1);
    };
    // No two registers in `regs` share a colour.
    const auto expect_apart = [&](const RegBitSet& regs) {
      for (Reg reg : regs) {
        for (Reg other : regs) {
          if (other != reg) EXPECT_NE(color(reg), color(other));
        }
      }
    };

    // Every parameter is written where the function is entered, one after
    // another, so no two of them can share a colour whether or not the body
    // reads them.
    const auto register_params = am_cfg.params;
    for (std::size_t i = 0; i < register_params.size(); ++i) {
      for (std::size_t j = i + 1; j < register_params.size(); ++j) {
        EXPECT_NE(color(register_params[i]), color(register_params[j]));
      }
    }

    for (const auto& block : am_cfg.Blocks()) {
      // The walk backwards over the block starts from what is live where it
      // exits, which takes in what the branch reads there.
      RegBitSet live = LiveOut(liveness, block);
      if (block.branch_cond.has_value()) live.Insert(*block.branch_cond);
      expect_apart(live);

      for (const auto& inst : block.instructions | std::views::reverse) {
        // What an instruction writes is apart from everything live after it,
        // whether or not anything reads it.
        if (auto target = GetTargetRegister(inst); target.has_value()) {
          for (Reg reg : live) {
            if (reg != *target) EXPECT_NE(color(*target), color(reg));
          }
        }
        AbstractMachineLivenessAnalysis::TransferLive(live, inst);
        expect_apart(live);
      }

      // The phi functions settle on their results together where the block
      // starts, beside everything live into it.
      for (const auto& phi : block.phis) live.Insert(phi.dst);
      expect_apart(live);
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

// Early values held to the end of a long function, past many registers
// written after them.
TEST(ColoringTest, ColorsEarlyValuesHeldLong) {
  ExpectValidColoring(EarlyValuesHeldLong(/*count=*/200, /*held=*/8));
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

// What follows a loop with no way out is never reached, and is gone by the
// time the registers are coloured, so nothing it names needs a colour.
TEST(ColoringTest, ColorsAFunctionWithCodeNothingReaches) {
  ExpectValidColoring(R"(
    fun main(): Int32 {
      mut val i: Int32 = 0
      loop {
        if i > 3 {
          return i
        }
        mut i = i + 1
      }
      return i + 2
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

// A copy between two registers that took one colour copies that register
// onto itself once they are merged, and is dropped. One between registers
// that took two colours is kept.
TEST(Test, MergingDropsCopiesOfARegisterOntoItself) {
  const Reg a{1, RegSize32};
  const Reg b{2, RegSize32};
  const Reg c{3, RegSize32};

  AbstractMachineControlFlowGraphBuilder builder;
  const auto block = builder.AddBlock();
  builder.SetFirst(block);
  builder.SetLast(block);
  builder.AddInstruction(block, SetReg{.src_val = 5, .dst_reg = a});
  builder.AddInstruction(block, MoveReg{.src_reg = a, .dst_reg = b});
  builder.AddInstruction(block, MoveReg{.src_reg = b, .dst_reg = c});
  builder.AddInstruction(block, Return{.res_reg = c});
  AbstractMachineControlFlowGraph am_cfg = std::move(builder).Build();

  // `a` and `b` share 19, and `c` has 20.
  MergeRegisters(RegisterColors({RegisterColors::kNone, 19, 19, 20}), am_cfg);

  const Reg r19{19, RegSize32};
  const Reg r20{20, RegSize32};
  const auto& instructions = am_cfg.GetBlock(block).instructions;
  EXPECT_TRUE(
      std::vector<Instruction>(instructions.begin(), instructions.end()) ==
      (std::vector<Instruction>{
          SetReg{.src_val = 5, .dst_reg = r19},
          MoveReg{.src_reg = r19, .dst_reg = r20},
          Return{.res_reg = r20},
      }));
}

}  // namespace
}  // namespace lucid
