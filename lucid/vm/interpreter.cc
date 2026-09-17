#include "lucid/vm/interpreter.h"

#include <cassert>
#include <optional>
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
  auto curr_block_ref = am_cfg.first;
  while (true) {
    const auto& curr_block = am_cfg.GetBlock(curr_block_ref);
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
      vm.Interpret(inst);
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

Interpreter::Interpreter(
    const HashMap<std::string_view, AbstractMachineControlFlowGraph>& am_cfgs,
    AbstractMachineState& am_state)
    : am_cfgs_(am_cfgs), am_state_(am_state) {}

int Interpreter::Result() const { return result_; }

std::optional<const int&> Interpreter::Get(Reg reg) const {
  return values_.Get(reg);
}

void Interpreter::Set(Reg reg, int value) { values_.Set(reg, value); }

Instruction Interpreter::Interpret(const Instruction& inst) {
  if (const auto* func_call = std::get_if<FuncCall>(&inst)) {
    return Interpret(*func_call);
  } else if (const auto* mov_reg = std::get_if<MoveReg>(&inst)) {
    return Interpret(*mov_reg);
  } else if (const auto* set_reg = std::get_if<SetReg>(&inst)) {
    return Interpret(*set_reg);
  } else if (const auto* gt_reg = std::get_if<GtReg>(&inst)) {
    return Interpret(*gt_reg);
  } else if (const auto* lt_reg = std::get_if<LtReg>(&inst)) {
    return Interpret(*lt_reg);
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

  return SetReg{
      .src_val = result,
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

Instruction Interpreter::Interpret(const LtReg& inst) {
  auto lhs_val = values_.Get(inst.lhs_reg);
  assert(lhs_val.has_value());

  auto rhs_val = values_.Get(inst.rhs_reg);
  assert(rhs_val.has_value());

  values_.Set(inst.res_reg, *lhs_val < *rhs_val);
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

Instruction Interpreter::Interpret(const AddReg& inst) {
  auto lhs_val = values_.Get(inst.lhs_reg);
  assert(lhs_val.has_value());

  auto rhs_val = values_.Get(inst.rhs_reg);
  assert(rhs_val.has_value());

  values_.Set(inst.res_reg, *lhs_val + *rhs_val);
  return inst;
}

Instruction Interpreter::Interpret(const SubReg& inst) {
  auto lhs_val = values_.Get(inst.lhs_reg);
  assert(lhs_val.has_value());

  auto rhs_val = values_.Get(inst.rhs_reg);
  assert(rhs_val.has_value());

  values_.Set(inst.res_reg, *lhs_val - *rhs_val);
  return inst;
}

Instruction Interpreter::Interpret(const MulReg& inst) {
  auto lhs_val = values_.Get(inst.lhs_reg);
  assert(lhs_val.has_value());

  auto rhs_val = values_.Get(inst.rhs_reg);
  assert(rhs_val.has_value());

  values_.Set(inst.res_reg, *lhs_val * *rhs_val);
  return inst;
}

Instruction Interpreter::Interpret(const DivReg& inst) {
  auto lhs_val = values_.Get(inst.lhs_reg);
  assert(lhs_val.has_value());

  auto rhs_val = values_.Get(inst.rhs_reg);
  assert(rhs_val.has_value());

  values_.Set(inst.res_reg, *lhs_val / *rhs_val);
  return inst;
}

Instruction Interpreter::Interpret(const ModReg& inst) {
  auto lhs_val = values_.Get(inst.lhs_reg);
  assert(lhs_val.has_value());

  auto rhs_val = values_.Get(inst.rhs_reg);
  assert(rhs_val.has_value());

  values_.Set(inst.res_reg, *lhs_val % *rhs_val);
  return inst;
}

}  // namespace lucid
