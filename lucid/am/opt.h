#pragma once

#include <list>

#include "lucid/am/instructions.h"

namespace lucid {

// Optimizes `instructions` for efficiency.
void OptimizeAbstractMachineInstructions(std::list<Instruction>& instructions);

}  // namespace lucid
