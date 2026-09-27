#pragma once

#include <cstddef>
#include <cstdint>
#include <optional>
#include <string_view>
#include <vector>

#include "lucid/am/cfg.h"
#include "lucid/am/instructions.h"
#include "lucid/am/state.h"
#include "lucid/core/container/hash_map.h"

namespace lucid {

// Interprets abstract machine instructions in `am_cfg` and returns the result.
int InterpretAbstractMachineFunction(
    const HashMap<std::string_view, AbstractMachineControlFlowGraph>& am_cfgs,
    const AbstractMachineControlFlowGraph& am_cfg,
    const std::vector<std::int64_t>& args, AbstractMachineState& am_state);

// Interprets abstract machine instructions.
class Interpreter {
 public:
  // `stack_slots` says how wide each slot of the frame is, in the order they
  // lie, and has to outlive the interpreter.
  explicit Interpreter(
      const HashMap<std::string_view, AbstractMachineControlFlowGraph>& am_cfgs,
      AbstractMachineState& am_state, const std::vector<int>& stack_slots);

  std::int64_t Result() const;

  std::optional<const std::int64_t&> Get(Reg reg) const;

  void Set(Reg reg, std::int64_t value);

  Instruction Interpret(const Instruction& inst);

 private:
  Instruction Interpret(const FuncCall& inst);
  Instruction Interpret(const MoveReg& inst);
  Instruction Interpret(const GtReg& inst);
  Instruction Interpret(const LtReg& inst);
  Instruction Interpret(const GeReg& inst);
  Instruction Interpret(const LeReg& inst);
  Instruction Interpret(const EqReg& inst);
  Instruction Interpret(const NotEqReg& inst);
  Instruction Interpret(const Return& inst);
  Instruction Interpret(const SetReg& inst);
  Instruction Interpret(const SetInt& inst);
  Instruction Interpret(const SetStr& inst);
  Instruction Interpret(const AddReg& inst);
  Instruction Interpret(const SubReg& inst);
  Instruction Interpret(const MulReg& inst);
  Instruction Interpret(const DivReg& inst);
  Instruction Interpret(const ModReg& inst);
  Instruction Interpret(const StoreStack& inst);
  Instruction Interpret(const StoreStackReg& inst);
  Instruction Interpret(const LoadStack& inst);
  Instruction Interpret(const LoadStackReg& inst);

  // Where the slot with this index begins, in bytes, which is where all the
  // slots before it end.
  std::size_t SlotOffset(std::size_t slot) const;

  // Reads and writes a value of `size` at `offset` bytes into the frame,
  // growing the frame to hold it. The bytes are laid out low first, as the
  // machine this stands in for lays them out.
  std::int64_t Read(std::size_t offset, RegSize size);
  void Write(std::size_t offset, RegSize size, std::int64_t value);

  const HashMap<std::string_view, AbstractMachineControlFlowGraph>& am_cfgs_;
  AbstractMachineState& am_state_;
  HashMap<Reg, std::int64_t> values_;

  // The frame that arrays and tuples live in, as bytes, beside the widths
  // that say where one slot of it ends and the next begins.
  const std::vector<int>& stack_slots_;
  std::vector<std::uint8_t> stack_;

  std::int64_t result_;
};

}  // namespace lucid
