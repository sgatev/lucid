#include "lucid/arm64/translator.h"

#include <array>
#include <cassert>
#include <charconv>
#include <cstdint>
#include <format>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

#include "lucid/am/cfg.h"
#include "lucid/am/instructions.h"
#include "lucid/am/state.h"
#include "lucid/arm64/assembler.h"
#include "lucid/core/container/graph/order.h"
#include "lucid/core/container/hash_map.h"
#include "lucid/core/container/hash_set.h"
#include "lucid/core/string/index.h"
#include "lucid/syntax/context.h"

namespace lucid {
namespace {

using namespace ::lucid::arm64;

// Casts an integer of type `I` to type `O`.
template <typename O, typename I>
constexpr O SafeCast(I i) noexcept {
  assert(std::in_range<O>(i));
  return static_cast<O>(i);
}

// A comparison a branch can decide for itself, rather than read the result of.
struct BranchComparison {
  Cond cond;
  Reg lhs;
  Reg rhs;
};

// Returns what `inst` compares, if a branch can carry the comparison.
std::optional<BranchComparison> AsBranchComparison(const Instruction& inst) {
  if (const auto* cinst = std::get_if<GtReg>(&inst)) {
    return BranchComparison{Cond::Gt, cinst->lhs_reg, cinst->rhs_reg};
  }
  if (const auto* cinst = std::get_if<LtReg>(&inst)) {
    return BranchComparison{Cond::Lt, cinst->lhs_reg, cinst->rhs_reg};
  }
  if (const auto* cinst = std::get_if<GeReg>(&inst)) {
    return BranchComparison{Cond::Ge, cinst->lhs_reg, cinst->rhs_reg};
  }
  if (const auto* cinst = std::get_if<LeReg>(&inst)) {
    return BranchComparison{Cond::Le, cinst->lhs_reg, cinst->rhs_reg};
  }
  if (const auto* cinst = std::get_if<EqReg>(&inst)) {
    return BranchComparison{Cond::Eq, cinst->lhs_reg, cinst->rhs_reg};
  }
  if (const auto* cinst = std::get_if<NotEqReg>(&inst)) {
    return BranchComparison{Cond::Ne, cinst->lhs_reg, cinst->rhs_reg};
  }
  return std::nullopt;
}

class Arm64BinaryGenerator {
 public:
  explicit Arm64BinaryGenerator(std::string_view func_name,
                                const std::vector<int>& stack_slots,
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
    for (int size : stack_slots_) stack_size_ += size;
    int quot = stack_size_ % 16;
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

    assembler_.Sub(SP, SP, Imm(SafeCast<std::int16_t>(stack_size_)));

    for (int i = kRegistersToPersist.size() - 1; i >= 0; --i) {
      assembler_.StrUnsignedOffset(
          X(kRegistersToPersist[i]), SP,
          Imm(SafeCast<std::int16_t>(stack_offsets_[i])));
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
  // Returns the comparison the branch at the end of `block` can decide for
  // itself.
  //
  // It is the one that computes what the block branches on, ends the block,
  // and leaves a result the block does not outlive. Anything else has to be
  // computed into a register, because something other than the branch reads
  // it.
  std::optional<BranchComparison> FusableComparison(
      const AbstractMachineControlFlowGraph::Block& block) const {
    if (!block.branch_cond.has_value()) return std::nullopt;
    if (block.instructions.empty()) return std::nullopt;

    const std::optional<BranchComparison> comparison =
        AsBranchComparison(block.instructions.back());
    if (!comparison.has_value()) return std::nullopt;

    const std::optional<Reg> result =
        GetTargetRegister(block.instructions.back());
    if (!result.has_value() || *result != *block.branch_cond) {
      return std::nullopt;
    }

    // Whether anything but the branch reads the result was settled before the
    // registers were given their colours, where one register still meant one
    // value. What is left to check is that spilling has not put anything
    // after the comparison since.
    if (!block.only_branch_reads_cond) return std::nullopt;

    return comparison;
  }

  void Process(const AbstractMachineControlFlowGraph::Block& block) {
    assembler_.Label(std::format("{}{}", func_name_, block.ref.id()));

    const std::optional<BranchComparison> fused = FusableComparison(block);
    // A fused comparison is emitted by the branch below rather than here.
    auto instructions_end = block.instructions.end();
    if (fused.has_value()) --instructions_end;
    for (auto it = block.instructions.begin(); it != instructions_end; ++it) {
      Process(block, *it);
    }

    if (block.branch_cond.has_value()) {
      std::string else_label_phi =
          std::format("{}{}_phi", func_name_, block.succs[1].id());
      std::string then_label_phi =
          std::format("{}{}_phi", func_name_, block.succs[0].id());

      if (fused.has_value()) {
        Compare(fused->lhs, fused->rhs);
        assembler_.B(fused->cond, then_label_phi);
        assembler_.B(else_label_phi);
      } else {
        assembler_.Cmp(W(block.branch_cond->id), Imm(0));
        assembler_.B(Cond::Eq, else_label_phi);
        assembler_.B(then_label_phi);
      }

      assembler_.Label(else_label_phi);
      std::string else_label =
          std::format("{}{}", func_name_, block.succs[1].id());
      ProcessPhiFunctions(am_cfg_.GetBlock(block.succs[1]), block.ref);
      assembler_.B(else_label);

      assembler_.Label(then_label_phi);
      std::string then_label =
          std::format("{}{}", func_name_, block.succs[0].id());
      ProcessPhiFunctions(am_cfg_.GetBlock(block.succs[0]), block.ref);
      assembler_.B(then_label);
    } else if (block.succs.size() == 1) {
      std::string phi_label = std::format("{}{}_{}_phi", func_name_,
                                          block.ref.id(), block.succs[0].id());
      assembler_.B(phi_label);

      assembler_.Label(phi_label);
      ProcessPhiFunctions(am_cfg_.GetBlock(block.succs[0]), block.ref);
      std::string label = std::format("{}{}", func_name_, block.succs[0].id());
      assembler_.B(label);
    }
  }

  // Compares `lhs` against `rhs`, in the width they are held in.
  void Compare(Reg lhs, Reg rhs) {
    switch (lhs.size) {
      case RegSize32:
        assembler_.Cmp(W(lhs.id), W(rhs.id));
        break;
      case RegSize64:
        assembler_.Cmp(X(lhs.id), X(rhs.id));
        break;
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
        assembler_.Mov(W(inst.dst_reg.id),
                       Imm(SafeCast<std::int16_t>(inst.src_val)));
        break;
      case RegSize64:
        assembler_.Mov(X(inst.dst_reg.id),
                       Imm(SafeCast<std::int16_t>(inst.src_val)));
        break;
    }
  }

  void Process(const AbstractMachineControlFlowGraph::Block& block,
               const SetInt& inst) {
    std::string label = std::format("long{}", inst.src_val);
    switch (inst.dst_reg.size) {
      case RegSize32:
        assembler_.Ldr(W(inst.dst_reg.id), label);
        break;
      case RegSize64:
        assembler_.Ldr(X(inst.dst_reg.id), label);
        break;
    }
  }

  void Process(const AbstractMachineControlFlowGraph::Block& block,
               const SetStr& inst) {
    std::string label = std::format("str{}", inst.src_val);
    assembler_.Adr(X(inst.dst_reg.id), label);
  }

  void Process(const AbstractMachineControlFlowGraph::Block& block,
               const Return& inst) {
    assembler_.Mov(W(0), W(inst.res_reg.id));

    for (int i = kRegistersToPersist.size() - 1; i >= 0; --i) {
      assembler_.LdrUnsignedOffset(
          X(kRegistersToPersist[i]), SP,
          Imm(SafeCast<std::int16_t>(stack_offsets_[i])));
    }

    assembler_.Add(SP, SP, Imm(SafeCast<std::int16_t>(stack_size_)));

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
               const GeReg& inst) {
    switch (inst.lhs_reg.size) {
      case RegSize32:
        assembler_.Cmp(W(inst.lhs_reg.id), W(inst.rhs_reg.id));
        assembler_.Cset(W(inst.res_reg.id), InvCond::Ge);
        break;
      case RegSize64:
        assembler_.Cmp(X(inst.lhs_reg.id), X(inst.rhs_reg.id));
        assembler_.Cset(X(inst.res_reg.id), InvCond::Ge);
        break;
    }
  }

  void Process(const AbstractMachineControlFlowGraph::Block& block,
               const LeReg& inst) {
    switch (inst.lhs_reg.size) {
      case RegSize32:
        assembler_.Cmp(W(inst.lhs_reg.id), W(inst.rhs_reg.id));
        assembler_.Cset(W(inst.res_reg.id), InvCond::Le);
        break;
      case RegSize64:
        assembler_.Cmp(X(inst.lhs_reg.id), X(inst.rhs_reg.id));
        assembler_.Cset(X(inst.res_reg.id), InvCond::Le);
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
               const LoadStackReg& inst) {
    switch (inst.dst_reg.size) {
      case RegSize32:
        assembler_.Add(W(inst.offset_reg.id), W(inst.offset_reg.id),
                       Imm(AdjustedOffset(inst.offset)));
        assembler_.Ldr(W(inst.dst_reg.id), SP, W(inst.offset_reg.id),
                       Extend::Uxtw);
        break;
      case RegSize64:
        assembler_.Add(X(inst.offset_reg.id), X(inst.offset_reg.id),
                       Imm(AdjustedOffset(inst.offset)));
        assembler_.Ldr(X(inst.dst_reg.id), SP, X(inst.offset_reg.id),
                       Extend::Lsl, Imm(0));
        break;
    }
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
    std::int16_t res;
    std::from_chars(sv.begin(), sv.end(), res);
    return Imm(res);
  }

  std::int16_t AdjustedOffset(std::size_t offset) {
    return SafeCast<std::int16_t>(
        stack_offsets_[kRegistersToPersist.size() + offset]);
  }

  static constexpr std::array<std::uint8_t, 10> kRegistersToPersist = {
      19, 20, 21, 22, 23, 24, 25, 26, 27, 28,
  };

  std::string_view func_name_;
  const std::vector<int>& stack_slots_;
  const AbstractMachineControlFlowGraph& am_cfg_;
  Assembler& assembler_;
  int stack_size_ = 0;
  std::vector<int> stack_offsets_;
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

void GenerateArmEndBinary(const SyntaxContext& syn_ctx,
                          const AbstractMachineState& am_state,
                          Assembler& assmebler) {
  for (std::size_t i = 0; i < am_state.strings.size(); ++i) {
    assmebler.Label(std::format("str{}", i));
    assmebler.Asciz(syn_ctx.DerefIdent(am_state.strings[i]));
  }
  for (const auto& v : am_state.ints) {
    assmebler.Label(std::format("long{}", v));
    assmebler.Long(v);
  }
}

void GenerateArmAssemblyBinary(std::string_view func_name,
                               const std::vector<int>& stack_slots,
                               const AbstractMachineControlFlowGraph& am_cfg,
                               Assembler& assmebler) {
  Arm64BinaryGenerator(func_name, stack_slots, am_cfg, assmebler).Generate();
}

}  // namespace lucid
