#pragma once

#include <cstdint>
#include <string_view>
#include <variant>

namespace lucid {

// A register in the Lucid abstract machine.
using RegId = int;

// Moves the value of an int32 register into another one.
struct MoveReg32 {
  // Source int32 register.
  RegId src_reg;

  // Destination int32 register.
  RegId dst_reg;

  bool operator==(const MoveReg32& other) const {
    return src_reg == other.src_reg && dst_reg == other.dst_reg;
  }
};

// Sets an int32 value in a register.
struct SetReg32 {
  // Source int32 value.
  std::string_view src_val;

  // Destination int32 register.
  RegId dst_reg;

  bool operator==(const SetReg32& other) const {
    return src_val == other.src_val && dst_reg == other.dst_reg;
  }
};

// Jumps to a labeled location.
struct Jump {
  // Label of the location to jump to.
  std::string_view label;

  bool operator==(const Jump& other) const { return label == other.label; }
};

// Returns to the location before the last jump.
struct Return {
  bool operator==(const Return& other) const { return true; }
};

// An instruction for the Lucid abstract machine.
using Instruction = std::variant<MoveReg32, SetReg32, Jump, Return>;

}  // namespace lucid
