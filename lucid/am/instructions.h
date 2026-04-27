#pragma once

#include <cassert>
#include <cstddef>
#include <cstdint>
#include <optional>
#include <ostream>
#include <string_view>
#include <variant>
#include <vector>

#include "lucid/core/hash/hash.h"

namespace lucid {

enum RegSize { RegSize32, RegSize64 };

// A register in the Lucid abstract machine.
struct RegId {
  std::int32_t id;
  RegSize size;

  bool operator==(const RegId&) const = default;

  friend std::ostream& operator<<(std::ostream& os, const RegId& reg) {
    os << reg.id << "(";
    switch (reg.size) {
      case RegSize32:
        os << "32";
        break;
      case RegSize64:
        os << "64";
        break;
    }
    os << ")";
    return os;
  }
};

inline std::size_t Hash(const RegId& reg) { return Hash(reg.id); }

// A no op instruction.
struct Nop {
  bool operator==(const Nop&) const = default;

  friend std::ostream& operator<<(std::ostream& os, const Nop& inst) {
    return os << "Nop {}";
  }
};

// Moves the value of a register into another one.
struct MoveReg {
  // Source register.
  RegId src_reg;

  // Destination register.
  RegId dst_reg;

  bool operator==(const MoveReg&) const = default;

  friend std::ostream& operator<<(std::ostream& os, const MoveReg& inst) {
    return os << "MoveReg { .src_reg=" << inst.src_reg
              << ", .dst_reg=" << inst.dst_reg << " }";
  }
};

// Sets a value in a register.
struct SetReg {
  // Source value.
  std::string_view src_val;

  // Destination register.
  RegId dst_reg;

  bool operator==(const SetReg&) const = default;

  friend std::ostream& operator<<(std::ostream& os, const SetReg& inst) {
    return os << "SetReg { .src_val=\"" << inst.src_val
              << "\", .dst_reg=" << inst.dst_reg << " }";
  }
};

// Sets string value in a register.
struct SetStr {
  // Source value.
  std::uintptr_t src_val;

  // Destination register.
  RegId dst_reg;

  bool operator==(const SetStr&) const = default;

  friend std::ostream& operator<<(std::ostream& os, const SetStr& inst) {
    return os << "SetStr { .src_val=\"" << inst.src_val
              << "\", .dst_reg=" << inst.dst_reg << " }";
  }
};

// Jumps to a labeled location.
struct Jump {
  // Label of the location to jump to.
  std::string_view label;

  bool operator==(const Jump&) const = default;

  friend std::ostream& operator<<(std::ostream& os, const Jump& inst) {
    return os << "Jump { .label=\"" << inst.label << "\" }";
  }
};

// Jumps to a labeled location.
struct UncondJump {
  // Label of the location to jump to.
  std::size_t label;

  bool operator==(const UncondJump&) const = default;

  friend std::ostream& operator<<(std::ostream& os, const UncondJump& inst) {
    return os << "UncondJump { .label=\"" << inst.label << "\" }";
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
    return os << "CondJump { .cond_reg=" << inst.cond_reg
              << ", .then_label=" << inst.then_label
              << ", .else_label=" << inst.else_label << " }";
  }
};

// A label in the list of instructions.
struct Label {
  // Identifier of the label.
  std::size_t id;

  bool operator==(const Label&) const = default;

  friend std::ostream& operator<<(std::ostream& os, const Label& inst) {
    return os << "Label { .id=" << inst.id << " }";
  }
};

// Returns to the location before the last jump.
struct Return {
  // Result register.
  RegId res_reg;

  bool operator==(const Return&) const = default;

  friend std::ostream& operator<<(std::ostream& os, const Return& inst) {
    return os << "Return { .res_reg=" << inst.res_reg << " }";
  }
};

// Adds the contents of two registers.
struct AddReg {
  // Result register.
  RegId res_reg;

  // First operand source register.
  RegId lhs_reg;

  // Second operand source register.
  RegId rhs_reg;

  bool operator==(const AddReg&) const = default;

  friend std::ostream& operator<<(std::ostream& os, const AddReg& inst) {
    return os << "AddReg { .res_reg=" << inst.res_reg
              << ", .lhs_reg=" << inst.lhs_reg << ", .rhs_reg=" << inst.rhs_reg
              << " }";
  }
};

// Subtracts the contents of one register from another.
struct SubReg {
  // Result register.
  RegId res_reg;

