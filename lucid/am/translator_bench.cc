#include <cassert>
#include <cstddef>
#include <cstdint>
#include <string_view>
#include <utility>
#include <variant>

#include "lucid/am/translator.h"
#include "lucid/core/benchmarking/benchmarking.h"
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

std::size_t CountInstructions(const SyntaxContext& syn_ctx,
                              const SyntaxControlFlowGraph& syn_cfg) {
  AbstractMachineState am_state;
  AbstractMachineControlFlowGraph am_cfg = GenerateAbstractMachineFunction(
      /*am_cfgs=*/{}, syn_ctx, syn_cfg, am_state);
  std::size_t instructions_count = 0;
  for (const auto& block : am_cfg.Blocks()) {
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

BENCHMARK(Function) {
  BenchmarkSnippet(state, R"(
    fun main(): Int32 {
      return 2 + 3
    }
  )");
}

}  // namespace
}  // namespace lucid
