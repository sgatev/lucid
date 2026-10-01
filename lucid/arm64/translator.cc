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

// Where a division by nothing goes, and what it says there. The message is
// written as it stands in the program, escapes and all, and is this many
// bytes once they are read.
constexpr std::string_view kDivideByZero = "_divide_by_zero";
constexpr std::string_view kDivideByZeroMessage = "_divide_by_zero_message";
constexpr std::string_view kDivideByZeroText = "\"division by zero\\n\"";
constexpr int kDivideByZeroTextBytes = 17;

// The status a program that divided by nothing leaves with, which is what a
// shell reports for a process stopped by an arithmetic trap: the signal for
// one, past the 128 that marks a signal.
constexpr int kDivideByZeroStatus = 128 + 8;

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
                                const FrameLayout& layout,
                                const AbstractMachineControlFlowGraph& am_cfg,
                                Assembler& assmebler)
      : func_name_(func_name),
        stack_slots_(stack_slots),
        layout_(layout),
        am_cfg_(am_cfg),
        assembler_(assmebler) {}

  void Generate() && {
    assembler_.Label(std::string(func_name_));
    assembler_.StpPreIndex(X(29), X(30), SP, Imm(-16));

    // The frame, from the stack pointer up: what this function leaves for the
    // calls it makes, then the registers it hands back as it found them, then
    // its own slots. What its own caller left for it stands above all of
    // that, past the frame record, and takes no room here.
    outgoing_size_ = OutgoingArgsSize(layout_.outgoing_args.count);

    // Where each of them begins, which is where everything before it ends,
    // carried past whatever the one it begins on has to be a multiple of. A
    // slot that stands somewhere the frame does not reach ends where it
    // began, because it takes none of the frame up.
    stack_offsets_.resize(stack_slots_.size() + kRegistersToPersist.size());
    int at = outgoing_size_;
    for (int i = 0; i < kRegistersToPersist.size(); ++i) {
      stack_offsets_[i] = at;
      at += 8;
    }
    for (int i = 0; i < stack_slots_.size(); ++i) {
      if (OwnSlot(i)) at = RoundUpTo(at, stack_slots_[i]);
      stack_offsets_[kRegistersToPersist.size() + i] = at;
      if (OwnSlot(i)) at += stack_slots_[i];
    }

    stack_size_ = RoundUpTo(at, 16);

    InCarriableSteps(stack_size_,
                     [&](Imm imm) { assembler_.Sub(SP, SP, imm); });

    for (int i = kRegistersToPersist.size() - 1; i >= 0; --i) {
      assembler_.StrUnsignedOffset(X(kRegistersToPersist[i]), SP,
                                   Imm(stack_offsets_[i]));
    }

    for (int param_idx = 1; const Reg param : am_cfg_.params) {
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
    // Handed back in the width it is held in. A string is an address, which
    // is twice the width a number is, and half of an address is not one.
    switch (inst.res_reg.size) {
      case RegSize32:
        assembler_.Mov(W(0), W(inst.res_reg.id));
        break;
      case RegSize64:
        assembler_.Mov(X(0), X(inst.res_reg.id));
        break;
    }

    for (int i = kRegistersToPersist.size() - 1; i >= 0; --i) {
      assembler_.LdrUnsignedOffset(X(kRegistersToPersist[i]), SP,
                                   Imm(stack_offsets_[i]));
    }

    InCarriableSteps(stack_size_,
                     [&](Imm imm) { assembler_.Add(SP, SP, imm); });

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

  // The numbers the language holds are signed, so the division that suits
  // them is the signed one: an unsigned divide reads a negative dividend as
  // the very large number its bits also stand for.
  //
  // A division by nothing has no answer, and the machine's own gives it one
  // anyway, so the divisor is looked at first and the program stops where
  // it is nothing.
  void Process(const AbstractMachineControlFlowGraph::Block& block,
               const DivReg& inst) {
    switch (inst.res_reg.size) {
      case RegSize32:
        assembler_.Cbz(W(inst.rhs_reg.id), kDivideByZero);
        assembler_.Sdiv(W(inst.res_reg.id), W(inst.lhs_reg.id),
                        W(inst.rhs_reg.id));
        break;
      case RegSize64:
        assembler_.Cbz(X(inst.rhs_reg.id), kDivideByZero);
        assembler_.Sdiv(X(inst.res_reg.id), X(inst.lhs_reg.id),
                        X(inst.rhs_reg.id));
        break;
    }
  }

  // What is left over is what the division did not account for, so it takes
  // a division and a multiply-subtract. The quotient stands in a register of
  // its own rather than in the one the answer goes to: the subtract reads
  // both sides again after the division has written, and a quotient written
  // over either of them would be reading what it had destroyed. Holding the
  // answer apart from both would do instead, but nothing that decides what
  // to put away in memory knows to count that, so the crowd it makes is one
  // no amount of spilling relieves.
  //
  // It is a division as well, so a divisor of nothing stops it the same way.
  void Process(const AbstractMachineControlFlowGraph::Block& block,
               const ModReg& inst) {
    switch (inst.res_reg.size) {
      case RegSize32:
        assembler_.Cbz(W(inst.rhs_reg.id), kDivideByZero);
        assembler_.Sdiv(W(kQuotientScratch), W(inst.lhs_reg.id),
                        W(inst.rhs_reg.id));
        assembler_.Msub(W(inst.res_reg.id), W(kQuotientScratch),
                        W(inst.rhs_reg.id), W(inst.lhs_reg.id));
        break;
      case RegSize64:
        assembler_.Cbz(X(inst.rhs_reg.id), kDivideByZero);
        assembler_.Sdiv(X(kQuotientScratch), X(inst.lhs_reg.id),
                        X(inst.rhs_reg.id));
        assembler_.Msub(X(inst.res_reg.id), X(kQuotientScratch),
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
    const SlotAddress at = AddressOf(inst.offset, inst.src_reg.size);
    switch (inst.src_reg.size) {
      case RegSize32:
        assembler_.StrUnsignedOffset(W(inst.src_reg.id), at.base, at.offset);
        break;
      case RegSize64:
        assembler_.StrUnsignedOffset(X(inst.src_reg.id), at.base, at.offset);
        break;
    }
  }

  void Process(const AbstractMachineControlFlowGraph::Block& block,
               const StoreStackReg& inst) {
    switch (inst.src_reg.size) {
      case RegSize32:
        InCarriableSteps(AdjustedOffset(inst.offset), [&](Imm imm) {
          assembler_.Add(W(inst.offset_reg.id), W(inst.offset_reg.id), imm);
        });
        assembler_.Str(W(inst.src_reg.id), SP, W(inst.offset_reg.id),
                       Extend::Uxtw, Imm(0));
        break;
      case RegSize64:
        InCarriableSteps(AdjustedOffset(inst.offset), [&](Imm imm) {
          assembler_.Add(X(inst.offset_reg.id), X(inst.offset_reg.id), imm);
        });
        assembler_.Str(X(inst.src_reg.id), SP, X(inst.offset_reg.id),
                       Extend::Lsl, Imm(0));
        break;
    }
  }

  void Process(const AbstractMachineControlFlowGraph::Block& block,
               const LoadStack& inst) {
    const SlotAddress at = AddressOf(inst.offset, inst.dst_reg.size);
    switch (inst.dst_reg.size) {
      case RegSize32:
        assembler_.LdrUnsignedOffset(W(inst.dst_reg.id), at.base, at.offset);
        break;
      case RegSize64:
        assembler_.LdrUnsignedOffset(X(inst.dst_reg.id), at.base, at.offset);
        break;
    }
  }

  void Process(const AbstractMachineControlFlowGraph::Block& block,
               const LoadStackReg& inst) {
    switch (inst.dst_reg.size) {
      case RegSize32:
        InCarriableSteps(AdjustedOffset(inst.offset), [&](Imm imm) {
          assembler_.Add(W(inst.offset_reg.id), W(inst.offset_reg.id), imm);
        });
        assembler_.Ldr(W(inst.dst_reg.id), SP, W(inst.offset_reg.id),
                       Extend::Uxtw);
        break;
      case RegSize64:
        InCarriableSteps(AdjustedOffset(inst.offset), [&](Imm imm) {
          assembler_.Add(X(inst.offset_reg.id), X(inst.offset_reg.id), imm);
        });
        assembler_.Ldr(X(inst.dst_reg.id), SP, X(inst.offset_reg.id),
                       Extend::Lsl, Imm(0));
        break;
    }
  }

  void Process(const AbstractMachineControlFlowGraph::Block& block,
               const FuncCall& inst) {
    // Only the arguments there are registers for are passed here. The rest
    // are already at the foot of the frame, which is where the stack pointer
    // stands at the call, put there by a `StoreArg` ahead of it.
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

  // Where the slot with this index begins, counted from the stack pointer.
  //
  // The arguments the caller left stand above this frame, past the frame
  // record the prologue pushed; the ones this function leaves for its own
  // calls stand at its foot. Everything else stands in the frame proper.
  int AdjustedOffset(std::size_t slot) {
    if (layout_.incoming_args.Holds(slot)) {
      return SafeCast<int>(stack_size_ + kFrameRecordSize +
                           (slot - layout_.incoming_args.first) * kArgSize);
    }
    if (layout_.outgoing_args.Holds(slot)) {
      return SafeCast<int>((slot - layout_.outgoing_args.first) * kArgSize);
    }
    return stack_offsets_[kRegistersToPersist.size() + slot];
  }

  // Whether the slot with this index is the function's own, rather than room
  // for arguments that stands somewhere the frame does not reach.
  bool OwnSlot(std::size_t slot) const {
    return !layout_.incoming_args.Holds(slot) &&
           !layout_.outgoing_args.Holds(slot);
  }

  // Where a slot is, as a load or a store reaches it: a register to count
  // from, and how far along it stands.
  struct SlotAddress {
    X base;
    Imm offset;
  };

  // Returns where the slot with this index is.
  //
  // A load or a store counts from the stack pointer in a field of twelve
  // bits, which counts accesses rather than bytes. A slot further off than
  // that field reaches has its address worked out first, into the register
  // an intra-procedure call would use, which nothing between one instruction
  // and the next here does.
  SlotAddress AddressOf(std::size_t slot, RegSize size) {
    const int offset = AdjustedOffset(slot);
    const int access_size = size == RegSize32 ? 4 : 8;
    if (offset / access_size <= 0b111111111111) {
      return {.base = SP, .offset = Imm(offset)};
    }

    X from = SP;
    InCarriableSteps(offset, [&](Imm imm) {
      assembler_.Add(X(kAddressScratch), from, imm);
      from = X(kAddressScratch);
    });
    return {.base = X(kAddressScratch), .offset = Imm(0)};
  }

  // Calls `emit` with immediates that add up to `value`.
  //
  // One add or subtract carries a value under 4096, or a whole number of
  // 4096s, and nothing between the two, so anything else takes two of them:
  // the whole 4096s, and then what is left over.
  template <typename EmitT>
  static void InCarriableSteps(int value, EmitT emit) {
    assert(value >= 0);

    if (const int pages = value - value % 4096; pages > 0) emit(Imm(pages));
    if (const int rest = value % 4096; rest > 0) emit(Imm(rest));
  }

  // Returns `value` carried up to a multiple of `to`.
  static int RoundUpTo(int value, int to) {
    const int over = value % to;
    return over == 0 ? value : value + to - over;
  }

  // How much room `count` arguments take on the stack, kept to what the stack
  // pointer has to be a multiple of.
  static int OutgoingArgsSize(std::size_t count) {
    return RoundUpTo(SafeCast<int>(count) * kArgSize, 16);
  }

  static constexpr int kArgSize = kArm64CallingConvention.stack_arg_size;

  // What the prologue pushes before the frame itself: the frame pointer and
  // the return address.
  static constexpr int kFrameRecordSize = 16;

  // The register an address too far for a field of its own is worked out in.
  static constexpr std::uint8_t kAddressScratch = 16;

  // The register a quotient stands in while what is left over is worked out.
  static constexpr std::uint8_t kQuotientScratch = 17;

  static constexpr std::array<std::uint8_t, 10> kRegistersToPersist = {
      19, 20, 21, 22, 23, 24, 25, 26, 27, 28,
  };

  std::string_view func_name_;
  const std::vector<int>& stack_slots_;
  const FrameLayout& layout_;
  const AbstractMachineControlFlowGraph& am_cfg_;
  Assembler& assembler_;
  int stack_size_ = 0;
  int outgoing_size_ = 0;
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

  // Where a division by nothing stops the program. What it printed before is
  // flushed first, so that it comes out ahead of the message rather than
  // after; then the message goes where errors go, and the program leaves.
  //
  // It is reached by a branch from the middle of a function rather than by a
  // call, which is why nothing is saved: nothing comes back here.
  assembler.Label(std::string(kDivideByZero));
  assembler.Mov(X(0), Imm(0));
  assembler.Bl(assembler.External("_fflush"));
  assembler.Mov(X(0), Imm(2));
  assembler.Adr(X(1), kDivideByZeroMessage);
  assembler.Mov(X(2), Imm(kDivideByZeroTextBytes));
  assembler.Bl(assembler.External("_write"));
  assembler.Mov(X(0), Imm(kDivideByZeroStatus));
  assembler.Bl(assembler.External("_exit"));

  // The message stands right after the code that writes it. Nothing runs
  // into it, because the code before it leaves the program.
  assembler.Label(std::string(kDivideByZeroMessage));
  assembler.Asciz(kDivideByZeroText);

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
                               const FrameLayout& layout,
                               const AbstractMachineControlFlowGraph& am_cfg,
                               Assembler& assmebler) {
  Arm64BinaryGenerator(func_name, stack_slots, layout, am_cfg, assmebler)
      .Generate();
}

}  // namespace lucid
