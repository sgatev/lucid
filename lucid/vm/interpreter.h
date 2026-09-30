#pragma once

#include <cstddef>
#include <cstdint>
#include <expected>
#include <optional>
#include <string_view>
#include <vector>

#include "lucid/am/cfg.h"
#include "lucid/am/instructions.h"
#include "lucid/am/state.h"
#include "lucid/core/container/hash_map.h"
#include "lucid/syntax/comp.h"

namespace lucid {

// How many calls deep compilation will follow a program before it gives up.
//
// A call made during compilation is followed by calling into the interpreter
// again, so each one costs a frame of the compiler's own stack rather than of
// a frame the program lays out. The limit stands well below the depth that
// stack runs out at, so that a program that recurses without end is told so
// rather than taking the compiler down with it.
//
// How deep that is depends on how the compiler was built: a build carrying
// sanitizers runs out at around 1900 where an ordinary one goes past 8000.
// The limit is one number rather than one per build, so that what compiles
// does not depend on how the compiler was made, and it is set low enough to
// hold for the narrowest of them.
inline constexpr int kMaxCompCallDepth = 1000;

// How much of a program compilation will work through before it gives up,
// counted in the instructions it runs and the blocks it enters. A block is
// counted as well as what it holds, so that a loop holding nothing is caught
// along with one holding something.
//
// The depth limit is what a program that calls without end runs into; this
// is what one that loops without end runs into instead. The interpreter
// works through a few million of these a second, so a compilation that
// reaches this one has spent several seconds with no end to it in sight.
inline constexpr std::int64_t kMaxCompSteps = 10'000'000;

// Interprets abstract machine instructions in `am_cfg` and returns the result.
//
// `depth` is how many calls compilation has already followed to reach this
// one. Returns an error where following this one would pass
// `kMaxCompCallDepth`, or where working the function out passes
// `kMaxCompSteps`.
std::expected<std::int64_t, CompError> InterpretAbstractMachineFunction(
    const HashMap<std::string_view, AbstractMachineControlFlowGraph>& am_cfgs,
    const AbstractMachineControlFlowGraph& am_cfg,
    const std::vector<std::int64_t>& args, AbstractMachineState& am_state,
    int depth = 0);

// Interprets abstract machine instructions.
class Interpreter {
 public:
  // `stack_slots` says how wide each slot of the frame is, in the order they
  // lie, and has to outlive the interpreter.
  // `depth` is how many calls compilation followed to reach the function
  // being interpreted, and `steps` is how much it has worked through so far.
  // Both are what a call made from here is measured against, and `steps` is
  // shared with every call followed from here so that what is spent is
  // counted once for the whole of a compilation rather than afresh for each
  // function it reaches.
  explicit Interpreter(
      const HashMap<std::string_view, AbstractMachineControlFlowGraph>& am_cfgs,
      AbstractMachineState& am_state, const std::vector<int>& stack_slots,
      int depth, std::int64_t& steps);

  std::int64_t Result() const;

  std::optional<const std::int64_t&> Get(Reg reg) const;

  void Set(Reg reg, std::int64_t value);

  // Interprets one instruction and returns what it comes to, which is an
  // instruction that sets the same value without doing the work again.
  //
  // A call is the one instruction that can fail, by standing too deep.
  std::expected<Instruction, CompError> Interpret(const Instruction& inst);

 private:
  std::expected<Instruction, CompError> Interpret(const FuncCall& inst);
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

  // How many calls compilation followed to reach what is being interpreted,
  // and how much of the program it has worked through altogether.
  int depth_;
  std::int64_t& steps_;

  std::int64_t result_;
};

}  // namespace lucid
