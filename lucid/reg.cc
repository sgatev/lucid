#include "lucid/reg.h"

#include "lucid/ast.h"
#include "lucid/cfg.h"
#include "lucid/dataflow.h"
#include "lucid/hash_map.h"
#include "lucid/hash_set.h"
#include "lucid/liveness.h"
#include "lucid/string_index.h"

namespace lucid {

HashMap<StringIndex::Ref, HashSet<StringIndex::Ref>> BuildInterferenceGraph(
    const SyntaxContext& ctx, const ControlFlowGraph& cfg) {
  LivenessAnalysis liveness_analysis(ctx);
  std::vector<std::optional<LivenessAnalysis::State>> liveness_block_states =
      RunBackwardDataflow(cfg, liveness_analysis);

  HashMap<StringIndex::Ref, HashSet<StringIndex::Ref>> interference_graph;
  for (const auto& state : liveness_block_states) {
    if (!state.has_value()) continue;

    for (StringIndex::Ref from : state->live_in) {
      interference_graph.Insert(from, {});
      for (StringIndex::Ref to : state->live_in) {
        interference_graph.Find(from)->Insert(to);
      }
    }
  }
  return interference_graph;
}

}  // namespace lucid
