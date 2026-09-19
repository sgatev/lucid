#include <cstddef>
#include <cstdint>
#include <filesystem>
#include <string>
#include <string_view>

#include "lucid/core/benchmarking/benchmarking.h"
#include "lucid/core/io/file.h"
#include "lucid/syntax/lexer.h"
#include "lucid/syntax/token.h"

using namespace std::string_literals;

namespace lucid {
namespace {

std::size_t CountTokens(std::string_view code) {
  std::size_t count = 0;
  Lexer lexer(code);
  while (lexer.next().kind != Token::Kind::End) ++count;
  return count;
}

void BenchmarkSnippet(BenchmarkState& state, std::string_view snippet) {
  static constexpr int kSnippetRepetitions = 10000;
  std::string code;
  code.reserve(snippet.size() * kSnippetRepetitions + 1);
  for (int i = 0; i < kSnippetRepetitions; ++i) code.append(snippet);
  code.append("\0"s);

  for (auto _ : state) DoNotOptimize(CountTokens(code));

  state.SetBytesProcessed(std::int64_t(state.MaxIterations()) *
                          std::int64_t(code.size()));
}

BENCHMARK(Function) {
  BenchmarkSnippet(state, R"(
    fun main(): Void {
      print("Hello, world!")
    }
  )");
}

BENCHMARK(Tuple) {
  BenchmarkSnippet(state, R"(
    comp val Point: Type = (
      x: Int32,
      y: Int32,
    )
  )");
}

BENCHMARK(Lambda) {
  BenchmarkSnippet(state, R"(
    fun sortByLength(ref names: List(String)): Void {
      sort(&names, (val a: String, val b: String): Bool {
        return a.len < b.len
      })
    }
  )");
}

BENCHMARK(Union) {
  BenchmarkSnippet(state, R"(
    comp val Nothing: Type = ()

    comp fun Optional(val T: Type): Type {
      return T | Nothing
    }
  )");
}

BENCHMARK(Comment) {
  BenchmarkSnippet(state, R"(
    # Returns the sum of two integers.
    fun sum(val a: Int32, val b: Int32): Int32 {
      return a + b # can overflow
    }
  )");
}

BENCHMARK(Number) {
  BenchmarkSnippet(state, R"(
    comp val c: Int64 = sum(21738572173857, 3229017232290172)
  )");
}

BENCHMARK(Branches) {
  BenchmarkSnippet(state, R"(
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

BENCHMARK(Examples) {
  std::string snippet;

  auto path = std::filesystem::current_path() / "examples";
  for (auto const& dir_entry : std::filesystem::directory_iterator{path}) {
    // Process only the Lucid sources. The directory also holds the build file,
    // which is not a program this lexer is meant to read.
    if (dir_entry.path().extension() != ".lu") continue;

    std::string content =
        ReadFile(dir_entry.path(), /*with_trailing_zero=*/false).value();
    snippet.append(content);
  }

  BenchmarkSnippet(state, snippet);
}

}  // namespace
}  // namespace lucid
