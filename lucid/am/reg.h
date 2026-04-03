#pragma once

#include <cstddef>
#include <vector>

#include "lucid/am/cfg.h"
#include "lucid/am/instructions.h"
#include "lucid/core/container/hash_map.h"
#include "lucid/core/container/hash_set.h"

namespace lucid {

void IntroduceSpilling(AbstractMachineControlFlowGraph& am_cfg,
                       std::vector<std::size_t>& stack_slots);

HashMap<RegId, HashSet<RegId>> BuildInterferenceGraph(
    const AbstractMachineControlFlowGraph& am_cfg);

HashMap<RegId, int> ColorInterferenceGraph(
    const AbstractMachineControlFlowGraph& am_cfg,
    const HashMap<RegId, HashSet<RegId>>& ig, int colors_count);

void MergeRegisters(const HashMap<RegId, int>& reg_colors,
                    AbstractMachineControlFlowGraph& am_cfg);

}  // namespace lucid
