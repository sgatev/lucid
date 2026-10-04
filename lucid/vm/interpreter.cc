#include "lucid/vm/interpreter.h"

#include <cassert>
#include <cstddef>
#include <cstdint>
#include <expected>
#include <optional>
#include <string>
#include <string_view>
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
std::expected<std::int64_t, CompError> InterpretFunction(
    const HashMap<std::string_view, AbstractMachineControlFlowGraph>& am_cfgs,
    const AbstractMachineControlFlowGraph& am_cfg,
    const std::vector<std::int64_t>& args, AbstractMachineState& am_state,
    int depth, std::int64_t& steps) {
  if (depth > kMaxCompCallDepth) {
    return std::unexpected(CompError("compilation followed more than " +
                                     std::to_string(kMaxCompCallDepth) +
                                     " calls into the program and gave up"));
  }

  Interpreter vm(am_cfgs, am_state, am_cfg.stack_slots, depth, steps);

  for (std::size_t i = 0; i < args.size() && i < am_cfg.params.size(); ++i) {
    vm.Set(am_cfg.params[i], args[i]);
  }

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

    for (const auto& phi : curr_block.phis) {
      assert(prev_block_ref != AbstractMachineControlFlowGraph::kNullBlockRef);

      int come_from = -1;
      for (int i = 0; i < curr_block.preds.size(); ++i) {
        if (curr_block.preds[i] == prev_block_ref) {
          come_from = i;
          break;
        }
      }
      assert(come_from == 0 || come_from == 1);

      auto src_val = vm.Get(phi.srcs[come_from]);
      assert(src_val.has_value());
      vm.Set(phi.dst, *src_val);
    }
    for (const auto& inst : curr_block.instructions) {
      std::expected<Instruction, CompError> res = vm.Interpret(inst);
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
    int depth, std::int64_t& steps)
    : am_cfgs_(am_cfgs),
      am_state_(am_state),
      stack_slots_(stack_slots),
      depth_(depth),
      steps_(steps) {}

std::int64_t Interpreter::Result() const { return result_; }

std::optional<const std::int64_t&> Interpreter::Get(Reg reg) const {
  return values_.Get(reg);
}

void Interpreter::Set(Reg reg, std::int64_t value) { values_.Set(reg, value); }

std::expected<Instruction, CompError> Interpreter::Interpret(
    const Instruction& inst) {
  if (const auto* func_call = std::get_if<FuncCall>(&inst)) {
    return Interpret(*func_call);
  } else if (const auto* mov_reg = std::get_if<MoveReg>(&inst)) {
    return Interpret(*mov_reg);
  } else if (const auto* set_reg = std::get_if<SetReg>(&inst)) {
    return Interpret(*set_reg);
  } else if (const auto* set_int = std::get_if<SetInt>(&inst)) {
    return Interpret(*set_int);
  } else if (const auto* set_str = std::get_if<SetStr>(&inst)) {
    return Interpret(*set_str);
  } else if (std::holds_alternative<Nop>(inst)) {
    return inst;
  } else if (const auto* gt_reg = std::get_if<GtReg>(&inst)) {
    return Interpret(*gt_reg);
  } else if (const auto* lt_reg = std::get_if<LtReg>(&inst)) {
    return Interpret(*lt_reg);
  } else if (const auto* ge_reg = std::get_if<GeReg>(&inst)) {
    return Interpret(*ge_reg);
  } else if (const auto* le_reg = std::get_if<LeReg>(&inst)) {
    return Interpret(*le_reg);
  } else if (const auto* eq_reg = std::get_if<EqReg>(&inst)) {
    return Interpret(*eq_reg);
  } else if (const auto* not_eq_reg = std::get_if<NotEqReg>(&inst)) {
    return Interpret(*not_eq_reg);
  } else if (const auto* ret = std::get_if<Return>(&inst)) {
    return Interpret(*ret);
  } else if (const auto* add_reg = std::get_if<AddReg>(&inst)) {
    return Interpret(*add_reg);
  } else if (const auto* sub_reg = std::get_if<SubReg>(&inst)) {
    return Interpret(*sub_reg);
  } else if (const auto* mul_reg = std::get_if<MulReg>(&inst)) {
    return Interpret(*mul_reg);
  } else if (const auto* div_reg = std::get_if<DivReg>(&inst)) {
    return Interpret(*div_reg);
  } else if (const auto* mod_reg = std::get_if<ModReg>(&inst)) {
    return Interpret(*mod_reg);
  } else if (const auto* store_stack = std::get_if<StoreStack>(&inst)) {
    return Interpret(*store_stack);
  } else if (const auto* store_stack_reg = std::get_if<StoreStackReg>(&inst)) {
    return Interpret(*store_stack_reg);
  } else if (const auto* load_stack = std::get_if<LoadStack>(&inst)) {
    return Interpret(*load_stack);
  } else if (const auto* load_stack_reg = std::get_if<LoadStackReg>(&inst)) {
    return Interpret(*load_stack_reg);
  }
  assert(false);
  return inst;
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
    stack_[offset + i] = static_cast<std::uint8_t>(value >> (8 * i));
  }
}

