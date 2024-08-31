#include "lucid/arm64_binary_gen.h"

#include <charconv>
#include <cstdint>
#include <ostream>
#include <string>
#include <unordered_map>
#include <vector>

#include "lucid/am.h"
#include "lucid/arm64.h"

namespace lucid {
namespace {

using namespace ::lucid::arm64;

class Arm64BinaryGenerator {
 public:
  explicit Arm64BinaryGenerator(const Function& func, std::ostream& out)
      : func_(func), out_(out) {}

  void Generate() && {
    arm_.Label(std::string(func_.name));
    arm_.StrPreIndex(X(29), X(30), SP, Imm(-16));

    for (std::size_t size : func_.stack_slots) stack_size_ += size;
    std::size_t quot = stack_size_ % 16;
    stack_size_ = quot == 0 ? stack_size_ : stack_size_ + 16 - quot;

    stack_offsets_.resize(func_.stack_slots.size() + 1);
    stack_offsets_[0] = 0;
    for (int i = 1; i < stack_offsets_.size(); ++i) {
      stack_offsets_[i] = stack_offsets_[i - 1] + func_.stack_slots[i - 1];
    }

    for (const auto& inst : func_.instructions) Process(inst);
    std::vector<std::uint32_t> arm64_insts = arm_.Encode();
    for (auto inst : arm64_insts) out_ << inst;
  }

 private:
  void Process(const Instruction& inst) {
    std::visit([this](auto&& inst) { Process(inst); }, inst);
  }

  void Process(const Nop&) {}

  void Process(const MoveReg32& inst) {
    arm_.Mov(W(inst.dst_reg), W(inst.src_reg));
  }

  void Process(const MoveReg64& inst) {
    arm_.Mov(X(inst.dst_reg), X(inst.src_reg));
  }

  void Process(const SetReg32& inst) {
    arm_.Mov(W(inst.dst_reg), ParseImm(inst.src_val));
  }

  void Process(const SetReg64& inst) {
    arm_.Mov(X(inst.dst_reg), ParseImm(inst.src_val));
  }

  void Process(const SetStr& inst) {
    std::string label = "str" + std::to_string(inst.src_val);
    arm_.Adr(X(inst.dst_reg), label);
  }

  void Process(const Return& inst) {
    arm_.LdpPostIndex(X(29), X(30), SP, Imm(16));
    arm_.Ret();
  }

  void Process(const Jump& inst) { arm_.Bl(inst.label); }

  void Process(const UncondJump& inst) {
    std::string label = std::string(func_.name) + std::to_string(inst.label);
    arm_.B(label);
  }

  void Process(const CondJump& inst) {
    arm_.Cmp(W(inst.cond_reg), Imm(0));

    std::string else_label =
        std::string(func_.name) + std::to_string(inst.else_label);
    arm_.B(Cond::Eq, else_label);

    std::string then_label =
        std::string(func_.name) + std::to_string(inst.then_label);
    arm_.B(then_label);
  }

  void Process(const Label& inst) {
    std::string label = std::string(func_.name) + std::to_string(inst.id);
    arm_.Label(label);
  }

  void Process(const AddReg32& inst) {
    arm_.Add(W(inst.res_reg), W(inst.lhs_reg), W(inst.rhs_reg));
  }

  void Process(const AddReg64& inst) {
    arm_.Add(X(inst.res_reg), X(inst.lhs_reg), X(inst.rhs_reg));
  }

  void Process(const SubReg32& inst) {
    arm_.Sub(W(inst.res_reg), W(inst.lhs_reg), W(inst.rhs_reg));
  }

  void Process(const SubReg64& inst) {
    arm_.Sub(X(inst.res_reg), X(inst.lhs_reg), X(inst.rhs_reg));
  }

  void Process(const MulReg32& inst) {
    arm_.Mul(W(inst.res_reg), W(inst.lhs_reg), W(inst.rhs_reg));
  }

  void Process(const MulReg64& inst) {
    arm_.Mul(X(inst.res_reg), X(inst.lhs_reg), X(inst.rhs_reg));
  }

  void Process(const DivReg32& inst) {
    arm_.Udiv(W(inst.res_reg), W(inst.lhs_reg), W(inst.rhs_reg));
  }

  void Process(const DivReg64& inst) {
    arm_.Udiv(X(inst.res_reg), X(inst.lhs_reg), X(inst.rhs_reg));
  }

  void Process(const ModReg32& inst) {
    arm_.Udiv(W(inst.res_reg), W(inst.lhs_reg), W(inst.rhs_reg));
    arm_.Msub(W(inst.res_reg), W(inst.res_reg), W(inst.rhs_reg),
              W(inst.lhs_reg));
  }

  void Process(const ModReg64& inst) {
    arm_.Udiv(X(inst.res_reg), X(inst.lhs_reg), X(inst.rhs_reg));
    arm_.Msub(X(inst.res_reg), X(inst.res_reg), X(inst.rhs_reg),
              X(inst.lhs_reg));
  }

