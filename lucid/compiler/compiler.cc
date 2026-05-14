#include "lucid/compiler/compiler.h"

#include <cstdlib>
#include <expected>
#include <format>
#include <fstream>
#include <iostream>
#include <optional>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

#include "lucid/am/cfg.h"
#include "lucid/am/ig.h"
#include "lucid/am/opt.h"
#include "lucid/am/reg.h"
#include "lucid/am/translator.h"
#include "lucid/arm64/assembler.h"
#include "lucid/arm64/macho.h"
#include "lucid/arm64/translator.h"
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

  std::expected<std::vector<FuncDefStmt>, CompileError> func_defs =
      ParseFuncDefs(src, syn_ctx);
  if (!func_defs) return std::unexpected(func_defs.error());

  AbstractMachineState am_state;
  arm64::Assembler assembler;
  GenerateArmStartBinary(assembler);
  for (auto& func : *func_defs) {
    if (auto res = InferExprTypes(syn_ctx, *func_defs, func);
        !res.has_value()) {
      return std::unexpected(res.error());
    }
    if (auto res = CheckComp(syn_ctx, *func_defs, func); !res.has_value()) {
      return std::unexpected(res.error());
    }
    SyntaxControlFlowGraph syn_cfg = BuildControlFlowGraph(syn_ctx, func);
    ConvertToStaticSingleAssignment(syn_ctx, syn_cfg);
    AbstractMachineControlFlowGraph am_cfg =
        GenerateAbstractMachineFunction(syn_ctx, syn_cfg, am_state);
    for (auto& block : am_cfg.blocks()) {
      OptimizeAbstractMachineInstructions(block.instructions);
    }
    static constexpr int kArmRegistersCount = 10;
    SpillRegisters(am_cfg, am_state, kArmRegistersCount);
    HashMap<RegId, HashSet<RegId>> am_ig = BuildInterferenceGraph(am_cfg);
    HashMap<RegId, int> am_ig_colors =
        ColorInterferenceGraph(am_cfg, am_ig, kArmRegistersCount);
    MergeRegisters(am_ig_colors, am_cfg);
    GenerateArmAssemblyBinary(syn_ctx.DerefIdent(func.name),
                              am_state.stack_slots, am_cfg, assembler);
  }
  GenerateArmEndBinary(syn_ctx, am_state.strings, assembler);
  WriteCompiledMachObject(assembler, out);
  return {};
}

}  // namespace

std::expected<std::vector<FuncDefStmt>, ParserError> ParseFuncDefs(
    std::string_view src, SyntaxContext& ctx) {
  std::vector<FuncDefStmt> func_defs;
  BufferedLexer<Lexer> lexer(Lexer{src});
  Parser parser(ctx, src, lexer);
  while (true) {
    std::expected<std::optional<FuncDefStmt>, ParserError> func_def =
        parser.ParseFuncDef();
    if (!func_def.has_value()) return std::unexpected(func_def.error());
    if (!func_def->has_value()) break;
    func_defs.push_back(std::move(func_def)->value());
  }
  return func_defs;
}

std::expected<void, CompileError> CompileCode(CompileConfig config) {
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
