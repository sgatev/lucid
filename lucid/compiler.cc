#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <iostream>
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
#include "lucid/version.h"

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

std::variant<std::vector<FuncDefStmt>, std::string> ParseFuncDefs(
    std::string_view src, Arena<Stmt>& arena) {
  std::vector<FuncDefStmt> func_defs;
  Lexer lexer(src);
  Parser parser(arena, src, lexer);
  while (true) {
    auto maybe_func_def = parser.ParseFuncDef();
    if (auto* err = std::get_if<ParserError>(&maybe_func_def)) {
      if (err->GetKind() == ParserError::Kind::End) break;
      return err->ToString();
    }
    func_defs.push_back(std::get<FuncDefStmt>(std::move(maybe_func_def)));
  }
  return func_defs;
}

std::optional<std::string> Compile(std::string_view src, std::ostream& out) {
  Arena<Stmt> arena;
  auto maybe_funcs = ParseFuncDefs(src, arena);
  if (auto* err = std::get_if<std::string>(&maybe_funcs)) {
    return *std::move(err);
  }
  GenerateArmStartSource(out);
  for (const auto& func : std::get<std::vector<FuncDefStmt>>(maybe_funcs)) {
    auto graph = BuildControlFlowGraph(arena, func);
    auto instructions = GenerateAbstractMachineInstructions(arena, graph);
    OptimizeAbstractMachineInstructions(instructions);
    GenerateArmAssemblySource(func.name, instructions, out);
  }
  return std::nullopt;
}

CommandResult Build(std::span<std::string_view> args) {
  if (args.size() != 2) {
    return {
        .return_code = 1,
        .err = "'build' command requires exactly 2 arguments",
    };
  }

  std::string_view binary_name = args[0];
  std::string_view src_path = args[1];

  // Load Lucid sources.
  const auto maybe_src = ReadFile(src_path);
  if (std::holds_alternative<FileError>(maybe_src)) {
    return {
        .return_code = 1,
        .err = "file error: could not read file " + std::string(src_path),
    };
  }
  const auto& src = std::get<std::string>(maybe_src);

  // Compile sources to assembly.
  auto build_dir = std::filesystem::temp_directory_path();
  auto assembly_path = build_dir / (std::string(binary_name) + ".s");
  {
    std::ofstream assembly_stream(assembly_path);
    if (auto err = Compile(src, assembly_stream); err) {
      return {.return_code = 1, .err = *err};
    }
  }

  // Translate assembly into object code.
  const std::string as_cmd = StringFormat(
      "as -arch arm64 -o %s.o %s", binary_name.data(), assembly_path.c_str());
  std::system(as_cmd.data());

  // Link object code and create a binary.
  const std::string ld_cmd = StringFormat(
      "ld -o %s %s.o -lSystem -syslibroot `xcrun -sdk macosx --show-sdk-path` "
      "-e _start -arch arm64",
      binary_name.data(), binary_name.data());
  std::system(ld_cmd.data());

  return {.return_code = 0};
}

CommandResult Version(std::span<std::string_view> args) {
  return {
      .return_code = 0,
      .out = "Commit: " + std::string(kGitCommit),
  };
}

}  // namespace

int Main(std::vector<std::string_view> args) {
  auto result = RunCommand(
      "lucid",
      {
          {
              .name = "build",
              .help = "Compiles the specified target and builds a binary.",
              .handler = Build,
          },
          {
              .name = "version",
              .help = "Prints version information for lucid.",
              .handler = Version,
          },
      },
      args);
  std::cout << result.out;
  std::cerr << result.err << std::endl;
  return result.return_code;
}

}  // namespace lucid

int main(int argc, char* argv[]) {
  return lucid::Main(std::vector<std::string_view>(argv + 1, argv + argc));
}
