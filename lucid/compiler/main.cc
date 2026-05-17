#include <cstdlib>
#include <expected>
#include <filesystem>
#include <iostream>
#include <sstream>
#include <string>
#include <string_view>
#include <utility>
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
#include "lucid/core/string/concat.h"
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
  if (ctx.args.size() != 1) {
    PrintError(ctx.err) << "'compile' command requires exactly 1 argument\n";
    return 1;
  }

  auto src_path = std::filesystem::absolute(ctx.args[0]);
  auto out_path = std::filesystem::current_path() / src_path.filename();
  out_path.replace_extension("o");

  return CompileCode({.src_path = src_path, .out_path = out_path})
      .and_then([] -> std::expected<int, CompileError> { return 0; })
      .or_else([&](CompileError err) -> std::expected<int, CompileError> {
        std::visit([&](const auto& err) { PrintError(ctx.err) << err; }, err);
        ctx.err << "\n";
        return 1;
      })
      .value();
}

int HandleBuildCommand(CommandContext ctx) {
  if (ctx.args.size() != 2) {
    PrintError(ctx.err) << "'build' command requires exactly 2 arguments\n";
    return 1;
  }

  auto src_path = std::filesystem::absolute(ctx.args[1]);
  auto bin_path = ctx.args[0];

  return BuildCode({.src_path = src_path, .out_path = bin_path})
      .and_then([]() -> std::expected<int, BuildError> { return 0; })
      .or_else([&](BuildError err) -> std::expected<int, BuildError> {
        std::visit([&](const auto& err) { PrintError(ctx.err) << err; }, err);
        ctx.err << "\n";
        return 1;
      })
      .value();
}

int HandleRunCommand(CommandContext ctx) {
  if (ctx.args.size() != 1) {
    PrintError(ctx.err) << "'run' command requires exactly 1 argument\n";
    return 1;
  }

  auto src_path = std::filesystem::absolute(ctx.args[0]);
  auto bin_path = std::filesystem::temp_directory_path() / src_path.filename();
  bin_path.replace_extension();

  return BuildCode({.src_path = src_path, .out_path = bin_path})
      .and_then([&]() -> std::expected<int, BuildError> {
        int status = std::system(bin_path.c_str());
        return WEXITSTATUS(status);
      })
      .or_else([&](BuildError err) -> std::expected<int, BuildError> {
        std::visit([&](const auto& err) { PrintError(ctx.err) << err; }, err);
        ctx.err << "\n";
        return 1;
      })
      .value();
}

int HandleParseCommand(CommandContext ctx) {
  if (ctx.args.size() != 1) {
    PrintError(ctx.err) << "'parse' command requires exactly 1 argument\n";
    return 1;
  }

  auto src_path = std::filesystem::absolute(ctx.args[0]);

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
        PrintError(ctx.err) << err;
        ctx.err << "\n";
        return 1;
      })
      .value();
}

int HandlePrintAstCommand(CommandContext ctx) {
  if (ctx.args.size() < 1 || ctx.args.size() > 2) {
    PrintError(ctx.err)
        << "'print-ast' command requires either 1 or 2 arguments\n";
    return 1;
  }

  auto src_path = std::filesystem::absolute(ctx.args[0]);
  const auto maybe_src = ReadFile(src_path, /*with_trailing_zero=*/true);
  if (!maybe_src.has_value()) {
    PrintError(ctx.err) << maybe_src.error();
    ctx.err << "\n";
    return 1;
  }
  const auto& src = maybe_src.value();

  SyntaxContext sctx;
  auto maybe_funcs = ParseFuncDefs(src, sctx);
  if (!maybe_funcs.has_value()) {
    PrintError(ctx.err) << maybe_funcs.error();
    ctx.err << "\n";
    return 1;
  }
  const auto& func_defs = maybe_funcs.value();

  if (ctx.args.size() == 2) {
    std::string_view id = ctx.args[1];
    if (id[0] == 'S') {
      id.remove_prefix(1);
      PrintStmt(sctx, std::stoi(std::string(id)), ctx.out);
    } else if (id[0] == 'E') {
      id.remove_prefix(1);
      PrintExpr(sctx, std::stoi(std::string(id)), ctx.out);
    } else {
      PrintError(ctx.err) << "second argument to 'print-ast' command must be "
                             "either 'S<index>' or 'E<index>'\n";
      return 1;
    }
  } else {
    for (const auto& func_def : func_defs) Print(sctx, func_def, ctx.out);
  }

  return 0;
}

