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
  };

  // Works out where every variable the function reads is live.
  explicit SyntaxLivenessAnalysis(const SyntaxContext& sctx,
                                  const SyntaxControlFlowGraph& scfg);

  // Works out where the variables in `vars` are live, and takes no other
  // variable to be live anywhere.
  //
  // What is live where is a set for every block, which every join copies, so
  // leaving out the variables no one is going to ask about saves the copying
  // for each of them. `vars` must outlive the analysis.
  SyntaxLivenessAnalysis(const SyntaxContext& sctx,
                         const SyntaxControlFlowGraph& scfg,
                         const HashSet<StringIndex::Ref>& vars);

  State Transfer(std::optional<State>&& prior_state,
                 const SyntaxControlFlowGraph::BlockRef& block_ref);

  void Transfer(State& state, const SyntaxControlFlowGraph::Sequence& seq);

  void Join(State& left, const State& right);

 private:
  // Whether `var` is one of the variables this analysis works out.
  bool Tracks(StringIndex::Ref var) const {
    return vars_ == nullptr || vars_->Contains(var);
  }

  const SyntaxContext& sctx_;
  const SyntaxControlFlowGraph& scfg_;
  // The variables worked out, or null for all of them.
  const HashSet<StringIndex::Ref>* vars_ = nullptr;
};

}  // namespace lucid
