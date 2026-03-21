#pragma once

#include "lucid/am.h"
#include "lucid/am_cfg.h"
#include "lucid/hash_map.h"
#include "lucid/hash_set.h"

namespace lucid {

HashMap<RegId, HashSet<RegId>> BuildInterferenceGraph(
    const AbstractMachineControlFlowGraph& am_cfg);

HashMap<RegId, int> ColorInterferenceGraph(
    const AbstractMachineControlFlowGraph& am_cfg,
    const HashMap<RegId, HashSet<RegId>>& ig, int colors_count);

}  // namespace lucid
