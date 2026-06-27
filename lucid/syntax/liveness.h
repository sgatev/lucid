#pragma once

#include <optional>

#include "lucid/core/container/hash_set.h"
#include "lucid/core/string/index.h"
#include "lucid/syntax/cfg.h"
#include "lucid/syntax/context.h"

namespace lucid {

// Liveness dataflow analysis over a syntax control flow graph.
class SyntaxLivenessAnalysis {
 public:
  struct State {
    bool operator==(const State&) const = default;

    // Registers that are live before entering the block modeled by this state.
    HashSet<StringIndex::Ref> live_in;

    // Registers that are live after exiting the block modeled by this state.
    HashSet<StringIndex::Ref> live_out;
  };

  explicit SyntaxLivenessAnalysis(const SyntaxContext& sctx,
                                  const SyntaxControlFlowGraph& scfg);

  State Transfer(std::optional<State>&& prior_state,
                 const SyntaxControlFlowGraph::BlockRef& block_ref);

  State Transfer(State&& state, const SyntaxControlFlowGraph::Sequence& seq);

  State Join(State&& left, const State& right);

 private:
  const SyntaxContext& sctx_;
  const SyntaxControlFlowGraph& scfg_;
};

}  // namespace lucid
