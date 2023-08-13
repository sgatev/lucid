#include <stdio.h>
#include <unistd.h>

#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <optional>
#include <span>
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
#include "lucid/cli.h"
#include "lucid/file.h"
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
std::string StringFormat(const std::string& fmt, Args... args) {
  int size = std::snprintf(nullptr, 0, fmt.c_str(), args...) + 1;
  if (size < 0) throw std::runtime_error("Error during formatting.");

  std::string result;
  result.resize(size);
  std::snprintf(result.data(), size, fmt.c_str(), args...);
  return result;
}

std::vector<FuncDefStmt> ParseFuncDefs(std::string_view src,
                                       Arena<Stmt>& arena) {
  std::vector<FuncDefStmt> func_defs;
  Lexer lexer(src);
  Parser parser(arena, src, lexer);
  while (true) {
    auto maybe_func_def = parser.ParseFuncDef();
    if (auto* err = std::get_if<ParserError>(&maybe_func_def)) {
      if (err->GetKind() == ParserError::Kind::End) break;
      std::cerr << err->ToString() << std::endl;
      exit(1);
    }
    func_defs.push_back(std::get<FuncDefStmt>(std::move(maybe_func_def)));
  }
  return func_defs;
}

void Compile(std::string_view src, Writer out) {
  Arena<Stmt> arena;
  std::vector<FuncDefStmt> funcs = ParseFuncDefs(src, arena);
  GenerateArmStartSource(out);
  for (const auto& func : funcs) {
    auto graph = BuildControlFlowGraph(arena, func);
    auto instructions = GenerateAbstractMachineInstructions(arena, graph);
    OptimizeAbstractMachineInstructions(instructions);
    GenerateArmAssemblySource(func.name, instructions, out);
  }
}

std::optional<std::string> Build(std::span<std::string_view> args) {
  if (args.size() != 2) return "'build' command requires exactly 2 arguments";

  std::string_view binary_name = args[0];
  std::string_view src_path = args[1];

  // Load Lucid sources.
  const auto maybe_src = ReadFile(src_path);
  if (std::holds_alternative<FileError>(maybe_src)) {
    std::cerr << "file error: could not read file " << src_path << std::endl;
    exit(1);
  }
  const auto& src = std::get<std::string>(maybe_src);

  // Compile sources to assembly.
  std::FILE* assembly_file = std::tmpfile();
  Compile(src, FileWriter(assembly_file));
  std::rewind(assembly_file);

  // Translate assembly into object code.
  dup2(fileno(assembly_file), 0);
  const std::string as_cmd =
      StringFormat("as -arch arm64 -o %s.o -- ", binary_name.data());
  std::system(as_cmd.data());

  // Link object code and create a binary.
  const std::string ld_cmd = StringFormat(
      "ld -o %s %s.o -lSystem -syslibroot `xcrun -sdk macosx --show-sdk-path` "
      "-e _start -arch arm64",
      binary_name.data(), binary_name.data());
  std::system(ld_cmd.data());

  return std::nullopt;
}

}  // namespace

int Main(std::vector<std::string_view> args) {
  auto maybe_error = RunCommand({{"build", Build}}, args);
  if (maybe_error.has_value()) {
    std::cerr << *maybe_error << std::endl;
    return 1;
  }
  return 0;
}

}  // namespace lucid

int main(int argc, char* argv[]) {
  return lucid::Main(std::vector<std::string_view>(argv + 1, argv + argc));
}