int HandlePrintSyntaxCfgCommand(CommandContext ctx) {
  if (ctx.args.size() < 1) {
    ctx.out << "Usage: " << Concat(ctx.path, " ") << " <path> (<identifier>)\n"
            << "\n"
            << "Examples:\n"
            << "  print-syntax-cfg //my/source/file.lu\n"
            << "  print-syntax-cfg //my/source/file.lu E2\n";
    return 0;
  }

  auto src_path = std::filesystem::absolute(ctx.args[0]);
  const auto maybe_src = ReadFile(src_path, /*with_trailing_zero=*/true);
  if (!maybe_src.has_value()) {
    PrintError(ctx.err) << maybe_src.error();
    ctx.err << "\n";
    return 1;
  }
  const auto& src = maybe_src.value();

  SyntaxContext sctx;
  auto maybe_funcs = ParseFuncDefs(src, sctx);
  if (!maybe_funcs.has_value()) {
    PrintError(ctx.err) << maybe_funcs.error();
    ctx.err << "\n";
    return 1;
  }
  const auto& func_defs = maybe_funcs.value();

  for (const auto& func_def : func_defs) {
    auto graph = BuildControlFlowGraph(sctx, func_def);
    ConvertToStaticSingleAssignment(sctx, graph);
    Print(sctx, graph, ctx.out);
  }

  return 0;
}

int HandlePrintAmCfgCommand(CommandContext ctx) {
  if (ctx.args.size() < 1) {
    ctx.out << "Usage: " << Concat(ctx.path, " ")
            << " (--regs=spill|merge) <path>\n"
            << "\n"
            << "Examples:\n"
            << "  print-am-cfg //my/source/file.lu\n"
            << "  print-am-cfg --regs=spill //my/source/file.lu\n";
    return 0;
  }

  auto src_path = std::filesystem::absolute(ctx.args[0]);
  const auto maybe_src = ReadFile(src_path, /*with_trailing_zero=*/true);
  if (!maybe_src.has_value()) {
    PrintError(ctx.err) << maybe_src.error();
    ctx.err << "\n";
    return 1;
  }
  const auto& src = maybe_src.value();

  SyntaxContext syn_ctx;
  auto maybe_funcs = ParseFuncDefs(src, syn_ctx);
  if (!maybe_funcs.has_value()) {
    PrintError(ctx.err) << maybe_funcs.error();
    ctx.err << "\n";
    return 1;
  }
  auto& func_defs = maybe_funcs.value();

  AbstractMachineState state;
  HashMap<std::string_view, AbstractMachineControlFlowGraph> am_cfgs;

  for (bool has_printed_func = false; auto& func_def : func_defs) {
    if (auto res = InferExprTypes(syn_ctx, func_defs, func_def);
        !res.has_value()) {
      PrintError(ctx.err) << res.error();
      ctx.err << "\n";
      return 1;
    }
    if (auto res = CheckComp(func_defs, syn_ctx, func_def); !res.has_value()) {
      PrintError(ctx.err) << res.error();
      ctx.err << "\n";
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
    if (ctx.flags.Get("regs") == "spill") {
      SpillRegisters(am_cfg, state, kArmRegistersCount);
    } else if (ctx.flags.Get("regs") == "merge") {
      SpillRegisters(am_cfg, state, kArmRegistersCount);
      HashMap<RegId, HashSet<RegId>> am_ig = BuildInterferenceGraph(am_cfg);
      HashMap<RegId, int> am_ig_colors =
          ColorInterferenceGraph(am_cfg, am_ig, kArmRegistersCount);
      MergeRegisters(am_ig_colors, am_cfg);
    }

    if (has_printed_func) std::cout << "\n";
    Print(syn_ctx.DerefIdent(func_def.name), am_cfg, ctx.out);
    am_cfgs.Insert(syn_ctx.DerefIdent(func_def.name), std::move(am_cfg));
    has_printed_func = true;
  }

  return 0;
}

int HandleVersionCommand(CommandContext ctx) {
  ctx.out << "Commit: " << kGitCommit << "\n";

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
