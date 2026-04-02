#include "lucid/ami_printer.h"

#include <functional>
#include <iostream>
#include <string_view>
#include <variant>

#include "lucid/am/cfg.h"
#include "lucid/am/instructions.h"

namespace lucid {
namespace {

void Blue(std::function<void()> f) {
  std::cout << "\033[34m";
  std::invoke(f);
  std::cout << "\033[0m";
}

}  // namespace

void Print(std::string_view func_name,
           const AbstractMachineControlFlowGraph& am_cfg) {
  Blue([&] { std::cout << func_name << "\n"; });
  for (const auto& block : am_cfg.blocks()) {
    for (const auto& inst : block.instructions) {
      std::visit(
          [](const auto& inst) {
            using T = std::decay_t<decltype(inst)>;
            if constexpr (std::is_same_v<T, Label>) {
              Blue([&] { std::cout << inst << "\n"; });
            } else {
              std::cout << "  " << inst << "\n";
            }
          },
          inst);
    }
  }
}

}  // namespace lucid
