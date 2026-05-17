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

// Tests the values in two registers for a "greater than" relationship.
struct GtReg {
  // Result register.
  RegId res_reg;

  // First operand register.
  RegId lhs_reg;

  // Second operand register.
  RegId rhs_reg;

  bool operator==(const GtReg&) const = default;

  friend std::ostream& operator<<(std::ostream& os, const GtReg& inst) {
    return os << "GtReg { .res_reg=" << inst.res_reg
              << ", .lhs_reg=" << inst.lhs_reg << ", .rhs_reg=" << inst.rhs_reg
              << " }";
  }
};

// Tests the values in two registers for a "less than" relationship.
struct LtReg {
  // Result register.
  RegId res_reg;

  // First operand register.
  RegId lhs_reg;

  // Second operand register.
  RegId rhs_reg;

  bool operator==(const LtReg&) const = default;

  friend std::ostream& operator<<(std::ostream& os, const LtReg& inst) {
    return os << "LtReg { .res_reg=" << inst.res_reg
              << ", .lhs_reg=" << inst.lhs_reg << ", .rhs_reg=" << inst.rhs_reg
              << " }";
  }
};

// Tests the values in two registers for an "equals" relationship.
struct EqReg {
  // Result register.
  RegId res_reg;

  // First operand register.
  RegId lhs_reg;

  // Second operand register.
  RegId rhs_reg;

  bool operator==(const EqReg&) const = default;

  friend std::ostream& operator<<(std::ostream& os, const EqReg& inst) {
    return os << "EqReg { .res_reg=" << inst.res_reg
              << ", .lhs_reg=" << inst.lhs_reg << ", .rhs_reg=" << inst.rhs_reg
              << " }";
  }
};

// Tests the values in two registers for a "not equals" relationship.
struct NotEqReg {
  // Result register.
  RegId res_reg;

  // First operand register.
  RegId lhs_reg;

  // Second operand register.
  RegId rhs_reg;

  bool operator==(const NotEqReg&) const = default;

  friend std::ostream& operator<<(std::ostream& os, const NotEqReg& inst) {
    return os << "NotEqReg { .res_reg=" << inst.res_reg
              << ", .lhs_reg=" << inst.lhs_reg << ", .rhs_reg=" << inst.rhs_reg
              << " }";
  }
};

// Stores the value of a register on the stack.
struct StoreStack {
  // Offset from the top of the stack where the value will be placed.
  std::size_t offset;

  // Source register.
  RegId src_reg;

  bool operator==(const StoreStack&) const = default;

  friend std::ostream& operator<<(std::ostream& os, const StoreStack& inst) {
    return os << "StoreStack { .offset=" << inst.offset
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

// Loads a value from the stack into a register.
struct LoadStack {
  // Offset from the top of the stack where the value is placed.
  std::size_t offset;

  // Destination register.
  RegId dst_reg;

  bool operator==(const LoadStack&) const = default;

  friend std::ostream& operator<<(std::ostream& os, const LoadStack& inst) {
    return os << "LoadStack { .offset=" << inst.offset
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
    std::variant<Nop, MoveReg, SetReg, SetStr, Return, AddReg, SubReg, MulReg,
                 DivReg, ModReg, GtReg, LtReg, EqReg, NotEqReg, StoreStack,
                 StoreStackReg32, StoreStackReg64, LoadStack, LoadStackReg32,
                 LoadStackReg64, FuncCall>;

// Returns the source registers used by the given instruction, if any.
inline std::vector<RegId> GetSourceRegisters(const Instruction& inst) {
  if (std::holds_alternative<SetReg>(inst) ||
      std::holds_alternative<SetStr>(inst) ||
      std::holds_alternative<LoadStack>(inst)) {
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
  } else if (auto* cinst = std::get_if<GtReg>(&inst)) {
    return {cinst->lhs_reg, cinst->rhs_reg};
  } else if (auto* cinst = std::get_if<LtReg>(&inst)) {
    return {cinst->lhs_reg, cinst->rhs_reg};
  } else if (auto* cinst = std::get_if<EqReg>(&inst)) {
    return {cinst->lhs_reg, cinst->rhs_reg};
  } else if (auto* cinst = std::get_if<NotEqReg>(&inst)) {
    return {cinst->lhs_reg, cinst->rhs_reg};
  } else if (auto* cinst = std::get_if<StoreStack>(&inst)) {
    return {cinst->src_reg};
  } else if (auto* cinst = std::get_if<StoreStackReg32>(&inst)) {
    return {cinst->src_reg, cinst->offset_reg};
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
  } else if (auto* cinst = std::get_if<Return>(&inst)) {
    return {cinst->res_reg};
  } else {
    assert(false && "unhandled instruction type");
  }
  return {};
}

// Returns the target register used by the given instruction, if any.
inline std::optional<RegId> GetTargetRegister(const Instruction& inst) {
  if (std::holds_alternative<StoreStack>(inst) ||
      std::holds_alternative<StoreStackReg32>(inst) ||
      std::holds_alternative<StoreStackReg64>(inst) ||
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
  } else if (auto* cinst = std::get_if<GtReg>(&inst)) {
    return cinst->res_reg;
  } else if (auto* cinst = std::get_if<LtReg>(&inst)) {
    return cinst->res_reg;
  } else if (auto* cinst = std::get_if<EqReg>(&inst)) {
    return cinst->res_reg;
  } else if (auto* cinst = std::get_if<NotEqReg>(&inst)) {
    return cinst->res_reg;
  } else if (auto* cinst = std::get_if<LoadStack>(&inst)) {
    return cinst->dst_reg;
  } else if (auto* cinst = std::get_if<LoadStackReg32>(&inst)) {
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
