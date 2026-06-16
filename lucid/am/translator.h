#pragma once

#include <string_view>

#include "lucid/am/cfg.h"
#include "lucid/am/state.h"
#include "lucid/core/container/hash_map.h"
#include "lucid/syntax/cfg.h"

namespace lucid {

// Generates abstract machine instructions for `syn_cfg`.
//
// The generated instructions are stored in `am_state`.
//
// Requires:
// - `syn_cfg` must be constructed in `syn_ctx`.
AbstractMachineControlFlowGraph GenerateAbstractMachineFunction(
    const HashMap<std::string_view, AbstractMachineControlFlowGraph>& am_cfgs,
    const SyntaxContext& syn_ctx, const SyntaxControlFlowGraph& syn_cfg,
    AbstractMachineState& am_state);

}  // namespace lucid
