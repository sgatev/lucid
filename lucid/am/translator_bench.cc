#include <cassert>
#include <cstddef>
#include <cstdint>
#include <expected>
#include <format>
#include <list>
#include <optional>
#include <string>
#include <string_view>
#include <utility>
#include <variant>

#include "lucid/am/state.h"
#include "lucid/am/translator.h"
#include "lucid/core/benchmarking/benchmarking.h"
#include "lucid/syntax/ast.h"
#include "lucid/syntax/cfg.h"
#include "lucid/syntax/comp.h"
#include "lucid/syntax/context.h"
#include "lucid/syntax/lexer.h"
#include "lucid/syntax/parser.h"
#include "lucid/syntax/scope.h"
#include "lucid/syntax/ssa.h"
#include "lucid/syntax/type.h"

namespace lucid {
namespace {

using namespace std::string_literals;

std::size_t CountInstructions(const SyntaxContext& syn_ctx,
                              const SyntaxControlFlowGraph& syn_cfg) {
  AbstractMachineState am_state;
  auto am_cfg = GenerateAbstractMachineFunction(
      /*am_cfgs=*/{}, syn_ctx, syn_cfg, am_state);
  assert(am_cfg.has_value());

  std::size_t instructions_count = 0;
  for (const auto& block : am_cfg->Blocks()) {
    instructions_count += block.instructions.size();
  }
  return instructions_count;
}

void BenchmarkSnippet(BenchmarkState& state, std::string_view snippet) {
  std::string code;
  code.append(snippet);
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

  for (auto _ : state) {
    DoNotOptimize(CountInstructions(syn_ctx, syn_cfg));
  }

  state.SetBytesProcessed(std::int64_t(state.MaxIterations()) *
                          std::int64_t(code.size()));
}

// A function of a few lines, named apart from the others by `index`.
std::string NumberedFunction(int index) {
  return std::format(R"(
    fun f{}(a: Int32): Int32 {{
      val b: Int32 = a + 1
      if b > 2 {{
        return b * 2
      }}
      return b - 1
    }}
  )",
                     index);
}

// Measures translating the last function of a program of `count` of them, in
// the context the whole program was read into, with one state for them all,
// as a compile keeps.
//
// What translating a function costs ought to be set by the function rather
// than by how much of the program came before it, so the two sizes below
// ought to cost the same.
void BenchmarkLastFunction(BenchmarkState& state, int count) {
  std::string code;
  for (int i = 0; i < count; ++i) code.append(NumberedFunction(i));
  code.append("\0"s);

  // Every function is taken through what comes before translation, which is
  // what fills the context to the size a compile would have it at.
  SyntaxContext syn_ctx;
  Parser parser(syn_ctx, code, Lexer(code));
  std::list<Def> defs;
  std::optional<SyntaxControlFlowGraph> syn_cfg;
  while (true) {
    std::expected<std::optional<Def>, ParserError> def_or_error =
        parser.Parse();
    assert(def_or_error.has_value());
    std::optional<Def> maybe_def = std::move(def_or_error).value();
    if (!maybe_def.has_value()) break;

    defs.push_back(std::move(maybe_def).value());
    auto& func_def = std::get<FuncDefStmt>(defs.back());
    syn_ctx.AddFuncDef(func_def);
    [[maybe_unused]] auto resolved = ResolveNames(syn_ctx, func_def);
    assert(resolved.has_value());
    [[maybe_unused]] auto typed = InferExprTypes(syn_ctx, func_def);
    assert(typed.has_value());
    [[maybe_unused]] auto checked = CheckComp(syn_ctx, func_def);
    assert(checked.has_value());
    syn_cfg.emplace(BuildControlFlowGraph(syn_ctx, func_def));
    ConvertToStaticSingleAssignment(syn_ctx, *syn_cfg);
  }

  AbstractMachineState am_state;
  for (auto _ : state) {
    auto am_cfg = GenerateAbstractMachineFunction(/*am_cfgs=*/{}, syn_ctx,
                                                  *syn_cfg, am_state);
    assert(am_cfg.has_value());
    DoNotOptimize(am_cfg->next_free_reg_id);
  }

  state.SetBytesProcessed(std::int64_t(state.MaxIterations()) *
                          std::int64_t(NumberedFunction(count - 1).size()));
}

BENCHMARK(LastOf10Functions) { BenchmarkLastFunction(state, 10); }

BENCHMARK(LastOf1000Functions) { BenchmarkLastFunction(state, 1000); }

BENCHMARK(Function) {
  BenchmarkSnippet(state, R"(
    fun main(): Int32 {
      return 2 + 3
    }
  )");
}

}  // namespace
}  // namespace lucid
