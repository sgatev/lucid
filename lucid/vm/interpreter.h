#pragma once

#include <string_view>
#include <vector>

#include "lucid/am/cfg.h"
#include "lucid/am/instructions.h"
#include "lucid/am/state.h"
#include "lucid/core/container/hash_map.h"
#include "lucid/core/container/optional_ref.h"

namespace lucid {

// Interprets abstract machine instructions in `am_cfg` and returns the result.
int InterpretAbstractMachineFunction(
    const HashMap<std::string_view, AbstractMachineControlFlowGraph>& am_cfgs,
    const AbstractMachineControlFlowGraph& am_cfg, const std::vector<int>& args,
    AbstractMachineState& am_state);

// Interprets abstract machine instructions.
class Interpreter {
 public:
  explicit Interpreter(
      const HashMap<std::string_view, AbstractMachineControlFlowGraph>& am_cfgs,
      AbstractMachineState& am_state);

  int Result() const;

  OptionalRef<const int> Get(Reg reg) const;

  void Set(Reg reg, int value);

  Instruction Interpret(const Instruction& inst);

 private:
  Instruction Interpret(const FuncCall& inst);
  Instruction Interpret(const MoveReg& inst);
  Instruction Interpret(const GtReg& inst);
  Instruction Interpret(const LtReg& inst);
  Instruction Interpret(const EqReg& inst);
  Instruction Interpret(const NotEqReg& inst);
  Instruction Interpret(const Return& inst);
  Instruction Interpret(const SetReg& inst);
  Instruction Interpret(const AddReg& inst);
  Instruction Interpret(const SubReg& inst);
  Instruction Interpret(const MulReg& inst);
  Instruction Interpret(const DivReg& inst);
  Instruction Interpret(const ModReg& inst);

  const HashMap<std::string_view, AbstractMachineControlFlowGraph>& am_cfgs_;
  AbstractMachineState& am_state_;
  HashMap<Reg, int> values_;
  int result_;
};

}  // namespace lucid
