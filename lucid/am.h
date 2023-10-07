#pragma once

#include <cstddef>
#include <ostream>
#include <string_view>
#include <variant>
#include <vector>

namespace lucid {

// A register in the Lucid abstract machine.
using RegId = int;

// A no op instruction.
struct Nop {
  bool operator==(const Nop&) const = default;

  friend std::ostream& operator<<(std::ostream& os, const Nop& inst) {
    return os << "{}";
  }
};

// Moves the value of an 32-bit register into another one.
struct MoveReg32 {
  // Source register.
  RegId src_reg;

  // Destination register.
  RegId dst_reg;

  bool operator==(const MoveReg32&) const = default;

  friend std::ostream& operator<<(std::ostream& os, const MoveReg32& inst) {
    return os << "{.src_reg=" << inst.src_reg << ", .dst_reg=" << inst.dst_reg
              << "}";
  }
};

// Moves the value of a 64-bit register into another one.
struct MoveReg64 {
  // Source register.
  RegId src_reg;

  // Destination register.
  RegId dst_reg;

  bool operator==(const MoveReg64&) const = default;

  friend std::ostream& operator<<(std::ostream& os, const MoveReg64& inst) {
    return os << "{.src_reg=" << inst.src_reg << ", .dst_reg=" << inst.dst_reg
              << "}";
  }
};

// Sets a 32-bit value in a register.
struct SetReg32 {
  // Source value.
  std::string_view src_val;

  // Destination register.
  RegId dst_reg;

  bool operator==(const SetReg32&) const = default;

  friend std::ostream& operator<<(std::ostream& os, const SetReg32& inst) {
    return os << "{.src_val=\"" << inst.src_val
              << "\", .dst_reg=" << inst.dst_reg << "}";
  }
};

// Sets 64-bit value in a register.
struct SetReg64 {
  // Source value.
  std::string_view src_val;

  // Destination register.
  RegId dst_reg;

  bool operator==(const SetReg64&) const = default;

  friend std::ostream& operator<<(std::ostream& os, const SetReg64& inst) {
    return os << "{.src_val=\"" << inst.src_val
              << "\", .dst_reg=" << inst.dst_reg << "}";
  }
};

// Jumps to a labeled location.
struct Jump {
  // Label of the location to jump to.
  std::string_view label;

  bool operator==(const Jump&) const = default;

  friend std::ostream& operator<<(std::ostream& os, const Jump& inst) {
    return os << "{.label=\"" << inst.label << "\"}";
  }
};

// Jumps to a labeled location.
struct UncondJump {
  // Label of the location to jump to.
  std::size_t label;

  bool operator==(const UncondJump&) const = default;

  friend std::ostream& operator<<(std::ostream& os, const UncondJump& inst) {
    return os << "{.label=\"" << inst.label << "\"}";
  }
};

// Jumps to a labeled location based on the value of a register.
struct CondJump {
  // Int32 register used as a condition for the jump.
  RegId cond_reg;

  // Label of the location to jump to if the value in the register is not zero.
  std::size_t then_label;

  // Label of the location to jump to if the value in the register is zero.
  std::size_t else_label;

  bool operator==(const CondJump&) const = default;

  friend std::ostream& operator<<(std::ostream& os, const CondJump& inst) {
    return os << "{.cond_reg=" << inst.cond_reg
              << ", .then_label=" << inst.then_label
              << ", .else_label=" << inst.else_label << "}";
  }
};

// A label in the list of instructions.
struct Label {
  // Identifier of the label.
  std::size_t id;

  bool operator==(const Label&) const = default;

  friend std::ostream& operator<<(std::ostream& os, const Label& inst) {
    return os << "{.id=" << inst.id << "}";
  }
};

// Returns to the location before the last jump.
struct Return {
  bool operator==(const Return&) const = default;

  friend std::ostream& operator<<(std::ostream& os, const Return& inst) {
    return os << "{}";
  }
};

// Adds the contents of two 32-bit registers.
struct AddReg32 {
  // Result register.
  RegId res_reg;

  // First operand source register.
  RegId lhs_reg;

  // Second operand source register.
  RegId rhs_reg;

  bool operator==(const AddReg32&) const = default;

