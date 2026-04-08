#include "lucid/arm64/translator.h"

#include <array>
#include <charconv>
#include <cstdint>
#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>

#include "lucid/am/cfg.h"
#include "lucid/am/instructions.h"
#include "lucid/arm64/assembler.h"
#include "lucid/core/container/graph/order.h"
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

    stack_size_ = kRegistersToPersist.size() * 8;
    for (std::size_t size : stack_slots_) stack_size_ += size;
    std::size_t quot = stack_size_ % 16;
    stack_size_ = quot == 0 ? stack_size_ : stack_size_ + 16 - quot;

    stack_offsets_.resize(stack_slots_.size() + kRegistersToPersist.size());
    stack_offsets_[0] = 0;
    for (int i = 1; i < kRegistersToPersist.size(); ++i) {
      stack_offsets_[i] = stack_offsets_[i - 1] + 8;
    }
    for (int i = 0; i < stack_slots_.size(); ++i) {
      stack_offsets_[kRegistersToPersist.size() + i] =
          stack_offsets_[kRegistersToPersist.size() + i - 1] + stack_slots_[i];
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
    for (const auto& inst : block.instructions) Process(block, inst);
  }

  void Process(const AbstractMachineControlFlowGraph::Block& block,
               const Instruction& inst) {
    std::visit([&](auto&& inst) { Process(block, inst); }, inst);
  }

  void Process(const AbstractMachineControlFlowGraph::Phi& phi, int i) {
    if (phi.bits == 32) {
      assembler_.Mov(W(15), W(phi.sources[i]));
      assembler_.Mov(W(phi.target), W(15));
    } else if (phi.bits == 64) {
      assembler_.Mov(X(15), X(phi.sources[i]));
      assembler_.Mov(X(phi.target), X(15));
    }
  }

  void Process(const AbstractMachineControlFlowGraph::Block& block,
               const Nop&) {}

  void Process(const AbstractMachineControlFlowGraph::Block& block,
               const MoveReg32& inst) {
    assembler_.Mov(W(inst.dst_reg), W(inst.src_reg));
  }

  void Process(const AbstractMachineControlFlowGraph::Block& block,
               const MoveReg64& inst) {
    assembler_.Mov(X(inst.dst_reg), X(inst.src_reg));
  }

  void Process(const AbstractMachineControlFlowGraph::Block& block,
               const SetReg32& inst) {
    assembler_.Mov(W(inst.dst_reg), ParseImm(inst.src_val));
  }

  void Process(const AbstractMachineControlFlowGraph::Block& block,
               const SetReg64& inst) {
    assembler_.Mov(X(inst.dst_reg), ParseImm(inst.src_val));
  }

  void Process(const AbstractMachineControlFlowGraph::Block& block,
               const SetStr& inst) {
    std::string label = "str" + std::to_string(inst.src_val);
    assembler_.Adr(X(inst.dst_reg), label);
  }

  void Process(const AbstractMachineControlFlowGraph::Block& block,
               const Return& inst) {
    assembler_.Mov(W(0), W(inst.res_reg));

    for (int i = kRegistersToPersist.size() - 1; i >= 0; --i) {
      assembler_.LdrUnsignedOffset(X(kRegistersToPersist[i]), SP,
                                   Imm(stack_offsets_[i]));
    }

    assembler_.Add(SP, SP, Imm(stack_size_));

    assembler_.LdpPostIndex(X(29), X(30), SP, Imm(16));
    assembler_.Ret();
  }

  void Process(const AbstractMachineControlFlowGraph::Block& block,
               const Jump& inst) {
    assembler_.Bl(inst.label);
  }

  void Process(const AbstractMachineControlFlowGraph::Block& block,
               const UncondJump& inst) {
    std::string phi_label = std::string(func_name_) +
                            std::to_string(block.ref.id()) + "_" +
                            std::to_string(inst.label) + "_phi";
    assembler_.B(phi_label);

    assembler_.Label(phi_label);
    for (const auto& phi : am_cfg_.get(inst.label).phis) {
      int idx;
      for (int i = 0; i < am_cfg_.get(inst.label).preds.size(); ++i) {
        if (am_cfg_.get(inst.label).preds[i] == block.ref) {
          idx = i;
        }
      }
      Process(phi, idx);
    }
    std::string label = std::string(func_name_) + std::to_string(inst.label);
    assembler_.B(label);
  }

  void Process(const AbstractMachineControlFlowGraph::Block& block,
               const CondJump& inst) {
    assembler_.Cmp(W(inst.cond_reg), Imm(0));

    std::string else_label_phi =
        std::string(func_name_) + std::to_string(inst.else_label) + "_phi";
    assembler_.B(Cond::Eq, else_label_phi);

    std::string then_label_phi =
        std::string(func_name_) + std::to_string(inst.then_label) + "_phi";
    assembler_.B(then_label_phi);

    assembler_.Label(else_label_phi);
    std::string else_label =
        std::string(func_name_) + std::to_string(inst.else_label);
    for (const auto& phi : am_cfg_.get(inst.else_label).phis) {
      int idx;
      for (int i = 0; i < am_cfg_.get(inst.else_label).preds.size(); ++i) {
        if (am_cfg_.get(inst.else_label).preds[i] == block.ref) {
          idx = i;
        }
      }
      Process(phi, idx);
    }
    assembler_.B(else_label);

    assembler_.Label(then_label_phi);
    std::string then_label =
        std::string(func_name_) + std::to_string(inst.then_label);
    for (const auto& phi : am_cfg_.get(inst.then_label).phis) {
      int idx;
      for (int i = 0; i < am_cfg_.get(inst.then_label).preds.size(); ++i) {
        if (am_cfg_.get(inst.then_label).preds[i] == block.ref) {
          idx = i;
        }
      }
      Process(phi, idx);
    }
    assembler_.B(then_label);
  }

  void Process(const AbstractMachineControlFlowGraph::Block& block,
               const Label& inst) {
    std::string label = std::string(func_name_) + std::to_string(inst.id);
    assembler_.Label(label);
  }

  void Process(const AbstractMachineControlFlowGraph::Block& block,
               const AddReg32& inst) {
    assembler_.Add(W(inst.res_reg), W(inst.lhs_reg), W(inst.rhs_reg));
  }

  void Process(const AbstractMachineControlFlowGraph::Block& block,
               const AddReg64& inst) {
    assembler_.Add(X(inst.res_reg), X(inst.lhs_reg), X(inst.rhs_reg));
  }

  void Process(const AbstractMachineControlFlowGraph::Block& block,
               const SubReg32& inst) {
    assembler_.Sub(W(inst.res_reg), W(inst.lhs_reg), W(inst.rhs_reg));
  }

  void Process(const AbstractMachineControlFlowGraph::Block& block,
               const SubReg64& inst) {
    assembler_.Sub(X(inst.res_reg), X(inst.lhs_reg), X(inst.rhs_reg));
  }

  void Process(const AbstractMachineControlFlowGraph::Block& block,
               const MulReg32& inst) {
    assembler_.Mul(W(inst.res_reg), W(inst.lhs_reg), W(inst.rhs_reg));
  }

  void Process(const AbstractMachineControlFlowGraph::Block& block,
               const MulReg64& inst) {
    assembler_.Mul(X(inst.res_reg), X(inst.lhs_reg), X(inst.rhs_reg));
  }

  void Process(const AbstractMachineControlFlowGraph::Block& block,
               const DivReg32& inst) {
    assembler_.Udiv(W(inst.res_reg), W(inst.lhs_reg), W(inst.rhs_reg));
  }

  void Process(const AbstractMachineControlFlowGraph::Block& block,
               const DivReg64& inst) {
    assembler_.Udiv(X(inst.res_reg), X(inst.lhs_reg), X(inst.rhs_reg));
  }

  void Process(const AbstractMachineControlFlowGraph::Block& block,
               const ModReg32& inst) {
    assembler_.Udiv(W(inst.res_reg), W(inst.lhs_reg), W(inst.rhs_reg));
    assembler_.Msub(W(inst.res_reg), W(inst.res_reg), W(inst.rhs_reg),
                    W(inst.lhs_reg));
  }

  void Process(const AbstractMachineControlFlowGraph::Block& block,
               const ModReg64& inst) {
    assembler_.Udiv(X(inst.res_reg), X(inst.lhs_reg), X(inst.rhs_reg));
    assembler_.Msub(X(inst.res_reg), X(inst.res_reg), X(inst.rhs_reg),
                    X(inst.lhs_reg));
  }

  void Process(const AbstractMachineControlFlowGraph::Block& block,
               const GtReg32& inst) {
    assembler_.Cmp(W(inst.lhs_reg), W(inst.rhs_reg));
    assembler_.Cset(W(inst.res_reg), InvCond::Gt);
  }

  void Process(const AbstractMachineControlFlowGraph::Block& block,
               const GtReg64& inst) {
    assembler_.Cmp(X(inst.lhs_reg), X(inst.rhs_reg));
    assembler_.Cset(X(inst.res_reg), InvCond::Gt);
  }

  void Process(const AbstractMachineControlFlowGraph::Block& block,
               const LtReg32& inst) {
    assembler_.Cmp(W(inst.lhs_reg), W(inst.rhs_reg));
    assembler_.Cset(W(inst.res_reg), InvCond::Lt);
  }

  void Process(const AbstractMachineControlFlowGraph::Block& block,
               const LtReg64& inst) {
    assembler_.Cmp(X(inst.lhs_reg), X(inst.rhs_reg));
    assembler_.Cset(X(inst.res_reg), InvCond::Lt);
  }

  void Process(const AbstractMachineControlFlowGraph::Block& block,
               const EqReg32& inst) {
    assembler_.Cmp(W(inst.lhs_reg), W(inst.rhs_reg));
    assembler_.Cset(W(inst.res_reg), InvCond::Eq);
  }

  void Process(const AbstractMachineControlFlowGraph::Block& block,
               const EqReg64& inst) {
    assembler_.Cmp(X(inst.lhs_reg), X(inst.rhs_reg));
    assembler_.Cset(X(inst.res_reg), InvCond::Eq);
  }

  void Process(const AbstractMachineControlFlowGraph::Block& block,
               const NotEqReg32& inst) {
    assembler_.Cmp(W(inst.lhs_reg), W(inst.rhs_reg));
    assembler_.Cset(W(inst.res_reg), InvCond::Ne);
  }

  void Process(const AbstractMachineControlFlowGraph::Block& block,
               const NotEqReg64& inst) {
    assembler_.Cmp(X(inst.lhs_reg), X(inst.rhs_reg));
    assembler_.Cset(X(inst.res_reg), InvCond::Ne);
  }

  void Process(const AbstractMachineControlFlowGraph::Block& block,
               const PushStack& inst) {
    assembler_.Sub(SP, SP, Imm(stack_size_));

    for (int i = kRegistersToPersist.size() - 1; i >= 0; --i) {
      assembler_.StrUnsignedOffset(X(kRegistersToPersist[i]), SP,
                                   Imm(stack_offsets_[i]));
    }

    int param_idx = 1;
    for (const auto& param : am_cfg_.params) {
      if (param.bits == 32) {
        assembler_.Mov(W(param.reg), W(param_idx++));
      } else if (param.bits == 64) {
        assembler_.Mov(X(param.reg), X(param_idx++));
      }
    }
  }

  void Process(const AbstractMachineControlFlowGraph::Block& block,
               const PopStack& inst) {}

  void Process(const AbstractMachineControlFlowGraph::Block& block,
               const StoreStack32& inst) {
    assembler_.StrUnsignedOffset(W(inst.src_reg), SP,
                                 Imm(AdjustedOffset(inst.offset)));
  }

  void Process(const AbstractMachineControlFlowGraph::Block& block,
               const StoreStackReg32& inst) {
    assembler_.Add(W(inst.offset_reg), W(inst.offset_reg),
                   Imm(AdjustedOffset(inst.offset)));
    assembler_.Str(W(inst.src_reg), SP, W(inst.offset_reg), Extend::Uxtw,
                   Imm(0));
  }

  void Process(const AbstractMachineControlFlowGraph::Block& block,
               const StoreStack64& inst) {
    assembler_.StrUnsignedOffset(X(inst.src_reg), SP,
                                 Imm(AdjustedOffset(inst.offset)));
  }

  void Process(const AbstractMachineControlFlowGraph::Block& block,
               const StoreStackReg64& inst) {
    assembler_.Add(X(inst.offset_reg), X(inst.offset_reg),
                   Imm(AdjustedOffset(inst.offset)));
    assembler_.Str(X(inst.src_reg), SP, X(inst.offset_reg), Extend::Lsl,
                   Imm(0));
  }

  void Process(const AbstractMachineControlFlowGraph::Block& block,
               const LoadStack32& inst) {
    assembler_.LdrUnsignedOffset(W(inst.dst_reg), SP,
                                 Imm(AdjustedOffset(inst.offset)));
  }

  void Process(const AbstractMachineControlFlowGraph::Block& block,
               const LoadStackReg32& inst) {
    assembler_.Add(W(inst.offset_reg), W(inst.offset_reg),
                   Imm(AdjustedOffset(inst.offset)));
    assembler_.Ldr(W(inst.dst_reg), SP, W(inst.offset_reg), Extend::Uxtw);
  }

  void Process(const AbstractMachineControlFlowGraph::Block& block,
               const LoadStack64& inst) {
    assembler_.LdrUnsignedOffset(X(inst.dst_reg), SP,
                                 Imm(AdjustedOffset(inst.offset)));
  }

  void Process(const AbstractMachineControlFlowGraph::Block& block,
               const LoadStackReg64& inst) {
    assembler_.Add(X(inst.offset_reg), X(inst.offset_reg),
                   Imm(AdjustedOffset(inst.offset)));
    assembler_.Ldr(X(inst.dst_reg), SP, X(inst.offset_reg), Extend::Lsl,
                   Imm(0));
  }

  void Process(const AbstractMachineControlFlowGraph::Block& block,
               const FuncCall& inst) {
    for (int i = 0; i < inst.args.size(); ++i) {
      if (inst.args[i].bits == 32) {
        assembler_.Mov(W(i + 1), W(inst.args[i].reg));
      } else {
        assembler_.Mov(X(i + 1), X(inst.args[i].reg));
      }
    }

    assembler_.Bl(inst.label);

    if (inst.res.has_value()) {
      if (inst.res->bits == 32) {
        assembler_.Mov(W(inst.res->reg), W(0));
      } else {
        assembler_.Mov(X(inst.res->reg), X(0));
      }
    }
  }

  Imm ParseImm(std::string_view sv) {
    std::uint16_t res;
    std::from_chars(sv.begin(), sv.end(), res);
    return Imm(res);
  }

  std::int16_t AdjustedOffset(std::size_t offset) {
    return stack_offsets_[kRegistersToPersist.size() + offset];
  }

  static constexpr std::array<std::uint8_t, 10> kRegistersToPersist = {
      19, 20, 21, 22, 23, 24, 25, 26, 27, 28,
  };

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
