#include "lucid/vm/interpreter.h"

#include <cassert>
#include <charconv>
#include <string>
#include <variant>
#include <vector>

#include "lucid/am/cfg.h"
#include "lucid/am/instructions.h"

namespace lucid {

int InterpretAbstractMachineFunction(
    const HashMap<std::string_view, AbstractMachineControlFlowGraph>& am_cfgs,
    const AbstractMachineControlFlowGraph& am_cfg, const std::vector<int>& args,
    AbstractMachineState& am_state) {
  Interpreter vm(am_cfgs, am_state);

  for (int i = 0; i < args.size(); ++i) {
    vm.Set(am_cfg.params[i], args[i]);
  }

  AbstractMachineControlFlowGraph::BlockRef prev_block_ref =
      AbstractMachineControlFlowGraph::kNullBlockRef;
  auto current_block_ref = am_cfg.first;
  while (true) {
    const auto& current_block = am_cfg.get(current_block_ref);
    for (const auto& phi : current_block.phis) {
      assert(prev_block_ref != AbstractMachineControlFlowGraph::kNullBlockRef);

      int come_from = -1;
      for (int i = 0; i < current_block.preds.size(); ++i) {
        if (current_block.preds[i] == prev_block_ref) {
          come_from = i;
          break;
        }
      }
      assert(come_from == 0 || come_from == 1);

      auto src_val = vm.Get(phi.srcs[come_from]);
      assert(src_val.has_value());
      vm.Set(phi.dst, *src_val);
    }
    for (const auto& inst : current_block.instructions) {
      vm.Interpret(inst);
    }
    if (current_block.branch_cond.has_value()) {
      assert(current_block.next.size() == 2);

      auto branch_cond_value = vm.Get(*current_block.branch_cond);
      assert(branch_cond_value.has_value());

      if (*branch_cond_value != 0) {
        prev_block_ref = current_block.ref;
        current_block_ref = current_block.next[0];
      } else {
        prev_block_ref = current_block.ref;
        current_block_ref = current_block.next[1];
      }
    } else if (current_block.next.size() == 1) {
      prev_block_ref = current_block.ref;
      current_block_ref = current_block.next[0];
    } else {
      assert(current_block.next.size() == 0);
      assert(current_block.ref == am_cfg.last);
      break;
    }
  }
  return vm.Result();
}

Interpreter::Interpreter(
    const HashMap<std::string_view, AbstractMachineControlFlowGraph>& am_cfgs,
    AbstractMachineState& am_state)
    : am_cfgs_(am_cfgs), am_state_(am_state) {}

int Interpreter::Result() const { return result_; }

OptionalRef<const int> Interpreter::Get(RegId reg) const {
  return values_.Get(reg);
}

void Interpreter::Set(RegId reg, int value) { values_.Set(reg, value); }

Instruction Interpreter::Interpret(const Instruction& inst) {
  if (const auto* func_call = std::get_if<FuncCall>(&inst)) {
    return Interpret(*func_call);
  } else if (const auto* mov_reg = std::get_if<MoveReg>(&inst)) {
    return Interpret(*mov_reg);
  } else if (const auto* set_reg = std::get_if<SetReg>(&inst)) {
    return Interpret(*set_reg);
  } else if (const auto* gt_reg = std::get_if<GtReg>(&inst)) {
    return Interpret(*gt_reg);
  } else if (const auto* ret = std::get_if<Return>(&inst)) {
    return Interpret(*ret);
  } else if (const auto* add_reg = std::get_if<AddReg>(&inst)) {
    return Interpret(*add_reg);
  }
  assert(false);
  return inst;
}

Instruction Interpreter::Interpret(const FuncCall& inst) {
  auto am_cfg = am_cfgs_.Get(inst.label);
  assert(am_cfg.has_value());

  std::vector<int> args;
  for (const auto& arg : inst.args) {
    auto arg_val = values_.Get(arg.reg);
    assert(arg_val.has_value());
    args.push_back(*arg_val);
  }
  int result =
      InterpretAbstractMachineFunction(am_cfgs_, *am_cfg, args, am_state_);

  assert(inst.res.has_value());

  values_.Set(inst.res->reg, result);

  am_state_.literals.push_back(std::to_string(result));
  return SetReg{
      .src_val = am_state_.literals.back(),
      .dst_reg = inst.res->reg,
  };
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

Instruction Interpreter::Interpret(const Return& inst) {
  auto res_val = values_.Get(inst.res_reg);
  assert(res_val.has_value());

  result_ = *res_val;
  return inst;
}

Instruction Interpreter::Interpret(const SetReg& inst) {
  int src_val;
  std::from_chars(inst.src_val.begin(), inst.src_val.end(), src_val);

  values_.Set(inst.dst_reg, src_val);
  return inst;
}

Instruction Interpreter::Interpret(const AddReg& inst) {
  auto lhs_val = values_.Get(inst.lhs_reg);
  assert(lhs_val.has_value());

  auto rhs_val = values_.Get(inst.rhs_reg);
  assert(rhs_val.has_value());

  values_.Set(inst.res_reg, *lhs_val + *rhs_val);
  return inst;
}

}  // namespace lucid
