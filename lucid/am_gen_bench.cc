#include <cstddef>
#include <cstdint>
#include <string_view>

#include "benchmark/benchmark.h"
#include "lucid/am_gen.h"
#include "lucid/arena.h"
#include "lucid/ast.h"
#include "lucid/cfg.h"
#include "lucid/lexer.h"
#include "lucid/parser.h"

std::size_t CountInstructions(const lucid::Arena<lucid::Stmt>& stmt_arena,
                              const lucid::Arena<lucid::Expr>& expr_arena,
                              const lucid::Arena<lucid::Type>& type_arena,
                              const lucid::ControlFlowGraph& graph,
                              lucid::AbstractMachineState& state) {
  lucid::GenerateAbstractMachineFunction(stmt_arena, expr_arena, type_arena,
                                         graph, state);
  return state.func.instructions.size();
}

void Benchmark(benchmark::State& state, std::string_view code) {
  lucid::Arena<lucid::Stmt> stmt_arena;
  lucid::Arena<lucid::Expr> expr_arena;
  lucid::Arena<lucid::Type> type_arena;
  auto func_def = std::get<lucid::FuncDefStmt>(
      lucid::Parser(stmt_arena, expr_arena, type_arena, code,
                    lucid::Lexer(code))
          .ParseFuncDef());
  auto graph = BuildControlFlowGraph(stmt_arena, expr_arena, func_def);
  lucid::AbstractMachineState am_state;

  for (auto _ : state) {
    benchmark::DoNotOptimize(
        CountInstructions(stmt_arena, expr_arena, type_arena, graph, am_state));
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
