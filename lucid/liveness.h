#pragma once

#include "lucid/ast.h"
#include "lucid/cfg.h"
#include "lucid/hash_set.h"
#include "lucid/string_index.h"

namespace lucid {

// Liveness dataflow analysis.
class LivenessAnalysis {
 public:
  struct State {
    bool operator==(const State&) const = default;

    // References of variables that are live before entering the block modeled
    // by this state.
    HashSet<StringIndex::Ref> live_in;

    // References of variables that are live after exiting the block modeled by
    // this state.
    HashSet<StringIndex::Ref> live_out;
  };

  explicit LivenessAnalysis(const SyntaxContext& ctx);

  State MakeInitial();

  State Transfer(State state, const ControlFlowGraph::Sequence& seq);

  State Join(State left, State right);

 private:
  const SyntaxContext& ctx_;
};

}  // namespace lucid
