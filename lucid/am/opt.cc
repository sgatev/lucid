#include "lucid/am/opt.h"

#include <cstddef>
#include <variant>
#include <vector>

#include "lucid/am/instructions.h"

namespace lucid {
namespace {

void RemoveUnnecessaryInstructions(std::vector<Instruction>& instructions) {
  for (std::size_t i = 0; i < instructions.size(); ++i) {
    // Remove MoveReg32 where src and dst registrars are the same.
    auto* inst = std::get_if<MoveReg32>(&instructions[i]);
    if (inst == nullptr) continue;
    if (inst->src_reg != inst->dst_reg) continue;
    instructions[i] = Nop{};
  }
}

}  // namespace

void OptimizeAbstractMachineInstructions(
    std::vector<Instruction>& instructions) {
  RemoveUnnecessaryInstructions(instructions);
}

}  // namespace lucid
