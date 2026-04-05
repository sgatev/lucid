#include "lucid/arm64/translator.h"

#include <charconv>
#include <cstdint>
#include <string>
#include <string_view>
#include <unordered_map>
#include <utility>
#include <vector>

#include "lucid/am/cfg.h"
#include "lucid/am/instructions.h"
#include "lucid/arm64/assembler.h"
#include "lucid/core/container/graph/order.h"
#include "lucid/core/container/hash_map.h"
#include "lucid/core/string/index.h"

namespace lucid {
namespace {

using namespace ::lucid::arm64;

class Arm64BinaryGenerator {
 public:
  explicit Arm64BinaryGenerator(std::string_view func_name,
                                const std::vector<std::size_t>& stack_slots,
                                const AbstractMachineControlFlowGraph& am_cfg,
                                Assembler& assmebler)
      : func_name_(func_name),
        stack_slots_(stack_slots),
        am_cfg_(am_cfg),
        assembler_(assmebler) {}

  void Generate() && {
    assembler_.Label(std::string(func_name_));
    assembler_.StpPreIndex(X(29), X(30), SP, Imm(-16));

    stack_size_ = 12 * 8;
    for (std::size_t size : stack_slots_) stack_size_ += size;
    std::size_t quot = stack_size_ % 16;
    stack_size_ = quot == 0 ? stack_size_ : stack_size_ + 16 - quot;

    stack_offsets_.resize(stack_slots_.size() + 12);
    stack_offsets_[0] = 0;
    for (int i = 1; i < 12; ++i) {
      stack_offsets_[i] = stack_offsets_[i - 1] + 8;
    }
    for (int i = 0; i < stack_slots_.size(); ++i) {
      stack_offsets_[12 + i] = stack_offsets_[12 + i - 1] + stack_slots_[i];
    }

    std::vector<AbstractMachineControlFlowGraph::BlockRef> block_refs =
        Vertices(am_cfg_);
    const CompareVertexOrder<AbstractMachineControlFlowGraph> compare(
        am_cfg_, ComputeReversePostOrder(am_cfg_));
    std::sort(block_refs.begin(), block_refs.end(), compare);
    for (const auto& ref : block_refs) Process(am_cfg_.get(ref));
  }

 private:
  void Process(const AbstractMachineControlFlowGraph::Block& block) {
    for (const auto& inst : block.instructions) Process(inst);
  }

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
    std::string label = std::string(func_name_) + std::to_string(inst.label);
    assembler_.B(label);
  }

  void Process(const CondJump& inst) {
    assembler_.Cmp(W(inst.cond_reg), Imm(0));

    std::string else_label =
        std::string(func_name_) + std::to_string(inst.else_label);
    assembler_.B(Cond::Eq, else_label);

    std::string then_label =
        std::string(func_name_) + std::to_string(inst.then_label);
    assembler_.B(then_label);
  }

  void Process(const Label& inst) {
    std::string label = std::string(func_name_) + std::to_string(inst.id);
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

    for (std::size_t i = 12; i >= 1; --i) {
      assembler_.StrUnsignedOffset(X(i), SP, Imm(stack_offsets_[i - 1]));
    }
  }

  void Process(const PopStack& inst) {
    for (std::size_t i = 12; i >= 1; --i) {
      assembler_.LdrUnsignedOffset(X(i), SP, Imm(stack_offsets_[i - 1]));
    }

    assembler_.Add(SP, SP, Imm(stack_size_));
  }

  void Process(const StoreStack32& inst) {
    assembler_.StrUnsignedOffset(W(inst.src_reg), SP,
                                 Imm(stack_offsets_[12 + inst.offset]));
  }

  void Process(const StoreStackReg32& inst) {
    assembler_.Add(W(inst.offset_reg), W(inst.offset_reg),
                   Imm(stack_offsets_[12 + inst.offset]));
    assembler_.Str(W(inst.src_reg), SP, W(inst.offset_reg), Extend::Uxtw,
                   Imm(0));
  }

  void Process(const StoreStack64& inst) {
    assembler_.StrUnsignedOffset(X(inst.src_reg), SP,
                                 Imm(stack_offsets_[12 + inst.offset]));
  }

  void Process(const StoreStackReg64& inst) {
    assembler_.Add(X(inst.offset_reg), X(inst.offset_reg),
                   Imm(stack_offsets_[12 + inst.offset]));
    assembler_.Str(X(inst.src_reg), SP, X(inst.offset_reg), Extend::Lsl,
                   Imm(0));
  }

  void Process(const LoadStack32& inst) {
    assembler_.LdrUnsignedOffset(W(inst.dst_reg), SP,
                                 Imm(stack_offsets_[12 + inst.offset]));
  }