  // First operand register.
  RegId lhs_reg;

  // Second operand register.
  RegId rhs_reg;

  bool operator==(const SubReg&) const = default;

  friend std::ostream& operator<<(std::ostream& os, const SubReg& inst) {
    return os << "SubReg { .res_reg=" << inst.res_reg
              << ", .lhs_reg=" << inst.lhs_reg << ", .rhs_reg=" << inst.rhs_reg
              << " }";
  }
};

// Multiplies the contents of two registers.
struct MulReg {
  // Result register.
  RegId res_reg;

  // First operand register.
  RegId lhs_reg;

  // Second operand register.
  RegId rhs_reg;

  bool operator==(const MulReg&) const = default;

  friend std::ostream& operator<<(std::ostream& os, const MulReg& inst) {
    return os << "MulReg { .res_reg=" << inst.res_reg
              << ", .lhs_reg=" << inst.lhs_reg << ", .rhs_reg=" << inst.rhs_reg
              << " }";
  }
};

// Divides the contents of one register by another.
struct DivReg {
  // Result register.
  RegId res_reg;

  // First operand register.
  RegId lhs_reg;

  // Second operand register.
  RegId rhs_reg;

  bool operator==(const DivReg&) const = default;

  friend std::ostream& operator<<(std::ostream& os, const DivReg& inst) {
    return os << "DivReg { .res_reg=" << inst.res_reg
              << ", .lhs_reg=" << inst.lhs_reg << ", .rhs_reg=" << inst.rhs_reg
              << " }";
  }
};

// Computes the remainder after dividing the contents of one register by
// another.
struct ModReg {
  // Result register.
  RegId res_reg;

  // First operand register.
  RegId lhs_reg;

  // Second operand register.
  RegId rhs_reg;

  bool operator==(const ModReg&) const = default;

  friend std::ostream& operator<<(std::ostream& os, const ModReg& inst) {
    return os << "ModReg { .res_reg=" << inst.res_reg
              << ", .lhs_reg=" << inst.lhs_reg << ", .rhs_reg=" << inst.rhs_reg
              << " }";
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
    return os << "GtReg32 { .res_reg=" << inst.res_reg
              << ", .lhs_reg=" << inst.lhs_reg << ", .rhs_reg=" << inst.rhs_reg
              << " }";
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
    return os << "GtReg64 { .res_reg=" << inst.res_reg
              << ", .lhs_reg=" << inst.lhs_reg << ", .rhs_reg=" << inst.rhs_reg
              << " }";
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
    return os << "LtReg32 { .res_reg=" << inst.res_reg
              << ", .lhs_reg=" << inst.lhs_reg << ", .rhs_reg=" << inst.rhs_reg
              << " }";
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
    return os << "LtReg64 { .res_reg=" << inst.res_reg
              << ", .lhs_reg=" << inst.lhs_reg << ", .rhs_reg=" << inst.rhs_reg
              << " }";
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
    return os << "EqReg32 { .res_reg=" << inst.res_reg
              << ", .lhs_reg=" << inst.lhs_reg << ", .rhs_reg=" << inst.rhs_reg
              << " }";
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
    return os << "EqReg64 { .res_reg=" << inst.res_reg
              << ", .lhs_reg=" << inst.lhs_reg << ", .rhs_reg=" << inst.rhs_reg
              << " }";
  }
};

// Tests the values in two 32-bit registers for a "not equals" relationship.
struct NotEqReg32 {
  // Result register.
  RegId res_reg;

  // First operand register.
  RegId lhs_reg;

  // Second operand register.
  RegId rhs_reg;

  bool operator==(const NotEqReg32&) const = default;

  friend std::ostream& operator<<(std::ostream& os, const NotEqReg32& inst) {
    return os << "NotEqReg32 { .res_reg=" << inst.res_reg
              << ", .lhs_reg=" << inst.lhs_reg << ", .rhs_reg=" << inst.rhs_reg
              << " }";
  }
};

// Tests the values in two 64-bit registers for a "not equals" relationship.
struct NotEqReg64 {
  // Result register.
  RegId res_reg;

  // First operand register.
  RegId lhs_reg;

