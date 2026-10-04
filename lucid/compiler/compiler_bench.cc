#include <algorithm>
#include <cctype>
#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <filesystem>
#include <format>
#include <ostream>
#include <streambuf>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

#include "lucid/compiler/compiler.h"
#include "lucid/core/benchmarking/benchmarking.h"
#include "lucid/core/container/hash_set.h"
#include "lucid/core/io/file.h"

using namespace std::string_literals;

namespace lucid {
namespace {

// A stream that takes whatever is written to it and keeps none of it, so that
// what is measured is the compiling rather than the growing of a buffer to
// hold its output.
class DiscardingStream : public std::ostream {
 public:
  DiscardingStream() : std::ostream(&buffer_) {}

 private:
  class DiscardingBuffer : public std::streambuf {
   protected:
    int_type overflow(int_type c) override { return c; }

    std::streamsize xsputn(const char*, std::streamsize count) override {
      return count;
    }
  };

  DiscardingBuffer buffer_;
};

// A program of the examples, by the name of the file it was read from.
struct Example {
  std::string name;
  std::string source;
};

// Returns the programs in `examples`, ordered by name so that every run reads
// them in the same order.
std::vector<Example> ReadExamples() {
  std::vector<Example> examples;
  const auto path = std::filesystem::current_path() / "examples";
  for (const auto& dir_entry : std::filesystem::directory_iterator{path}) {
    // Only the Lucid sources: the directory also holds the build file.
    if (dir_entry.path().extension() != ".lu") continue;

    examples.push_back({
        .name = dir_entry.path().stem().string(),
        .source = ReadFile(dir_entry.path()).value(),
    });
  }
  std::ranges::sort(examples, {}, &Example::name);
  return examples;
}

// Compiles `src`, which must end with a zero byte, and fails loudly if it
// does not compile: a benchmark of a program the compiler rejects measures
// how fast it gives up.
void Compile(std::string_view src, std::ostream& out) {
  if (!CompileSource(src, out).has_value()) {
    std::abort();
  }
}

// Every program of the examples, each compiled on its own as it is written,
// what it asks to have run during compilation included.
BENCHMARK(Examples) {
  std::vector<std::string> sources;
  std::int64_t bytes = 0;
  for (Example& example : ReadExamples()) {
    sources.push_back(std::move(example.source) + "\0"s);
    bytes += static_cast<std::int64_t>(sources.back().size());
  }

  DiscardingStream out;
  for (auto _ : state) {
    for (const std::string& source : sources) Compile(source, out);
  }
  state.SetBytesProcessed(std::int64_t(state.MaxIterations()) * bytes);
}

// The functions the compiler stands in for itself, which a program defines
// once whatever else it does.
constexpr std::string_view kBuiltins[] = {"printString", "sleep"};

// Returns `source` with every identifier in `names` followed by `suffix`.
std::string Rename(std::string_view source, const HashSet<std::string>& names,
                   std::string_view suffix) {
  const auto in_identifier = [](char c) {
    return std::isalnum(static_cast<unsigned char>(c)) || c == '_';
  };

  std::string renamed;
  renamed.reserve(source.size());
  std::size_t at = 0;
  while (at < source.size()) {
    if (!in_identifier(source[at])) {
      renamed += source[at++];
      continue;
    }
    std::size_t end = at;
    while (end < source.size() && in_identifier(source[end])) ++end;

    const std::string_view identifier = source.substr(at, end - at);
    renamed += identifier;
    if (names.Contains(std::string(identifier))) renamed += suffix;
    at = end;
  }
  return renamed;
}

// Returns a single program of about `size` bytes, made of the functions of
// the examples copied over and over, each copy with names of its own.
//
// One program rather than many, so that what grows with `size` is what the
// compiler does for each function, and not the work it does once for a whole
// program. The examples that run anything during compilation are left out,
// because how long that takes says how long the program runs, not how long
// compiling it does; `Examples` measures them.
std::string ExampleFunctions(std::size_t size) {
  struct Functions {
    std::string name;
    HashSet<std::string> names;
    std::vector<std::string> sources;
  };

  std::vector<Functions> examples;
  // The first definition of each builtin, which the program holds once.
  std::vector<std::string> builtins;
  HashSet<std::string> builtins_seen;
  for (const Example& example : ReadExamples()) {
    if (example.source.find("comp fun") != std::string::npos) continue;

    Functions functions{.name = example.name};
    // A function starts where a line starts with `fun`, and runs up to where
    // the next one does.
    std::size_t start = example.source.find("fun ");
    while (start != std::string::npos) {
      std::size_t next = example.source.find("\nfun ", start);
      if (next != std::string::npos) ++next;

      const std::string_view function =
          std::string_view(example.source).substr(start, next - start);
      const std::size_t name_start = function.find(' ') + 1;
      const std::string name(
          function.substr(name_start, function.find('(') - name_start));
      if (std::ranges::find(kBuiltins, name) == std::end(kBuiltins)) {
        functions.names.Insert(name);
        functions.sources.emplace_back(function);
      } else if (builtins_seen.Insert(name)) {
        builtins.emplace_back(function);
      }
      start = next;
    }
    examples.push_back(std::move(functions));
  }

  std::string program;
  for (const std::string& builtin : builtins) program += builtin + "\n";
  for (int copy = 0; program.size() < size; ++copy) {
    for (const Functions& functions : examples) {
      const std::string suffix = std::format("_{}_{}", functions.name, copy);
      for (const std::string& function : functions.sources) {
        program += Rename(function, functions.names, suffix);
        program += "\n";
      }
    }
  }
  program += "fun main(): Int32 {\n  return 0\n}\n";
  return program;
}

// A megabyte of the functions of the examples, as one program.
BENCHMARK(ExampleFunctions) {
  const std::string source = ExampleFunctions(1 << 20) + "\0"s;

  DiscardingStream out;
  for (auto _ : state) Compile(source, out);
  state.SetBytesProcessed(std::int64_t(state.MaxIterations()) *
                          std::int64_t(source.size()));
}

}  // namespace
}  // namespace lucid
