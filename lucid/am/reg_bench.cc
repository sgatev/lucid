#include <cassert>
#include <cstddef>
#include <cstdint>
#include <expected>
#include <optional>
#include <string>
#include <string_view>
#include <utility>
#include <variant>

#include "lucid/am/abi.h"
#include "lucid/am/cfg.h"
#include "lucid/am/liveness.h"
#include "lucid/am/opt.h"
#include "lucid/am/reg.h"
#include "lucid/am/reg_programs.h"
#include "lucid/am/state.h"
#include "lucid/am/translator.h"
#include "lucid/core/benchmarking/benchmarking.h"
#include "lucid/core/container/hash_map.h"
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

// Everything the allocator needs, built from a snippet.
//
// The syntax context holds a reference into itself, so it cannot be moved out
// of here; the benchmark runs inside instead.
template <typename BenchmarkT>
void WithAbstractMachineFunction(std::string_view snippet, BenchmarkT run) {
  std::string code(snippet);
  code.append("\0"s);

  SyntaxContext syn_ctx;
  std::expected<std::optional<Def>, ParserError> def_or_error =
      Parser(syn_ctx, code, Lexer(code)).Parse();
  assert(def_or_error.has_value());

  std::optional<Def> maybe_def = std::move(def_or_error).value();
  assert(maybe_def.has_value());

  Def def = std::move(maybe_def).value();
  assert(std::holds_alternative<FuncDefStmt>(def));
  auto& func_def = std::get<FuncDefStmt>(def);

  auto infer_types_res = InferExprTypes(syn_ctx, func_def);
  assert(infer_types_res.has_value());
  auto check_comp_res = CheckComp(syn_ctx, func_def);
  assert(check_comp_res.has_value());

  SyntaxControlFlowGraph syn_cfg = BuildControlFlowGraph(syn_ctx, func_def);
  ConvertToStaticSingleAssignment(syn_ctx, syn_cfg);

  AbstractMachineState am_state;
  auto am_cfg_or_error = GenerateAbstractMachineFunction(
      /*am_cfgs=*/{}, syn_ctx, syn_cfg, am_state);
  assert(am_cfg_or_error.has_value());

  AbstractMachineControlFlowGraph am_cfg = std::move(am_cfg_or_error).value();
  OptimizeAbstractMachineFunction(am_cfg);
  LowerCallingConvention(am_cfg, kCallingConvention);

  run(am_cfg, am_state);
}

// Measures colouring on its own, over a function that is spilled once.
//
// Colouring reads the function and what is live in it, and returns colours of
// its own, so an iteration leaves nothing behind for the next one to find.
void BenchmarkColoring(BenchmarkState& state, std::string_view snippet) {
  WithAbstractMachineFunction(
      snippet, [&](AbstractMachineControlFlowGraph& am_cfg,
                   AbstractMachineState& am_state) {
        const AbstractMachineLiveness liveness =
            SpillRegisters(am_cfg, am_state, kRegistersCount);

        for (auto _ : state) {
          DoNotOptimize(ColorRegisters(am_cfg, liveness, kRegistersCount));
        }
      });

  state.SetBytesProcessed(std::int64_t(state.MaxIterations()) *
                          std::int64_t(snippet.size()));
  state.SetLinesProcessed(std::int64_t(state.MaxIterations()) *
                          CountLines(snippet));
}

// Measures allocation whole: spilling, colouring and the rewrite.
//
// Spilling and the rewrite both change the function, so each iteration works
// on a copy of it. The copy is measured along with the rest, which is what
// makes this the coarser of the two measurements.
void BenchmarkAllocation(BenchmarkState& state, std::string_view snippet) {
  WithAbstractMachineFunction(
      snippet, [&](AbstractMachineControlFlowGraph& am_cfg,
                   AbstractMachineState& am_state) {
        for (auto _ : state) {
          AbstractMachineControlFlowGraph cfg = am_cfg;
          AbstractMachineState reg_state = am_state;

          const AbstractMachineLiveness liveness =
              SpillRegisters(cfg, reg_state, kRegistersCount);
          const RegisterColors colors =
              ColorRegisters(cfg, liveness, kRegistersCount);
          MergeRegisters(colors, cfg);
          DoNotOptimize(colors);
        }
      });

  state.SetBytesProcessed(std::int64_t(state.MaxIterations()) *
                          std::int64_t(snippet.size()));
  state.SetLinesProcessed(std::int64_t(state.MaxIterations()) *
                          CountLines(snippet));
}

// Colouring over a chain of values, few of them live at once, at three sizes.
//
// The three read together: colouring takes each register once, where it is
// written, so the cost is expected to grow with the size of the function and
// no faster.
BENCHMARK(ColorChain64) { BenchmarkColoring(state, ChainedValues(64)); }

BENCHMARK(ColorChain256) { BenchmarkColoring(state, ChainedValues(256)); }

BENCHMARK(ColorChain512) { BenchmarkColoring(state, ChainedValues(512)); }

// Colouring over values that are all live at once, until the spilling puts
// all but as many as there are registers away.
BENCHMARK(ColorLive64) { BenchmarkColoring(state, LiveValues(64)); }

// Allocation whole, for what a change to colouring is worth in context.
BENCHMARK(AllocateChain256) { BenchmarkAllocation(state, ChainedValues(256)); }

BENCHMARK(AllocateLive64) { BenchmarkAllocation(state, LiveValues(64)); }

// Colouring over a run of branches with nothing to spill, so that what is
// measured includes a phi function at every place the sides meet, which
// straight-line code never has. Seven values crossing is as many as fit: the
// condition and what each side adds take the rest.
BENCHMARK(ColorDiamonds30) {
  BenchmarkColoring(state, Diamonds(/*count=*/30, /*crossing=*/7));
}

// Allocation over runs of branches with more crossing each of them than
// there are registers, at two lengths. Read together they say how allocation
// grows with the length of the function, which spilling once made grow far
// faster than the function did, by searching all of it again after each value
// it put away.
BENCHMARK(AllocateDiamonds15) {
  BenchmarkAllocation(state, Diamonds(/*count=*/15, /*crossing=*/12));
}

BENCHMARK(AllocateDiamonds30) {
  BenchmarkAllocation(state, Diamonds(/*count=*/30, /*crossing=*/12));
}

// Allocation over one branch that more values cross than there are
// registers, which spills what a phi function reads.
BENCHMARK(AllocateBranching40) {
  BenchmarkAllocation(state, BranchingValues(/*crossing=*/40, /*inside=*/0));
}

// Allocation over a loop carrying more values round than there are
// registers, each with a phi function where the loop is entered.
BENCHMARK(AllocateLoopCarried30) {
  BenchmarkAllocation(state, LoopCarriedValues(/*carried=*/30));
}

// Allocation over a long function that holds a few early values to its end,
// at two lengths, one four times the other. Read together they say whether
// allocation grows with the length of the function and no faster, which it
// once did not: what was live where was held by register ID, and cost every
// block as much as the highest register live there.
BENCHMARK(AllocateHeldLong500) {
  BenchmarkAllocation(state, EarlyValuesHeldLong(/*count=*/500, /*held=*/8));
}

BENCHMARK(AllocateHeldLong2000) {
  BenchmarkAllocation(state, EarlyValuesHeldLong(/*count=*/2000, /*held=*/8));
}

}  // namespace
}  // namespace lucid
