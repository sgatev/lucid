#include <cstddef>
#include <expected>
#include <optional>
#include <string>
#include <string_view>
#include <utility>

#include "benchmark/benchmark.h"
#include "lucid/syntax/ast.h"
#include "lucid/syntax/buffered_lexer.h"
#include "lucid/syntax/lexer.h"
#include "lucid/syntax/parser.h"

using namespace std::string_literals;

void Benchmark(benchmark::State& state, std::string_view snippet) {
  static constexpr int kSnippetRepetitions = 10000;
  std::string code;
  code.reserve(snippet.size() * kSnippetRepetitions + 1);
  for (int i = 0; i < kSnippetRepetitions; ++i) code.append(snippet);
  code.append("\0"s);

  lucid::SyntaxContext ctx;
  lucid::BufferedLexer<lucid::Lexer> lexer(lucid::Lexer{code});

  for (auto _ : state) {
    lexer.Reset();

    lucid::Parser parser(ctx, code, lexer);
    std::size_t count = 0;
    for (int i = 0; i < kSnippetRepetitions; ++i) {
      std::expected<std::optional<lucid::Def>, lucid::ParserError>
          def_or_error = parser.ParseDef();
      if (!def_or_error.has_value()) break;

      std::optional<lucid::Def> maybe_def = std::move(def_or_error).value();
      if (!maybe_def.has_value()) break;

      ++count;
    }
    benchmark::DoNotOptimize(count);
  }
  state.SetBytesProcessed(std::int64_t(state.iterations()) *
                          std::int64_t(code.size()));
}

static void BM_Function(benchmark::State& state) {
  Benchmark(state, R"(
    let main = () -> Int32 {
      return 0
    }
  )");
}
BENCHMARK(BM_Function);

static void BM_Comment(benchmark::State& state) {
  Benchmark(state, R"(
    # Returns the sum of two integers.
    let sum = (a: Int32, b: Int32) -> Int32 {
      return a + b # can overflow
    }
  )");
}
BENCHMARK(BM_Comment);

static void BM_Branches(benchmark::State& state) {
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
