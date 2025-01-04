#include <cstdlib>
#include <filesystem>
#include <format>
#include <fstream>
#include <iostream>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

#include "lucid/am_gen.h"
#include "lucid/arm64.h"
#include "lucid/arm64_gen.h"
#include "lucid/ast.h"
#include "lucid/cfg.h"
#include "lucid/cli.h"
#include "lucid/file.h"
#include "lucid/lexer.h"
#include "lucid/macho.h"
#include "lucid/opt.h"
#include "lucid/parser.h"
#include "lucid/result.h"
#include "lucid/type.h"
#include "lucid/version.h"

namespace lucid {
namespace {

Result<std::vector<FuncDefStmt>, ParserError> ParseFuncDefs(
    std::string_view src, SyntaxContext& ctx) {
  std::vector<FuncDefStmt> func_defs;
  Lexer lexer(src);
  Parser parser(ctx, src, lexer);
  while (true) {
    auto maybe_func_def = parser.ParseFuncDef();
    if (auto* err = std::get_if<ParserError>(&maybe_func_def)) {
      return std::move(*err);
    }

    auto func_def =
        std::get<std::optional<FuncDefStmt>>(std::move(maybe_func_def));
    if (!func_def.has_value()) break;

    func_defs.push_back(std::move(*func_def));
  }
  return func_defs;
}

Result<void, ParserError, TypeError> CompileSource(std::string_view src,
                                                   std::ostream& out) {
  SyntaxContext ctx;
  auto maybe_funcs = ParseFuncDefs(src, ctx);
  if (maybe_funcs.HasError()) return maybe_funcs.GetError();
  auto& func_defs = maybe_funcs.GetValue();
  AbstractMachineState state;
  arm64::Assembler assembler;
  GenerateArmStartBinary(assembler);
  for (auto& func : func_defs) {
    if (auto res = InferExprTypes(ctx, func_defs, func); res.HasError()) {
      return res.GetError();
    }
    auto graph = BuildControlFlowGraph(ctx, func);
    GenerateAbstractMachineFunction(ctx, graph, state);
    OptimizeAbstractMachineInstructions(state.func.instructions);
    GenerateArmAssemblyBinary(state.func, assembler);
  }
  GenerateArmEndBinary(state.strings, assembler);
  WriteCompiledMachObject(assembler, out);
  return {};
}

struct CompileConfig {
  std::filesystem::path src_path;
  std::filesystem::path out_path;
};

Result<void, ReadFileError, ParserError, TypeError> Compile(
    CompileConfig config) {
  const auto maybe_src = ReadFile(config.src_path, /*with_trailing_zero=*/true);
  if (maybe_src.HasError()) return maybe_src.GetError();
  const auto& src = maybe_src.GetValue();

  std::ofstream out(config.out_path, std::ios::out | std::ios::binary);
  return CompileSource(src, out);
}

struct BuildConfig {
  std::filesystem::path src_path;
  std::filesystem::path out_path;
};

Result<void, ReadFileError, ParserError, TypeError> Build(BuildConfig config) {
  auto obj_path = config.out_path;
  obj_path.replace_extension("o");

  if (auto res = Compile({.src_path = config.src_path, .out_path = obj_path});
      res.HasError()) {
    return res.GetError();
  }

  std::system(std::format("ld -o {} {} -lSystem -syslibroot `xcrun -sdk "
                          "macosx --show-sdk-path` -e _start -arch arm64",
                          config.out_path.c_str(), obj_path.c_str())
                  .data());
  return {};
}

int HandleCompileCommand(CommandContext ctx) {
  if (ctx.args.size() != 1) {
    PrintError(ctx.err) << "'compile' command requires exactly 1 argument\n";
    return 1;
  }

  auto src_path = std::filesystem::absolute(ctx.args[0]);
  auto out_path = std::filesystem::current_path() / src_path.filename();
  out_path.replace_extension("o");

  if (auto res = Compile({.src_path = src_path, .out_path = out_path});
      res.HasError()) {
    res.OutputError(PrintError(ctx.err));
    return 1;
  }

  return 0;
}

int HandleBuildCommand(CommandContext ctx) {
  if (ctx.args.size() != 2) {
    PrintError(ctx.err) << "'build' command requires exactly 2 arguments\n";
    return 1;
  }

  auto src_path = std::filesystem::absolute(ctx.args[1]);
  auto bin_path = ctx.args[0];

  if (auto res = Build({.src_path = src_path, .out_path = bin_path});
      res.HasError()) {
    res.OutputError(PrintError(ctx.err));
    return 1;
  }

  return 0;
}

int HandleRunCommand(CommandContext ctx) {
  if (ctx.args.size() != 1) {
    PrintError(ctx.err) << "'run' command requires exactly 1 argument\n";
    return 1;
  }

  auto src_path = std::filesystem::absolute(ctx.args[0]);
  auto bin_path = std::filesystem::temp_directory_path() / src_path.filename();
  bin_path.replace_extension();

  if (auto res = Build({.src_path = src_path, .out_path = bin_path});
      res.HasError()) {
    res.OutputError(PrintError(ctx.err));
    return 1;
  }

  int status = std::system(bin_path.c_str());
  return WEXITSTATUS(status);
}

int HandleParseCommand(CommandContext ctx) {
  if (ctx.args.size() != 1) {
    PrintError(ctx.err) << "'parse' command requires exactly 1 argument\n";
    return 1;
  }

  auto src_path = std::filesystem::absolute(ctx.args[0]);
  const auto maybe_src = ReadFile(src_path, /*with_trailing_zero=*/true);
  if (maybe_src.HasError()) {
    maybe_src.OutputError(PrintError(ctx.err));
    return 1;
  }
  const auto& src = maybe_src.GetValue();

  SyntaxContext sctx;
  auto maybe_funcs = ParseFuncDefs(src, sctx);
  if (maybe_funcs.HasError()) {
    maybe_funcs.OutputError(PrintError(ctx.err));
    return 1;
  }

  return 0;
}

int HandleVersionCommand(CommandContext ctx) {
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
              .handler = HandleBuildCommand,
          },
          {
              .name = "compile",
              .help = "Compiles the specified target.",
              .handler = HandleCompileCommand,
          },
          {
              .name = "run",
              .help = "Compiles the specified target, builds a binary, and "
                      "runs it.",
              .handler = HandleRunCommand,
          },
          {
              .name = "parse",
              .help = "Parses the specified target.",
              .handler = HandleParseCommand,
          },
          {
              .name = "version",
              .help = "Prints version information for lucid.",
              .handler = HandleVersionCommand,
          },
      },
      {.args = args, .flags = {}, .out = std::cout, .err = std::cerr});
}

}  // namespace lucid

int main(int argc, char* argv[]) {
  return lucid::Main({argv + 1, argv + argc});
}
