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
#include "lucid/syntax/lexer.h"
#include "lucid/syntax/parser.h"

std::size_t CountInstructions(const lucid::SyntaxContext& syn_ctx,
                              const lucid::SyntaxControlFlowGraph& syn_cfg,
                              lucid::AbstractMachineState& am_state) {
  lucid::AbstractMachineControlFlowGraph am_cfg =
      lucid::GenerateAbstractMachineFunction(/*am_cfgs=*/{}, syn_ctx, syn_cfg,
                                             am_state);
  std::size_t instructions_count = 0;
  for (const auto& block : am_cfg.Blocks()) {
    instructions_count += block.instructions.size();
  }
  return instructions_count;
}

void Benchmark(benchmark::State& state, std::string_view code) {
  lucid::SyntaxContext ctx;

  std::expected<std::optional<lucid::Def>, lucid::ParserError> def_or_error =
      lucid::Parser(ctx, code, lucid::Lexer(code)).ParseDef();
  assert(def_or_error.has_value());

  std::optional<lucid::Def> maybe_def = std::move(def_or_error).value();
  assert(maybe_def.has_value());

  lucid::Def def = std::move(maybe_def).value();
  assert(std::holds_alternative<lucid::FuncDefStmt>(def));

  auto func_def = std::get<lucid::FuncDefStmt>(std::move(def));
  lucid::SyntaxControlFlowGraph syn_cfg =
      lucid::BuildControlFlowGraph(ctx, func_def);
  lucid::AbstractMachineState am_state;

  for (auto _ : state) {
    benchmark::DoNotOptimize(CountInstructions(ctx, syn_cfg, am_state));
  }

  state.SetBytesProcessed(std::int64_t(state.iterations()) *
                          std::int64_t(code.size()));
}

static void BM_Function(benchmark::State& state) {
  Benchmark(state, R"(
    let main = () -> Int32 {
      return 2 + 3
    }
  )");
}
BENCHMARK(BM_Function);

BENCHMARK_MAIN();
