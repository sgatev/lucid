#include <cstddef>
#include <cstdint>
#include <string_view>

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
  for (const auto& block : am_cfg.blocks()) {
    instructions_count += block.instructions.size();
  }
  return instructions_count;
}

void Benchmark(benchmark::State& state, std::string_view code) {
  lucid::SyntaxContext ctx;
  auto func_def =
      lucid::Parser(ctx, code, lucid::Lexer(code)).ParseFuncDef().value();
  auto graph = BuildControlFlowGraph(ctx, func_def.value());
  lucid::AbstractMachineState am_state;

  for (auto _ : state) {
    benchmark::DoNotOptimize(CountInstructions(ctx, graph, am_state));
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
