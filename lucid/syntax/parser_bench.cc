#include <cstddef>
#include <expected>
#include <filesystem>
#include <optional>
#include <string>
#include <string_view>
#include <utility>

#include "lucid/core/benchmarking/benchmarking.h"
#include "lucid/core/io/file.h"
#include "lucid/syntax/ast.h"
#include "lucid/syntax/buffered_lexer.h"
#include "lucid/syntax/lexer.h"
#include "lucid/syntax/parser.h"

using namespace std::string_literals;

namespace lucid {
namespace {

void BenchmarkSnippet(BenchmarkState& state, std::string_view snippet) {
  static constexpr int kSnippetRepetitions = 10000;
  std::string code;
  code.reserve(snippet.size() * kSnippetRepetitions + 1);
  for (int i = 0; i < kSnippetRepetitions; ++i) code.append(snippet);
  code.append("\0"s);

  BufferedLexer<Lexer> lexer(Lexer{code});

  for (auto _ : state) {
    lexer.Reset();

    // A context of its own per iteration. Parsing appends to it, so one shared
    // across the run would grow without bound, and every iteration would be
    // measured against a larger arena than the one before it.
    SyntaxContext ctx;
    Parser parser(ctx, code, lexer);
    std::size_t count = 0;
    while (true) {
      std::expected<std::optional<Def>, ParserError> def_or_error =
          parser.Parse();
      if (!def_or_error.has_value()) break;

      std::optional<Def> maybe_def = std::move(def_or_error).value();
      if (!maybe_def.has_value()) break;

      ++count;
    }
    DoNotOptimize(count);
  }
  state.SetBytesProcessed(std::int64_t(state.MaxIterations()) *
                          std::int64_t(code.size()));
}

BENCHMARK(Function) {
  BenchmarkSnippet(state, R"(
    fun main(): Int32 {
      return 0
    }
  )");
}

BENCHMARK(Comment) {
  BenchmarkSnippet(state, R"(
    # Returns the sum of two integers.
    fun sum(a: Int32, b: Int32): Int32 {
      return a + b # can overflow
    }
  )");
}

BENCHMARK(Branches) {
  BenchmarkSnippet(state, R"(
    fun gcd(a: Int32, b: Int32): Int32 {
      loop {
        if a == b {
          break
        }
        if a > b {
          &a = a - b
        } else {
          &b = b - a
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
    // Only the Lucid sources: the directory also holds the build file, which
    // is not a program this parser can read.
    if (dir_entry.path().extension() != ".lu") continue;

    std::string content =
        ReadFile(dir_entry.path(), /*with_trailing_zero=*/false).value();
    snippet.append(content);
  }

  BenchmarkSnippet(state, snippet);
}

}  // namespace
}  // namespace lucid
