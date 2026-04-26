#include "lucid/am/opt.h"

#include <cstddef>
#include <list>
#include <variant>

#include "lucid/am/instructions.h"

namespace lucid {
namespace {

void RemoveUnnecessaryInstructions(std::list<Instruction>& instructions) {
  for (auto& inst : instructions) {
    // Remove MoveReg where src and dst registrars are the same.
    auto* cinst = std::get_if<MoveReg>(&inst);
    if (cinst == nullptr) continue;
    if (cinst->src_reg != cinst->dst_reg) continue;
    inst = Nop{};
  }
}

}  // namespace

void OptimizeAbstractMachineInstructions(std::list<Instruction>& instructions) {
  RemoveUnnecessaryInstructions(instructions);
}

}  // namespace lucid
