#include "lucid/arm64_gen.h"

#include <functional>
#include <map>
#include <ostream>
#include <string>
#include <string_view>
#include <utility>
#include <variant>

#include "lucid/am.h"
#include "lucid/am_gen.h"

namespace lucid {
namespace {

// Generates 64-bit ARM assembly source code.
class Arm64Generator {
 public:
  explicit Arm64Generator(std::string_view func_name,
                          const std::vector<Instruction>& instructions,
                          std::ostream& out)
      : func_name_(func_name), instructions_(instructions), out_(out) {
    out_reg_[0] = "X0";
    out_reg_[1] = "X1";
    out_reg_[2] = "X2";
    out_reg_[3] = "X3";
  }

  void Generate() && {
    Append(func_name_);
    Append(":\n");
    for (const auto& inst : instructions_) Process(inst);
  }

 private:
  void Process(const Instruction& inst) {
    std::visit([this](auto&& inst) { Process(inst); }, inst);
  }

  void Process(const Nop&) {}

  void Process(const MoveReg32& inst) {
    Append("mov ");
    Append(out_reg_[inst.dst_reg]);
    Append(", ");
    Append(out_reg_[inst.src_reg]);
    Append("\n");
  }

  void Process(const SetReg32& inst) {
    Append("mov ");
    Append(out_reg_[inst.dst_reg]);
    Append(", #");
    Append(inst.src_val);
    Append("\n");
  }

  void Process(const Return& inst) { Append("RET\n"); }

  void Process(const Jump& inst) {
    Append("stp X29, X30, [sp, #-16]!\n");
    Append("BL ");
    Append(inst.label);
    Append("\n");
    Append("ldp X29, X30, [sp], #16\n");
  }

  void Process(const AddReg32& inst) {
    Append("ADD ");
    Append(out_reg_[inst.res_reg]);
    Append(", ");
    Append(out_reg_[inst.lhs_reg]);
    Append(", ");
    Append(out_reg_[inst.rhs_reg]);
    Append("\n");
  }

  void Process(const SubReg32& inst) {
    Append("SUB ");
    Append(out_reg_[inst.res_reg]);
    Append(", ");
    Append(out_reg_[inst.lhs_reg]);
    Append(", ");
    Append(out_reg_[inst.rhs_reg]);
    Append("\n");
  }

  void Process(const MulReg32& inst) {
    Append("MUL ");
    Append(out_reg_[inst.res_reg]);
    Append(", ");
    Append(out_reg_[inst.lhs_reg]);
    Append(", ");
    Append(out_reg_[inst.rhs_reg]);
    Append("\n");
  }

  void Process(const DivReg32& inst) {
    Append("UDIV ");
    Append(out_reg_[inst.res_reg]);
    Append(", ");
    Append(out_reg_[inst.lhs_reg]);
    Append(", ");
    Append(out_reg_[inst.rhs_reg]);
    Append("\n");
  }

  void Append(std::string_view s) { out_ << s; }

  std::string_view func_name_;
  const std::vector<Instruction>& instructions_;
  std::ostream& out_;
  std::map<RegId, std::string> out_reg_;
};

}  // namespace

void GenerateArmStartSource(std::ostream& out) {
  out << R"(.global _start
.align 2
_start:
  stp X29, X30, [sp, #-16]!
  BL main
  ldp X29, X30, [sp], #16
  mov X16, #1
  svc #0x80
)";
}

void GenerateArmAssemblySource(std::string_view func_name,
                               const std::vector<Instruction>& instructions,
                               std::ostream& out) {
  Arm64Generator(func_name, instructions, out).Generate();
}

}  // namespace lucid
