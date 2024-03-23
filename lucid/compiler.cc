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
    std::string_view src, Arena<Stmt>& stmt_arena, Arena<Expr>& expr_arena,
    Arena<Type>& type_arena) {
  std::vector<FuncDefStmt> func_defs;
  Lexer lexer(src);
  Parser parser(stmt_arena, expr_arena, type_arena, src, lexer);
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
  Arena<Stmt> stmt_arena;
  Arena<Expr> expr_arena;
  Arena<Type> type_arena;
  auto maybe_funcs = ParseFuncDefs(src, stmt_arena, expr_arena, type_arena);
  if (auto* err = std::get_if<ParserError>(&maybe_funcs)) return *err;
  auto& func_defs = std::get<std::vector<FuncDefStmt>>(maybe_funcs);
  auto func_types =
      ExtractFuncTypes(stmt_arena, expr_arena, type_arena, func_defs);
  AbstractMachineState state;
  GenerateArmStartSource(out);
  for (auto& func : func_defs) {
    if (auto err = InferExprTypes(stmt_arena, expr_arena, type_arena,
                                  func_types, func);
        err)
      return *err;
    auto graph = BuildControlFlowGraph(stmt_arena, expr_arena, func);
    GenerateAbstractMachineFunction(stmt_arena, expr_arena, type_arena, graph,
                                    state);
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
  const auto src_or_err =
      ReadFile(src_path.c_str(), /*with_trailing_zero=*/true);
  if (auto* err = std::get_if<ReadFileError>(&src_or_err); err != nullptr) {
    PrintError(ctx.err) << *err << "'\n";
    return 1;
  }
  const auto& src = std::get<std::string>(src_or_err);

  // Compile sources to assembly.
  auto build_dir = std::filesystem::temp_directory_path();
  auto assembly_path = build_dir / (std::string(binary_name) + ".s");
  {
    std::ofstream assembly_stream(assembly_path);
    if (auto err = CompileSource(src, assembly_stream); err) {
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
  const auto src_or_err =
      ReadFile(src_path.c_str(), /*with_trailing_zero=*/true);
  if (auto* err = std::get_if<ReadFileError>(&src_or_err); err != nullptr) {
    PrintError(ctx.err) << *err << "'\n";
    return 1;
  }
  const auto& src = std::get<std::string>(src_or_err);

  // Compile sources to assembly.
  auto assembly_path = std::filesystem::current_path() / src_path.filename();
  assembly_path.replace_extension("s");
  {
    std::ofstream assembly_stream(assembly_path);
    if (auto err = CompileSource(src, assembly_stream); err) {
      std::visit([&ctx](auto& err) { PrintError(ctx.err) << err << "\n"; },
                 *err);
      return 1;
    }
  }

  return 0;
}

int Run(CommandContext ctx) {
  if (ctx.args.size() != 1) {
    PrintError(ctx.err) << "'run' command requires exactly 1 argument\n";
    return 1;
  }

  auto build_dir = std::filesystem::temp_directory_path();
  std::string_view binary_name = "temp_binary";
  auto binary_path = build_dir / binary_name;

  // Load Lucid sources.
  auto src_path = std::filesystem::absolute(ctx.args[0]);
  const auto src_or_err =
      ReadFile(src_path.c_str(), /*with_trailing_zero=*/true);
  if (auto* err = std::get_if<ReadFileError>(&src_or_err); err != nullptr) {
    PrintError(ctx.err) << *err << "'\n";
    return 1;
  }
  const auto& src = std::get<std::string>(src_or_err);

  // Compile sources to assembly.
  auto assembly_path = build_dir / (std::string(binary_name) + ".s");
  {
    std::ofstream assembly_stream(assembly_path);
    if (auto err = CompileSource(src, assembly_stream); err) {
      std::visit([&ctx](auto& err) { PrintError(ctx.err) << err << "\n"; },
                 *err);
      return 1;
    }
  }

  // Translate assembly into object code.
  const std::string as_cmd = StringFormat(
      "as -arch arm64 -o %s.o %s", binary_path.c_str(), assembly_path.c_str());
  std::system(as_cmd.data());

  // Link object code and create a binary.
  const std::string ld_cmd = StringFormat(
      "ld -o %s %s.o -lSystem -syslibroot `xcrun -sdk macosx --show-sdk-path` "
      "-e _start -arch arm64",
      binary_path.c_str(), binary_path.c_str());
  std::system(ld_cmd.data());

  // Run the binary.
  int status = std::system(binary_path.c_str());
  return WEXITSTATUS(status);
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
              .name = "run",
              .help = "Compiles the specified target, builds a binary, and "
                      "runs it.",
              .handler = Run,
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
