#include <cassert>
#include <cstddef>
#include <format>
#include <list>
#include <optional>
#include <string>
#include <utility>
#include <vector>

#include "lucid/core/benchmarking/benchmarking.h"
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

// A straight line of values, each read once by the next.
//
// One block and no join points, so nothing needs a phi function: what grows
// with `count` is the definitions to be renamed.
std::string ChainedValues(int count, int index) {
  std::string code =
      std::format("fun f{}(): Int32 {{\n  val v0: Int32 = 1\n", index);
  for (int i = 1; i < count; ++i) {
    code += std::format("  val v{}: Int32 = v{} + {}\n", i, i - 1, i);
  }
  code += std::format("  return v{}\n}}\n", count - 1);
  return code;
}

// A run of branches, each assigning the same variables on both sides.
//
// Every branch closes over a join point, and every variable assigned in it
// needs a phi function there, so this grows what phi placement has to do
// rather than what renaming has to do.
std::string Branches(int count, int vars, int index) {
  std::string code = std::format("fun f{}(): Int32 {{\n", index);
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

// A loop whose variables are assigned inside it, so each needs a phi function
// at the head of the loop, where the edge back into it joins.
std::string Loop(int vars, int index) {
  std::string code = std::format("fun f{}(): Int32 {{\n", index);
  for (int v = 0; v < vars; ++v) {
    code += std::format("  val x{}: Int32 = {}\n", v, v + 1);
  }
  code += "  loop {\n    if x0 > 100 {\n      break\n    }\n";
  for (int v = 0; v < vars; ++v) {
    code += std::format("    &x{} = x{} + 1\n", v, v);
  }
  code += "  }\n  return x0\n}\n";
  return code;
}

// Measures the conversion over one function per iteration.
//
// The conversion renames the variables of a function in the syntax context
// itself and not only in the graph it is handed, so a function cannot be
// converted twice: a copy of its graph would still name the statements that
// the first conversion rewrote. Every iteration is therefore given a function
// of its own.
//
// They share one context, which is how a compile works: functions are
// converted one after another against the context they were all parsed into,
// and converting one leaves the statements of the others alone. A context
// apiece would cost more in tables and arenas than the functions themselves.
//
// A run therefore holds as many functions as it measures conversions, and the
// conversion leaves a good deal behind per statement, so the sizes below are
// what keeps that within a few hundred megabytes.
template <typename MakeFunctionT>
void BenchmarkFunctions(BenchmarkState& state, MakeFunctionT make_function) {
  const std::size_t count = state.MaxIterations();

  std::string code;
  for (std::size_t i = 0; i < count; ++i) {
    code.append(make_function(static_cast<int>(i)));
  }
  code.append("\0"s);

  SyntaxContext syn_ctx;
  Parser parser(syn_ctx, code, Lexer(code));

  std::list<Def> defs;
  std::vector<SyntaxControlFlowGraph> syn_cfgs;
  syn_cfgs.reserve(count);
  while (true) {
    std::expected<std::optional<Def>, ParserError> def_or_error =
        parser.Parse();
    assert(def_or_error.has_value());
    std::optional<Def> maybe_def = std::move(def_or_error).value();
    if (!maybe_def.has_value()) break;

    defs.push_back(std::move(maybe_def).value());
    auto& func_def = std::get<FuncDefStmt>(defs.back());
    syn_ctx.AddFuncDef(func_def);

    auto infer_types_res = InferExprTypes(syn_ctx, func_def);
    assert(infer_types_res.has_value());
    auto check_comp_res = CheckComp(syn_ctx, func_def);
    assert(check_comp_res.has_value());

    syn_cfgs.push_back(BuildControlFlowGraph(syn_ctx, func_def));
  }
  assert(syn_cfgs.size() == count);

  std::size_t next = 0;
  for (auto _ : state) {
    ConvertToStaticSingleAssignment(syn_ctx, syn_cfgs[next]);
    DoNotOptimize(syn_cfgs[next].blocks().Size());
    ++next;
  }
}

// Each of these hands every iteration a function of its own, so the sizes are
// chosen to be work worth measuring rather than the smallest that shows the
// shape: a run of thousands of tiny conversions would cost more to prepare
// than it measures.

// Renaming, over a graph with no join points at all.
BENCHMARK(ChainedValues512) {
  BenchmarkFunctions(state, [](int i) { return ChainedValues(512, i); });
}

BENCHMARK(ChainedValues1024) {
  BenchmarkFunctions(state, [](int i) { return ChainedValues(1024, i); });
}

// Phi placement, over graphs with a join point per branch.
BENCHMARK(Branches64) {
  BenchmarkFunctions(state, [](int i) { return Branches(64, 4, i); });
}

BENCHMARK(Branches128) {
  BenchmarkFunctions(state, [](int i) { return Branches(128, 4, i); });
}

// Phi placement where what grows is the variables to place rather than the
// joins to place them at.
BENCHMARK(BranchesWideVars) {
  BenchmarkFunctions(state, [](int i) { return Branches(48, 16, i); });
}

// A back edge, where every variable the loop assigns needs a phi function at
// its head.
BENCHMARK(LoopAssignments) {
  BenchmarkFunctions(state, [](int i) { return Loop(128, i); });
}

}  // namespace
}  // namespace lucid
