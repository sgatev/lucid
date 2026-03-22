#include "lucid/ami_printer.h"

#include <functional>
#include <iostream>
#include <string_view>
#include <variant>
#include <vector>

#include "lucid/am.h"

namespace lucid {
namespace {

void Blue(std::function<void()> f) {
  std::cout << "\033[34m";
  std::invoke(f);
  std::cout << "\033[0m";
}

}  // namespace

void Print(std::string_view func_name,
           const std::vector<Instruction>& instructions) {
  Blue([&] { std::cout << func_name << "\n"; });
  for (const auto& inst : instructions) {
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

}  // namespace lucid
