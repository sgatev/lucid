#include "lucid/arm64_gen.h"

#include <cstdint>
#include <ostream>
#include <string>
#include <string_view>
#include <unordered_map>
#include <variant>
#include <vector>

#include "lucid/am.h"
#include "lucid/am_gen.h"

namespace lucid {
namespace {

// Generates 64-bit ARM assembly source code.
class Arm64Generator {
 public:
  explicit Arm64Generator(const Function& func, std::ostream& out)
      : func_(func), out_(out) {}

  void Generate() && {
    Append(func_.name);
    Append(":\n");
    Append("STP X29, X30, [SP, #-16]!\n");

    for (std::size_t size : func_.stack_slots) stack_size_ += size;
    std::size_t quot = stack_size_ % 16;
    stack_size_ = quot == 0 ? stack_size_ : stack_size_ + 16 - quot;

    stack_offsets_.resize(func_.stack_slots.size() + 1);
    stack_offsets_[0] = 0;
    for (int i = 1; i < stack_offsets_.size(); ++i) {
      stack_offsets_[i] = stack_offsets_[i - 1] + func_.stack_slots[i - 1];
    }

    for (const auto& inst : func_.instructions) Process(inst);
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

  void Process(const SetStr& inst) {
    Append("ADR X");
    Append(inst.dst_reg);
    Append(", str");
    Append(inst.src_val);
    Append("\n");
  }

  void Process(const Return& inst) {
    Append("LDP X29, X30, [SP], #16\n");
    Append("RET\n");
  }

  void Process(const Jump& inst) {
    Append("BL ");
    Append(inst.label);
    Append("\n");
  }

  void Process(const UncondJump& inst) {
    Append("B ");
    Append(func_.name);
    Append(inst.label);
    Append("\n");
  }

  void Process(const CondJump& inst) {
    Append("CMP W");
    Append(inst.cond_reg);
    Append(", 0\n");
    Append("B.EQ ");
    Append(func_.name);
    Append(inst.else_label);
    Append("\n");
    Append("B ");
    Append(func_.name);
    Append(inst.then_label);
    Append("\n");
  }

  void Process(const Label& inst) {
    Append(func_.name);
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

  void Process(const NotEqReg32& inst) {
    Append("CMP W");
    Append(inst.lhs_reg);
    Append(", W");
    Append(inst.rhs_reg);
    Append("\n");
    Append("CSET W");
    Append(inst.res_reg);
    Append(", NE\n");
  }

  void Process(const NotEqReg64& inst) {
    Append("CMP X");
    Append(inst.lhs_reg);
    Append(", X");
    Append(inst.rhs_reg);
    Append("\n");
    Append("CSET X");
    Append(inst.res_reg);
    Append(", NE\n");
  }

  void Process(const PushStack& inst) {
    Append("SUB SP, SP, #");
    Append(stack_size_);
    Append("\n");
  }

  void Process(const PopStack& inst) {
    Append("ADD SP, SP, #");
    Append(stack_size_);
    Append("\n");
  }

  void Process(const StoreStack32& inst) {
    Append("STR W");
    Append(inst.src_reg);
    Append(", [SP, #");
    Append(stack_offsets_[inst.offset]);
    Append("]\n");
  }

  void Process(const StoreStack64& inst) {
    Append("STR X");
    Append(inst.src_reg);
    Append(", [SP, #");
    Append(stack_offsets_[inst.offset]);
    Append("]\n");
  }

  void Process(const LoadStack32& inst) {
    Append("LDR W");
    Append(inst.dst_reg);
    Append(", [SP, #");
    Append(stack_offsets_[inst.offset]);
    Append("]\n");
  }

  void Process(const LoadStack64& inst) {
    Append("LDR X");
    Append(inst.dst_reg);
    Append(", [SP, #");
    Append(stack_offsets_[inst.offset]);
    Append("]\n");
  }

  void Append(std::string_view s) { out_ << s; }
  void Append(std::size_t s) { out_ << s; }

  const Function& func_;
  std::ostream& out_;
  std::size_t stack_size_ = 0;
  std::vector<std::size_t> stack_offsets_;
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
  BL _exit
)";
}

void GenerateArmEndSource(
    const std::unordered_map<std::uintptr_t, std::string>& strings,
    std::ostream& out) {
  out << R"(
NumberFormat: .asciz "%d"
StringFormat: .asciz "%s"
)";
  for (auto [k, v] : strings) {
    out << "str" << std::to_string(k) << ": .asciz " << v << "\n";
  }
}

void GenerateArmAssemblySource(const Function& func, std::ostream& out) {
  Arm64Generator(func, out).Generate();
}

}  // namespace lucid
