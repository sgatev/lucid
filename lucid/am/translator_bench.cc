#include <cassert>
#include <cstddef>
#include <cstdint>
#include <string_view>
#include <utility>
#include <variant>

#include "benchmark/benchmark.h"
#include "lucid/am/translator.h"
#include "lucid/syntax/ast.h"
#include "lucid/syntax/cfg.h"
#include "lucid/syntax/comp.h"
#include "lucid/syntax/lexer.h"
#include "lucid/syntax/parser.h"
#include "lucid/syntax/ssa.h"
#include "lucid/syntax/type.h"

using namespace std::string_literals;

std::size_t CountInstructions(const lucid::SyntaxContext& syn_ctx,
                              const lucid::SyntaxControlFlowGraph& syn_cfg) {
  lucid::AbstractMachineState am_state;
  lucid::AbstractMachineControlFlowGraph am_cfg =
      lucid::GenerateAbstractMachineFunction(/*am_cfgs=*/{}, syn_ctx, syn_cfg,
                                             am_state);
  std::size_t instructions_count = 0;
  for (const auto& block : am_cfg.Blocks()) {
    instructions_count += block.instructions.size();
  }
  return instructions_count;
}

void Benchmark(benchmark::State& state, std::string_view snippet) {
  std::string code;
  code.append(snippet);
  code.append("\0"s);

  lucid::SyntaxContext syn_ctx;

  std::expected<std::optional<lucid::Def>, lucid::ParserError> def_or_error =
      lucid::Parser(syn_ctx, code, lucid::Lexer(code)).ParseDef();
  assert(def_or_error.has_value());

  std::optional<lucid::Def> maybe_def = std::move(def_or_error).value();
  assert(maybe_def.has_value());

  lucid::Def def = std::move(maybe_def).value();
  assert(std::holds_alternative<lucid::FuncDefStmt>(def));

  auto func_def = std::get<lucid::FuncDefStmt>(std::move(def));

  auto infer_types_res = lucid::InferExprTypes(syn_ctx, func_def);
  assert(infer_types_res.has_value());

  auto check_comp_res = lucid::CheckComp(syn_ctx, func_def);
  assert(check_comp_res.has_value());

  lucid::SyntaxControlFlowGraph syn_cfg =
      lucid::BuildControlFlowGraph(syn_ctx, func_def);
  lucid::ConvertToStaticSingleAssignment(syn_ctx, syn_cfg);

  for (auto _ : state) {
    benchmark::DoNotOptimize(CountInstructions(syn_ctx, syn_cfg));
  }

  state.SetBytesProcessed(std::int64_t(state.iterations()) *
                          std::int64_t(code.size()));
}

static void BM_Function(benchmark::State& state) {
  Benchmark(state, R"(
    fun main(): Int32 {
      return 2 + 3
    }
  )");
}
BENCHMARK(BM_Function);

BENCHMARK_MAIN();
