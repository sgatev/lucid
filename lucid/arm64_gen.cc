#include "lucid/arm64_gen.h"

#include <charconv>
#include <cstdint>
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
  explicit Arm64BinaryGenerator(const Function& func, Assembler& assmebler)
      : func_(func), assembler_(assmebler) {}

  void Generate() && {
    assembler_.Label(std::string(func_.name));
    assembler_.StpPreIndex(X(29), X(30), SP, Imm(-16));

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
    assembler_.Mov(W(inst.dst_reg), W(inst.src_reg));
  }

  void Process(const MoveReg64& inst) {
    assembler_.Mov(X(inst.dst_reg), X(inst.src_reg));
  }

  void Process(const SetReg32& inst) {
    assembler_.Mov(W(inst.dst_reg), ParseImm(inst.src_val));
  }

  void Process(const SetReg64& inst) {
    assembler_.Mov(X(inst.dst_reg), ParseImm(inst.src_val));
  }

  void Process(const SetStr& inst) {
    std::string label = "str" + std::to_string(inst.src_val);
    assembler_.Adr(X(inst.dst_reg), label);
  }

  void Process(const Return& inst) {
    assembler_.LdpPostIndex(X(29), X(30), SP, Imm(16));
    assembler_.Ret();
  }

  void Process(const Jump& inst) { assembler_.Bl(inst.label); }

  void Process(const UncondJump& inst) {
    std::string label = std::string(func_.name) + std::to_string(inst.label);
    assembler_.B(label);
  }

  void Process(const CondJump& inst) {
    assembler_.Cmp(W(inst.cond_reg), Imm(0));

    std::string else_label =
        std::string(func_.name) + std::to_string(inst.else_label);
    assembler_.B(Cond::Eq, else_label);

    std::string then_label =
        std::string(func_.name) + std::to_string(inst.then_label);
    assembler_.B(then_label);
  }

  void Process(const Label& inst) {
    std::string label = std::string(func_.name) + std::to_string(inst.id);
    assembler_.Label(label);
  }

  void Process(const AddReg32& inst) {
    assembler_.Add(W(inst.res_reg), W(inst.lhs_reg), W(inst.rhs_reg));
  }

  void Process(const AddReg64& inst) {
    assembler_.Add(X(inst.res_reg), X(inst.lhs_reg), X(inst.rhs_reg));
  }

  void Process(const SubReg32& inst) {
    assembler_.Sub(W(inst.res_reg), W(inst.lhs_reg), W(inst.rhs_reg));
  }

  void Process(const SubReg64& inst) {
    assembler_.Sub(X(inst.res_reg), X(inst.lhs_reg), X(inst.rhs_reg));
  }

  void Process(const MulReg32& inst) {
    assembler_.Mul(W(inst.res_reg), W(inst.lhs_reg), W(inst.rhs_reg));
  }

  void Process(const MulReg64& inst) {
    assembler_.Mul(X(inst.res_reg), X(inst.lhs_reg), X(inst.rhs_reg));
  }

  void Process(const DivReg32& inst) {
    assembler_.Udiv(W(inst.res_reg), W(inst.lhs_reg), W(inst.rhs_reg));
  }

  void Process(const DivReg64& inst) {
    assembler_.Udiv(X(inst.res_reg), X(inst.lhs_reg), X(inst.rhs_reg));
  }

  void Process(const ModReg32& inst) {
    assembler_.Udiv(W(inst.res_reg), W(inst.lhs_reg), W(inst.rhs_reg));
    assembler_.Msub(W(inst.res_reg), W(inst.res_reg), W(inst.rhs_reg),
                    W(inst.lhs_reg));
  }

  void Process(const ModReg64& inst) {
    assembler_.Udiv(X(inst.res_reg), X(inst.lhs_reg), X(inst.rhs_reg));
    assembler_.Msub(X(inst.res_reg), X(inst.res_reg), X(inst.rhs_reg),
                    X(inst.lhs_reg));
  }

  void Process(const GtReg32& inst) {
    assembler_.Cmp(W(inst.lhs_reg), W(inst.rhs_reg));
    assembler_.Cset(W(inst.res_reg), InvCond::Gt);
  }

  void Process(const GtReg64& inst) {
    assembler_.Cmp(X(inst.lhs_reg), X(inst.rhs_reg));
    assembler_.Cset(X(inst.res_reg), InvCond::Gt);
  }

  void Process(const LtReg32& inst) {
    assembler_.Cmp(W(inst.lhs_reg), W(inst.rhs_reg));
    assembler_.Cset(W(inst.res_reg), InvCond::Lt);
  }

  void Process(const LtReg64& inst) {
    assembler_.Cmp(X(inst.lhs_reg), X(inst.rhs_reg));
    assembler_.Cset(X(inst.res_reg), InvCond::Lt);
  }

  void Process(const EqReg32& inst) {
    assembler_.Cmp(W(inst.lhs_reg), W(inst.rhs_reg));
    assembler_.Cset(W(inst.res_reg), InvCond::Eq);
  }

  void Process(const EqReg64& inst) {
    assembler_.Cmp(X(inst.lhs_reg), X(inst.rhs_reg));
    assembler_.Cset(X(inst.res_reg), InvCond::Eq);
  }

  void Process(const NotEqReg32& inst) {
    assembler_.Cmp(W(inst.lhs_reg), W(inst.rhs_reg));
    assembler_.Cset(W(inst.res_reg), InvCond::Ne);
  }

  void Process(const NotEqReg64& inst) {
    assembler_.Cmp(X(inst.lhs_reg), X(inst.rhs_reg));
    assembler_.Cset(X(inst.res_reg), InvCond::Ne);
  }

  void Process(const PushStack& inst) {
    assembler_.Sub(SP, SP, Imm(stack_size_));
  }

  void Process(const PopStack& inst) {
    assembler_.Add(SP, SP, Imm(stack_size_));
  }

  void Process(const StoreStack32& inst) {
    assembler_.StrUnsignedOffset(W(inst.src_reg), SP,
                                 Imm(stack_offsets_[inst.offset]));
  }

  void Process(const StoreStackReg32& inst) {
    assembler_.Add(W(inst.offset_reg), W(inst.offset_reg),
                   Imm(stack_offsets_[inst.offset]));
    assembler_.Str(W(inst.src_reg), SP, W(inst.offset_reg), Extend::Uxtw,
                   Imm(0));
  }

  void Process(const StoreStack64& inst) {
    assembler_.StrUnsignedOffset(X(inst.src_reg), SP,
                                 Imm(stack_offsets_[inst.offset]));
  }

  void Process(const StoreStackReg64& inst) {
    assembler_.Add(X(inst.offset_reg), X(inst.offset_reg),
                   Imm(stack_offsets_[inst.offset]));
    assembler_.Str(X(inst.src_reg), SP, X(inst.offset_reg), Extend::Lsl,
                   Imm(0));
  }

  void Process(const LoadStack32& inst) {
    assembler_.LdrUnsignedOffset(W(inst.dst_reg), SP,
                                 Imm(stack_offsets_[inst.offset]));
  }

  void Process(const LoadStackReg32& inst) {
    assembler_.Add(W(inst.offset_reg), W(inst.offset_reg),
                   Imm(stack_offsets_[inst.offset]));
    assembler_.Ldr(W(inst.dst_reg), SP, W(inst.offset_reg), Extend::Uxtw);
  }

  void Process(const LoadStack64& inst) {
    assembler_.LdrUnsignedOffset(X(inst.dst_reg), SP,
                                 Imm(stack_offsets_[inst.offset]));
  }

  void Process(const LoadStackReg64& inst) {
    assembler_.Add(X(inst.offset_reg), X(inst.offset_reg),
                   Imm(stack_offsets_[inst.offset]));
    assembler_.Ldr(X(inst.dst_reg), SP, X(inst.offset_reg), Extend::Lsl,
                   Imm(0));
  }

  Imm ParseImm(std::string_view sv) {
    std::uint16_t res;
    std::from_chars(sv.begin(), sv.end(), res);
    return Imm(res);
  }

  const Function& func_;
  Assembler& assembler_;
  std::size_t stack_size_ = 0;
  std::vector<std::size_t> stack_offsets_;
};

}  // namespace