  // Second operand register.
  RegId rhs_reg;

  bool operator==(const NotEqReg64&) const = default;

  friend std::ostream& operator<<(std::ostream& os, const NotEqReg64& inst) {
    return os << "NotEqReg64 { .res_reg=" << inst.res_reg
              << ", .lhs_reg=" << inst.lhs_reg << ", .rhs_reg=" << inst.rhs_reg
              << " }";
  }
};

// Pushes bytes onto the stack.
struct PushStack {
  bool operator==(const PushStack&) const = default;

  friend std::ostream& operator<<(std::ostream& os, const PushStack& inst) {
    return os << "PushStack {}";
  }
};

// Pops bytes from the stack.
struct PopStack {
  bool operator==(const PopStack&) const = default;

  friend std::ostream& operator<<(std::ostream& os, const PopStack& inst) {
    return os << "PopStack {}";
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
    return os << "StoreStack32 { .offset=" << inst.offset
              << ", .src_reg=" << inst.src_reg << " }";
  }
};

// Stores the value of a 32-bit register on the stack.
struct StoreStackReg32 {
  // Initial offset from the top of the stack.
  std::size_t offset;

  // 32-bit register whose value is added to `offset` to reach the destination
  // address.
  RegId offset_reg;

  // Source register.
  RegId src_reg;

  bool operator==(const StoreStackReg32&) const = default;

  friend std::ostream& operator<<(std::ostream& os,
                                  const StoreStackReg32& inst) {
    return os << "StoreStackReg32 { .offset=" << inst.offset
              << ", .offset_reg=" << inst.offset_reg
              << ", .src_reg=" << inst.src_reg << " }";
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
    return os << "StoreStack64 { .offset=" << inst.offset
              << ", .src_reg=" << inst.src_reg << " }";
  }
};

// Stores the value of a 64-bit register on the stack.
struct StoreStackReg64 {
  // Initial offset from the top of the stack.
  std::size_t offset;

  // 64-bit register whose value is added to `offset` to reach the destination
  // address.
  RegId offset_reg;

  // Source register.
  RegId src_reg;

  bool operator==(const StoreStackReg64&) const = default;

  friend std::ostream& operator<<(std::ostream& os,
                                  const StoreStackReg64& inst) {
    return os << "StoreStackReg64 { .offset=" << inst.offset
              << ", .offset_reg=" << inst.offset_reg
              << ", .src_reg=" << inst.src_reg << " }";
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
    return os << "LoadStack32 { .offset=" << inst.offset
              << ", .dst_reg=" << inst.dst_reg << " }";
  }
};

// Loads a value from the stack into a 32-bit register.
struct LoadStackReg32 {
  // Initial offset from the top of the stack.
  std::size_t offset;

  // 32-bit register whose value is added to `offset` to reach the address of
  // the value.
  RegId offset_reg;

  // Destination register.
  RegId dst_reg;

  bool operator==(const LoadStackReg32&) const = default;

  friend std::ostream& operator<<(std::ostream& os,
                                  const LoadStackReg32& inst) {
    return os << "LoadStackReg32 { .offset=" << inst.offset
              << ", .offset_reg=" << inst.offset_reg
              << ", .dst_reg=" << inst.dst_reg << " }";
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
    return os << "LoadStack64 { .offset=" << inst.offset
              << ", .dst_reg=" << inst.dst_reg << " }";
  }
};

// Loads a value from the stack into a 64-bit register.
struct LoadStackReg64 {
  // Initial offset from the top of the stack.
  std::size_t offset;

  // 64-bit register whose value is added to `offset` to reach the address of
  // the value.
  RegId offset_reg;

  // Destination register.
  RegId dst_reg;

  bool operator==(const LoadStackReg64&) const = default;

  friend std::ostream& operator<<(std::ostream& os,
                                  const LoadStackReg64& inst) {
    return os << "LoadStackReg64 { .offset=" << inst.offset
              << ", .offset_reg=" << inst.offset_reg
              << ", .dst_reg=" << inst.dst_reg << " }";
  }
};

// Calls a function.
struct FuncCall {
  // Label of the function to call.
  std::string_view label;

  struct Slot {
    RegId reg;

    bool operator==(const Slot&) const = default;
  };

