#include "lucid/vm/interpreter.h"

#include <algorithm>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <expected>
#include <optional>
#include <string>
#include <string_view>
#include <type_traits>
#include <variant>
#include <vector>

#include "lucid/am/cfg.h"
#include "lucid/am/instructions.h"
#include "lucid/am/state.h"
#include "lucid/core/container/hash_map.h"
#include "lucid/syntax/comp.h"

namespace lucid {
namespace {

// Returns `value` as a register of `size` holds it. A number that runs past
// the width comes back round, as it does in the machine the interpreter
// stands in for, rather than being held wider than any register could.
std::int64_t Wrap(std::int64_t value, RegSize size) {
  switch (size) {
    case RegSize32:
      return static_cast<std::int32_t>(static_cast<std::uint32_t>(value));
    case RegSize64:
      return value;
  }
}

// Adds, subtracts and multiplies over the bits of the operands, which is how
// the machine does it. Doing the same in a signed type would make an overflow
// undefined, where in the language it only comes back round.
std::int64_t Add(std::int64_t lhs, std::int64_t rhs) {
  return static_cast<std::int64_t>(static_cast<std::uint64_t>(lhs) +
                                   static_cast<std::uint64_t>(rhs));
}

std::int64_t Sub(std::int64_t lhs, std::int64_t rhs) {
  return static_cast<std::int64_t>(static_cast<std::uint64_t>(lhs) -
                                   static_cast<std::uint64_t>(rhs));
}

std::int64_t Mul(std::int64_t lhs, std::int64_t rhs) {
  return static_cast<std::int64_t>(static_cast<std::uint64_t>(lhs) *
                                   static_cast<std::uint64_t>(rhs));
}

// Divides as the machine does, which is what the program would come to if it
// divided while running. Dividing the least number by minus one has no
// answer in C++ and has one there: it runs past the width and comes back
// round to where it started.
//
// Requires:
// - `rhs` must not be zero, which has no answer anywhere.
std::int64_t Div(std::int64_t lhs, std::int64_t rhs) {
  assert(rhs != 0);
  if (rhs == -1) return Sub(0, lhs);
  return lhs / rhs;
}

// The remainder that goes with `Div`: what is left of `lhs` once `rhs` times
// the quotient is taken away.
//
// Requires:
// - `rhs` must not be zero.
std::int64_t Mod(std::int64_t lhs, std::int64_t rhs) {
  assert(rhs != 0);
  if (rhs == -1) return 0;
  return lhs % rhs;
}

// What a division by nothing during compilation is reported as. The program
// would stop there were it running; compiling it stops there instead.
CompError DivideByZero() { return CompError("compilation divided by zero"); }

// Interprets `am_cfg`, spending from the `steps` the whole compilation
// shares. The function every call made from here reaches in turn.
//
// The arguments that lie in the frame, such as arrays, are `frame_args`, as
// the bytes they are made of one after another. A result that lies in the
// frame is put in `frame_result` the same way, where one is given.
std::expected<std::int64_t, CompError> InterpretFunction(
    const HashMap<std::string_view, AbstractMachineControlFlowGraph>& am_cfgs,
    const AbstractMachineControlFlowGraph& am_cfg,
    const std::vector<std::int64_t>& args, AbstractMachineState& am_state,
    int depth, std::int64_t& steps,
    const std::vector<std::uint8_t>& frame_args = {},
    std::vector<std::uint8_t>* frame_result = nullptr) {
  if (depth > kMaxCompCallDepth) {
    return std::unexpected(CompError("compilation followed more than " +
                                     std::to_string(kMaxCompCallDepth) +
                                     " calls into the program and gave up"));
  }

  Interpreter vm(am_cfgs, am_state, am_cfg.stack_slots,
                 static_cast<std::size_t>(am_cfg.next_free_reg_id), depth,
                 steps);

  for (std::size_t i = 0; i < args.size() && i < am_cfg.params.size(); ++i) {
    vm.Set(am_cfg.params[i], args[i]);
  }
  vm.SetSlotBytes(am_cfg.frame_params_slot, frame_args);

  AbstractMachineControlFlowGraph::BlockRef prev_block_ref =
      AbstractMachineControlFlowGraph::kNullBlockRef;
  auto curr_block_ref = am_cfg.first;
  while (true) {
    const auto& curr_block = am_cfg.GetBlock(curr_block_ref);

    steps += 1 + static_cast<std::int64_t>(curr_block.instructions.size());
    if (steps > kMaxCompSteps) {
      return std::unexpected(CompError("compilation worked through more than " +
                                       std::to_string(kMaxCompSteps) +
                                       " instructions and gave up"));
    }

    // Every phi of the block takes its value from the same predecessor, so
    // which one that is is worked out once on the way in rather than for each.
    int come_from = -1;
    if (!curr_block.phis.empty()) {
      assert(prev_block_ref != AbstractMachineControlFlowGraph::kNullBlockRef);
      for (int i = 0; i < curr_block.preds.size(); ++i) {
        if (curr_block.preds[i] == prev_block_ref) {
          come_from = i;
          break;
        }
      }
      assert(come_from == 0 || come_from == 1);
    }
    for (const auto& phi : curr_block.phis) {
      auto src_val = vm.Get(phi.srcs[come_from]);
      assert(src_val.has_value());
      vm.Set(phi.dst, *src_val);
    }
    for (const auto& inst : curr_block.instructions) {
      std::expected<void, CompError> res = vm.Interpret(inst);
      if (!res.has_value()) return std::unexpected(res.error());
    }
    if (curr_block.branch_cond.has_value()) {
      assert(curr_block.succs.size() == 2);

      auto branch_cond_value = vm.Get(*curr_block.branch_cond);
      assert(branch_cond_value.has_value());

      if (*branch_cond_value != 0) {
        prev_block_ref = curr_block.ref;
        curr_block_ref = curr_block.succs[0];
      } else {
        prev_block_ref = curr_block.ref;
        curr_block_ref = curr_block.succs[1];
      }
    } else if (curr_block.succs.size() == 1) {
      prev_block_ref = curr_block.ref;
      curr_block_ref = curr_block.succs[0];
    } else {
      assert(curr_block.succs.size() == 0);
      assert(curr_block.ref == am_cfg.last);
      break;
    }
  }
  if (frame_result != nullptr) {
    *frame_result = vm.SlotBytes(am_cfg.result_slot, am_cfg.result_slot_count);
  }
  return vm.Result();
}

}  // namespace

std::expected<std::int64_t, CompError> InterpretAbstractMachineFunction(
    const HashMap<std::string_view, AbstractMachineControlFlowGraph>& am_cfgs,
    const AbstractMachineControlFlowGraph& am_cfg,
    const std::vector<std::int64_t>& args, AbstractMachineState& am_state,
    int depth) {
  std::int64_t steps = 0;
  return InterpretFunction(am_cfgs, am_cfg, args, am_state, depth, steps);
}

Interpreter::Interpreter(
    const HashMap<std::string_view, AbstractMachineControlFlowGraph>& am_cfgs,
    AbstractMachineState& am_state, const std::vector<int>& stack_slots,
    std::size_t reg_count, int depth, std::int64_t& steps)
    : am_cfgs_(am_cfgs),
      am_state_(am_state),
      values_(reg_count),
      stack_slots_(stack_slots),
      depth_(depth),
      steps_(steps) {}

std::int64_t Interpreter::Result() const { return result_; }

std::optional<const std::int64_t&> Interpreter::Get(Reg reg) const {
  const auto id = static_cast<std::size_t>(reg.id);
  if (id >= values_.size()) return std::nullopt;
  const std::optional<std::int64_t>& value = values_[id];
  if (!value.has_value()) return std::nullopt;
  return *value;
}

void Interpreter::Set(Reg reg, std::int64_t value) {
  const auto id = static_cast<std::size_t>(reg.id);
  if (id >= values_.size()) values_.resize(id + 1);
  values_[id] = value;
}

std::expected<void, CompError> Interpreter::Interpret(const Instruction& inst) {
  return std::visit(
      [this](const auto& inst) -> std::expected<void, CompError> {
        if constexpr (std::is_void_v<decltype(Interpret(inst))>) {
          Interpret(inst);
          return {};
        } else {
          return Interpret(inst);
        }
      },
      inst);
}

std::expected<void, CompError> Interpreter::Fold(Instruction& inst) {
  std::expected<void, CompError> res = Interpret(inst);
  if (!res.has_value()) return res;

  // Any other instruction sets its register without calling into anything,
  // and is left as it is.
  const auto* func_call = std::get_if<FuncCall>(&inst);
  if (func_call == nullptr) return {};

  assert(func_call->res.has_value());
  const Reg reg = func_call->res->reg;
  auto result = Get(reg);
  assert(result.has_value());

  auto am_cfg = am_cfgs_.Get(func_call->label);
  assert(am_cfg.has_value());

  // The call is replaced by what it evaluated to, as an instruction that
  // sets a value of the kind the function returns.
  switch (am_cfg->result_kind) {
    case ValueKind::Number:
      inst = SetValue(*result, reg, am_state_);
      break;
    case ValueKind::String:
      inst = SetStr{
          .src_val = static_cast<std::uint32_t>(*result),
          .dst_reg = reg,
      };
      break;
  }
  return {};
}

std::int64_t Interpreter::ReadSlot(std::size_t slot) {
  return Read(SlotOffset(slot),
              stack_slots_[slot] == 4 ? RegSize32 : RegSize64);
}

std::vector<std::uint8_t> Interpreter::SlotBytes(std::size_t first,
                                                 std::size_t count) {
  const std::size_t begin = SlotOffset(first);
  const std::size_t end = SlotOffset(first + count);
  if (stack_.size() < end) stack_.resize(end, 0);
  return {stack_.begin() + static_cast<std::ptrdiff_t>(begin),
          stack_.begin() + static_cast<std::ptrdiff_t>(end)};
}

void Interpreter::SetSlotBytes(std::size_t first,
                               const std::vector<std::uint8_t>& bytes) {
  const std::size_t offset = SlotOffset(first);
  if (stack_.size() < offset + bytes.size()) {
    stack_.resize(offset + bytes.size(), 0);
  }
  std::ranges::copy(bytes,
                    stack_.begin() + static_cast<std::ptrdiff_t>(offset));
}

std::size_t Interpreter::SlotOffset(std::size_t slot) const {
  std::size_t offset = 0;
  for (std::size_t i = 0; i < slot && i < stack_slots_.size(); ++i) {
    offset += static_cast<std::size_t>(stack_slots_[i]);
  }
  return offset;
}

namespace {

// The bytes a value of this size takes in the frame.
std::size_t WidthOf(RegSize size) { return size == RegSize32 ? 4 : 8; }

}  // namespace

std::int64_t Interpreter::Read(std::size_t offset, RegSize size) {
  const std::size_t width = WidthOf(size);
  if (stack_.size() < offset + width) stack_.resize(offset + width, 0);

  std::uint64_t value = 0;
  for (std::size_t i = 0; i < width; ++i) {
    value |= static_cast<std::uint64_t>(stack_[offset + i]) << (8 * i);
  }
  // A slot narrower than the register holding it keeps its sign on the way
  // out, as a load of that width does.
  if (size == RegSize32) return static_cast<std::int32_t>(value);
  return static_cast<std::int64_t>(value);
}

void Interpreter::Write(std::size_t offset, RegSize size, std::int64_t value) {
  const std::size_t width = WidthOf(size);
  if (stack_.size() < offset + width) stack_.resize(offset + width, 0);

  for (std::size_t i = 0; i < width; ++i) {
    stack_[offset + i] =
        static_cast<std::uint8_t>(static_cast<std::uint64_t>(value) >> (8 * i));
  }
}

void Interpreter::Interpret(const StoreStack& inst) {
  auto src_val = Get(inst.src_reg);
  assert(src_val.has_value());

  Write(SlotOffset(inst.offset), inst.src_reg.size, *src_val);
}

void Interpreter::Interpret(const StoreStackReg& inst) {
  auto src_val = Get(inst.src_reg);
  assert(src_val.has_value());
  auto offset_val = Get(inst.offset_reg);
  assert(offset_val.has_value());

  Write(SlotOffset(inst.offset) +
            static_cast<std::size_t>(*offset_val) * inst.offset_scale,
        inst.src_reg.size, *src_val);
}

void Interpreter::Interpret(const LoadStack& inst) {
  Set(inst.dst_reg, Read(SlotOffset(inst.offset), inst.dst_reg.size));
}

void Interpreter::Interpret(const LoadStackReg& inst) {
  auto offset_val = Get(inst.offset_reg);
  assert(offset_val.has_value());

  Set(inst.dst_reg,
      Read(SlotOffset(inst.offset) +
               static_cast<std::size_t>(*offset_val) * inst.offset_scale,
           inst.dst_reg.size));
}

std::expected<void, CompError> Interpreter::Interpret(const FuncCall& inst) {
  auto am_cfg = am_cfgs_.Get(inst.label);
  assert(am_cfg.has_value());

  std::vector<std::int64_t> args;
  for (const auto& arg : inst.args) {
    auto arg_val = Get(arg.reg);
    assert(arg_val.has_value());
    args.push_back(*arg_val);
  }
  // An array handed over is passed as its bytes, which the function finds
  // in the slots of its own it takes such arguments in.
  std::vector<std::uint8_t> frame_args;
  if (inst.frame_args_slot != FuncCall::kNoFrameArgsSlot) {
    frame_args =
        SlotBytes(inst.frame_args_slot, am_cfg->frame_params_slot_count);
  }
  // An array the function returns comes back as its bytes, which go in the
  // slots the call set aside for it.
  if (inst.res_slot != FuncCall::kNoResSlot) {
    std::vector<std::uint8_t> bytes;
    std::expected<std::int64_t, CompError> result =
        InterpretFunction(am_cfgs_, *am_cfg, args, am_state_, depth_ + 1,
                          steps_, frame_args, &bytes);
    if (!result.has_value()) return std::unexpected(result.error());

    SetSlotBytes(inst.res_slot, bytes);
    return {};
  }

  std::expected<std::int64_t, CompError> result = InterpretFunction(
      am_cfgs_, *am_cfg, args, am_state_, depth_ + 1, steps_, frame_args);
  if (!result.has_value()) return std::unexpected(result.error());

  assert(inst.res.has_value());

  Set(inst.res->reg, *result);
  return {};
}

void Interpreter::Interpret(const Nop& /*inst*/) {}

// A string is held by where it stands among the strings the program carries,
// which is what the register is set to.
void Interpreter::Interpret(const SetStr& inst) {
  Set(inst.dst_reg, inst.src_val);
}

void Interpreter::Interpret(const MoveReg& inst) {
  auto src_val = Get(inst.src_reg);
  assert(src_val.has_value());

  Set(inst.dst_reg, *src_val);
}

void Interpreter::Interpret(const GtReg& inst) {
  auto lhs_val = Get(inst.lhs_reg);
  assert(lhs_val.has_value());

  auto rhs_val = Get(inst.rhs_reg);
  assert(rhs_val.has_value());

  Set(inst.res_reg, *lhs_val > *rhs_val);
}

void Interpreter::Interpret(const LtReg& inst) {
  auto lhs_val = Get(inst.lhs_reg);
  assert(lhs_val.has_value());

  auto rhs_val = Get(inst.rhs_reg);
  assert(rhs_val.has_value());

  Set(inst.res_reg, *lhs_val < *rhs_val);
}

void Interpreter::Interpret(const GeReg& inst) {
  auto lhs_val = Get(inst.lhs_reg);
  assert(lhs_val.has_value());

  auto rhs_val = Get(inst.rhs_reg);
  assert(rhs_val.has_value());

  Set(inst.res_reg, *lhs_val >= *rhs_val);
}

void Interpreter::Interpret(const LeReg& inst) {
  auto lhs_val = Get(inst.lhs_reg);
  assert(lhs_val.has_value());

  auto rhs_val = Get(inst.rhs_reg);
  assert(rhs_val.has_value());

  Set(inst.res_reg, *lhs_val <= *rhs_val);
}

void Interpreter::Interpret(const EqReg& inst) {
  auto lhs_val = Get(inst.lhs_reg);
  assert(lhs_val.has_value());

  auto rhs_val = Get(inst.rhs_reg);
  assert(rhs_val.has_value());

  Set(inst.res_reg, *lhs_val == *rhs_val);
}

void Interpreter::Interpret(const NotEqReg& inst) {
  auto lhs_val = Get(inst.lhs_reg);
  assert(lhs_val.has_value());

  auto rhs_val = Get(inst.rhs_reg);
  assert(rhs_val.has_value());

  Set(inst.res_reg, *lhs_val != *rhs_val);
}

void Interpreter::Interpret(const Return& inst) {
  auto res_val = Get(inst.res_reg);
  assert(res_val.has_value());

  result_ = *res_val;
}

void Interpreter::Interpret(const SetReg& inst) {
  Set(inst.dst_reg, inst.src_val);
}

void Interpreter::Interpret(const SetInt& inst) {
  Set(inst.dst_reg, inst.src_val);
}

void Interpreter::Interpret(const AddReg& inst) {
  auto lhs_val = Get(inst.lhs_reg);
  assert(lhs_val.has_value());

  auto rhs_val = Get(inst.rhs_reg);
  assert(rhs_val.has_value());

  Set(inst.res_reg, Wrap(Add(*lhs_val, *rhs_val), inst.res_reg.size));
}

void Interpreter::Interpret(const SubReg& inst) {
  auto lhs_val = Get(inst.lhs_reg);
  assert(lhs_val.has_value());

  auto rhs_val = Get(inst.rhs_reg);
  assert(rhs_val.has_value());

  Set(inst.res_reg, Wrap(Sub(*lhs_val, *rhs_val), inst.res_reg.size));
}

void Interpreter::Interpret(const MulReg& inst) {
  auto lhs_val = Get(inst.lhs_reg);
  assert(lhs_val.has_value());

  auto rhs_val = Get(inst.rhs_reg);
  assert(rhs_val.has_value());

  Set(inst.res_reg, Wrap(Mul(*lhs_val, *rhs_val), inst.res_reg.size));
}

std::expected<void, CompError> Interpreter::Interpret(const DivReg& inst) {
  auto lhs_val = Get(inst.lhs_reg);
  assert(lhs_val.has_value());

  auto rhs_val = Get(inst.rhs_reg);
  assert(rhs_val.has_value());
  if (*rhs_val == 0) return std::unexpected(DivideByZero());

  Set(inst.res_reg, Wrap(Div(*lhs_val, *rhs_val), inst.res_reg.size));
  return {};
}

std::expected<void, CompError> Interpreter::Interpret(const ModReg& inst) {
  auto lhs_val = Get(inst.lhs_reg);
  assert(lhs_val.has_value());

  auto rhs_val = Get(inst.rhs_reg);
  assert(rhs_val.has_value());
  if (*rhs_val == 0) return std::unexpected(DivideByZero());

  Set(inst.res_reg, Wrap(Mod(*lhs_val, *rhs_val), inst.res_reg.size));
  return {};
}

}  // namespace lucid