  void Process(const LoadStackReg32& inst) {
    assembler_.Add(W(inst.offset_reg), W(inst.offset_reg),
                   Imm(stack_offsets_[12 + inst.offset]));
    assembler_.Ldr(W(inst.dst_reg), SP, W(inst.offset_reg), Extend::Uxtw);
  }

  void Process(const LoadStack64& inst) {
    assembler_.LdrUnsignedOffset(X(inst.dst_reg), SP,
                                 Imm(stack_offsets_[12 + inst.offset]));
  }

  void Process(const LoadStackReg64& inst) {
    assembler_.Add(X(inst.offset_reg), X(inst.offset_reg),
                   Imm(stack_offsets_[12 + inst.offset]));
    assembler_.Ldr(X(inst.dst_reg), SP, X(inst.offset_reg), Extend::Lsl,
                   Imm(0));
  }

  void Process(const FuncCall& inst) {
    HashMap<RegId, std::pair<FuncCall::Slot, std::uint8_t>> reg_mappings;
    for (std::uint8_t target_reg_id = 1; const auto& arg : inst.args) {
      reg_mappings.Insert(arg.reg, {arg, target_reg_id++});
    }

    while (!reg_mappings.empty()) {
      std::uint8_t temp_reg_id = 15;

      auto [arg, target_reg_id] = reg_mappings.begin()->second;
      reg_mappings.Remove(arg.reg);

      std::uint8_t from_reg_id = arg.reg;
      std::uint8_t to_reg_id = target_reg_id;
      while (true) {
        auto next_mapping = reg_mappings.Find(to_reg_id);

        if (next_mapping.has_value()) {
          if (next_mapping->first.bits == 32) {
            assembler_.Mov(W(temp_reg_id), W(to_reg_id));
          } else {
            assembler_.Mov(X(temp_reg_id), X(to_reg_id));
          }
        }

        if (arg.bits == 32) {
          assembler_.Mov(W(to_reg_id), W(from_reg_id));
        } else {
          assembler_.Mov(X(to_reg_id), X(from_reg_id));
        }

        if (!next_mapping.has_value()) break;

        std::swap(from_reg_id, temp_reg_id);
        to_reg_id = next_mapping->second;
        reg_mappings.Remove(next_mapping->first.reg);
      }
    }

    assembler_.Bl(inst.label);

    if (inst.res.bits == 32) {
      assembler_.Mov(W(inst.res.reg), W(0));
    } else {
      assembler_.Mov(X(inst.res.reg), X(0));
    }
  }

  Imm ParseImm(std::string_view sv) {
    std::uint16_t res;
    std::from_chars(sv.begin(), sv.end(), res);
    return Imm(res);
  }

  std::string_view func_name_;
  const std::vector<std::size_t>& stack_slots_;
  const AbstractMachineControlFlowGraph& am_cfg_;
  Assembler& assembler_;
  std::size_t stack_size_ = 0;
  std::vector<std::size_t> stack_offsets_;
};

}  // namespace

void GenerateArmStartBinary(Assembler& assembler) {
  assembler.Global("_start");
  assembler.StpPreIndex(X(29), X(30), SP, Imm(-16));
  assembler.Bl("main");
  assembler.LdpPostIndex(X(29), X(30), SP, Imm(16));
  assembler.Ret();

  assembler.Label("_print_string");
  assembler.StpPreIndex(X(29), X(30), SP, Imm(-16));
  assembler.Mov(X(0), X(1));
  assembler.Bl(assembler.External("_printf"));
  assembler.LdpPostIndex(X(29), X(30), SP, Imm(16));
  assembler.Ret();

  assembler.Label("_sleep");
  assembler.StpPreIndex(X(29), X(30), SP, Imm(-16));
  assembler.Mov(X(2), X(1));
  assembler.Mov(X(3), Imm(0));
  assembler.StpPreIndex(X(2), X(3), SP, Imm(-16));
  assembler.Mov(X(0), SP);
  assembler.Mov(X(1), Imm(0));
  assembler.Bl(assembler.External("_nanosleep"));
  assembler.Add(SP, SP, Imm(16));
  assembler.LdpPostIndex(X(29), X(30), SP, Imm(16));
  assembler.Ret();
}

void GenerateArmEndBinary(
    const SyntaxContext& ctx,
    const std::unordered_map<std::uintptr_t, StringIndex::Ref>& strings,
    Assembler& assmebler) {
  for (const auto& [k, v] : strings) {
    assmebler.Label("str" + std::to_string(k));
    assmebler.Asciz(ctx.DerefIdent(v));
  }
}

void GenerateArmAssemblyBinary(std::string_view func_name,
                               const std::vector<std::size_t>& stack_slots,
                               const AbstractMachineControlFlowGraph& am_cfg,
                               Assembler& assmebler) {
  Arm64BinaryGenerator(func_name, stack_slots, am_cfg, assmebler).Generate();
}

}  // namespace lucid
