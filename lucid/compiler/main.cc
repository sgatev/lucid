#include <sys/wait.h>
#include <unistd.h>

#include <charconv>
#include <cstdint>
#include <cstdlib>
#include <expected>
#include <filesystem>
#include <format>
#include <optional>
#include <sstream>
#include <string>
#include <string_view>
#include <system_error>
#include <utility>
#include <variant>
#include <vector>

#include "lucid/am/abi.h"
#include "lucid/am/cfg.h"
#include "lucid/am/cfg_printer.h"
#include "lucid/am/liveness.h"
#include "lucid/am/opt.h"
#include "lucid/am/reg.h"
#include "lucid/am/state.h"
#include "lucid/am/translator.h"
#include "lucid/arm64/translator.h"
#include "lucid/compiler/compiler.h"
#include "lucid/compiler/version.h"
#include "lucid/core/cli/cli.h"
#include "lucid/core/container/hash_map.h"
#include "lucid/core/io/file.h"
#include "lucid/syntax/ast.h"
#include "lucid/syntax/ast_printer.h"
#include "lucid/syntax/cfg.h"
#include "lucid/syntax/cfg_printer.h"
#include "lucid/syntax/comp.h"
#include "lucid/syntax/context.h"
#include "lucid/syntax/parser.h"
#include "lucid/syntax/ssa.h"
#include "lucid/syntax/type.h"