  friend std::ostream& operator<<(std::ostream& os, const AddReg32& inst) {
    return os << "{.res_reg=" << inst.res_reg << ", .lhs_reg=" << inst.lhs_reg
              << ", .rhs_reg=" << inst.rhs_reg << "}";
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

  bool operator==(const AddReg64&) const = default;

  friend std::ostream& operator<<(std::ostream& os, const AddReg64& inst) {
    return os << "{.res_reg=" << inst.res_reg << ", .lhs_reg=" << inst.lhs_reg
              << ", .rhs_reg=" << inst.rhs_reg << "}";
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

  bool operator==(const SubReg32&) const = default;

  friend std::ostream& operator<<(std::ostream& os, const SubReg32& inst) {
    return os << "{.res_reg=" << inst.res_reg << ", .lhs_reg=" << inst.lhs_reg
              << ", .rhs_reg=" << inst.rhs_reg << "}";
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

  bool operator==(const SubReg64&) const = default;

  friend std::ostream& operator<<(std::ostream& os, const SubReg64& inst) {
    return os << "{.res_reg=" << inst.res_reg << ", .lhs_reg=" << inst.lhs_reg
              << ", .rhs_reg=" << inst.rhs_reg << "}";
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

  bool operator==(const MulReg32&) const = default;

  friend std::ostream& operator<<(std::ostream& os, const MulReg32& inst) {
    return os << "{.res_reg=" << inst.res_reg << ", .lhs_reg=" << inst.lhs_reg
              << ", .rhs_reg=" << inst.rhs_reg << "}";
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

  bool operator==(const MulReg64&) const = default;

  friend std::ostream& operator<<(std::ostream& os, const MulReg64& inst) {
    return os << "{.res_reg=" << inst.res_reg << ", .lhs_reg=" << inst.lhs_reg
              << ", .rhs_reg=" << inst.rhs_reg << "}";
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

  bool operator==(const DivReg32&) const = default;

  friend std::ostream& operator<<(std::ostream& os, const DivReg32& inst) {
    return os << "{.res_reg=" << inst.res_reg << ", .lhs_reg=" << inst.lhs_reg
              << ", .rhs_reg=" << inst.rhs_reg << "}";
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

  bool operator==(const DivReg64&) const = default;

  friend std::ostream& operator<<(std::ostream& os, const DivReg64& inst) {
    return os << "{.res_reg=" << inst.res_reg << ", .lhs_reg=" << inst.lhs_reg
              << ", .rhs_reg=" << inst.rhs_reg << "}";
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

  bool operator==(const GtReg32&) const = default;

  friend std::ostream& operator<<(std::ostream& os, const GtReg32& inst) {
    return os << "{.res_reg=" << inst.res_reg << ", .lhs_reg=" << inst.lhs_reg
              << ", .rhs_reg=" << inst.rhs_reg << "}";
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

  bool operator==(const GtReg64&) const = default;

  friend std::ostream& operator<<(std::ostream& os, const GtReg64& inst) {
    return os << "{.res_reg=" << inst.res_reg << ", .lhs_reg=" << inst.lhs_reg
              << ", .rhs_reg=" << inst.rhs_reg << "}";
  }
};

// Tests the values in two 32-bit registers for a "less than" relationship.
struct LtReg32 {
  // Result register.
  RegId res_reg;

  // First operand register.
  RegId lhs_reg;

  // Second operand register.
  RegId rhs_reg;

  bool operator==(const LtReg32&) const = default;

  friend std::ostream& operator<<(std::ostream& os, const LtReg32& inst) {
    return os << "{.res_reg=" << inst.res_reg << ", .lhs_reg=" << inst.lhs_reg
              << ", .rhs_reg=" << inst.rhs_reg << "}";
  }
};

// Tests the values in two 64-bit registers for a "less than" relationship.
struct LtReg64 {
  // Result register.
  RegId res_reg;

  // First operand register.
  RegId lhs_reg;

  // Second operand register.
  RegId rhs_reg;

  bool operator==(const LtReg64&) const = default;

  friend std::ostream& operator<<(std::ostream& os, const LtReg64& inst) {
    return os << "{.res_reg=" << inst.res_reg << ", .lhs_reg=" << inst.lhs_reg
              << ", .rhs_reg=" << inst.rhs_reg << "}";
  }
};

// Tests the values in two 32-bit registers for an "equals" relationship.
struct EqReg32 {
  // Result register.
  RegId res_reg;

  // First operand register.
  RegId lhs_reg;

  // Second operand register.
  RegId rhs_reg;

  bool operator==(const EqReg32&) const = default;

  friend std::ostream& operator<<(std::ostream& os, const EqReg32& inst) {
    return os << "{.res_reg=" << inst.res_reg << ", .lhs_reg=" << inst.lhs_reg
              << ", .rhs_reg=" << inst.rhs_reg << "}";
  }
};

// Tests the values in two 64-bit registers for an "equals" relationship.
struct EqReg64 {
  // Result register.
  RegId res_reg;

  // First operand register.
  RegId lhs_reg;

  // Second operand register.
  RegId rhs_reg;

  bool operator==(const EqReg64&) const = default;

  friend std::ostream& operator<<(std::ostream& os, const EqReg64& inst) {
    return os << "{.res_reg=" << inst.res_reg << ", .lhs_reg=" << inst.lhs_reg
              << ", .rhs_reg=" << inst.rhs_reg << "}";
  }
};

// Pushes bytes onto the stack.
struct PushStack {
  bool operator==(const PushStack&) const = default;

  friend std::ostream& operator<<(std::ostream& os, const PushStack& inst) {
    return os << "{}";
  }
};

// Pops bytes from the stack.
struct PopStack {
  bool operator==(const PopStack&) const = default;

  friend std::ostream& operator<<(std::ostream& os, const PopStack& inst) {
    return os << "{}";
  }
};

// Stores the value of a 32-bit register on the stack.
struct StoreStack32 {
  // Offset from the top of the stack where the value will be placed.
  std::size_t offset;

  // Source register.
  RegId src_reg;

  bool operator==(const StoreStack32&) const = default;

  friend std::ostream& operator<<(std::ostream& os, const StoreStack32& inst) {
    return os << "{.offset=" << inst.offset << ", .src_reg=" << inst.src_reg
              << "}";
  }
};

// Stores the value of a 64-bit register on the stack.
struct StoreStack64 {
  // Offset from the top of the stack where the value will be placed.
  std::size_t offset;

  // Source register.
  RegId src_reg;

  bool operator==(const StoreStack64&) const = default;

  friend std::ostream& operator<<(std::ostream& os, const StoreStack64& inst) {
    return os << "{.offset=" << inst.offset << ", .src_reg=" << inst.src_reg
              << "}";
  }
};

// Loads a value from the stack into a 32-bit register.
struct LoadStack32 {
  // Offset from the top of the stack where the value is placed.
  std::size_t offset;

  // Destination register.
  RegId dst_reg;

  bool operator==(const LoadStack32&) const = default;

  friend std::ostream& operator<<(std::ostream& os, const LoadStack32& inst) {
    return os << "{.offset=" << inst.offset << ", .dst_reg=" << inst.dst_reg
              << "}";
  }
};

// Loads a value from the stack into a 64-bit register.
struct LoadStack64 {
  // Offset from the top of the stack where the value is placed.
  std::size_t offset;

  // Destination register.
  RegId dst_reg;

  bool operator==(const LoadStack64&) const = default;

  friend std::ostream& operator<<(std::ostream& os, const LoadStack64& inst) {
    return os << "{.offset=" << inst.offset << ", .dst_reg=" << inst.dst_reg
              << "}";
  }
};

// An instruction for the Lucid abstract machine.
using Instruction = std::variant<
    Nop, MoveReg32, MoveReg64, SetReg32, SetReg64, Jump, UncondJump, CondJump,
    Label, Return, AddReg32, AddReg64, SubReg32, SubReg64, MulReg32, MulReg64,
    DivReg32, DivReg64, GtReg32, GtReg64, LtReg32, LtReg64, EqReg32, EqReg64,
    PushStack, PopStack, StoreStack32, StoreStack64, LoadStack32, LoadStack64>;

// Abstract machine function definition.
struct Function {
  // Name of the function.
  std::string_view name;

  // Abstract machine stack slots.
  std::vector<std::size_t> stack_slots;

  // Abstract machine instructions of the function.
  std::vector<Instruction> instructions;
};

}  // namespace lucid
