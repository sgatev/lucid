#include "lucid/arm64_gen.h"

#include <map>
#include <ostream>
#include <string>
#include <string_view>
#include <variant>
#include <vector>

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
    out_reg_[0] = "W0";
    out_reg_[1] = "W1";
    out_reg_[2] = "W2";
    out_reg_[3] = "W3";
    out_reg_[4] = "W4";
    out_reg_[5] = "W5";
    out_reg_[6] = "W6";
    out_reg_[7] = "W7";
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
    Append("MOV ");
    Append(out_reg_[inst.dst_reg]);
    Append(", ");
    Append(out_reg_[inst.src_reg]);
    Append("\n");
  }

  void Process(const SetReg32& inst) {
    Append("MOV ");
    Append(out_reg_[inst.dst_reg]);
    Append(", #");
    Append(inst.src_val);
    Append("\n");
  }

  void Process(const Return& inst) { Append("RET\n"); }

  void Process(const Jump& inst) {
    Append("STP X29, X30, [sp, #-16]!\n");
    Append("BL ");
    Append(inst.label);
    Append("\n");
    Append("LDP X29, X30, [sp], #16\n");
  }

  void Process(const CondJump& inst) {
    Append("CMP ");
    Append(out_reg_[inst.cond_reg]);
    Append(", 0\n");
    Append("B.EQ ");
    Append(func_name_);
    Append(std::to_string(inst.else_label));
    Append("\n");
    Append("B.NE ");
    Append(func_name_);
    Append(std::to_string(inst.then_label));
    Append("\n");
  }

  void Process(const Label& inst) {
    Append(func_name_);
    Append(std::to_string(inst.id));
    Append(":\n");
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

  void Process(const GtReg32& inst) {
    Append("CMP ");
    Append(out_reg_[inst.lhs_reg]);
    Append(", ");
    Append(out_reg_[inst.rhs_reg]);
    Append("\n");
    Append("CSET ");
    Append(out_reg_[inst.res_reg]);
    Append(", GT\n");
  }

  void Append(std::string_view s) { out_ << s; }

  std::string_view func_name_;
  const std::vector<Instruction>& instructions_;
  std::ostream& out_;
  std::map<RegId, std::string_view> out_reg_;
};

}  // namespace

void GenerateArmStartSource(std::ostream& out) {
  out << R"(.global _start
.align 2
_start:
  STP X29, X30, [sp, #-16]!
  BL main
  LDP X29, X30, [sp], #16
  MOV X16, #1
  SVC #0x80
)";
}

void GenerateArmAssemblySource(std::string_view func_name,
                               const std::vector<Instruction>& instructions,
                               std::ostream& out) {
  Arm64Generator(func_name, instructions, out).Generate();
}

}  // namespace lucid
