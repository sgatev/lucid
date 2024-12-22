#include <cstddef>
#include <cstdint>
#include <optional>
#include <string_view>

#include "benchmark/benchmark.h"
#include "lucid/am_gen.h"
#include "lucid/ast.h"
#include "lucid/cfg.h"
#include "lucid/lexer.h"
#include "lucid/parser.h"

std::size_t CountInstructions(const lucid::SyntaxContext& ctx,
                              const lucid::ControlFlowGraph& graph,
                              lucid::AbstractMachineState& state) {
  lucid::GenerateAbstractMachineFunction(ctx, graph, state);
  return state.func.instructions.size();
}

void Benchmark(benchmark::State& state, std::string_view code) {
  lucid::SyntaxContext ctx;
  auto func_def = std::get<std::optional<lucid::FuncDefStmt>>(
      lucid::Parser(ctx, code, lucid::Lexer(code)).ParseFuncDef());
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
