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
           const AbstractMachineControlFlowGraph& am_cfg) {
  Blue([&] {
    std::cout << func_name << "(";
    for (bool has_printed_param = false; const auto& param : am_cfg.params) {
      if (has_printed_param) std::cout << ", ";
      std::cout << param.reg << "(" << param.bits << ")";
      has_printed_param = true;
    }
    std::cout << ")";
  });
  std::cout << " {\n";
  for (const auto& block : am_cfg.blocks()) {
    Blue([&] { std::cout << "  B" << block.ref.id() << ":\n"; });
    for (const auto& phi : block.phis) {
      std::cout << "    " << "φ(" << phi.target << ") = (";
      for (bool has_printed_source = false; const auto& source : phi.sources) {
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
  std::cout << "}\n";
}

}  // namespace lucid