Instruction Interpreter::Interpret(const StoreStack& inst) {
  auto src_val = values_.Get(inst.src_reg);
  assert(src_val.has_value());

  Write(SlotOffset(inst.offset), inst.src_reg.size, *src_val);
  return inst;
}

Instruction Interpreter::Interpret(const StoreStackReg& inst) {
  auto src_val = values_.Get(inst.src_reg);
  assert(src_val.has_value());
  auto offset_val = values_.Get(inst.offset_reg);
  assert(offset_val.has_value());

  Write(SlotOffset(inst.offset) + static_cast<std::size_t>(*offset_val),
        inst.src_reg.size, *src_val);
  return inst;
}

Instruction Interpreter::Interpret(const LoadStack& inst) {
  values_.Set(inst.dst_reg, Read(SlotOffset(inst.offset), inst.dst_reg.size));
  return inst;
}

Instruction Interpreter::Interpret(const LoadStackReg& inst) {
  auto offset_val = values_.Get(inst.offset_reg);
  assert(offset_val.has_value());

  values_.Set(inst.dst_reg, Read(SlotOffset(inst.offset) +
                                     static_cast<std::size_t>(*offset_val),
                                 inst.dst_reg.size));
  return inst;
}

std::expected<Instruction, CompError> Interpreter::Interpret(
    const FuncCall& inst) {
  auto am_cfg = am_cfgs_.Get(inst.label);
  assert(am_cfg.has_value());

  std::vector<std::int64_t> args;
  for (const auto& arg : inst.args) {
    auto arg_val = values_.Get(arg.reg);
    assert(arg_val.has_value());
    args.push_back(*arg_val);
  }
  std::expected<std::int64_t, CompError> result =
      InterpretFunction(am_cfgs_, *am_cfg, args, am_state_, depth_ + 1, steps_);
  if (!result.has_value()) return std::unexpected(result.error());

  assert(inst.res.has_value());

  values_.Set(inst.res->reg, *result);

  // The call is replaced by what it evaluated to, as an instruction that
  // sets a value of the kind the function returns.
  switch (am_cfg->result_kind) {
    case ValueKind::Number:
      return SetValue(*result, inst.res->reg, am_state_);
    case ValueKind::String:
      return SetStr{
          .src_val = static_cast<std::uint32_t>(*result),
          .dst_reg = inst.res->reg,
      };
  }
}

// A string is held by where it stands among the strings the program carries,
// which is what the register is set to.
Instruction Interpreter::Interpret(const SetStr& inst) {
  values_.Set(inst.dst_reg, inst.src_val);
  return inst;
}

Instruction Interpreter::Interpret(const MoveReg& inst) {
  auto src_val = values_.Get(inst.src_reg);
  assert(src_val.has_value());

  values_.Set(inst.dst_reg, *src_val);
  return inst;
}

Instruction Interpreter::Interpret(const GtReg& inst) {
  auto lhs_val = values_.Get(inst.lhs_reg);
  assert(lhs_val.has_value());

  auto rhs_val = values_.Get(inst.rhs_reg);
  assert(rhs_val.has_value());

  values_.Set(inst.res_reg, *lhs_val > *rhs_val);
  return inst;
}

Instruction Interpreter::Interpret(const LtReg& inst) {
  auto lhs_val = values_.Get(inst.lhs_reg);
  assert(lhs_val.has_value());

  auto rhs_val = values_.Get(inst.rhs_reg);
  assert(rhs_val.has_value());

  values_.Set(inst.res_reg, *lhs_val < *rhs_val);
  return inst;
}

