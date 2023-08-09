#pragma once

#include <vector>

#include "lucid/am.h"

namespace lucid {

// Optimizes `instructions` for efficiency.
void OptimizeAbstractMachineInstructions(
    std::vector<Instruction>& instructions);

}  // namespace lucid
