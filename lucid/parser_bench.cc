#include <cstddef>
#include <sstream>
#include <string>
#include <string_view>
#include <variant>

#include "benchmark/benchmark.h"
#include "lucid/arena.h"
#include "lucid/ast.h"
#include "lucid/lexer.h"
#include "lucid/parser.h"
#include "lucid/token.h"

std::size_t CountTokens(std::string_view code) {
  lucid::SyntaxContext ctx;
  lucid::Lexer lexer(code);
  lucid::Parser parser(ctx, code, lexer);
  std::size_t cnt = 0;
  while (true) {
    auto node = parser.ParseFuncDef();
    if (auto *err = std::get_if<lucid::ParserError>(&node)) {
      if (err->GetKind() == lucid::ParserError::Kind::End) return cnt;
    }
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
    let main = () -> Int32 {
      return 0
    }
  )");
}
BENCHMARK(BM_Function);

BENCHMARK_MAIN();
