#include <cstddef>
#include <cstdint>
#include <string>
#include <string_view>

#include "benchmark/benchmark.h"
#include "lucid/lexer.h"
#include "lucid/token.h"

using namespace std::string_literals;

std::size_t CountTokens(std::string_view code) {
  std::size_t count = 0;
  lucid::Lexer lexer(code);
  while (true) {
    const lucid::Token token = lexer.next();
    if (token.kind == lucid::Token::Kind::End) break;
    ++count;
  }
  return count;
}

void Benchmark(benchmark::State &state, std::string_view code) {
  static constexpr int kCodeRepetitions = 10000;
  std::string repeated_code_with_null;
  repeated_code_with_null.reserve(code.size() * kCodeRepetitions + 1);
  for (int i = 0; i < kCodeRepetitions; ++i) {
    repeated_code_with_null.append(code);
  }
  repeated_code_with_null.append("\0"s);

  for (auto _ : state) {
    benchmark::DoNotOptimize(CountTokens(repeated_code_with_null));
  }

  state.SetBytesProcessed(std::int64_t(state.iterations()) *
                          std::int64_t(repeated_code_with_null.size()));
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
      x: Int32,
      y: Int32,
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
    let sum = (a: Int32, b: Int32) -> Int32 {
      return a + b # can overflow
    }
  )");
}
BENCHMARK(BM_Comment);

static void BM_Number(benchmark::State &state) {
  Benchmark(state, R"(
    let c = sum(21738572173857, 3229017232290172)
  )");
}
BENCHMARK(BM_Number);

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
