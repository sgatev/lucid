#include "lucid/arm64_gen.h"

#include <map>
#include <ostream>
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
      : func_name_(func_name), instructions_(instructions), out_(out) {}

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
    Append("MOV W");
    Append(inst.dst_reg);
    Append(", W");
    Append(inst.src_reg);
    Append("\n");
  }

  void Process(const MoveReg64& inst) {
    Append("MOV X");
    Append(inst.dst_reg);
    Append(", X");
    Append(inst.src_reg);
    Append("\n");
  }

  void Process(const SetReg32& inst) {
    Append("MOV W");
    Append(inst.dst_reg);
    Append(", #");
    Append(inst.src_val);
    Append("\n");
  }

  void Process(const SetReg64& inst) {
    Append("MOV X");
    Append(inst.dst_reg);
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

  void Process(const UncondJump& inst) {
    Append("B ");
    Append(func_name_);
    Append(inst.label);
    Append("\n");
  }

  void Process(const CondJump& inst) {
    Append("CMP W");
    Append(inst.cond_reg);
    Append(", 0\n");
    Append("B.EQ ");
    Append(func_name_);
    Append(inst.else_label);
    Append("\n");
    Append("B ");
    Append(func_name_);
    Append(inst.then_label);
    Append("\n");
  }

  void Process(const Label& inst) {
    Append(func_name_);
    Append(inst.id);
    Append(":\n");
  }

  void Process(const AddReg32& inst) {
    Append("ADD W");
    Append(inst.res_reg);
    Append(", W");
    Append(inst.lhs_reg);
    Append(", W");
    Append(inst.rhs_reg);
    Append("\n");
  }

  void Process(const AddReg64& inst) {
    Append("ADD X");
    Append(inst.res_reg);
    Append(", X");
    Append(inst.lhs_reg);
    Append(", X");
    Append(inst.rhs_reg);
    Append("\n");
  }

  void Process(const SubReg32& inst) {
    Append("SUB W");
    Append(inst.res_reg);
    Append(", W");
    Append(inst.lhs_reg);
    Append(", W");
    Append(inst.rhs_reg);
    Append("\n");
  }

  void Process(const SubReg64& inst) {
    Append("SUB X");
    Append(inst.res_reg);
    Append(", X");
    Append(inst.lhs_reg);
    Append(", X");
    Append(inst.rhs_reg);
    Append("\n");
  }

  void Process(const MulReg32& inst) {
    Append("MUL W");
    Append(inst.res_reg);
    Append(", W");
    Append(inst.lhs_reg);
    Append(", W");
    Append(inst.rhs_reg);
    Append("\n");
  }

  void Process(const MulReg64& inst) {
    Append("MUL X");
    Append(inst.res_reg);
    Append(", X");
    Append(inst.lhs_reg);
    Append(", X");
    Append(inst.rhs_reg);
    Append("\n");
  }

  void Process(const DivReg32& inst) {
    Append("UDIV W");
    Append(inst.res_reg);
    Append(", W");
    Append(inst.lhs_reg);
    Append(", W");
    Append(inst.rhs_reg);
    Append("\n");
  }

  void Process(const DivReg64& inst) {
    Append("UDIV X");
    Append(inst.res_reg);
    Append(", X");
    Append(inst.lhs_reg);
    Append(", X");
    Append(inst.rhs_reg);
    Append("\n");
  }

  void Process(const GtReg32& inst) {
    Append("CMP W");
    Append(inst.lhs_reg);
    Append(", W");
    Append(inst.rhs_reg);
    Append("\n");
    Append("CSET W");
    Append(inst.res_reg);
    Append(", GT\n");
  }

  void Process(const GtReg64& inst) {
    Append("CMP X");
    Append(inst.lhs_reg);
    Append(", X");
    Append(inst.rhs_reg);
    Append("\n");
    Append("CSET X");
    Append(inst.res_reg);
    Append(", GT\n");
  }

  void Process(const LtReg32& inst) {
    Append("CMP W");
    Append(inst.lhs_reg);
    Append(", W");
    Append(inst.rhs_reg);
    Append("\n");
    Append("CSET W");
    Append(inst.res_reg);
    Append(", LT\n");
  }

  void Process(const LtReg64& inst) {
    Append("CMP X");
    Append(inst.lhs_reg);
    Append(", X");
    Append(inst.rhs_reg);
    Append("\n");
    Append("CSET X");
    Append(inst.res_reg);
    Append(", LT\n");
  }

  void Process(const EqReg32& inst) {
    Append("CMP W");
    Append(inst.lhs_reg);
    Append(", W");
    Append(inst.rhs_reg);
    Append("\n");
    Append("CSET W");
    Append(inst.res_reg);
    Append(", EQ\n");
  }

  void Process(const EqReg64& inst) {
    Append("CMP X");
    Append(inst.lhs_reg);
    Append(", X");
    Append(inst.rhs_reg);
    Append("\n");
    Append("CSET X");
    Append(inst.res_reg);
    Append(", EQ\n");
  }

  void Process(const PushStack& inst) {
    Append("SUB SP, SP, #");
    std::size_t quot = inst.size % 16;
    std::size_t size = quot == 0 ? inst.size : inst.size + 16 - quot;
    Append(size);
    Append("\n");
  }

  void Process(const PopStack& inst) {
    Append("ADD SP, SP, #");
    std::size_t quot = inst.size % 16;
    std::size_t size = quot == 0 ? inst.size : inst.size + 16 - quot;
    Append(size);
    Append("\n");
  }

  void Process(const StoreStack32& inst) {
    Append("STR W");
    Append(inst.src_reg);
    Append(", [SP, #");
    Append(inst.offset);
    Append("]\n");
  }

  void Process(const StoreStack64& inst) {
    Append("STR X");
    Append(inst.src_reg);
    Append(", [SP, #");
    Append(inst.offset);
    Append("]\n");
  }

  void Process(const LoadStack32& inst) {
    Append("LDR W");
    Append(inst.dst_reg);
    Append(", [SP, #");
    Append(inst.offset);
    Append("]\n");
  }

  void Process(const LoadStack64& inst) {
    Append("LDR X");
    Append(inst.dst_reg);
    Append(", [SP, #");
    Append(inst.offset);
    Append("]\n");
  }

  void Append(std::string_view s) { out_ << s; }
  void Append(std::size_t s) { out_ << s; }

  std::string_view func_name_;
  const std::vector<Instruction>& instructions_;
  std::ostream& out_;
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
