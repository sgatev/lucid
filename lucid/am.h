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

// Moves the value of an 32-bit register into another one.
struct MoveReg32 {
  // Source register.
  RegId src_reg;

  // Destination register.
  RegId dst_reg;

  bool operator==(const MoveReg32& other) const {
    return src_reg == other.src_reg && dst_reg == other.dst_reg;
  }
};

// Moves the value of a 64-bit register into another one.
struct MoveReg64 {
  // Source register.
  RegId src_reg;

  // Destination register.
  RegId dst_reg;

  bool operator==(const MoveReg64& other) const {
    return src_reg == other.src_reg && dst_reg == other.dst_reg;
  }
};

// Sets a 32-bit value in a register.
struct SetReg32 {
  // Source value.
  std::string_view src_val;

  // Destination register.
  RegId dst_reg;

  bool operator==(const SetReg32& other) const {
    return src_val == other.src_val && dst_reg == other.dst_reg;
  }
};

// Sets 64-bit value in a register.
struct SetReg64 {
  // Source value.
  std::string_view src_val;

  // Destination register.
  RegId dst_reg;

  bool operator==(const SetReg64& other) const {
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

// Adds the contents of two 32-bit registers.
struct AddReg32 {
  // Result register.
  RegId res_reg;

  // First operand source register.
  RegId lhs_reg;

  // Second operand source register.
  RegId rhs_reg;

  bool operator==(const AddReg32& other) const {
    return res_reg == other.res_reg && lhs_reg == other.lhs_reg &&
           rhs_reg == other.rhs_reg;
  }
};

// Adds the contents of two 64-bit registers.
struct AddReg64 {
  // Result register.
  RegId res_reg;

  // First operand register.
  RegId lhs_reg;

  // Second operand register.
  RegId rhs_reg;

  bool operator==(const AddReg64& other) const {
    return res_reg == other.res_reg && lhs_reg == other.lhs_reg &&
           rhs_reg == other.rhs_reg;
  }
};

// Subtracts the contents of one 32-bit register from another.
struct SubReg32 {
  // Result register.
  RegId res_reg;

  // First operand register.
  RegId lhs_reg;

  // Second operand register.
  RegId rhs_reg;

  bool operator==(const SubReg32& other) const {
    return res_reg == other.res_reg && lhs_reg == other.lhs_reg &&
           rhs_reg == other.rhs_reg;
  }
};

// Subtracts the contents of one 64-bit register from another.
struct SubReg64 {
  // Result register.
  RegId res_reg;

  // First operand register.
  RegId lhs_reg;

  // Second operand register.
  RegId rhs_reg;

  bool operator==(const SubReg64& other) const {
    return res_reg == other.res_reg && lhs_reg == other.lhs_reg &&
           rhs_reg == other.rhs_reg;
  }
};

// Multiplies the contents of two 32-bit registers.
struct MulReg32 {
  // Result register.
  RegId res_reg;

  // First operand register.
  RegId lhs_reg;

  // Second operand register.
  RegId rhs_reg;

  bool operator==(const MulReg32& other) const {
    return res_reg == other.res_reg && lhs_reg == other.lhs_reg &&
           rhs_reg == other.rhs_reg;
  }
};

// Multiplies the contents of two 64-bit registers.
struct MulReg64 {
  // Result register.
  RegId res_reg;

  // First operand register.
  RegId lhs_reg;

  // Second operand register.
  RegId rhs_reg;

  bool operator==(const MulReg64& other) const {
    return res_reg == other.res_reg && lhs_reg == other.lhs_reg &&
           rhs_reg == other.rhs_reg;
  }
};

// Divides the contents of one 32-bit register by another.
struct DivReg32 {
  // Result register.
  RegId res_reg;

  // First operand register.
  RegId lhs_reg;

  // Second operand register.
  RegId rhs_reg;

  bool operator==(const DivReg32& other) const {
    return res_reg == other.res_reg && lhs_reg == other.lhs_reg &&
           rhs_reg == other.rhs_reg;
  }
};

// Divides the contents of one 64-bit register by another.
struct DivReg64 {
  // Result register.
  RegId res_reg;

  // First operand register.
  RegId lhs_reg;

  // Second operand register.
  RegId rhs_reg;

  bool operator==(const DivReg64& other) const {
    return res_reg == other.res_reg && lhs_reg == other.lhs_reg &&
           rhs_reg == other.rhs_reg;
  }
};

// Tests the values in two 32-bit registers for a "greater than" relationship.
struct GtReg32 {
  // Result register.
  RegId res_reg;

  // First operand register.
  RegId lhs_reg;

  // Second operand register.
  RegId rhs_reg;

  bool operator==(const GtReg32& other) const {
    return res_reg == other.res_reg && lhs_reg == other.lhs_reg &&
           rhs_reg == other.rhs_reg;
  }
};

// Tests the values in two 64-bit registers for a "greater than" relationship.
struct GtReg64 {
  // Result register.
  RegId res_reg;

  // First operand register.
  RegId lhs_reg;

  // Second operand register.
  RegId rhs_reg;

  bool operator==(const GtReg64& other) const {
    return res_reg == other.res_reg && lhs_reg == other.lhs_reg &&
           rhs_reg == other.rhs_reg;
  }
};

// Tests the values in two int32 registers for a "less than" relationship.
struct LtReg32 {
  // Result int32 register.
  RegId res_reg;

  // First operand int32 register.
  RegId lhs_reg;

  // Second operand int32 register.
  RegId rhs_reg;

  bool operator==(const LtReg32& other) const {
    return res_reg == other.res_reg && lhs_reg == other.lhs_reg &&
           rhs_reg == other.rhs_reg;
  }
};

// Tests the values in two 32-bit registers for a "less than" relationship.
struct LtReg64 {
  // Result register.
  RegId res_reg;

  // First operand register.
  RegId lhs_reg;

  // Second operand register.
  RegId rhs_reg;

  bool operator==(const LtReg64& other) const {
    return res_reg == other.res_reg && lhs_reg == other.lhs_reg &&
           rhs_reg == other.rhs_reg;
  }
};

// Pushes bytes onto the stack.
struct PushStack {
  // Number of bytes to push.
  std::size_t size;

  bool operator==(const PushStack& other) const { return size == other.size; }
};

// Pops bytes from the stack.
struct PopStack {
  // Number of bytes to pop.
  std::size_t size;

  bool operator==(const PopStack& other) const { return size == other.size; }
};

// Stores the value of a 32-bit register on the stack.
struct StoreStack32 {
  // Offset from the top of the stack where the value will be placed.
  std::size_t offset;

  // Source register.
  RegId src_reg;

  bool operator==(const StoreStack32& other) const {
    return offset == other.offset && src_reg == other.src_reg;
  }
};

// Stores the value of a 64-bit register on the stack.
struct StoreStack64 {
  // Offset from the top of the stack where the value will be placed.
  std::size_t offset;

  // Source register.
  RegId src_reg;

  bool operator==(const StoreStack64& other) const {
    return offset == other.offset && src_reg == other.src_reg;
  }
};

// Loads a value from the stack into a 32-bit register.
struct LoadStack32 {
  // Offset from the top of the stack where the value is placed.
  std::size_t offset;

  // Destination register.
  RegId dst_reg;

  bool operator==(const LoadStack32& other) const {
    return offset == other.offset && dst_reg == other.dst_reg;
  }
};

// Loads a value from the stack into a 64-bit register.
struct LoadStack64 {
  // Offset from the top of the stack where the value is placed.
  std::size_t offset;

  // Destination register.
  RegId dst_reg;

  bool operator==(const LoadStack64& other) const {
    return offset == other.offset && dst_reg == other.dst_reg;
  }
};

// An instruction for the Lucid abstract machine.
using Instruction =
    std::variant<Nop, MoveReg32, MoveReg64, SetReg32, SetReg64, Jump, CondJump,
                 Label, Return, AddReg32, AddReg64, SubReg32, SubReg64,
                 MulReg32, MulReg64, DivReg32, DivReg64, GtReg32, GtReg64,
                 LtReg32, LtReg64, PushStack, PopStack, StoreStack32,
                 StoreStack64, LoadStack32, LoadStack64>;

}  // namespace lucid
