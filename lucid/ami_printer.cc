#include "lucid/ami_printer.h"

#include <iostream>
#include <string_view>
#include <variant>
#include <vector>

#include "lucid/am.h"

namespace lucid {

void Print(std::string_view func_name,
           const std::vector<Instruction>& instructions) {
  std::cout << func_name << "\n";
  for (const auto& inst : instructions) {
    std::visit(
        [](const auto& inst) {
          using T = std::decay_t<decltype(inst)>;
          if constexpr (std::is_same_v<T, Nop>) {
            std::cout << "  Nop";
          } else if constexpr (std::is_same_v<T, MoveReg32>) {
            std::cout << "  MoveReg32 { .src_reg = " << inst.src_reg
                      << ", .dst_reg = " << inst.dst_reg << " }";
          } else if constexpr (std::is_same_v<T, MoveReg64>) {
            std::cout << "  MoveReg64";
          } else if constexpr (std::is_same_v<T, SetReg32>) {
            std::cout << "  SetReg32 { .src_val = " << inst.src_val
                      << ", .dst_reg = " << inst.dst_reg << " }";
          } else if constexpr (std::is_same_v<T, SetReg64>) {
            std::cout << "  SetReg64";
          } else if constexpr (std::is_same_v<T, SetStr>) {
            std::cout << "  SetStr";
          } else if constexpr (std::is_same_v<T, Jump>) {
            std::cout << "  Jump { .label = " << inst.label << " }";
          } else if constexpr (std::is_same_v<T, UncondJump>) {
            std::cout << "  UncondJump { .label = " << inst.label << " }";
          } else if constexpr (std::is_same_v<T, CondJump>) {
            std::cout << "  CondJump { .cond_reg = " << inst.cond_reg
                      << " .then_label = " << inst.then_label
                      << ", .else_label = " << inst.else_label << " }";
          } else if constexpr (std::is_same_v<T, Label>) {
            std::cout << "Label { .id = " << inst.id << " }";
          } else if constexpr (std::is_same_v<T, Return>) {
            std::cout << "  Return";
          } else if constexpr (std::is_same_v<T, AddReg32>) {
            std::cout << "  AddReg32 { .res_reg = " << inst.res_reg
                      << ", .lhs_reg = " << inst.lhs_reg
                      << ", .rhs_reg = " << inst.rhs_reg << " }";
          } else if constexpr (std::is_same_v<T, AddReg64>) {
            std::cout << "  AddReg64 { .res_reg = " << inst.res_reg
                      << ", .lhs_reg = " << inst.lhs_reg
                      << ", .rhs_reg = " << inst.rhs_reg << " }";
          } else if constexpr (std::is_same_v<T, SubReg32>) {
            std::cout << "  SubReg32 { .res_reg = " << inst.res_reg
                      << ", .lhs_reg = " << inst.lhs_reg
                      << ", .rhs_reg = " << inst.rhs_reg << " }";
          } else if constexpr (std::is_same_v<T, SubReg64>) {
            std::cout << "  SubReg64 { .res_reg = " << inst.res_reg
                      << ", .lhs_reg = " << inst.lhs_reg
                      << ", .rhs_reg = " << inst.rhs_reg << " }";
          } else if constexpr (std::is_same_v<T, MulReg32>) {
            std::cout << "  MulReg32 { .res_reg = " << inst.res_reg
                      << ", .lhs_reg = " << inst.lhs_reg
                      << ", .rhs_reg = " << inst.rhs_reg << " }";
          } else if constexpr (std::is_same_v<T, MulReg64>) {
            std::cout << "  MulReg64 { .res_reg = " << inst.res_reg
                      << ", .lhs_reg = " << inst.lhs_reg
                      << ", .rhs_reg = " << inst.rhs_reg << " }";
          } else if constexpr (std::is_same_v<T, DivReg32>) {
            std::cout << "  DivReg32 { .res_reg = " << inst.res_reg
                      << ", .lhs_reg = " << inst.lhs_reg
                      << ", .rhs_reg = " << inst.rhs_reg << " }";
          } else if constexpr (std::is_same_v<T, DivReg64>) {
            std::cout << "  DivReg64 { .res_reg = " << inst.res_reg
                      << ", .lhs_reg = " << inst.lhs_reg
                      << ", .rhs_reg = " << inst.rhs_reg << " }";
          } else if constexpr (std::is_same_v<T, ModReg32>) {
            std::cout << "  ModReg32 { .res_reg = " << inst.res_reg
                      << ", .lhs_reg = " << inst.lhs_reg
                      << ", .rhs_reg = " << inst.rhs_reg << " }";
          } else if constexpr (std::is_same_v<T, ModReg64>) {
            std::cout << "  ModReg64 { .res_reg = " << inst.res_reg
                      << ", .lhs_reg = " << inst.lhs_reg
                      << ", .rhs_reg = " << inst.rhs_reg << " }";
          } else if constexpr (std::is_same_v<T, GtReg32>) {
            std::cout << "  GtReg32 { .res_reg = " << inst.res_reg
                      << ", .lhs_reg = " << inst.lhs_reg
                      << ", .rhs_reg = " << inst.rhs_reg << " }";
          } else if constexpr (std::is_same_v<T, GtReg64>) {
            std::cout << "  GtReg64 { .res_reg = " << inst.res_reg
                      << ", .lhs_reg = " << inst.lhs_reg
                      << ", .rhs_reg = " << inst.rhs_reg << " }";
          } else if constexpr (std::is_same_v<T, LtReg32>) {
            std::cout << "  LtReg32 { .res_reg = " << inst.res_reg
                      << ", .lhs_reg = " << inst.lhs_reg
                      << ", .rhs_reg = " << inst.rhs_reg << " }";
          } else if constexpr (std::is_same_v<T, LtReg64>) {
            std::cout << "  LtReg64 { .res_reg = " << inst.res_reg
                      << ", .lhs_reg = " << inst.lhs_reg
                      << ", .rhs_reg = " << inst.rhs_reg << " }";
          } else if constexpr (std::is_same_v<T, EqReg32>) {
            std::cout << "  EqReg32 { .res_reg = " << inst.res_reg
                      << ", .lhs_reg = " << inst.lhs_reg
                      << ", .rhs_reg = " << inst.rhs_reg << " }";
          } else if constexpr (std::is_same_v<T, EqReg64>) {
            std::cout << "  EqReg64 { .res_reg = " << inst.res_reg
                      << ", .lhs_reg = " << inst.lhs_reg
                      << ", .rhs_reg = " << inst.rhs_reg << " }";
          } else if constexpr (std::is_same_v<T, NotEqReg32>) {
            std::cout << "  NotEqReg32 { .res_reg = " << inst.res_reg
                      << ", .lhs_reg = " << inst.lhs_reg
                      << ", .rhs_reg = " << inst.rhs_reg << " }";
          } else if constexpr (std::is_same_v<T, NotEqReg64>) {
            std::cout << "  NotEqReg64 { .res_reg = " << inst.res_reg
                      << ", .lhs_reg = " << inst.lhs_reg
                      << ", .rhs_reg = " << inst.rhs_reg << " }";
          } else if constexpr (std::is_same_v<T, PushStack>) {
            std::cout << "  PushStack";
          } else if constexpr (std::is_same_v<T, PopStack>) {
            std::cout << "  PopStack";
          } else if constexpr (std::is_same_v<T, StoreStack32>) {
            std::cout << "  StoreStack32 { .offset = " << inst.offset
                      << ", .src_reg = " << inst.src_reg << " }";
          } else if constexpr (std::is_same_v<T, StoreStackReg32>) {
            std::cout << "  StoreStackReg32 { .offset = " << inst.offset
                      << ", .offset_reg = " << inst.offset_reg
                      << ", .src_reg = " << inst.src_reg << " }";
          } else if constexpr (std::is_same_v<T, StoreStack64>) {
            std::cout << "  StoreStack64 { .offset = " << inst.offset
                      << ", .src_reg = " << inst.src_reg << " }";
          } else if constexpr (std::is_same_v<T, StoreStackReg64>) {
            std::cout << "  StoreStackReg64 { .offset = " << inst.offset
                      << ", .offset_reg = " << inst.offset_reg
                      << ", .src_reg = " << inst.src_reg << " }";
          } else if constexpr (std::is_same_v<T, LoadStack32>) {
            std::cout << "  LoadStack32 { .offset = " << inst.offset
                      << ", dst_reg = " << inst.dst_reg << " }";
          } else if constexpr (std::is_same_v<T, LoadStackReg32>) {
            std::cout << "  LoadStackReg32 { .offset = " << inst.offset
                      << ", dst_reg = " << inst.dst_reg << " }";
          } else if constexpr (std::is_same_v<T, LoadStack64>) {
            std::cout << "  LoadStack64 { .offset = " << inst.offset
                      << ", dst_reg = " << inst.dst_reg << " }";
          } else if constexpr (std::is_same_v<T, LoadStackReg64>) {
            std::cout << "  LoadStackReg64 { .offset = " << inst.offset
                      << ", dst_reg = " << inst.dst_reg << " }";
          }
        },
        inst);
    std::cout << "\n";
  }
}

}  // namespace lucid
