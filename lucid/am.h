#pragma once

#include <cstddef>
#include <string>
#include <string_view>
#include <variant>

namespace lucid {

// A register in the Lucid abstract machine.
using RegId = int;

// A no op instruction.
struct Nop {
  bool operator==(const Nop&) const { return true; }
};

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

// Jumps to a labeled location based on the value of a register.
struct CondJump {
  // Int32 register used as a condition for the jump.
  RegId cond_reg;

  // Label of the location to jump to if the value in the register is not zero.
  std::size_t then_label;

  // Label of the location to jump to if the value in the register is zero.
  std::size_t else_label;

  bool operator==(const CondJump& other) const {
    return cond_reg == other.cond_reg && then_label == other.then_label &&
           else_label == other.else_label;
  }
};

// A label in the list of instructions.
struct Label {
  // Identifier of the label.
  std::size_t id;

  bool operator==(const Label& other) const { return id == other.id; }
};

// Returns to the location before the last jump.
struct Return {
  bool operator==(const Return&) const { return true; }
};

// Adds the contents of two int32 registers.
struct AddReg32 {
  // Result int32 register.
  RegId res_reg;

  // First operand int32 register.
  RegId lhs_reg;

  // Second operand int32 register.
  RegId rhs_reg;

  bool operator==(const AddReg32& other) const {
    return res_reg == other.res_reg && lhs_reg == other.lhs_reg &&
           rhs_reg == other.rhs_reg;
  }
};

// Subtracts the contents of one int32 register from another.
struct SubReg32 {
  // Result int32 register.
  RegId res_reg;

  // First operand int32 register.
  RegId lhs_reg;

  // Second operand int32 register.
  RegId rhs_reg;

  bool operator==(const SubReg32& other) const {
    return res_reg == other.res_reg && lhs_reg == other.lhs_reg &&
           rhs_reg == other.rhs_reg;
  }
};

// Multiplies the contents of two int32 registers.
struct MulReg32 {
  // Result int32 register.
  RegId res_reg;

  // First operand int32 register.
  RegId lhs_reg;

  // Second operand int32 register.
  RegId rhs_reg;

  bool operator==(const MulReg32& other) const {
    return res_reg == other.res_reg && lhs_reg == other.lhs_reg &&
           rhs_reg == other.rhs_reg;
  }
};

// Divides the contents of one int32 register by another.
struct DivReg32 {
  // Result int32 register.
  RegId res_reg;

  // First operand int32 register.
  RegId lhs_reg;

  // Second operand int32 register.
  RegId rhs_reg;

  bool operator==(const DivReg32& other) const {
    return res_reg == other.res_reg && lhs_reg == other.lhs_reg &&
           rhs_reg == other.rhs_reg;
  }
};

// Compares the values stored in two int32 registers.
struct GtReg32 {
  // Result int32 register.
  RegId res_reg;

  // First operand int32 register.
  RegId lhs_reg;

  // Second operand int32 register.
  RegId rhs_reg;

  bool operator==(const GtReg32& other) const {
    return res_reg == other.res_reg && lhs_reg == other.lhs_reg &&
           rhs_reg == other.rhs_reg;
  }
};

// An instruction for the Lucid abstract machine.
using Instruction =
    std::variant<Nop, MoveReg32, SetReg32, Jump, CondJump, Label, Return,
                 AddReg32, SubReg32, MulReg32, DivReg32, GtReg32>;

}  // namespace lucid
