#include <cstddef>
#include <cstdint>
#include <string>
#include <string_view>

#include "benchmark/benchmark.h"
#include "lucid/syntax/lexer.h"
#include "lucid/syntax/token.h"

using namespace std::string_literals;

std::size_t CountTokens(std::string_view code) {
  std::size_t count = 0;
  lucid::Lexer lexer(code);
  while (lexer.next().kind != lucid::Token::Kind::End) ++count;
  return count;
}

void Benchmark(benchmark::State& state, std::string_view snippet) {
  static constexpr int kSnippetRepetitions = 10000;
  std::string code;
  code.reserve(snippet.size() * kSnippetRepetitions + 1);
  for (int i = 0; i < kSnippetRepetitions; ++i) code.append(snippet);
  code.append("\0"s);

  for (auto _ : state) benchmark::DoNotOptimize(CountTokens(code));

  state.SetBytesProcessed(std::int64_t(state.iterations()) *
                          std::int64_t(code.size()));
}

static void BM_Function(benchmark::State& state) {
  Benchmark(state, R"(
    fun main(): Void {
      print("Hello, world!")
    }
  )");
}
BENCHMARK(BM_Function);

static void BM_Tuple(benchmark::State& state) {
  Benchmark(state, R"(
    comp val Point: Type = (
      x: Int32,
      y: Int32,
    )
  )");
}
BENCHMARK(BM_Tuple);

static void BM_Lambda(benchmark::State& state) {
  Benchmark(state, R"(
    fun sortByLength(ref names: List(String)): Void {
      sort(&names, (val a: String, val b: String): Bool {
        return a.len < b.len
      })
    }
  )");
}
BENCHMARK(BM_Lambda);

static void BM_Union(benchmark::State& state) {
  Benchmark(state, R"(
    comp val Nothing: Type = ()

    comp fun Optional(val T: Type): Type {
      return T | Nothing
    }
  )");
}
BENCHMARK(BM_Union);

static void BM_Comment(benchmark::State& state) {
  Benchmark(state, R"(
    # Returns the sum of two integers.
    fun sum(val a: Int32, val b: Int32): Int32 {
      return a + b # can overflow
    }
  )");
}
BENCHMARK(BM_Comment);

static void BM_Number(benchmark::State& state) {
  Benchmark(state, R"(
    comp val c: Int64 = sum(21738572173857, 3229017232290172)
  )");
}
BENCHMARK(BM_Number);

static void BM_Branches(benchmark::State& state) {
  Benchmark(state, R"(
    fun gcd(var a: Int32, var b: Int32): Int32 {
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
