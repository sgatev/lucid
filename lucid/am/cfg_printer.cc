#include "lucid/am/cfg_printer.h"

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
           const AbstractMachineControlFlowGraph& amcfg) {
  Blue([&] {
    std::cout << func_name << "(";
    for (bool has_printed_param = false; const auto& param : amcfg.params) {
      if (has_printed_param) std::cout << ", ";
      std::cout << param;
      has_printed_param = true;
    }
    std::cout << "):\n";
  });
  for (const auto& block : amcfg.blocks()) {
    Blue([&] { std::cout << "  B" << block.ref.id() << ":\n"; });
    for (const auto& phi : block.phis) {
      std::cout << "    " << "φ(" << phi.dst << ") = (";
      for (bool has_printed_source = false; const auto& source : phi.srcs) {
        if (has_printed_source) std::cout << ", ";
        std::cout << source;
        has_printed_source = true;
      }
      std::cout << ")" << "\n";
    }
    for (const auto& inst : block.instructions) {
      std::visit(
          [](const auto& inst) {
            using T = std::decay_t<decltype(inst)>;
            if constexpr (std::is_same_v<T, Label>) {
            } else {
              std::cout << "    " << inst << "\n";
            }
          },
          inst);
    }
  }
}

}  // namespace lucid
