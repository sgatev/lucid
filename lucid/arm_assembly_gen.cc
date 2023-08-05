#include "lucid/arm_assembly_gen.h"

#include <cstddef>
#include <map>
#include <string>
#include <string_view>
#include <variant>
#include <vector>

#include "lucid/am.h"
#include "lucid/am_gen.h"
#include "lucid/arena.h"
#include "lucid/ast.h"
#include "lucid/cfg.h"
#include "lucid/string_builder.h"

namespace lucid {
namespace {

// Generates 64-bit ARM assembly source code.
class ArmAssemblySourceGenerator {
 public:
  explicit ArmAssemblySourceGenerator(const Arena<Stmt>& arena)
      : arena_(arena) {
    out_reg_[0] = "X0";
    out_reg_[1] = "X1";
  }

  // Adds source code for `func` to the generated source.
  void Process(const FuncDefStmt& func) {
    Append(func.name);
    Append(":\n");
    Indent();

    auto graph = BuildControlFlowGraph(arena_, func);
    auto instructions = GenerateAbstractMachineInstructions(arena_, graph);
    for (const auto& inst : instructions) Process(inst);
    UnIndent();
  }

  // Extracts and returns the source code produced by this generator.
  std::string ConsumeGeneratedSource() && {
    return std::move(output_builder_).Build();
  }

 private:
  void Process(const Instruction& inst) {
    std::visit([this](auto&& inst) { Process(inst); }, inst);
  }

  void Process(const MoveReg32& inst) {
    AppendIndent();
    Append("mov ");
    Append(out_reg_[inst.dst_reg]);
    Append(", ");
    Append(out_reg_[inst.src_reg]);
    Append("\n");
  }

  void Process(const SetReg32& inst) {
    AppendIndent();
    Append("mov ");
    Append(out_reg_[inst.dst_reg]);
    Append(", #");
    Append(inst.src_val);
    Append("\n");
  }

  void Process(const Return& inst) {
    AppendIndent();
    Append("RET\n");
  }

  void Process(const Jump& inst) {
    AppendIndent();
    Append("stp X29, X30, [sp, #-16]!\n");
    AppendIndent();
    Append("BL ");
    Append(inst.label);
    Append("\n");
    AppendIndent();
    Append("ldp X29, X30, [sp], #16\n");
  }

  void Indent() { indent_ += 2; }

  void UnIndent() { indent_ -= 2; }

  void AppendIndent() {
    for (int i = 0; i < indent_; ++i) Append(" ");
  }

  void Append(std::string_view s) { output_builder_.Append(s); }

  const Arena<Stmt>& arena_;
  StringBuilder output_builder_;
  std::size_t indent_ = 0;
  std::map<RegId, std::string> out_reg_;
};

}  // namespace

std::string GenerateArmStartSource() {
  return R"(.global _start
.align 2
_start:
  stp X29, X30, [sp, #-16]!
  BL main
  ldp X29, X30, [sp], #16
  mov X16, #1
  svc #0x80
)";
}

std::string GenerateArmAssemblySource(const Arena<Stmt>& arena,
                                      const std::vector<FuncDefStmt>& funcs) {
  ArmAssemblySourceGenerator gen(arena);
  for (const auto& func : funcs) gen.Process(func);
  return std::move(gen).ConsumeGeneratedSource();
}

}  // namespace lucid
