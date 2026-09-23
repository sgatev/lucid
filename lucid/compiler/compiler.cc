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
#include "lucid/syntax/ssa.h"
#include "lucid/syntax/type.h"

namespace lucid {
namespace {

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

      if (auto res = InferExprTypes(syn_ctx, *func_def); !res.has_value()) {
        return std::unexpected(res.error());
      }
      if (auto res = CheckComp(syn_ctx, *func_def); !res.has_value()) {
        return std::unexpected(res.error());
      }
      SyntaxControlFlowGraph syn_cfg =
          BuildControlFlowGraph(syn_ctx, *func_def);
      ConvertToStaticSingleAssignment(syn_ctx, syn_cfg);
      AbstractMachineControlFlowGraph am_cfg =
          GenerateAbstractMachineFunction(am_cfgs, syn_ctx, syn_cfg, am_state);
      OptimizeAbstractMachineFunction(am_cfg);
      static constexpr int kArmRegistersCount = 10;
      SpillRegisters(am_cfg, am_state, kArmRegistersCount);
      InterferenceGraph am_ig = BuildInterferenceGraph(am_cfg);
      HashMap<Reg, int> am_ig_colors =
          ColorInterferenceGraph(am_cfg, am_ig, kArmRegistersCount);
      MergeRegisters(am_ig_colors, am_cfg);
      GenerateArmAssemblyBinary(syn_ctx.DerefIdent(func_def->name),
                                am_cfg.stack_slots, am_cfg, assembler);
      am_cfgs.Insert(syn_ctx.DerefIdent(func_def->name), std::move(am_cfg));
    } else if (const auto* type_def = std::get_if<TypeDefStmt>(&defs.back())) {
      syn_ctx.RegisterType(type_def->name, type_def->type);
    }
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