namespace lucid {
namespace {

// Returns `path` resolved against the directory the compiler was asked to
// work in. Under `bazel run` that is the directory the command was typed in,
// which Bazel passes on in `BUILD_WORKING_DIRECTORY` because it starts the
// compiler elsewhere, among the files it was built with. Anywhere else it is
// the current directory. A path that is already absolute is left as it is.
std::filesystem::path FromWorkingDirectory(std::string_view path) {
  const char* bazel_working_directory = std::getenv("BUILD_WORKING_DIRECTORY");
  const std::filesystem::path base =
      bazel_working_directory != nullptr
          ? std::filesystem::path(bazel_working_directory)
          : std::filesystem::current_path();
  return base / path;
}

int HandleCompileCommand(CommandContext ctx) {
  std::optional<std::string_view> src_file = ctx.TakeArg();
  if (!src_file) {
    ctx.Err() << "'compile' command requires exactly 1 argument\n";
    return 1;
  }

  auto src_path = FromWorkingDirectory(*src_file);
  auto out_path = FromWorkingDirectory(src_path.filename().string());
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

  auto src_path = FromWorkingDirectory(*src_file);

  return BuildCode({.src_path = src_path,
                    .out_path = FromWorkingDirectory(*bin_path)})
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

  // Somewhere to put the binary that no other run of the compiler is using.
  // The name of the source is not enough on its own: two sources of the same
  // name, built at the same time, would each run what the other built.
  auto src_path = FromWorkingDirectory(*src_file);
  auto bin_path = std::filesystem::temp_directory_path() /
                  std::format("{}-{}", src_path.stem().string(), getpid());

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

  auto src_path = FromWorkingDirectory(*src_file);

  return ReadFile(src_path, /*with_trailing_zero=*/true)
      .or_else([](const ReadFileError& err)
                   -> std::expected<std::string, std::string> {
        std::stringstream ss;
        ss << err;
        return std::unexpected(ss.str());
      })
      .and_then([](const std::string& src)
                    -> std::expected<std::vector<Def>, std::string> {
        SyntaxContext syn_ctx;
        return ParseDefs(src, syn_ctx)
            .or_else([](ParserError err)
                         -> std::expected<std::vector<Def>, std::string> {
              std::stringstream ss;
              ss << err;
              return std::unexpected(ss.str());
            });
      })
      .and_then([](const std::vector<Def>&) -> std::expected<int, std::string> {
        return 0;
      })
      .or_else([&](const std::string& err) -> std::expected<int, std::string> {
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

  auto src_path = FromWorkingDirectory(*src_file);
  const auto maybe_src = ReadFile(src_path, /*with_trailing_zero=*/true);
  if (!maybe_src.has_value()) {
    ctx.Err() << maybe_src.error() << "\n";
    return 1;
  }
  const auto& src = maybe_src.value();

  SyntaxContext syn_ctx;
  std::expected<std::vector<Def>, ParserError> defs_or_error =
      ParseDefs(src, syn_ctx);
  if (!defs_or_error.has_value()) {
    ctx.Err() << defs_or_error.error() << "\n";
    return 1;
  }
  const auto& defs = defs_or_error.value();

  if (id) {
    // An identifier names a node the way the printer labels one: a kind
    // followed by an index, as in `S2`.
    const char kind = id->empty() ? '\0' : id->front();
    const std::string_view index_digits = id->empty() ? *id : id->substr(1);
    std::uint32_t index = 0;
    const auto [parsed_end, parse_error] = std::from_chars(
        index_digits.data(), index_digits.data() + index_digits.size(), index);
    if ((kind != 'S' && kind != 'E') || index_digits.empty() ||
        parse_error != std::errc() ||
        parsed_end != index_digits.data() + index_digits.size()) {
      ctx.Err() << "second argument to 'print-ast' command must be "
                   "either 'S<index>' or 'E<index>'\n";
      return 1;
    }

    // An index the parse never produced refers to nothing, and reading it
    // would be reading past the nodes there are.
    if (kind == 'S' && !syn_ctx.ContainsStmt(index)) {
      ctx.Err() << "no statement '" << *id << "' in " << *src_file << "\n";
      return 1;
    }
    if (kind == 'E' && !syn_ctx.ContainsExpr(index)) {
      ctx.Err() << "no expression '" << *id << "' in " << *src_file << "\n";
      return 1;
    }

    if (kind == 'S') {
      PrintStmt(syn_ctx, index, ctx.Out());
    } else {
      PrintExpr(syn_ctx, index, ctx.Out());
    }
  } else {
    for (const auto& def : defs) {
      if (const auto* func_def = std::get_if<FuncDefStmt>(&def)) {
        Print(syn_ctx, *func_def, ctx.Out());
      }
    }
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

  auto src_path = FromWorkingDirectory(*src_file);
  const auto maybe_src = ReadFile(src_path, /*with_trailing_zero=*/true);
  if (!maybe_src.has_value()) {
    ctx.Err() << maybe_src.error() << "\n";
    return 1;
  }
  const auto& src = maybe_src.value();

  SyntaxContext syn_ctx;
  std::expected<std::vector<Def>, ParserError> defs_or_error =
      ParseDefs(src, syn_ctx);
  if (!defs_or_error.has_value()) {
    ctx.Err() << defs_or_error.error() << "\n";
    return 1;
  }
  const auto& defs = defs_or_error.value();

  for (bool has_printed_func = false; const auto& def : defs) {
    if (const auto* func_def = std::get_if<FuncDefStmt>(&def)) {
      if (has_printed_func) ctx.Out() << "\n";

      SyntaxControlFlowGraph syn_cfg =
          BuildControlFlowGraph(syn_ctx, *func_def);
      ConvertToStaticSingleAssignment(syn_ctx, syn_cfg);
      Print(syn_ctx, syn_cfg, ctx.Out());
      has_printed_func = true;
    }
  }

  return 0;
}

int HandlePrintAmCfgCommand(CommandContext ctx) {
  std::optional<std::string_view> src_file = ctx.TakeArg();
  if (!src_file) {
    ctx.Out() << "Usage: " << ctx.CurrentCommand()
              << " (--regs=abi|spill|merge) <path>\n"
              << "\n"
              << "Examples:\n"
              << "  print-am-cfg //my/source/file.lu\n"
              << "  print-am-cfg --regs=spill //my/source/file.lu\n";
    return 0;
  }

  auto src_path = FromWorkingDirectory(*src_file);
  const auto maybe_src = ReadFile(src_path, /*with_trailing_zero=*/true);
  if (!maybe_src.has_value()) {
    ctx.Err() << maybe_src.error() << "\n";
    return 1;
  }
  const auto& src = maybe_src.value();

  SyntaxContext syn_ctx;
  std::expected<std::vector<Def>, ParserError> defs_or_error =
      ParseDefs(src, syn_ctx);
  if (!defs_or_error.has_value()) {
    ctx.Err() << defs_or_error.error() << "\n";
    return 1;
  }
  auto& defs = defs_or_error.value();

  AbstractMachineState am_state;
  HashMap<std::string_view, AbstractMachineControlFlowGraph> am_cfgs;

  for (bool has_printed_func = false; auto& def : defs) {
    if (auto* func_def = std::get_if<FuncDefStmt>(&def)) {
      syn_ctx.AddFuncDef(*func_def);

      if (auto res = InferExprTypes(syn_ctx, *func_def); !res.has_value()) {
        ctx.Err() << res.error() << "\n";
        return 1;
      }
      if (auto res = CheckComp(syn_ctx, *func_def); !res.has_value()) {
        ctx.Err() << res.error() << "\n";
        return 1;
      }
      SyntaxControlFlowGraph syn_cfg =
          BuildControlFlowGraph(syn_ctx, *func_def);
      ConvertToStaticSingleAssignment(syn_ctx, syn_cfg);

      auto am_cfg_or_error =
          GenerateAbstractMachineFunction(am_cfgs, syn_ctx, syn_cfg, am_state);
      if (!am_cfg_or_error.has_value()) {
        ctx.Err() << am_cfg_or_error.error() << "\n";
        return 1;
      }

      AbstractMachineControlFlowGraph am_cfg =
          std::move(am_cfg_or_error).value();
      OptimizeAbstractMachineFunction(am_cfg);

      // What compile-time evaluation runs is the function as the abstract
      // machine has it, before anything about a particular machine's
      // registers has been put on it. It can only call a comp function, so
      // no other is kept: keeping every one copied each graph only to free
      // it at the end.
      if (func_def->is_comp) {
        am_cfgs.Insert(syn_ctx.DerefIdent(func_def->name), am_cfg);
      }

      // The stages the flag names run in order, each over what the one
      // before it left, so that asking for a later one shows the earlier
      // ones too. Asking for none of them prints the graph as the abstract
      // machine has it, where every parameter is still a register and no
      // machine has been chosen to run it on.
      static constexpr int kArmRegistersCount = 10;
      const std::optional<std::string_view> regs = ctx.Flag("regs");
      if (regs == "abi" || regs == "spill" || regs == "merge") {
        LowerCallingConvention(am_cfg, kArm64CallingConvention);
      }
      if (regs == "spill") {
        SpillRegisters(am_cfg, am_state, kArmRegistersCount);
      } else if (regs == "merge") {
        const AbstractMachineLiveness liveness =
            SpillRegisters(am_cfg, am_state, kArmRegistersCount);
        const RegisterColors colors =
            ColorRegisters(am_cfg, liveness, kArmRegistersCount);
        MergeRegisters(colors, am_cfg);
      }

      if (has_printed_func) ctx.Out() << "\n";
      Print(syn_ctx.DerefIdent(func_def->name), am_cfg, ctx.Out());
      has_printed_func = true;
    } else if (const auto* type_def = std::get_if<TypeDefStmt>(&def)) {
      syn_ctx.RegisterType(type_def->name, type_def->type);
    }
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
              .flags = {{
                  .name = "regs",
                  .help = "How far to take the registers: 'abi' to settle "
                          "which parameters and arguments get one, 'spill' to "
                          "go on and put the rest in memory, 'merge' to go on "
                          "and colour what is left.",
              }},
              .handler = HandlePrintAmCfgCommand,
          },
          {
              .name = "version",
              .help = "Prints version information for lucid.",
              .handler = HandleVersionCommand,
          },
      },
      std::move(ctx));
}

}  // namespace

int Run(std::vector<std::string_view> args) {
  return RunProgram({.name = "lucid", .handler = HandleRoot},
                    StandardRootCommandContext(args));
}

}  // namespace lucid

int main(int argc, char* argv[]) { return lucid::Run({argv, argv + argc}); }
