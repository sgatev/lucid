#include <cstdlib>
#include <filesystem>
#include <format>
#include <fstream>
#include <iostream>
#include <optional>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

#include "lucid/am/cfg.h"
#include "lucid/am/cfg_printer.h"
#include "lucid/am/translator.h"
#include "lucid/arm64.h"
#include "lucid/arm64_gen.h"
#include "lucid/cli.h"
#include "lucid/file.h"
#include "lucid/macho.h"
#include "lucid/opt.h"
#include "lucid/reg.h"
#include "lucid/result.h"
#include "lucid/ssa.h"
#include "lucid/static.h"
#include "lucid/syntax/ast.h"
#include "lucid/syntax/ast_printer.h"
#include "lucid/syntax/buffered_lexer.h"
#include "lucid/syntax/cfg.h"
#include "lucid/syntax/cfg_printer.h"
#include "lucid/syntax/lexer.h"
#include "lucid/syntax/parser.h"
#include "lucid/type.h"
#include "lucid/version.h"

namespace lucid {
namespace {

Result<std::vector<FuncDefStmt>, ParserError> ParseFuncDefs(
    std::string_view src, SyntaxContext& ctx) {
  std::vector<FuncDefStmt> func_defs;
  BufferedLexer<Lexer> lexer(Lexer{src});
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

Result<void, ParserError, TypeError, StaticError> CompileSource(
    std::string_view src, std::ostream& out) {
  SyntaxContext ctx;
  auto maybe_funcs = ParseFuncDefs(src, ctx);
  if (maybe_funcs.HasError()) return maybe_funcs.GetError();
  auto& func_defs = maybe_funcs.GetValue();
  // TODO: Avoid optional state here
  std::optional<AbstractMachineState> state;
  arm64::Assembler assembler;
  GenerateArmStartBinary(assembler);
  for (auto& func : func_defs) {
    if (!state.has_value()) {
      state.emplace(AbstractMachineState{});
    }
    if (auto res = InferExprTypes(ctx, func_defs, func); res.HasError()) {
      return res.GetError();
    }
    ControlFlowGraph cfg = BuildControlFlowGraph(ctx, func);
    if (auto res = InferStaticExprs(ctx, cfg); res.HasError()) {
      return res.GetError();
    }
    ConvertToStaticSingleAssignment(ctx, cfg);
    DestroyStaticSingleAssignment(ctx, cfg);
    AbstractMachineControlFlowGraph am_cfg =
        GenerateAbstractMachineFunction(ctx, cfg, *state);
    for (auto& block : am_cfg.blocks()) {
      OptimizeAbstractMachineInstructions(block.instructions);
    }
    HashMap<RegId, HashSet<RegId>> am_ig = BuildInterferenceGraph(am_cfg);
    HashMap<RegId, int> am_ig_colors =
        ColorInterferenceGraph(am_cfg, am_ig, 12);
    MergeRegisters(am_ig_colors, am_cfg);
    GenerateArmAssemblyBinary(ctx.DerefIdent(func.name), state->stack_slots,
                              am_cfg, assembler);
  }
  GenerateArmEndBinary(ctx, state->strings, assembler);
  WriteCompiledMachObject(assembler, out);
  return {};
}

struct CompileConfig {
  std::filesystem::path src_path;
  std::filesystem::path out_path;
};

Result<void, ReadFileError, ParserError, TypeError, StaticError> Compile(
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

Result<void, ReadFileError, ParserError, TypeError, StaticError> Build(
    BuildConfig config) {
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

int HandlePrintAstCommand(CommandContext ctx) {
  if (ctx.args.size() < 1 || ctx.args.size() > 2) {
    PrintError(ctx.err)
        << "'print-ast' command requires either 1 or 2 arguments\n";
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
  const auto& func_defs = maybe_funcs.GetValue();

  if (ctx.args.size() == 2) {
    std::string_view id = ctx.args[1];
    if (id[0] == 'S') {
      id.remove_prefix(1);
      PrintStmt(sctx, std::stoi(std::string(id)));
    } else if (id[0] == 'E') {
      id.remove_prefix(1);
      PrintExpr(sctx, std::stoi(std::string(id)));
    } else {
      PrintError(ctx.err) << "second argument to 'print-ast' command must be "
                             "either 'S<index>' or 'E<index>'\n";
      return 1;
    }
  } else {
    for (const auto& func_def : func_defs) Print(sctx, func_def);
  }

  return 0;
}

int HandlePrintCfgCommand(CommandContext ctx) {
  if (ctx.args.size() < 1) {
    PrintError(ctx.err) << "'print-cfg' command requires 1 argument\n";
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
  const auto& func_defs = maybe_funcs.GetValue();

  for (const auto& func_def : func_defs) {
    auto graph = BuildControlFlowGraph(sctx, func_def);
    if (ctx.flags["ssa"] == "after-init") {
      ConvertToStaticSingleAssignment(sctx, graph);
    } else if (ctx.flags["ssa"] == "after-deinit") {
      ConvertToStaticSingleAssignment(sctx, graph);
      DestroyStaticSingleAssignment(sctx, graph);
    }
    Print(sctx, graph);
  }

  return 0;
}

int HandlePrintAmiCommand(CommandContext ctx) {
  if (ctx.args.size() < 1) {
    PrintError(ctx.err) << "'print-ami' command requires 1 argument\n";
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
  auto& func_defs = maybe_funcs.GetValue();

  for (auto& func_def : func_defs) {
    if (auto res = InferExprTypes(sctx, func_defs, func_def); res.HasError()) {
      res.OutputError(PrintError(ctx.err));
      return 1;
    }
    auto cfg = BuildControlFlowGraph(sctx, func_def);
    if (ctx.flags["ssa"] == "after-init") {
      ConvertToStaticSingleAssignment(sctx, cfg);
    } else if (ctx.flags["ssa"] == "after-deinit") {
      ConvertToStaticSingleAssignment(sctx, cfg);
      DestroyStaticSingleAssignment(sctx, cfg);
    }

    AbstractMachineState state;
    AbstractMachineControlFlowGraph am_cfg =
        GenerateAbstractMachineFunction(sctx, cfg, state);
    for (auto& block : am_cfg.blocks()) {
      OptimizeAbstractMachineInstructions(block.instructions);
    }
    if (ctx.flags.contains("merge-regs")) {
      HashMap<RegId, HashSet<RegId>> am_ig = BuildInterferenceGraph(am_cfg);
      HashMap<RegId, int> am_ig_colors =
          ColorInterferenceGraph(am_cfg, am_ig, 14);
      MergeRegisters(am_ig_colors, am_cfg);
    }
    Print(sctx.DerefIdent(func_def.name), am_cfg);
  }

  return 0;
}

int HandleVersionCommand(CommandContext ctx) {
  ctx.out << "Commit: " << kGitCommit << "\n";

  return 0;
}

}  // namespace

int Run(std::vector<std::string_view> args) {
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
              .name = "print-ast",
              .help = "Parses the specified target and prints the AST.",
              .handler = HandlePrintAstCommand,
          },
          {
              .name = "print-cfg",
              .help = "Parses the specified target and prints the CFG.",
              .handler = HandlePrintCfgCommand,
          },
          {
              .name = "print-ami",
              .help = "Parses the specified target and prints the AMI.",
              .handler = HandlePrintAmiCommand,
          },
          {
              .name = "version",
              .help = "Prints version information for lucid.",
              .handler = HandleVersionCommand,
          },
      },
      StandardRootCommandContext(args));
}

}  // namespace lucid

int main(int argc, char* argv[]) { return lucid::Run({argv + 1, argv + argc}); }
