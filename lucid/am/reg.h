#pragma once

#include <cstddef>

#include "lucid/am/cfg.h"
#include "lucid/am/instructions.h"
#include "lucid/am/state.h"
#include "lucid/core/container/hash_map.h"
#include "lucid/core/container/hash_set.h"

namespace lucid {

void SpillRegisters(AbstractMachineControlFlowGraph& am_cfg,
                    AbstractMachineState& am_state, int max_clique_size);

HashMap<Reg, int> ColorInterferenceGraph(
    const AbstractMachineControlFlowGraph& am_cfg,
    const HashMap<Reg, HashSet<Reg>>& am_ig, int colors_count);

void MergeRegisters(const HashMap<Reg, int>& reg_colors,
                    AbstractMachineControlFlowGraph& am_cfg);

}  // namespace lucid
