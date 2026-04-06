#include <cstdlib>
#include <filesystem>
#include <string>
#include <string_view>
#include <vector>

#include "lucid/am/cfg.h"
#include "lucid/am/cfg_printer.h"
#include "lucid/am/ig.h"
#include "lucid/am/opt.h"
#include "lucid/am/reg.h"
#include "lucid/am/translator.h"
#include "lucid/compiler/compiler.h"
#include "lucid/compiler/version.h"
#include "lucid/core/cli/cli.h"
#include "lucid/core/functional/result.h"
#include "lucid/core/io/file.h"
#include "lucid/syntax/ast.h"
#include "lucid/syntax/ast_printer.h"
#include "lucid/syntax/cfg.h"
#include "lucid/syntax/cfg_printer.h"
#include "lucid/syntax/ssa.h"
#include "lucid/syntax/type.h"

namespace lucid {
namespace {

int HandleCompileCommand(CommandContext ctx) {
  if (ctx.args.size() != 1) {
    PrintError(ctx.err) << "'compile' command requires exactly 1 argument\n";
    return 1;
  }

  auto src_path = std::filesystem::absolute(ctx.args[0]);
  auto out_path = std::filesystem::current_path() / src_path.filename();
  out_path.replace_extension("o");

  if (auto res = CompileCode({.src_path = src_path, .out_path = out_path});
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

  if (auto res = BuildCode({.src_path = src_path, .out_path = bin_path});
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

  if (auto res = BuildCode({.src_path = src_path, .out_path = bin_path});
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
    ConvertToStaticSingleAssignment(sctx, cfg);
    DestroyStaticSingleAssignment(sctx, cfg);
    AbstractMachineState state;
    AbstractMachineControlFlowGraph am_cfg =
        GenerateAbstractMachineFunction(sctx, cfg, state);
    for (auto& block : am_cfg.blocks()) {
      OptimizeAbstractMachineInstructions(block.instructions);
    }
    if (ctx.flags["regs"] == "spill") {
      SpillRegisters(am_cfg, state.stack_slots);
    } else if (ctx.flags["regs"] == "merge") {
      SpillRegisters(am_cfg, state.stack_slots);
      HashMap<RegId, HashSet<RegId>> am_ig = BuildInterferenceGraph(am_cfg);
      HashMap<RegId, int> am_ig_colors =
          ColorInterferenceGraph(am_cfg, am_ig, 10);
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
