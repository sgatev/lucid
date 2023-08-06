#include "lucid/arm64_gen.h"

#include <cstddef>
#include <map>
#include <string>
#include <string_view>
#include <variant>

#include "lucid/am.h"
#include "lucid/am_gen.h"
#include "lucid/string_builder.h"

namespace lucid {
namespace {

// Generates 64-bit ARM assembly source code.
class ArmAssemblySourceGenerator {
 public:
  explicit ArmAssemblySourceGenerator(
      std::string_view func_name, const std::vector<Instruction>& instructions)
      : func_name_(func_name), instructions_(instructions) {
    out_reg_[0] = "X0";
    out_reg_[1] = "X1";
    out_reg_[2] = "X2";
    out_reg_[3] = "X3";
  }

  std::string Generate() && {
    Append(func_name_);
    Append(":\n");
    Indent();
    for (const auto& inst : instructions_) Process(inst);
    UnIndent();

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

  void Process(const AddReg32& inst) {
    AppendIndent();
    Append("ADD ");
    Append(out_reg_[inst.res_reg]);
    Append(", ");
    Append(out_reg_[inst.lhs_reg]);
    Append(", ");
    Append(out_reg_[inst.rhs_reg]);
    Append("\n");
  }

  void Indent() { indent_ += 2; }

  void UnIndent() { indent_ -= 2; }

  void AppendIndent() {
    for (int i = 0; i < indent_; ++i) Append(" ");
  }

  void Append(std::string_view s) { output_builder_.Append(s); }

  std::string_view func_name_;
  const std::vector<Instruction>& instructions_;
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

std::string GenerateArmAssemblySource(
    std::string_view func_name, const std::vector<Instruction>& instructions) {
  return ArmAssemblySourceGenerator(func_name, instructions).Generate();
}

}  // namespace lucid