void GenerateArmStartBinary(Assembler& assembler) {
  assembler.StpPreIndex(X(29), X(30), SP, Imm(-16));
  assembler.Bl("main");
  assembler.LdpPostIndex(X(29), X(30), SP, Imm(16));
  assembler.Mov(X(16), Imm(1));
  assembler.Svc(Imm(0));
  assembler.Label("_print_string");
  assembler.Mov(X(0), Imm(1));
  assembler.Mov(X(16), Imm(4));
  assembler.Svc(Imm(0));
  assembler.Ret();
  assembler.Label("_sleep");
  assembler.StpPreIndex(X(29), X(30), SP, Imm(-16));
  assembler.Mov(X(2), X(1));
  assembler.Mov(X(3), Imm(0));
  assembler.StpPreIndex(X(2), X(3), SP, Imm(-16));
  assembler.Mov(X(0), SP);
  assembler.Mov(X(1), Imm(0));
  assembler.Bl("");  // reloc
  assembler.Add(SP, SP, Imm(16));
  assembler.LdpPostIndex(X(29), X(30), SP, Imm(16));
  assembler.Ret();
}

void GenerateArmEndBinary(
    const std::unordered_map<std::uintptr_t, std::string>& strings,
    Assembler& assmebler) {
  for (const auto& [k, v] : strings) {
    assmebler.Label("str" + std::to_string(k));
    assmebler.Asciz(v);
  }
}

void GenerateArmAssemblyBinary(const Function& func, Assembler& assmebler) {
  Arm64BinaryGenerator(func, assmebler).Generate();
}

}  // namespace lucid
