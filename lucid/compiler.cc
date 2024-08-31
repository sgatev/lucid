#include <cstdlib>
#include <filesystem>
#include <format>
#include <fstream>
#include <iostream>
#include <string>
#include <string_view>
#include <utility>
#include <variant>
#include <vector>

#include "lucid/am_gen.h"
#include "lucid/arena.h"
#include "lucid/arm64.h"
#include "lucid/arm64_binary_gen.h"
#include "lucid/arm64_gen.h"
#include "lucid/ast.h"
#include "lucid/cfg.h"
#include "lucid/cli.h"
#include "lucid/file.h"
#include "lucid/lexer.h"
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

Result<void, ParserError, TypeError> CompileSource(
    std::string_view src, std::ostream& out, bool arm64_binary_gen = false) {
  SyntaxContext ctx;
  auto maybe_funcs = ParseFuncDefs(src, ctx);
  if (maybe_funcs.HasError()) return maybe_funcs.GetError();
  auto& func_defs = maybe_funcs.GetValue();
  auto func_types = ExtractFuncTypes(ctx, func_defs);
  AbstractMachineState state;
  arm64::Arm64 arm;
  if (arm64_binary_gen) {
    GenerateArmStartBinary(arm);
  } else {
    GenerateArmStartSource(out);
  }
  for (auto& func : func_defs) {
    if (auto err = InferExprTypes(ctx, func_types, func); err) return *err;
    auto graph = BuildControlFlowGraph(ctx, func);
    GenerateAbstractMachineFunction(ctx, graph, state);
    OptimizeAbstractMachineInstructions(state.func.instructions);
    if (arm64_binary_gen) {
      GenerateArmAssemblyBinary(state.func, arm);
    } else {
      GenerateArmAssemblySource(state.func, out);
    }
  }
  if (arm64_binary_gen) {
    std::vector<std::uint32_t> arm64_insts = arm.Encode();
    for (auto inst : arm64_insts) {
      out.write(reinterpret_cast<const char*>(&inst), 4);
    }
  } else {
    GenerateArmEndSource(state.strings, out);
  }
  return {};
}

Result<void, ReadFileError, ParserError, TypeError> DoCompile(
    std::filesystem::path src_path, std::filesystem::path asm_path,
    bool arm64_binary_gen = false) {
  // Read Lucid sources.
  const auto maybe_src = ReadFile(src_path, /*with_trailing_zero=*/true);
  if (maybe_src.HasError()) return maybe_src.GetError();
  const auto& src = maybe_src.GetValue();

  // Compile sources to assembly.
  std::ofstream assembly_stream(asm_path, std::ios::out | std::ios::binary);
  if (auto err = CompileSource(src, assembly_stream, arm64_binary_gen);
      err.HasError()) {
    return err.GetError();
  }

  return {};
}

Result<void, ReadFileError, ParserError, TypeError> DoBuild(
    std::filesystem::path bin_path, std::filesystem::path src_path) {
  auto asm_path = bin_path;
  asm_path.replace_extension("s");

  if (auto res = DoCompile(src_path, asm_path); res.HasError()) {
    return res.GetError();
  }

  // Translate assembly into object code.
  const std::string as_cmd = std::format("as -arch arm64 -o {}.o {}",
                                         bin_path.c_str(), asm_path.c_str());
  std::system(as_cmd.data());

  // Link object code and create a binary.
  const std::string ld_cmd = std::format(
      "ld -o {} {}.o -lSystem -syslibroot `xcrun -sdk macosx --show-sdk-path` "
      "-e _start -arch arm64",
      bin_path.c_str(), bin_path.c_str());
  std::system(ld_cmd.data());

  return {};
}

int Compile(CommandContext ctx) {
  if (ctx.args.size() != 1) {
    PrintError(ctx.err) << "'compile' command requires exactly 1 argument\n";
    return 1;
  }

  auto src_path = std::filesystem::absolute(ctx.args[0]);
  auto asm_path = std::filesystem::current_path() / src_path.filename();
  asm_path.replace_extension("s");

  if (auto res = DoCompile(src_path, asm_path); res.HasError()) {
    res.OutputError(PrintError(ctx.err));
    return 1;
  }

  return 0;
}

int CompileBinary(CommandContext ctx) {
  if (ctx.args.size() != 1) {
    PrintError(ctx.err)
        << "'compile_binary' command requires exactly 1 argument\n";
    return 1;
  }

  auto src_path = std::filesystem::absolute(ctx.args[0]);
  auto asm_path = std::filesystem::current_path() / src_path.filename();
  asm_path.replace_extension("s");

  if (auto res = DoCompile(src_path, asm_path, true); res.HasError()) {
    res.OutputError(PrintError(ctx.err));
    return 1;
  }

  return 0;
}

int Build(CommandContext ctx) {
  if (ctx.args.size() != 2) {
    PrintError(ctx.err) << "'build' command requires exactly 2 arguments\n";
    return 1;
  }

  auto src_path = std::filesystem::absolute(ctx.args[1]);
  auto bin_path = std::filesystem::temp_directory_path() / ctx.args[0];

  if (auto res = DoBuild(bin_path, src_path); res.HasError()) {
    res.OutputError(PrintError(ctx.err));
    return 1;
  }

  return 0;
}

int Run(CommandContext ctx) {
  if (ctx.args.size() != 1) {
    PrintError(ctx.err) << "'run' command requires exactly 1 argument\n";
    return 1;
  }

  auto src_path = std::filesystem::absolute(ctx.args[0]);
  auto bin_path = std::filesystem::temp_directory_path() / src_path.filename();
  bin_path.replace_extension();

  if (auto res = DoBuild(bin_path, src_path); res.HasError()) {
    res.OutputError(PrintError(ctx.err));
    return 1;
  }

  // Run the binary.
  int status = std::system(bin_path.c_str());
  return WEXITSTATUS(status);
}

int Parse(CommandContext ctx) {
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
              .name = "compile_binary",
              .help =
                  "Compiles the specified target with ARM64 binary generator.",
              .handler = CompileBinary,
          },
          {
              .name = "run",
              .help = "Compiles the specified target, builds a binary, and "
                      "runs it.",
              .handler = Run,
          },
          {
              .name = "parse",
              .help = "Parses the specified target.",
              .handler = Parse,
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
