#include <cstddef>
#include <cstdint>
#include <sstream>
#include <string>
#include <string_view>

#include "benchmark/benchmark.h"
#include "lucid/am_gen.h"
#include "lucid/arena.h"
#include "lucid/ast.h"
#include "lucid/cfg.h"
#include "lucid/lexer.h"
#include "lucid/parser.h"

std::size_t CountInstructions(const lucid::Arena<lucid::Stmt>& arena,
                              const lucid::ControlFlowGraph& graph) {
  return lucid::GenerateAbstractMachineInstructions(arena, graph).size();
}

void Benchmark(benchmark::State& state, std::string_view code) {
  lucid::Arena<lucid::Stmt> arena;
  auto maybe_func_def_stmt =
      lucid::Parser(arena, code, lucid::Lexer(code)).ParseFuncDef();
  auto func_def = std::get<lucid::FuncDefStmt>(maybe_func_def_stmt);
  auto graph = BuildControlFlowGraph(arena, func_def);

  for (auto _ : state) {
    benchmark::DoNotOptimize(CountInstructions(arena, graph));
  }

  state.SetBytesProcessed(std::int64_t(state.iterations()) *
                          std::int64_t(code.size()));
}

static void BM_Function(benchmark::State& state) {
  Benchmark(state, R"(
    let main = () -> Int {
      return 2 + 3
    }
  )");
}
BENCHMARK(BM_Function);

BENCHMARK_MAIN();
