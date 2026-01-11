#pragma once

#include "lucid/ast.h"
#include "lucid/cfg.h"
#include "lucid/hash_map.h"
#include "lucid/hash_set.h"
#include "lucid/string_index.h"

namespace lucid {

HashMap<StringIndex::Ref, HashSet<StringIndex::Ref>> BuildInterferenceGraph(
    const SyntaxContext& ctx, const ControlFlowGraph& cfg);

HashMap<StringIndex::Ref, int> ColorInterferenceGraph(
    const SyntaxContext& ctx, const ControlFlowGraph& cfg,
    const HashMap<StringIndex::Ref, HashSet<StringIndex::Ref>>& ig,
    int colors_count);

}  // namespace lucid
