#include <cassert>
#include <cstddef>
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
#include "lucid/am/reg.h"
#include "lucid/am/state.h"
#include "lucid/am/translator.h"
#include "lucid/core/benchmarking/benchmarking.h"
#include "lucid/core/container/hash_map.h"
#include "lucid/core/container/hash_set.h"
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

// A function of `count` values, each one feeding only the next.
//
// A value dies where the one after it is born, so few are ever live at once
// and the interference graph stays sparse: what grows with `count` is the
// number of registers in the graph rather than the edges between them.
std::string ChainedValues(int count) {
  std::string code = "fun main(): Int32 {\n  val v0: Int32 = 1\n";
  for (int i = 1; i < count; ++i) {
    code += std::format("  val v{}: Int32 = v{} + {}\n", i, i - 1, i);
  }
  code += std::format("  return v{}\n}}\n", count - 1);
  return code;
}

// A function of `count` values that are all live at once.
//
// Every value is read after the last one is written, so each interferes with
// every other: the graph is dense, and with only ten registers to colour it
// with, the allocator has to spill.
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
  AbstractMachineControlFlowGraph am_cfg = GenerateAbstractMachineFunction(
      /*am_cfgs=*/{}, syn_ctx, syn_cfg, am_state);
  for (auto& block : am_cfg.Blocks()) {
    OptimizeAbstractMachineInstructions(block.instructions);
  }

  run(am_cfg, am_state);
}

// Measures colouring on its own, over a graph that is built once.
//
// Colouring reads the graph and returns a map of its own, so an iteration
// leaves nothing behind for the next one to find.
void BenchmarkColoring(BenchmarkState& state, std::string_view snippet) {
  WithAbstractMachineFunction(
      snippet, [&](AbstractMachineControlFlowGraph& am_cfg,
                   AbstractMachineState& am_state) {
        SpillRegisters(am_cfg, am_state, kRegistersCount);
        const HashMap<Reg, HashSet<Reg>> am_ig = BuildInterferenceGraph(am_cfg);

        for (auto _ : state) {
          DoNotOptimize(
              ColorInterferenceGraph(am_cfg, am_ig, kRegistersCount).size());
        }
      });
}

// Measures building the interference graph on its own, over a function that
// is spilled once.
//
// Building reads the function and returns a graph of its own, so an iteration
// leaves nothing behind for the next one to find.
void BenchmarkBuildingGraph(BenchmarkState& state, std::string_view snippet) {
  WithAbstractMachineFunction(
      snippet, [&](AbstractMachineControlFlowGraph& am_cfg,
                   AbstractMachineState& am_state) {
        SpillRegisters(am_cfg, am_state, kRegistersCount);

        for (auto _ : state) {
          DoNotOptimize(BuildInterferenceGraph(am_cfg).size());
        }
      });
}

// Measures allocation whole: spilling, the graph, colouring and the rewrite.
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

          SpillRegisters(cfg, reg_state, kRegistersCount);
          const HashMap<Reg, HashSet<Reg>> ig = BuildInterferenceGraph(cfg);
          const HashMap<Reg, int> colors =
              ColorInterferenceGraph(cfg, ig, kRegistersCount);
          MergeRegisters(colors, cfg);
          DoNotOptimize(colors.size());
        }
      });
}

// Colouring over a sparse graph, at three sizes.
//
// The three read together: the ordering that colouring starts from picks the
// highest-scoring register by scanning every register still in the running,
// so the cost of one is expected to grow faster than the size of the graph.
BENCHMARK(ColorChain64) { BenchmarkColoring(state, ChainedValues(64)); }

BENCHMARK(ColorChain256) { BenchmarkColoring(state, ChainedValues(256)); }

BENCHMARK(ColorChain512) { BenchmarkColoring(state, ChainedValues(512)); }

// Colouring over a dense graph, where every register interferes with the rest.
BENCHMARK(ColorLive64) { BenchmarkColoring(state, LiveValues(64)); }

// Building the graph, over a sparse graph at two sizes and a dense one.
//
// What it costs is set by how many registers are live at once rather than by
// how many there are, so the dense case is the one that moves.
BENCHMARK(BuildIgChain256) {
  BenchmarkBuildingGraph(state, ChainedValues(256));
}

BENCHMARK(BuildIgChain512) {
  BenchmarkBuildingGraph(state, ChainedValues(512));
}

BENCHMARK(BuildIgLive64) { BenchmarkBuildingGraph(state, LiveValues(64)); }

// Allocation whole, for what a change to colouring is worth in context.
BENCHMARK(AllocateChain256) { BenchmarkAllocation(state, ChainedValues(256)); }

BENCHMARK(AllocateLive64) { BenchmarkAllocation(state, LiveValues(64)); }

}  // namespace
}  // namespace lucid
