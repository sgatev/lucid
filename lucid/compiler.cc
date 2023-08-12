#include <stdio.h>
#include <unistd.h>

#include <cstdio>
#include <cstdlib>
#include <memory>
#include <stdexcept>
#include <string>
#include <string_view>
#include <variant>
#include <vector>

#include "lucid/am_gen.h"
#include "lucid/arena.h"
#include "lucid/arm64_gen.h"
#include "lucid/ast.h"
#include "lucid/cfg.h"
#include "lucid/lexer.h"
#include "lucid/opt.h"
#include "lucid/parser.h"
#include "lucid/writer.h"

namespace lucid {
namespace {

// Format `args` according to the format string `fmt`.
//
// From https://stackoverflow.com/a/26221725.
template <typename... Args>
std::string string_format(const std::string& fmt, Args... args) {
  int size = std::snprintf(nullptr, 0, fmt.c_str(), args...) + 1;
  if (size < 0) throw std::runtime_error("Error during formatting.");

  std::string result;
  result.resize(size);
  std::snprintf(result.data(), size, fmt.c_str(), args...);
  return result;
}

std::vector<FuncDefStmt> GenerateFromSource(Arena<Stmt>& arena,
                                            std::string_view src) {
  std::vector<FuncDefStmt> func_defs;
  Lexer lexer(src);
  Parser parser(arena, src, lexer);
  while (true) {
    auto maybe_func_def = parser.ParseFuncDef();
    auto* stmt_ref = std::get_if<StmtRef>(&maybe_func_def);
    if (stmt_ref == nullptr) break;

    auto* func_def_stmt = std::get_if<FuncDefStmt>(&arena.get(*stmt_ref));
    if (func_def_stmt == nullptr) break;

    func_defs.push_back(*func_def_stmt);
  }
  return func_defs;
}

}  // namespace

int Main(std::string_view input) {
  // Load Lucis sources.
  Arena<Stmt> arena;
  std::vector<FuncDefStmt> funcs;
  if (input == "empty_main") {
    funcs = GenerateFromSource(arena, R"(
      let main = () -> Int {
        return 0
      }
    )");
  } else if (input == "func_call") {
    funcs = GenerateFromSource(arena, R"(
      let id = (x: Int) -> Int {
        return x
      }

      let main = () -> Int {
        return id(21)
      }
    )");
  } else if (input == "add_ints") {
    funcs = GenerateFromSource(arena, R"(
      let main = () -> Int {
        return 2 + 3
      }
    )");
  } else if (input == "sub_ints") {
    funcs = GenerateFromSource(arena, R"(
      let main = () -> Int {
        return 7 - 5
      }
    )");
  } else if (input == "mul_ints") {
    funcs = GenerateFromSource(arena, R"(
      let main = () -> Int {
        return 3 * 7
      }
    )");
  } else if (input == "div_ints") {
    funcs = GenerateFromSource(arena, R"(
      let main = () -> Int {
        return 8 / 2
      }
    )");
  }

  // Generate 64-bit ARM assembly.
  std::FILE* out = std::tmpfile();
  auto writer = FileWriter(out);
  GenerateArmStartSource(writer);
  for (const auto& func : funcs) {
    auto graph = BuildControlFlowGraph(arena, func);
    auto instructions = GenerateAbstractMachineInstructions(arena, graph);
    OptimizeAbstractMachineInstructions(instructions);
    GenerateArmAssemblySource(func.name, instructions, writer);
  }
  std::fseek(out, 0, SEEK_SET);

  // Translate assembly into object code.
  dup2(fileno(out), 0);
  const std::string as_cmd =
      string_format("as -arch arm64 -o %s.o -- ", input.data());
  std::system(as_cmd.data());

  // Link object code and create a binary.
  const std::string ld_cmd = string_format(
      "ld -o %s %s.o -lSystem -syslibroot `xcrun -sdk macosx --show-sdk-path` "
      "-e _start -arch arm64",
      input.data(), input.data());
  std::system(ld_cmd.data());

  return 0;
}

}  // namespace lucid

int main(int argc, char* argv[]) { return lucid::Main(argv[1]); }