Instruction Interpreter::Interpret(const GeReg& inst) {
  auto lhs_val = values_.Get(inst.lhs_reg);
  assert(lhs_val.has_value());

  auto rhs_val = values_.Get(inst.rhs_reg);
  assert(rhs_val.has_value());

  values_.Set(inst.res_reg, *lhs_val >= *rhs_val);
  return inst;
}

Instruction Interpreter::Interpret(const LeReg& inst) {
  auto lhs_val = values_.Get(inst.lhs_reg);
  assert(lhs_val.has_value());

  auto rhs_val = values_.Get(inst.rhs_reg);
  assert(rhs_val.has_value());

  values_.Set(inst.res_reg, *lhs_val <= *rhs_val);
  return inst;
}

Instruction Interpreter::Interpret(const EqReg& inst) {
  auto lhs_val = values_.Get(inst.lhs_reg);
  assert(lhs_val.has_value());

  auto rhs_val = values_.Get(inst.rhs_reg);
  assert(rhs_val.has_value());

  values_.Set(inst.res_reg, *lhs_val == *rhs_val);
  return inst;
}

Instruction Interpreter::Interpret(const NotEqReg& inst) {
  auto lhs_val = values_.Get(inst.lhs_reg);
  assert(lhs_val.has_value());

  auto rhs_val = values_.Get(inst.rhs_reg);
  assert(rhs_val.has_value());

  values_.Set(inst.res_reg, *lhs_val != *rhs_val);
  return inst;
}

Instruction Interpreter::Interpret(const Return& inst) {
  auto res_val = values_.Get(inst.res_reg);
  assert(res_val.has_value());

  result_ = *res_val;
  return inst;
}

Instruction Interpreter::Interpret(const SetReg& inst) {
  values_.Set(inst.dst_reg, inst.src_val);
  return inst;
}

Instruction Interpreter::Interpret(const SetInt& inst) {
  values_.Set(inst.dst_reg, inst.src_val);
  return inst;
}

Instruction Interpreter::Interpret(const AddReg& inst) {
  auto lhs_val = values_.Get(inst.lhs_reg);
  assert(lhs_val.has_value());

  auto rhs_val = values_.Get(inst.rhs_reg);
  assert(rhs_val.has_value());

  values_.Set(inst.res_reg, Wrap(Add(*lhs_val, *rhs_val), inst.res_reg.size));
  return inst;
}

Instruction Interpreter::Interpret(const SubReg& inst) {
  auto lhs_val = values_.Get(inst.lhs_reg);
  assert(lhs_val.has_value());

  auto rhs_val = values_.Get(inst.rhs_reg);
  assert(rhs_val.has_value());

  values_.Set(inst.res_reg, Wrap(Sub(*lhs_val, *rhs_val), inst.res_reg.size));
  return inst;
}

Instruction Interpreter::Interpret(const MulReg& inst) {
  auto lhs_val = values_.Get(inst.lhs_reg);
  assert(lhs_val.has_value());

  auto rhs_val = values_.Get(inst.rhs_reg);
  assert(rhs_val.has_value());

  values_.Set(inst.res_reg, Wrap(Mul(*lhs_val, *rhs_val), inst.res_reg.size));
  return inst;
}

std::expected<Instruction, CompError> Interpreter::Interpret(
    const DivReg& inst) {
  auto lhs_val = values_.Get(inst.lhs_reg);
  assert(lhs_val.has_value());

  auto rhs_val = values_.Get(inst.rhs_reg);
  assert(rhs_val.has_value());
  if (*rhs_val == 0) return std::unexpected(DivideByZero());

  values_.Set(inst.res_reg, Wrap(Div(*lhs_val, *rhs_val), inst.res_reg.size));
  return inst;
}

std::expected<Instruction, CompError> Interpreter::Interpret(
    const ModReg& inst) {
  auto lhs_val = values_.Get(inst.lhs_reg);
  assert(lhs_val.has_value());

  auto rhs_val = values_.Get(inst.rhs_reg);
  assert(rhs_val.has_value());
  if (*rhs_val == 0) return std::unexpected(DivideByZero());

  values_.Set(inst.res_reg, Wrap(Mod(*lhs_val, *rhs_val), inst.res_reg.size));
  return inst;
}

}  // namespace lucid
