#include "lucid/compiler/compiler.h"

#include <cstdlib>
#include <format>
#include <fstream>
#include <iostream>
#include <optional>
#include <string_view>
#include <utility>
#include <vector>

#include "lucid/am/cfg.h"
#include "lucid/am/opt.h"
#include "lucid/am/reg.h"
#include "lucid/am/translator.h"
#include "lucid/arm64/assembler.h"
#include "lucid/arm64/macho.h"
#include "lucid/arm64/translator.h"
#include "lucid/core/functional/result.h"
#include "lucid/core/io/file.h"
#include "lucid/syntax/ast.h"
#include "lucid/syntax/buffered_lexer.h"
#include "lucid/syntax/cfg.h"
#include "lucid/syntax/lexer.h"
#include "lucid/syntax/parser.h"
#include "lucid/syntax/ssa.h"
#include "lucid/syntax/type.h"

namespace lucid {
namespace {

Result<void, ParserError, TypeError> CompileSource(std::string_view src,
                                                   std::ostream& out) {
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
    SyntaxControlFlowGraph scfg = BuildControlFlowGraph(ctx, func);
    ConvertToStaticSingleAssignment(ctx, scfg);
    DestroyStaticSingleAssignment(ctx, scfg);
    AbstractMachineControlFlowGraph am_cfg =
        GenerateAbstractMachineFunction(ctx, scfg, *state);
    for (auto& block : am_cfg.blocks()) {
      OptimizeAbstractMachineInstructions(block.instructions);
    }
    SpillRegisters(am_cfg, state->stack_slots);
    HashMap<RegId, HashSet<RegId>> am_ig = BuildInterferenceGraph(am_cfg);
    HashMap<RegId, int> am_ig_colors =
        ColorInterferenceGraph(am_cfg, am_ig, 10);
    MergeRegisters(am_ig_colors, am_cfg);
    GenerateArmAssemblyBinary(ctx.DerefIdent(func.name), state->stack_slots,
                              am_cfg, assembler);
  }
  GenerateArmEndBinary(ctx, state->strings, assembler);
  WriteCompiledMachObject(assembler, out);
  return {};
}

}  // namespace

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

Result<void, ReadFileError, ParserError, TypeError> CompileCode(
    CompileConfig config) {
  const auto maybe_src = ReadFile(config.src_path, /*with_trailing_zero=*/true);
  if (maybe_src.HasError()) return maybe_src.GetError();
  const auto& src = maybe_src.GetValue();

  std::ofstream out(config.out_path, std::ios::out | std::ios::binary);
  return CompileSource(src, out);
}

Result<void, ReadFileError, ParserError, TypeError> BuildCode(
    BuildConfig config) {
  auto obj_path = config.out_path;
  obj_path.replace_extension("o");

  if (auto res =
          CompileCode({.src_path = config.src_path, .out_path = obj_path});
      res.HasError()) {
    return res.GetError();
  }

  std::system(std::format("ld -o {} {} -lSystem -syslibroot `xcrun -sdk "
                          "macosx --show-sdk-path` -e _start -arch arm64",
                          config.out_path.c_str(), obj_path.c_str())
                  .data());
  return {};
}

}  // namespace lucid
