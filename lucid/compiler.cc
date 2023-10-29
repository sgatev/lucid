#include <cstdio>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <optional>
#include <stdexcept>
#include <string>
#include <string_view>
#include <unordered_map>
#include <utility>
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
#include "lucid/type.h"
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

std::variant<std::vector<FuncDefStmt>, ParserError> ParseFuncDefs(
    std::string_view src, Arena<Stmt>& arena) {
  std::vector<FuncDefStmt> func_defs;
  Lexer lexer(src);
  Parser parser(arena, src, lexer);
  while (true) {
    auto maybe_func_def = parser.ParseFuncDef();
    if (auto* err = std::get_if<ParserError>(&maybe_func_def)) {
      if (err->GetKind() == ParserError::Kind::End) break;
      return std::move(*err);
    }
    func_defs.push_back(std::get<FuncDefStmt>(std::move(maybe_func_def)));
  }
  return func_defs;
}

using CompileError = std::variant<ParserError, TypeError>;

std::optional<CompileError> CompileSource(std::string_view src,
                                          std::ostream& out) {
  Arena<Stmt> arena;
  auto maybe_funcs = ParseFuncDefs(src, arena);
  if (auto* err = std::get_if<ParserError>(&maybe_funcs)) return *err;
  auto& func_defs = std::get<std::vector<FuncDefStmt>>(maybe_funcs);
  auto func_types = ExtractFuncTypes(arena, func_defs);
  AbstractMachineState state;
  GenerateArmStartSource(out);
  for (auto& func : func_defs) {
    if (auto err = InferExprTypes(arena, func_types, func); err) return *err;
    auto graph = BuildControlFlowGraph(arena, func);
    GenerateAbstractMachineFunction(arena, graph, state);
    OptimizeAbstractMachineInstructions(state.func.instructions);
    GenerateArmAssemblySource(state.func, out);
  }
  GenerateArmEndSource(state.strings, out);
  return std::nullopt;
}

int Build(CommandContext ctx) {
  if (ctx.args.size() != 2) {
    PrintError(ctx.err) << "'build' command requires exactly 2 arguments\n";
    return 1;
  }

  std::string_view binary_name = ctx.args[0];

  // Load Lucid sources.
  auto src_path = std::filesystem::absolute(ctx.args[1]);
  const auto src = ReadFile(src_path.c_str());
  if (!src.has_value()) {
    PrintError(ctx.err) << "could not read file '" << ctx.args[1] << "'\n";
    return 1;
  }

  // Compile sources to assembly.
  auto build_dir = std::filesystem::temp_directory_path();
  auto assembly_path = build_dir / (std::string(binary_name) + ".s");
  {
    std::ofstream assembly_stream(assembly_path);
    if (auto err = CompileSource(*src, assembly_stream); err) {
      std::visit([&ctx](auto& err) { PrintError(ctx.err) << err << "\n"; },
                 *err);
      return 1;
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

  return 0;
}

int Compile(CommandContext ctx) {
  if (ctx.args.size() != 1) {
    PrintError(ctx.err) << "'compile' command requires exactly 1 argument\n";
    return 1;
  }

  // Load Lucid sources.
  auto src_path = std::filesystem::absolute(ctx.args[0]);
  const auto src = ReadFile(src_path.c_str());
  if (!src.has_value()) {
    PrintError(ctx.err) << "could not read file '" << ctx.args[0] << "'\n";
    return 1;
  }

  // Compile sources to assembly.
  auto assembly_path = std::filesystem::current_path() / src_path.filename();
  assembly_path.replace_extension("s");
  {
    std::ofstream assembly_stream(assembly_path);
    if (auto err = CompileSource(*src, assembly_stream); err) {
      std::visit([&ctx](auto& err) { PrintError(ctx.err) << err << "\n"; },
                 *err);
      return 1;
    }
  }

  return 0;
}

int Version(CommandContext ctx) {
  ctx.out << "Commit: " << kGitCommit << "\n";
  return 0;
}

}  // namespace

int Main(std::vector<std::string_view> args) {
  return RunCommand(
      "lucid",
      {
          {
              .name = "build",
              .help = "Compiles the specified target and builds a binary.",
              .handler = Build,
          },
          {
              .name = "compile",
              .help = "Compiles the specified target.",
              .handler = Compile,
          },
          {
              .name = "version",
              .help = "Prints version information for lucid.",
              .handler = Version,
          },
      },
      {args, std::cout, std::cerr});
}

}  // namespace lucid

int main(int argc, char* argv[]) {
  return lucid::Main(std::vector<std::string_view>(argv + 1, argv + argc));
}
