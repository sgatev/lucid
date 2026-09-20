#include <cassert>
#include <cstddef>
#include <cstdint>
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
#include "lucid/arm64/assembler.h"
#include "lucid/arm64/translator.h"
#include "lucid/core/benchmarking/benchmarking.h"
#include "lucid/core/container/hash_map.h"
#include "lucid/core/container/hash_set.h"
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

// The registers the ARM64 backend has to colour with.
constexpr int kRegistersCount = 10;

// A straight line of values, each read once by the next: one block, and an
// instruction to emit per value.
std::string ChainedValues(int count) {
  std::string code = "fun main(): Int32 {\n  val v0: Int32 = 1\n";
  for (int i = 1; i < count; ++i) {
    code += std::format("  val v{}: Int32 = v{} + {}\n", i, i - 1, i);
  }
  code += std::format("  return v{}\n}}\n", count - 1);
  return code;
}

// A run of branches, each assigning the same variables on both sides.
//
// Every branch is two blocks joining into a third, and the phi functions at
// the join turn into moves that the backend has to order. This grows the
// blocks and the phis rather than the instructions within a block.
std::string Branches(int count, int vars) {
  std::string code = "fun main(): Int32 {\n";
  for (int v = 0; v < vars; ++v) {
    code += std::format("  val x{}: Int32 = {}\n", v, v + 1);
  }
  for (int i = 0; i < count; ++i) {
    code += std::format("  if x0 > {} {{\n", i);
    for (int v = 0; v < vars; ++v) {
      code += std::format("    &x{} = x{} + 1\n", v, v);
    }
    code += "  } else {\n";
    for (int v = 0; v < vars; ++v) {
      code += std::format("    &x{} = x{} - 1\n", v, v);
    }
    code += "  }\n";
  }
  code += "  return x0\n}\n";
  return code;
}

// Values that are all live at once, so that the allocator spills and the
// backend has loads and stores to emit around the stack.
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

// Measures the backend over a function that is compiled up to it once.
//
// Generating reads the function and writes to the assembler it is given, so
// each iteration is given an assembler of its own: one shared between them
// would grow with every iteration and measure the growing rather than the
// generating.
void BenchmarkSnippet(BenchmarkState& state, std::string_view snippet) {
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
  OptimizeAbstractMachineFunction(am_cfg);
  SpillRegisters(am_cfg, am_state, kRegistersCount);
  const HashMap<Reg, HashSet<Reg>> am_ig = BuildInterferenceGraph(am_cfg);
  const HashMap<Reg, int> colors =
      ColorInterferenceGraph(am_cfg, am_ig, kRegistersCount);
  MergeRegisters(colors, am_cfg);

  const std::string func_name(syn_ctx.DerefIdent(func_def.name));

  for (auto _ : state) {
    arm64::Assembler assembler;
    GenerateArmAssemblyBinary(func_name, am_cfg.stack_slots, am_cfg, assembler);
    DoNotOptimize(assembler);
  }

  state.SetBytesProcessed(std::int64_t(state.MaxIterations()) *
                          std::int64_t(snippet.size()));
}

// Emission over one block, at two sizes.
BENCHMARK(ChainedValues256) { BenchmarkSnippet(state, ChainedValues(256)); }

BENCHMARK(ChainedValues512) { BenchmarkSnippet(state, ChainedValues(512)); }

// Emission over many blocks, where the phi functions at each join become
// moves, at two sizes.
BENCHMARK(Branches32) { BenchmarkSnippet(state, Branches(32, 4)); }

BENCHMARK(Branches64) { BenchmarkSnippet(state, Branches(64, 4)); }

// Emission for a function the allocator had to spill, so that the stack is
// used rather than only registers.
BENCHMARK(SpilledValues64) { BenchmarkSnippet(state, LiveValues(64)); }

}  // namespace
}  // namespace lucid
