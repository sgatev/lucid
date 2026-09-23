#pragma once

#include <cstddef>

#include "lucid/am/cfg.h"
#include "lucid/am/ig.h"
#include "lucid/am/instructions.h"
#include "lucid/am/liveness.h"
#include "lucid/am/state.h"
#include "lucid/core/container/hash_map.h"
#include "lucid/core/container/hash_set.h"

namespace lucid {

// Spills registers until no more than `max_clique_size` of them are live at
// any one point, and returns what the last analysis of the graph settled on.
//
// Deciding what to spill is mostly a matter of working out what is live
// where, and the answer that ends it is the answer for the graph as it is
// left, which is what an interference graph over it is built from.
AbstractMachineLiveness SpillRegisters(AbstractMachineControlFlowGraph& am_cfg,
                                       AbstractMachineState& am_state,
                                       int max_clique_size);

HashMap<Reg, int> ColorInterferenceGraph(
    const AbstractMachineControlFlowGraph& am_cfg,
    const InterferenceGraph& am_ig, int colors_count);

void MergeRegisters(const HashMap<Reg, int>& reg_colors,
                    AbstractMachineControlFlowGraph& am_cfg);

}  // namespace lucid
