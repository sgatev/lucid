#pragma once

#include "lucid/am/cfg.h"
#include "lucid/am/instructions.h"
#include "lucid/core/container/hash_map.h"
#include "lucid/core/container/hash_set.h"

namespace lucid {

// Builds an interference graph of the abstract machine program given its
// control flow graph.
HashMap<RegId, HashSet<RegId>> BuildInterferenceGraph(
    const AbstractMachineControlFlowGraph& am_cfg);

}  // namespace lucid
