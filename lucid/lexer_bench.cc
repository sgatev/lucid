#include <cstddef>
#include <cstdint>
#include <sstream>
#include <string>
#include <string_view>

#include "benchmark/benchmark.h"
#include "lucid/lexer.h"
#include "lucid/token.h"

std::size_t CountTokens(std::string_view code) {
  lucid::Lexer lexer(code);
  std::size_t cnt = 0;
  while (true) {
    lucid::Token token = lexer.next();
    if (token.kind == lucid::Token::Kind::End) return cnt;
    ++cnt;
  }
}

void Benchmark(benchmark::State &state, std::string_view snippet) {
  std::string code;
  code.reserve(snippet.size() * 10000);
  for (int i = 0; i < 10000; i++) code.append(snippet);

  for (auto _ : state) benchmark::DoNotOptimize(CountTokens(code));

  state.SetBytesProcessed(std::int64_t(state.iterations()) *
                          std::int64_t(code.size()));
}

static void BM_Function(benchmark::State &state) {
  Benchmark(state, R"(
    let main = () -> Void {
      print("Hello, world!")
    }
  )");
}
BENCHMARK(BM_Function);

static void BM_Tuple(benchmark::State &state) {
  Benchmark(state, R"(
    let Point = (
      x: Int,
      y: Int,
    )
  )");
}
BENCHMARK(BM_Tuple);

static void BM_Lambda(benchmark::State &state) {
  Benchmark(state, R"(
    let sortByLength = (mut names: List(String)) -> Void {
      sort(mut names, (a: String, b: String) -> Bool {
        return a.len < b.len
      })
    }
  )");
}
BENCHMARK(BM_Lambda);

static void BM_Union(benchmark::State &state) {
  Benchmark(state, R"(
    let Nothing = ()

    let Optional = (T: Type) -> Type {
      return T | Nothing
    }
  )");
}
BENCHMARK(BM_Union);

static void BM_Comment(benchmark::State &state) {
  Benchmark(state, R"(
    # Returns the sum of two integers.
    let sum = (a: Int, b: Int) -> Int {
      return a + b # can overflow
    }
  )");
}
BENCHMARK(BM_Comment);

static void BM_Number(benchmark::State &state) {
  Benchmark(state, R"(
    let c = sum(2173857, 32290172)
  )");
}
BENCHMARK(BM_Number);

BENCHMARK_MAIN();
