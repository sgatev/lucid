#include <cstddef>
#include <string>
#include <string_view>

#include "benchmark/benchmark.h"
#include "lucid/ast.h"
#include "lucid/buffered_lexer.h"
#include "lucid/lexer.h"
#include "lucid/parser.h"

void Benchmark(benchmark::State &state, std::string_view snippet) {
  std::string code;
  code.reserve(snippet.size() * 1000);
  for (int i = 0; i < 1000; i++) code.append(snippet);

  for (auto _ : state) {
    state.PauseTiming();
    {
      lucid::SyntaxContext ctx;
      lucid::BufferedLexer<lucid::Lexer, 1000> lexer(lucid::Lexer{code});
      lucid::Parser parser(ctx, code, lexer);

      state.ResumeTiming();
      std::size_t cnt = 0;
      for (int i = 0; i < 1000; ++i) cnt += parser.ParseFuncDef().index();
      benchmark::DoNotOptimize(cnt);
      state.PauseTiming();
    }
    state.ResumeTiming();
  }
  state.SetBytesProcessed(std::int64_t(state.iterations()) *
                          std::int64_t(code.size()));
}

static void BM_Function(benchmark::State &state) {
  Benchmark(state, R"(
    let main = () -> Int32 {
      return 0
    }
  )");
}
BENCHMARK(BM_Function);

static void BM_Comment(benchmark::State &state) {
  Benchmark(state, R"(
    # Returns the sum of two integers.
    let sum = (a: Int32, b: Int32) -> Int32 {
      return a + b # can overflow
    }
  )");
}
BENCHMARK(BM_Comment);

static void BM_Branches(benchmark::State &state) {
  Benchmark(state, R"(
    let gcd = (a: Int32, b: Int32) -> Int32 {
      loop {
        if a == b {
          break
        }
        if a > b {
          a = a - b
        } else {
          b = b - a
        }
      }
      return a
    }
  )");
}
BENCHMARK(BM_Branches);

BENCHMARK_MAIN();
