#pragma once

#include <list>

#include "lucid/am/cfg.h"
#include "lucid/am/instructions.h"

namespace lucid {

// Optimizes `instructions` for efficiency.
void OptimizeAbstractMachineInstructions(std::list<Instruction>& instructions);

// Optimizes `am_cfg` for efficiency, and records what a backend can only
// learn here.
//
// Called before the registers are given their colours, which is what lets it
// see one value per register.
void OptimizeAbstractMachineFunction(AbstractMachineControlFlowGraph& am_cfg);

}  // namespace lucid
