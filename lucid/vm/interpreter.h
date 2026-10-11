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
  // `reg_count` is how many registers the function uses, which is room for a
  // value for each of them up front. A register past it still takes a value,
  // as one made after the interpreter was: the function generator works out
  // what it can while it is still making the function's registers.
  // `depth` is how many calls compilation followed to reach the function
  // being interpreted, and `steps` is how much it has worked through so far.
  // Both are what a call made from here is measured against, and `steps` is
  // shared with every call followed from here so that what is spent is
  // counted once for the whole of a compilation rather than afresh for each
  // function it reaches.
  explicit Interpreter(
      const HashMap<std::string_view, AbstractMachineControlFlowGraph>& am_cfgs,
      AbstractMachineState& am_state, const std::vector<int>& stack_slots,
      std::size_t reg_count, int depth, std::int64_t& steps);

  std::int64_t Result() const;

  std::optional<const std::int64_t&> Get(Reg reg) const;

  void Set(Reg reg, std::int64_t value);

  // Interprets `inst`.
  std::expected<void, CompError> Interpret(const Instruction& inst);

  // Interprets `inst` and replaces it with what it evaluates to.
  std::expected<void, CompError> Fold(Instruction& inst);

 private:
  std::expected<void, CompError> Interpret(const FuncCall& inst);
  void Interpret(const Nop& inst);
  void Interpret(const MoveReg& inst);
  void Interpret(const GtReg& inst);
  void Interpret(const LtReg& inst);
  void Interpret(const GeReg& inst);
  void Interpret(const LeReg& inst);
  void Interpret(const EqReg& inst);
  void Interpret(const NotEqReg& inst);
  void Interpret(const Return& inst);
  void Interpret(const SetReg& inst);
  void Interpret(const SetInt& inst);
  void Interpret(const SetStr& inst);
  void Interpret(const AddReg& inst);
  void Interpret(const SubReg& inst);
  void Interpret(const MulReg& inst);
  std::expected<void, CompError> Interpret(const DivReg& inst);
  std::expected<void, CompError> Interpret(const ModReg& inst);
  void Interpret(const StoreStack& inst);
  void Interpret(const StoreStackReg& inst);
  void Interpret(const LoadStack& inst);
  void Interpret(const LoadStackReg& inst);

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
  // The value of each register, by its ID, or nothing for one not set yet.
  // A register is told apart by its ID alone: before registers are given to a
  // machine, which is all the interpreter runs, no two of different sizes
  // share one.
  std::vector<std::optional<std::int64_t>> values_;

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
