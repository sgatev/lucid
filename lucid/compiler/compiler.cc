#include "lucid/compiler/compiler.h"

#include <cassert>
#include <cstdlib>
#include <expected>
#include <format>
#include <fstream>
#include <iostream>
#include <list>
#include <optional>
#include <string>
#include <string_view>
#include <utility>
#include <variant>
#include <vector>

#include "lucid/am/abi.h"
#include "lucid/am/cfg.h"
#include "lucid/am/ig.h"
#include "lucid/am/opt.h"
#include "lucid/am/reg.h"
#include "lucid/am/translator.h"
#include "lucid/arm64/assembler.h"
#include "lucid/arm64/macho.h"
#include "lucid/arm64/translator.h"
#include "lucid/core/container/hash_map.h"
#include "lucid/core/io/file.h"
#include "lucid/syntax/ast.h"
#include "lucid/syntax/buffered_lexer.h"
#include "lucid/syntax/cfg.h"
#include "lucid/syntax/comp.h"
#include "lucid/syntax/lexer.h"
#include "lucid/syntax/parser.h"
#include "lucid/syntax/scope.h"
#include "lucid/syntax/ssa.h"
#include "lucid/syntax/type.h"

namespace lucid {
namespace {

// Checks that `main`, where the program starts, is one the start of the
// program can call: one that takes nothing, because nothing is passed to it,
// and returns a number or a `Bool`, the low byte of which becomes the status
// the process leaves with. A string would leave with part of where it is
// held, and `Void` with whatever was last left where a result goes.
//
// Requires:
// - The types of `main` must have been inferred, so that its result is not
//   an array or a tuple.
std::expected<void, TypeError> CheckEntryPoint(const SyntaxContext& syn_ctx,
                                               const FuncDefStmt& main) {
  if (main.params.size() != 0) {
    return std::unexpected(TypeError(std::format(
        "function 'main' takes no parameters, not {}", main.params.size())));
  }

  const auto& result = std::get<BasicType>(syn_ctx.DerefType(main.result_type));
  const std::string_view name = syn_ctx.DerefIdent(result.name);
  if (name != "Int32" && name != "Int64" && name != "Bool") {
    return std::unexpected(TypeError(std::format(
        "function 'main' returns Int32, Int64 or Bool, not {}", name)));
  }
  return {};
}

std::expected<void, CompileError> CompileSource(std::string_view src,
                                                std::ostream& out) {
  SyntaxContext syn_ctx;

  std::list<Def> defs;
  BufferedLexer<Lexer> lexer(Lexer{src});
  Parser parser(syn_ctx, src, lexer);

  AbstractMachineState am_state;
  HashMap<std::string_view, AbstractMachineControlFlowGraph> am_cfgs;
  arm64::Assembler assembler;
  GenerateArmStartBinary(assembler);
  while (true) {
    std::expected<std::optional<Def>, ParserError> def_or_error =
        parser.Parse();
    if (!def_or_error.has_value()) return std::unexpected(def_or_error.error());

    std::optional<Def> maybe_def = std::move(def_or_error).value();
    if (!maybe_def.has_value()) break;

    defs.push_back(std::move(maybe_def).value());

    if (auto* func_def = std::get_if<FuncDefStmt>(&defs.back())) {
      syn_ctx.AddFuncDef(*func_def);

      if (auto res = ResolveNames(syn_ctx, *func_def); !res.has_value()) {
        return std::unexpected(res.error());
      }
      if (auto res = InferExprTypes(syn_ctx, *func_def); !res.has_value()) {
        return std::unexpected(res.error());
      }
      if (syn_ctx.DerefIdent(func_def->name) == "main") {
        if (auto res = CheckEntryPoint(syn_ctx, *func_def); !res.has_value()) {
          return std::unexpected(res.error());
        }
      }
      if (auto res = CheckComp(syn_ctx, *func_def); !res.has_value()) {
        return std::unexpected(res.error());
      }
      SyntaxControlFlowGraph syn_cfg =
          BuildControlFlowGraph(syn_ctx, *func_def);
      ConvertToStaticSingleAssignment(syn_ctx, syn_cfg);
      auto am_cfg_or_error =
          GenerateAbstractMachineFunction(am_cfgs, syn_ctx, syn_cfg, am_state);
      if (!am_cfg_or_error.has_value()) {
        return std::unexpected(am_cfg_or_error.error());
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

      const FrameLayout layout =
          LowerCallingConvention(am_cfg, kArm64CallingConvention);
      static constexpr int kArmRegistersCount = 10;
      const AbstractMachineLiveness liveness =
          SpillRegisters(am_cfg, am_state, kArmRegistersCount);
      InterferenceGraph am_ig = BuildInterferenceGraph(am_cfg, liveness);
      RegisterColors am_ig_colors =
          ColorInterferenceGraph(am_cfg, am_ig, kArmRegistersCount);
      MergeRegisters(am_ig_colors, am_cfg);
      GenerateArmAssemblyBinary(syn_ctx.DerefIdent(func_def->name),
                                am_cfg.stack_slots, layout, am_cfg, assembler);
    } else if (const auto* type_def = std::get_if<TypeDefStmt>(&defs.back())) {
      syn_ctx.RegisterType(type_def->name, type_def->type);
    }
  }
  // The start of the program calls `main`, so there is nothing to write
  // without one.
  if (syn_ctx.FindFuncDef(syn_ctx.AddIdent("main")) == nullptr) {
    return std::unexpected(TypeError("no function 'main' to start from"));
  }

  GenerateArmEndBinary(syn_ctx, am_state, assembler);
  WriteCompiledMachObject(assembler, out);
  return {};
}

}  // namespace

std::expected<std::vector<Def>, ParserError> ParseDefs(std::string_view src,
                                                       SyntaxContext& ctx) {
  std::vector<Def> defs;
  BufferedLexer<Lexer> lexer(Lexer{src});
  Parser parser(ctx, src, lexer);
  while (true) {
    std::expected<std::optional<Def>, ParserError> def_or_error =
        parser.Parse();
    if (!def_or_error.has_value()) return std::unexpected(def_or_error.error());

    std::optional<Def> maybe_def = std::move(def_or_error).value();
    if (!maybe_def.has_value()) break;

    defs.push_back(std::move(maybe_def).value());
  }
  return defs;
}

std::expected<void, CompileError> CompileCode(const CompileConfig& config) {
  std::expected<std::string, ReadFileError> src =
      ReadFile(config.src_path, /*with_trailing_zero=*/true);
  if (!src) return std::unexpected(src.error());

  std::ofstream out(config.out_path, std::ios::out | std::ios::binary);
  return CompileSource(*src, out);
}

std::expected<void, BuildError> BuildCode(BuildConfig cfg) {
  std::filesystem::path obj_path = cfg.out_path;
  obj_path.replace_extension("o");

  return CompileCode({.src_path = cfg.src_path, .out_path = obj_path})
      .and_then([&] -> std::expected<void, BuildError> {
        std::system(std::format("ld -o {} {} -lSystem -syslibroot `xcrun -sdk "
                                "macosx --show-sdk-path` -e _start -arch arm64",
                                cfg.out_path.c_str(), obj_path.c_str())
                        .data());
        return {};
      });
}

}  // namespace lucid
