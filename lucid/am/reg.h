#pragma once

#include <cstddef>

#include "lucid/am/cfg.h"
#include "lucid/am/instructions.h"
#include "lucid/am/translator.h"
#include "lucid/core/container/hash_map.h"
#include "lucid/core/container/hash_set.h"

namespace lucid {

void SpillRegisters(AbstractMachineControlFlowGraph& am_cfg,
                    AbstractMachineState& am_state);

HashMap<RegId, int> ColorInterferenceGraph(
    const AbstractMachineControlFlowGraph& am_cfg,
    const HashMap<RegId, HashSet<RegId>>& am_ig, int colors_count);

void MergeRegisters(const HashMap<RegId, int>& reg_colors,
                    AbstractMachineControlFlowGraph& am_cfg);

}  // namespace lucid
