#include <cstdlib>
#include <expected>
#include <filesystem>
#include <iostream>
#include <optional>
#include <sstream>
#include <string>
#include <string_view>
#include <utility>
#include <variant>
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
#include "lucid/core/io/file.h"
#include "lucid/syntax/ast.h"
#include "lucid/syntax/ast_printer.h"
#include "lucid/syntax/cfg.h"
#include "lucid/syntax/cfg_printer.h"
#include "lucid/syntax/parser.h"
#include "lucid/syntax/ssa.h"
#include "lucid/syntax/type.h"

namespace lucid {
namespace {

int HandleCompileCommand(CommandContext ctx) {
  std::optional<std::string_view> src_file = ctx.TakeArg();
  if (!src_file) {
    ctx.Err() << "'compile' command requires exactly 1 argument\n";
    return 1;
  }

  auto src_path = std::filesystem::absolute(*src_file);
  auto out_path = std::filesystem::current_path() / src_path.filename();
  out_path.replace_extension("o");

  return CompileCode({.src_path = src_path, .out_path = out_path})
      .and_then([] -> std::expected<int, CompileError> { return 0; })
      .or_else([&](CompileError err) -> std::expected<int, CompileError> {
        std::visit([&](const auto& err) { ctx.Err() << err << "\n"; }, err);
        return 1;
      })
      .value();
}

int HandleBuildCommand(CommandContext ctx) {
  std::optional<std::string_view> bin_path = ctx.TakeArg();
  std::optional<std::string_view> src_file = ctx.TakeArg();
  if (!bin_path || !src_file) {
    ctx.Err() << "'build' command requires exactly 2 arguments\n";
    return 1;
  }

  auto src_path = std::filesystem::absolute(*src_file);

  return BuildCode({.src_path = src_path, .out_path = *bin_path})
      .and_then([]() -> std::expected<int, BuildError> { return 0; })
      .or_else([&](BuildError err) -> std::expected<int, BuildError> {
        std::visit([&](const auto& err) { ctx.Err() << err << "\n"; }, err);
        return 1;
      })
      .value();
}

int HandleRunCommand(CommandContext ctx) {
  std::optional<std::string_view> src_file = ctx.TakeArg();
  if (!src_file) {
    ctx.Err() << "'run' command requires exactly 1 argument\n";
    return 1;
  }

  auto src_path = std::filesystem::absolute(*src_file);
  auto bin_path = std::filesystem::temp_directory_path() / src_path.filename();
  bin_path.replace_extension();

  return BuildCode({.src_path = src_path, .out_path = bin_path})
      .and_then([&]() -> std::expected<int, BuildError> {
        int status = std::system(bin_path.c_str());
        return WEXITSTATUS(status);
      })
      .or_else([&](BuildError err) -> std::expected<int, BuildError> {
        std::visit([&](const auto& err) { ctx.Err() << err << "\n"; }, err);
        return 1;
      })
      .value();
}

int HandleParseCommand(CommandContext ctx) {
  std::optional<std::string_view> src_file = ctx.TakeArg();
  if (!src_file) {
    ctx.Err() << "'parse' command requires exactly 1 argument\n";
    return 1;
  }

  auto src_path = std::filesystem::absolute(*src_file);

  return ReadFile(src_path, /*with_trailing_zero=*/true)
      .or_else(
          [](ReadFileError err) -> std::expected<std::string, std::string> {
            std::stringstream ss;
            ss << err;
            return std::unexpected(ss.str());
          })
      .and_then([](std::string src)
                    -> std::expected<std::vector<FuncDefStmt>, std::string> {
        SyntaxContext sctx;
        return ParseFuncDefs(src, sctx).or_else(
            [](ParserError err)
                -> std::expected<std::vector<FuncDefStmt>, std::string> {
              std::stringstream ss;
              ss << err;
              return std::unexpected(ss.str());
            });
      })
      .and_then(
          [](std::vector<FuncDefStmt>) -> std::expected<int, std::string> {
            return 0;
          })
      .or_else([&](std::string err) -> std::expected<int, std::string> {
        ctx.Err() << err << "\n";
        return 1;
      })
      .value();
}

int HandlePrintAstCommand(CommandContext ctx) {
  std::optional<std::string_view> src_file = ctx.TakeArg();
  if (!src_file) {
    ctx.Err() << "'print-ast' command requires either 1 or 2 arguments\n";
    return 1;
  }

  std::optional<std::string_view> id = ctx.TakeArg();

  auto src_path = std::filesystem::absolute(*src_file);
  const auto maybe_src = ReadFile(src_path, /*with_trailing_zero=*/true);
  if (!maybe_src.has_value()) {
    ctx.Err() << maybe_src.error() << "\n";
    return 1;
  }
  const auto& src = maybe_src.value();

  SyntaxContext sctx;
  auto maybe_funcs = ParseFuncDefs(src, sctx);
  if (!maybe_funcs.has_value()) {
    ctx.Err() << maybe_funcs.error() << "\n";
    return 1;
  }
  const auto& func_defs = maybe_funcs.value();

  if (id) {
    if (id->front() == 'S') {
      PrintStmt(sctx, std::stoi(std::string(id->substr(1))), ctx.Out());
    } else if (id->front() == 'E') {
      PrintExpr(sctx, std::stoi(std::string(id->substr(1))), ctx.Out());
    } else {
      ctx.Err() << "second argument to 'print-ast' command must be "
                   "either 'S<index>' or 'E<index>'\n";
      return 1;
    }
  } else {
    for (const auto& func_def : func_defs) Print(sctx, func_def, ctx.Out());
  }

  return 0;
}

int HandlePrintSyntaxCfgCommand(CommandContext ctx) {
  std::optional<std::string_view> src_file = ctx.TakeArg();
  if (!src_file) {
    ctx.Out() << "Usage: " << ctx.CurrentCommand() << " <path> (<identifier>)\n"
              << "\n"
              << "Examples:\n"
              << "  print-syntax-cfg //my/source/file.lu\n"
              << "  print-syntax-cfg //my/source/file.lu E2\n";
    return 0;
  }

  auto src_path = std::filesystem::absolute(*src_file);
  const auto maybe_src = ReadFile(src_path, /*with_trailing_zero=*/true);
  if (!maybe_src.has_value()) {
    ctx.Err() << maybe_src.error() << "\n";
    return 1;
  }
  const auto& src = maybe_src.value();

  SyntaxContext sctx;
  auto maybe_funcs = ParseFuncDefs(src, sctx);
  if (!maybe_funcs.has_value()) {
    ctx.Err() << maybe_funcs.error() << "\n";
    return 1;
  }
  const auto& func_defs = maybe_funcs.value();

  for (const auto& func_def : func_defs) {
    auto graph = BuildControlFlowGraph(sctx, func_def);
    ConvertToStaticSingleAssignment(sctx, graph);
    Print(sctx, graph, ctx.Out());
  }

  return 0;
}

int HandlePrintAmCfgCommand(CommandContext ctx) {
  std::optional<std::string_view> src_file = ctx.TakeArg();
  if (!src_file) {
    ctx.Out() << "Usage: " << ctx.CurrentCommand()
              << " (--regs=spill|merge) <path>\n"
              << "\n"
              << "Examples:\n"
              << "  print-am-cfg //my/source/file.lu\n"
              << "  print-am-cfg --regs=spill //my/source/file.lu\n";
    return 0;
  }

  auto src_path = std::filesystem::absolute(*src_file);
  const auto maybe_src = ReadFile(src_path, /*with_trailing_zero=*/true);
  if (!maybe_src.has_value()) {
    ctx.Err() << maybe_src.error() << "\n";
    return 1;
  }
  const auto& src = maybe_src.value();

  SyntaxContext syn_ctx;
  auto maybe_funcs = ParseFuncDefs(src, syn_ctx);
  if (!maybe_funcs.has_value()) {
    ctx.Err() << maybe_funcs.error() << "\n";
    return 1;
  }
  auto& func_defs = maybe_funcs.value();

  AbstractMachineState state;
  HashMap<std::string_view, AbstractMachineControlFlowGraph> am_cfgs;

  for (bool has_printed_func = false; auto& func_def : func_defs) {
    if (auto res = InferExprTypes(syn_ctx, func_defs, func_def);
        !res.has_value()) {
      ctx.Err() << res.error() << "\n";
      return 1;
    }
    if (auto res = CheckComp(func_defs, syn_ctx, func_def); !res.has_value()) {
      ctx.Err() << res.error() << "\n";
      return 1;
    }
    SyntaxControlFlowGraph syn_cfg = BuildControlFlowGraph(syn_ctx, func_def);
    ConvertToStaticSingleAssignment(syn_ctx, syn_cfg);

    AbstractMachineControlFlowGraph am_cfg =
        GenerateAbstractMachineFunction(am_cfgs, syn_ctx, syn_cfg, state);
    for (auto& block : am_cfg.blocks()) {
      OptimizeAbstractMachineInstructions(block.instructions);
    }
    static constexpr int kArmRegistersCount = 10;
    if (ctx.Flag("regs") == "spill") {
      SpillRegisters(am_cfg, state, kArmRegistersCount);
    } else if (ctx.Flag("regs") == "merge") {
      SpillRegisters(am_cfg, state, kArmRegistersCount);
      HashMap<Reg, HashSet<Reg>> am_ig = BuildInterferenceGraph(am_cfg);
      HashMap<Reg, int> am_ig_colors =
          ColorInterferenceGraph(am_cfg, am_ig, kArmRegistersCount);
      MergeRegisters(am_ig_colors, am_cfg);
    }

    if (has_printed_func) std::cout << "\n";
    Print(syn_ctx.DerefIdent(func_def.name), am_cfg, ctx.Out());
    am_cfgs.Insert(syn_ctx.DerefIdent(func_def.name), std::move(am_cfg));
    has_printed_func = true;
  }

  return 0;
}

int HandleVersionCommand(CommandContext ctx) {
  ctx.Out() << "Commit: " << kGitCommit << "\n";

  return 0;
}

int HandleRoot(CommandContext ctx) {
  return RunCommand(
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
              .name = "print-syntax-cfg",
              .help = "Parses the specified target and prints the syntax CFG.",
              .handler = HandlePrintSyntaxCfgCommand,
          },
          {
              .name = "print-am-cfg",
              .help = "Parses the specified target and prints the abstract "
                      "machine CFG.",
              .handler = HandlePrintAmCfgCommand,
          },
          {
              .name = "version",
              .help = "Prints version information for lucid.",
              .handler = HandleVersionCommand,
          },
      },
      ctx);
}

}  // namespace

int Run(std::vector<std::string_view> args) {
  args[0] = "lucid";
  return RunCommand({{.name = args[0], .handler = HandleRoot}},
                    StandardRootCommandContext(args));
}

}  // namespace lucid

int main(int argc, char* argv[]) { return lucid::Run({argv, argv + argc}); }