  // Arguments to pass to the function.
  std::vector<Slot> args;

  // Result from the function.
  std::optional<Slot> res;

  bool operator==(const FuncCall&) const = default;

  friend std::ostream& operator<<(std::ostream& os, const FuncCall& inst) {
    os << "FuncCall { .label=\"" << inst.label << "\" .args = [";
    for (bool has_printed_arg = false; const auto& arg : inst.args) {
      if (has_printed_arg) os << ", ";
      os << arg.reg;
      has_printed_arg = true;
    }
    os << "]";
    if (inst.res.has_value()) os << " .res=" << inst.res->reg;
    os << " }";
    return os;
  }
};

// An instruction for the Lucid abstract machine.
using Instruction =
    std::variant<Nop, MoveReg, SetReg, SetStr, Jump, UncondJump, CondJump,
                 Label, Return, AddReg, SubReg, MulReg, DivReg, ModReg, GtReg32,
                 GtReg64, LtReg32, LtReg64, EqReg32, EqReg64, NotEqReg32,
                 NotEqReg64, PushStack, PopStack, StoreStack32, StoreStackReg32,
                 StoreStack64, StoreStackReg64, LoadStack32, LoadStackReg32,
                 LoadStack64, LoadStackReg64, FuncCall>;

// Returns the source registers used by the given instruction, if any.
inline std::vector<RegId> GetSourceRegisters(const Instruction& inst) {
  if (std::holds_alternative<PushStack>(inst) ||
      std::holds_alternative<PopStack>(inst) ||
      std::holds_alternative<Label>(inst) ||
      std::holds_alternative<Jump>(inst) ||
      std::holds_alternative<UncondJump>(inst) ||
      std::holds_alternative<SetReg>(inst) ||
      std::holds_alternative<SetStr>(inst) ||
      std::holds_alternative<LoadStack32>(inst) ||
      std::holds_alternative<LoadStack64>(inst)) {
    return {};
  } else if (auto* cinst = std::get_if<MoveReg>(&inst)) {
    return {cinst->src_reg};
  } else if (auto* cinst = std::get_if<AddReg>(&inst)) {
    return {cinst->lhs_reg, cinst->rhs_reg};
  } else if (auto* cinst = std::get_if<SubReg>(&inst)) {
    return {cinst->lhs_reg, cinst->rhs_reg};
  } else if (auto* cinst = std::get_if<MulReg>(&inst)) {
    return {cinst->lhs_reg, cinst->rhs_reg};
  } else if (auto* cinst = std::get_if<DivReg>(&inst)) {
    return {cinst->lhs_reg, cinst->rhs_reg};
  } else if (auto* cinst = std::get_if<ModReg>(&inst)) {
    return {cinst->lhs_reg, cinst->rhs_reg};
  } else if (auto* cinst = std::get_if<GtReg32>(&inst)) {
    return {cinst->lhs_reg, cinst->rhs_reg};
  } else if (auto* cinst = std::get_if<GtReg64>(&inst)) {
    return {cinst->lhs_reg, cinst->rhs_reg};
  } else if (auto* cinst = std::get_if<LtReg32>(&inst)) {
    return {cinst->lhs_reg, cinst->rhs_reg};
  } else if (auto* cinst = std::get_if<LtReg64>(&inst)) {
    return {cinst->lhs_reg, cinst->rhs_reg};
  } else if (auto* cinst = std::get_if<EqReg32>(&inst)) {
    return {cinst->lhs_reg, cinst->rhs_reg};
  } else if (auto* cinst = std::get_if<EqReg64>(&inst)) {
    return {cinst->lhs_reg, cinst->rhs_reg};
  } else if (auto* cinst = std::get_if<NotEqReg32>(&inst)) {
    return {cinst->lhs_reg, cinst->rhs_reg};
  } else if (auto* cinst = std::get_if<NotEqReg64>(&inst)) {
    return {cinst->lhs_reg, cinst->rhs_reg};
  } else if (auto* cinst = std::get_if<StoreStack32>(&inst)) {
    return {cinst->src_reg};
  } else if (auto* cinst = std::get_if<StoreStackReg32>(&inst)) {
    return {cinst->src_reg, cinst->offset_reg};
  } else if (auto* cinst = std::get_if<StoreStack64>(&inst)) {
    return {cinst->src_reg};
  } else if (auto* cinst = std::get_if<StoreStackReg64>(&inst)) {
    return {cinst->src_reg, cinst->offset_reg};
  } else if (auto* cinst = std::get_if<LoadStackReg32>(&inst)) {
    return {cinst->offset_reg};
  } else if (auto* cinst = std::get_if<LoadStackReg64>(&inst)) {
    return {cinst->offset_reg};
  } else if (auto* cinst = std::get_if<FuncCall>(&inst)) {
    std::vector<RegId> source_regs;
    for (const auto& arg : cinst->args) source_regs.push_back(arg.reg);
    return source_regs;
  } else if (auto* cinst = std::get_if<CondJump>(&inst)) {
    return {cinst->cond_reg};
  } else if (auto* cinst = std::get_if<Return>(&inst)) {
    return {cinst->res_reg};
  } else {
    assert(false && "unhandled instruction type");
  }
  return {};
}

// Returns the target register used by the given instruction, if any.
inline std::optional<RegId> GetTargetRegister(const Instruction& inst) {
  if (std::holds_alternative<PushStack>(inst) ||
      std::holds_alternative<PopStack>(inst) ||
      std::holds_alternative<Label>(inst) ||
      std::holds_alternative<Jump>(inst) ||
      std::holds_alternative<UncondJump>(inst) ||
      std::holds_alternative<StoreStack32>(inst) ||
      std::holds_alternative<StoreStackReg32>(inst) ||
      std::holds_alternative<StoreStack64>(inst) ||
      std::holds_alternative<StoreStackReg64>(inst) ||
      std::holds_alternative<CondJump>(inst) ||
      std::holds_alternative<Return>(inst)) {
    return std::nullopt;
  } else if (auto* cinst = std::get_if<MoveReg>(&inst)) {
    return cinst->dst_reg;
  } else if (auto* cinst = std::get_if<SetReg>(&inst)) {
    return cinst->dst_reg;
  } else if (auto* cinst = std::get_if<SetStr>(&inst)) {
    return cinst->dst_reg;
  } else if (auto* cinst = std::get_if<AddReg>(&inst)) {
    return cinst->res_reg;
  } else if (auto* cinst = std::get_if<SubReg>(&inst)) {
    return cinst->res_reg;
  } else if (auto* cinst = std::get_if<MulReg>(&inst)) {
    return cinst->res_reg;
  } else if (auto* cinst = std::get_if<DivReg>(&inst)) {
    return cinst->res_reg;
  } else if (auto* cinst = std::get_if<ModReg>(&inst)) {
    return cinst->res_reg;
  } else if (auto* cinst = std::get_if<GtReg32>(&inst)) {
    return cinst->res_reg;
  } else if (auto* cinst = std::get_if<GtReg64>(&inst)) {
    return cinst->res_reg;
  } else if (auto* cinst = std::get_if<LtReg32>(&inst)) {
    return cinst->res_reg;
  } else if (auto* cinst = std::get_if<LtReg64>(&inst)) {
    return cinst->res_reg;
  } else if (auto* cinst = std::get_if<EqReg32>(&inst)) {
    return cinst->res_reg;
  } else if (auto* cinst = std::get_if<EqReg64>(&inst)) {
    return cinst->res_reg;
  } else if (auto* cinst = std::get_if<NotEqReg32>(&inst)) {
    return cinst->res_reg;
  } else if (auto* cinst = std::get_if<NotEqReg64>(&inst)) {
    return cinst->res_reg;
  } else if (auto* cinst = std::get_if<LoadStack32>(&inst)) {
    return cinst->dst_reg;
  } else if (auto* cinst = std::get_if<LoadStackReg32>(&inst)) {
    return cinst->dst_reg;
  } else if (auto* cinst = std::get_if<LoadStack64>(&inst)) {
    return cinst->dst_reg;
  } else if (auto* cinst = std::get_if<LoadStackReg64>(&inst)) {
    return cinst->dst_reg;
  } else if (auto* cinst = std::get_if<FuncCall>(&inst)) {
    if (cinst->res.has_value()) return cinst->res->reg;
    return std::nullopt;
  } else {
    assert(false && "unhandled instruction type");
  }
  return std::nullopt;
}

}  // namespace lucid