  void Process(const GtReg32& inst) {
    arm_.Cmp(W(inst.lhs_reg), W(inst.rhs_reg));
    arm_.Cset(W(inst.res_reg), InvCond::Gt);
  }

  void Process(const GtReg64& inst) {
    arm_.Cmp(X(inst.lhs_reg), X(inst.rhs_reg));
    arm_.Cset(X(inst.res_reg), InvCond::Gt);
  }

  void Process(const LtReg32& inst) {
    arm_.Cmp(W(inst.lhs_reg), W(inst.rhs_reg));
    arm_.Cset(W(inst.res_reg), InvCond::Lt);
  }

  void Process(const LtReg64& inst) {
    arm_.Cmp(X(inst.lhs_reg), X(inst.rhs_reg));
    arm_.Cset(X(inst.res_reg), InvCond::Lt);
  }

  void Process(const EqReg32& inst) {
    arm_.Cmp(W(inst.lhs_reg), W(inst.rhs_reg));
    arm_.Cset(W(inst.res_reg), InvCond::Eq);
  }

  void Process(const EqReg64& inst) {
    arm_.Cmp(X(inst.lhs_reg), X(inst.rhs_reg));
    arm_.Cset(X(inst.res_reg), InvCond::Eq);
  }

  void Process(const NotEqReg32& inst) {
    arm_.Cmp(W(inst.lhs_reg), W(inst.rhs_reg));
    arm_.Cset(W(inst.res_reg), InvCond::Ne);
  }

  void Process(const NotEqReg64& inst) {
    arm_.Cmp(X(inst.lhs_reg), X(inst.rhs_reg));
    arm_.Cset(X(inst.res_reg), InvCond::Ne);
  }

  void Process(const PushStack& inst) { arm_.Sub(SP, SP, Imm(stack_size_)); }

  void Process(const PopStack& inst) { arm_.Add(SP, SP, Imm(stack_size_)); }

  void Process(const StoreStack32& inst) {
    /*Append("STR W");
    Append(inst.src_reg);
    Append(", [SP, #");
    Append(stack_offsets_[inst.offset]);
    Append("]\n");*/
  }

  void Process(const StoreStackReg32& inst) {
    arm_.Add(W(inst.offset_reg), W(inst.offset_reg),
             Imm(stack_offsets_[inst.offset]));

    /*Append("STR W");
    Append(inst.src_reg);
    Append(", [SP, W");
    Append(inst.offset_reg);
    Append(", uxtw #0");
    Append("]\n");*/
  }

  void Process(const StoreStack64& inst) {
    /*Append("STR X");
    Append(inst.src_reg);
    Append(", [SP, #");
    Append(stack_offsets_[inst.offset]);
    Append("]\n");*/
  }

  void Process(const StoreStackReg64& inst) {
    arm_.Add(X(inst.offset_reg), X(inst.offset_reg),
             Imm(stack_offsets_[inst.offset]));

    /*Append("STR X");
    Append(inst.src_reg);
    Append(", [SP, X");
    Append(inst.offset_reg);
    Append(", lsl #0");
    Append("]\n");*/
  }

  void Process(const LoadStack32& inst) {
    /*Append("LDR W");
    Append(inst.dst_reg);
    Append(", [SP, #");
    Append(stack_offsets_[inst.offset]);
    Append("]\n");*/
  }

  void Process(const LoadStackReg32& inst) {
    arm_.Add(W(inst.offset_reg), W(inst.offset_reg),
             Imm(stack_offsets_[inst.offset]));

    /*Append("LDR W");
    Append(inst.dst_reg);
    Append(", [SP, W");
    Append(inst.offset_reg);
    Append(", uxtw");
    Append("]\n");*/
  }

  void Process(const LoadStack64& inst) {
    /*Append("LDR X");
    Append(inst.dst_reg);
    Append(", [SP, #");
    Append(stack_offsets_[inst.offset]);
    Append("]\n");*/
  }

  void Process(const LoadStackReg64& inst) {
    arm_.Add(X(inst.offset_reg), X(inst.offset_reg),
             Imm(stack_offsets_[inst.offset]));

    /*Append("LDR X");
    Append(inst.dst_reg);
    Append(", [SP, X");
    Append(inst.offset_reg);
    Append(", lsl #0");
    Append("]\n");*/
  }

  Imm ParseImm(std::string_view sv) {
    std::uint16_t res;
    std::from_chars(sv.begin(), sv.end(), res);
    return Imm(res);
  }

  const Function& func_;
  std::ostream& out_;
  std::size_t stack_size_ = 0;
  Arm64 arm_;
  std::vector<std::size_t> stack_offsets_;
};

}  // namespace

void GenerateArmStartBinary(std::ostream& out) {}

void GenerateArmEndBinary(
    const std::unordered_map<std::uintptr_t, std::string>& strings,
    std::ostream& out) {}

void GenerateArmAssemblyBinary(const Function& func, std::ostream& out) {
  Arm64BinaryGenerator(func, out).Generate();
}

}  // namespace lucid
