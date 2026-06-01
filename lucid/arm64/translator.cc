#include "lucid/arm64/translator.h"

#include <array>
#include <charconv>
#include <cstdint>
#include <string>
#include <string_view>
#include <vector>

#include "lucid/am/cfg.h"
#include "lucid/am/instructions.h"
#include "lucid/arm64/assembler.h"
#include "lucid/core/container/graph/order.h"
#include "lucid/core/container/hash_map.h"
#include "lucid/core/container/hash_set.h"
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

    assembler_.Sub(SP, SP, Imm(stack_size_));

    for (int i = kRegistersToPersist.size() - 1; i >= 0; --i) {
      assembler_.StrUnsignedOffset(X(kRegistersToPersist[i]), SP,
                                   Imm(stack_offsets_[i]));
    }

    for (int param_idx = 1; const auto& param : am_cfg_.params) {
      switch (param.size) {
        case RegSize32:
          assembler_.Mov(W(param.id), W(param_idx++));
          break;
        case RegSize64:
          assembler_.Mov(X(param.id), X(param_idx++));
          break;
      }
    }

    std::vector<AbstractMachineControlFlowGraph::BlockRef> block_refs =
        Vertices(am_cfg_);
    std::sort(block_refs.begin(), block_refs.end(),
              CompareReversePostOrder(am_cfg_));
    for (const auto& ref : block_refs) Process(am_cfg_.GetBlock(ref));
  }

 private:
  void Process(const AbstractMachineControlFlowGraph::Block& block) {
    assembler_.Label(std::string(func_name_) + std::to_string(block.ref.id()));

    for (const auto& inst : block.instructions) Process(block, inst);

    if (block.branch_cond.has_value()) {
      assembler_.Cmp(W(block.branch_cond->id), Imm(0));

      std::string else_label_phi =
          std::string(func_name_) + std::to_string(block.next[1].id()) + "_phi";
      assembler_.B(Cond::Eq, else_label_phi);

      std::string then_label_phi =
          std::string(func_name_) + std::to_string(block.next[0].id()) + "_phi";
      assembler_.B(then_label_phi);

      assembler_.Label(else_label_phi);
      std::string else_label =
          std::string(func_name_) + std::to_string(block.next[1].id());
      ProcessPhiFunctions(am_cfg_.GetBlock(block.next[1]), block.ref);
      assembler_.B(else_label);

      assembler_.Label(then_label_phi);
      std::string then_label =
          std::string(func_name_) + std::to_string(block.next[0].id());
      ProcessPhiFunctions(am_cfg_.GetBlock(block.next[0]), block.ref);
      assembler_.B(then_label);
    } else if (block.next.size() == 1) {
      std::string phi_label = std::string(func_name_) +
                              std::to_string(block.ref.id()) + "_" +
                              std::to_string(block.next[0].id()) + "_phi";
      assembler_.B(phi_label);

      assembler_.Label(phi_label);
      ProcessPhiFunctions(am_cfg_.GetBlock(block.next[0]), block.ref);
      std::string label =
          std::string(func_name_) + std::to_string(block.next[0].id());
      assembler_.B(label);
    }
  }

  void Process(const AbstractMachineControlFlowGraph::Block& block,
               const Instruction& inst) {
    std::visit([&](auto&& inst) { Process(block, inst); }, inst);
  }

  void Process(const AbstractMachineControlFlowGraph::Block& block,
               const Nop&) {}

  void Process(const AbstractMachineControlFlowGraph::Block& block,
               const MoveReg& inst) {
    switch (inst.dst_reg.size) {
      case RegSize32:
        assembler_.Mov(W(inst.dst_reg.id), W(inst.src_reg.id));
        break;
      case RegSize64:
        assembler_.Mov(X(inst.dst_reg.id), X(inst.src_reg.id));
        break;
    }
  }

  void Process(const AbstractMachineControlFlowGraph::Block& block,
               const SetReg& inst) {
    switch (inst.dst_reg.size) {
      case RegSize32:
        assembler_.Mov(W(inst.dst_reg.id), ParseImm(inst.src_val));
        break;
      case RegSize64:
        assembler_.Mov(X(inst.dst_reg.id), ParseImm(inst.src_val));
        break;
    }
  }

  void Process(const AbstractMachineControlFlowGraph::Block& block,
               const SetStr& inst) {
    std::string label = "str" + std::to_string(inst.src_val);
    assembler_.Adr(X(inst.dst_reg.id), label);
  }

  void Process(const AbstractMachineControlFlowGraph::Block& block,
               const Return& inst) {
    assembler_.Mov(W(0), W(inst.res_reg.id));

    for (int i = kRegistersToPersist.size() - 1; i >= 0; --i) {
      assembler_.LdrUnsignedOffset(X(kRegistersToPersist[i]), SP,
                                   Imm(stack_offsets_[i]));
    }

    assembler_.Add(SP, SP, Imm(stack_size_));

    assembler_.LdpPostIndex(X(29), X(30), SP, Imm(16));
    assembler_.Ret();
  }

  void ProcessPhiFunctions(
      const AbstractMachineControlFlowGraph::Block& block,
      AbstractMachineControlFlowGraph::BlockRef pred_block_ref) {
    int pred_block_idx;
    for (int i = 0; i < block.preds.size(); ++i) {
      if (block.preds[i] == pred_block_ref) {
        pred_block_idx = i;
        break;
      }
    }

    HashMap<Reg, Reg> dst_to_srcs;
    HashSet<Reg> srcs;
    for (const auto& phi : block.phis) {
      dst_to_srcs.Insert(phi.dst, phi.srcs[pred_block_idx]);
      srcs.Insert(phi.srcs[pred_block_idx]);
    }
    while (!dst_to_srcs.empty()) {
      bool removed = false;
      for (const auto [target, source] : dst_to_srcs) {
        if (srcs.Contains(target)) continue;

        dst_to_srcs.Remove(target);
        srcs.Remove(source);

        switch (target.size) {
          case RegSize32:
            assembler_.Mov(W(target.id), W(source.id));
            break;
          case RegSize64:
            assembler_.Mov(X(target.id), X(source.id));
            break;
        }
        removed = true;
        break;
      }
      if (!removed) {
        const auto [target, source] = *dst_to_srcs.begin();
        switch (target.size) {
          case RegSize32:
            assembler_.Mov(W(15), W(target.id));
            break;
          case RegSize64:
            assembler_.Mov(X(15), X(target.id));
            break;
        }
        for (const auto [t, s] : dst_to_srcs) {
          if (s == target) {
            dst_to_srcs.Set(t, Reg{.id = 15, .size = target.size});
          }
        }
        srcs.Remove(target);
        srcs.Insert(Reg{.id = 15, .size = target.size});
      }
    }
  }

  void Process(const AbstractMachineControlFlowGraph::Block& block,
               const AddReg& inst) {
    switch (inst.res_reg.size) {
      case RegSize32:
        assembler_.Add(W(inst.res_reg.id), W(inst.lhs_reg.id),
                       W(inst.rhs_reg.id));
        break;
      case RegSize64:
        assembler_.Add(X(inst.res_reg.id), X(inst.lhs_reg.id),
                       X(inst.rhs_reg.id));
        break;
    }
  }

  void Process(const AbstractMachineControlFlowGraph::Block& block,
               const SubReg& inst) {
    switch (inst.res_reg.size) {
      case RegSize32:
        assembler_.Sub(W(inst.res_reg.id), W(inst.lhs_reg.id),
                       W(inst.rhs_reg.id));
        break;
      case RegSize64:
        assembler_.Sub(X(inst.res_reg.id), X(inst.lhs_reg.id),
                       X(inst.rhs_reg.id));
        break;
    }
  }

  void Process(const AbstractMachineControlFlowGraph::Block& block,
               const MulReg& inst) {
    switch (inst.res_reg.size) {
      case RegSize32:
        assembler_.Mul(W(inst.res_reg.id), W(inst.lhs_reg.id),
                       W(inst.rhs_reg.id));
        break;
      case RegSize64:
        assembler_.Mul(X(inst.res_reg.id), X(inst.lhs_reg.id),
                       X(inst.rhs_reg.id));
        break;
    }
  }

  void Process(const AbstractMachineControlFlowGraph::Block& block,
               const DivReg& inst) {
    switch (inst.res_reg.size) {
      case RegSize32:
        assembler_.Udiv(W(inst.res_reg.id), W(inst.lhs_reg.id),
                        W(inst.rhs_reg.id));
        break;
      case RegSize64:
        assembler_.Udiv(X(inst.res_reg.id), X(inst.lhs_reg.id),
                        X(inst.rhs_reg.id));
        break;
    }
  }

  void Process(const AbstractMachineControlFlowGraph::Block& block,
               const ModReg& inst) {
    switch (inst.res_reg.size) {
      case RegSize32:
        assembler_.Udiv(W(inst.res_reg.id), W(inst.lhs_reg.id),
                        W(inst.rhs_reg.id));
        assembler_.Msub(W(inst.res_reg.id), W(inst.res_reg.id),
                        W(inst.rhs_reg.id), W(inst.lhs_reg.id));
        break;
      case RegSize64:
        assembler_.Udiv(X(inst.res_reg.id), X(inst.lhs_reg.id),
                        X(inst.rhs_reg.id));
        assembler_.Msub(X(inst.res_reg.id), X(inst.res_reg.id),
                        X(inst.rhs_reg.id), X(inst.lhs_reg.id));
        break;
    }
  }

  void Process(const AbstractMachineControlFlowGraph::Block& block,
               const GtReg& inst) {
    switch (inst.lhs_reg.size) {
      case RegSize32:
        assembler_.Cmp(W(inst.lhs_reg.id), W(inst.rhs_reg.id));
        assembler_.Cset(W(inst.res_reg.id), InvCond::Gt);
        break;
      case RegSize64:
        assembler_.Cmp(X(inst.lhs_reg.id), X(inst.rhs_reg.id));
        assembler_.Cset(X(inst.res_reg.id), InvCond::Gt);
        break;
    }
  }

  void Process(const AbstractMachineControlFlowGraph::Block& block,
               const LtReg& inst) {
    switch (inst.lhs_reg.size) {
      case RegSize32:
        assembler_.Cmp(W(inst.lhs_reg.id), W(inst.rhs_reg.id));
        assembler_.Cset(W(inst.res_reg.id), InvCond::Lt);
        break;
      case RegSize64:
        assembler_.Cmp(X(inst.lhs_reg.id), X(inst.rhs_reg.id));
        assembler_.Cset(X(inst.res_reg.id), InvCond::Lt);
        break;
    }
  }

  void Process(const AbstractMachineControlFlowGraph::Block& block,
               const EqReg& inst) {
    switch (inst.lhs_reg.size) {
      case RegSize32:
        assembler_.Cmp(W(inst.lhs_reg.id), W(inst.rhs_reg.id));
        assembler_.Cset(W(inst.res_reg.id), InvCond::Eq);
        break;
      case RegSize64:
        assembler_.Cmp(X(inst.lhs_reg.id), X(inst.rhs_reg.id));
        assembler_.Cset(X(inst.res_reg.id), InvCond::Eq);
        break;
    }
  }

  void Process(const AbstractMachineControlFlowGraph::Block& block,
               const NotEqReg& inst) {
    switch (inst.lhs_reg.size) {
      case RegSize32:
        assembler_.Cmp(W(inst.lhs_reg.id), W(inst.rhs_reg.id));
        assembler_.Cset(W(inst.res_reg.id), InvCond::Ne);
        break;
      case RegSize64:
        assembler_.Cmp(X(inst.lhs_reg.id), X(inst.rhs_reg.id));
        assembler_.Cset(X(inst.res_reg.id), InvCond::Ne);
        break;
    }
  }

  void Process(const AbstractMachineControlFlowGraph::Block& block,
               const StoreStack& inst) {
    switch (inst.src_reg.size) {
      case RegSize32:
        assembler_.StrUnsignedOffset(W(inst.src_reg.id), SP,
                                     Imm(AdjustedOffset(inst.offset)));
        break;
      case RegSize64:
        assembler_.StrUnsignedOffset(X(inst.src_reg.id), SP,
                                     Imm(AdjustedOffset(inst.offset)));
        break;
    }
  }

  void Process(const AbstractMachineControlFlowGraph::Block& block,
               const StoreStackReg& inst) {
    switch (inst.src_reg.size) {
      case RegSize32:
        assembler_.Add(W(inst.offset_reg.id), W(inst.offset_reg.id),
                       Imm(AdjustedOffset(inst.offset)));
        assembler_.Str(W(inst.src_reg.id), SP, W(inst.offset_reg.id),
                       Extend::Uxtw, Imm(0));
        break;
      case RegSize64:
        assembler_.Add(X(inst.offset_reg.id), X(inst.offset_reg.id),
                       Imm(AdjustedOffset(inst.offset)));
        assembler_.Str(X(inst.src_reg.id), SP, X(inst.offset_reg.id),
                       Extend::Lsl, Imm(0));
        break;
    }
  }

  void Process(const AbstractMachineControlFlowGraph::Block& block,
               const LoadStack& inst) {
    switch (inst.dst_reg.size) {
      case RegSize32:
        assembler_.LdrUnsignedOffset(W(inst.dst_reg.id), SP,
                                     Imm(AdjustedOffset(inst.offset)));
        break;
      case RegSize64:
        assembler_.LdrUnsignedOffset(X(inst.dst_reg.id), SP,
                                     Imm(AdjustedOffset(inst.offset)));
        break;
    }
  }

  void Process(const AbstractMachineControlFlowGraph::Block& block,
               const LoadStackReg32& inst) {
    assembler_.Add(W(inst.offset_reg.id), W(inst.offset_reg.id),
                   Imm(AdjustedOffset(inst.offset)));
    assembler_.Ldr(W(inst.dst_reg.id), SP, W(inst.offset_reg.id), Extend::Uxtw);
  }

  void Process(const AbstractMachineControlFlowGraph::Block& block,
               const LoadStackReg64& inst) {
    assembler_.Add(X(inst.offset_reg.id), X(inst.offset_reg.id),
                   Imm(AdjustedOffset(inst.offset)));
    assembler_.Ldr(X(inst.dst_reg.id), SP, X(inst.offset_reg.id), Extend::Lsl,
                   Imm(0));
  }

  void Process(const AbstractMachineControlFlowGraph::Block& block,
               const FuncCall& inst) {
    for (int i = 0; i < inst.args.size(); ++i) {
      switch (inst.args[i].reg.size) {
        case RegSize32:
          assembler_.Mov(W(i + 1), W(inst.args[i].reg.id));
          break;
        case RegSize64:
          assembler_.Mov(X(i + 1), X(inst.args[i].reg.id));
          break;
      }
    }

    assembler_.Bl(inst.label);

    if (inst.res.has_value()) {
      switch (inst.res->reg.size) {
        case RegSize32:
          assembler_.Mov(W(inst.res->reg.id), W(0));
          break;
        case RegSize64:
          assembler_.Mov(X(inst.res->reg.id), X(0));
          break;
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
    const SyntaxContext& syn_ctx,
    const HashMap<std::uintptr_t, StringIndex::Ref>& strings,
    Assembler& assmebler) {
  for (const auto& [k, v] : strings) {
    assmebler.Label("str" + std::to_string(k));
    assmebler.Asciz(syn_ctx.DerefIdent(v));
  }
}

void GenerateArmAssemblyBinary(std::string_view func_name,
                               const std::vector<std::size_t>& stack_slots,
                               const AbstractMachineControlFlowGraph& am_cfg,
                               Assembler& assmebler) {
  Arm64BinaryGenerator(func_name, stack_slots, am_cfg, assmebler).Generate();
}

}  // namespace lucid
