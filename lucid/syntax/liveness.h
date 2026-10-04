#pragma once

#include <cstdint>
#include <optional>
#include <vector>

#include "lucid/core/container/hash_map.h"
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

    // Variables that are live before entering the block modeled by this
    // state, as a bit for each variable the analysis works out, in the order
    // it numbered them. Joining two states is then a pass over a few words
    // rather than an insert for every variable.
    std::vector<std::uint64_t> live_in;
  };

  // Works out where every variable the function reads is live.
  explicit SyntaxLivenessAnalysis(const SyntaxContext& sctx,
                                  const SyntaxControlFlowGraph& scfg);

  // Works out where the variables in `vars` are live, and takes no other
  // variable to be live anywhere.
  //
  // Every variable worked out is a bit in the state of every block, so
  // leaving out the variables no one is going to ask about keeps the states
  // small.
  SyntaxLivenessAnalysis(const SyntaxContext& sctx,
                         const SyntaxControlFlowGraph& scfg,
                         const HashSet<StringIndex::Ref>& vars);

  State Transfer(std::optional<State>&& prior_state,
                 const SyntaxControlFlowGraph::BlockRef& block_ref);

  void Transfer(State& state, const SyntaxControlFlowGraph::Sequence& seq);

  void Join(State& left, const State& right);

  // Returns whether `var` is live where `state` says.
  bool IsLiveIn(const State& state, StringIndex::Ref var) const;

  // Returns the variables live where `state` says, in the order the analysis
  // numbered them.
  std::vector<StringIndex::Ref> LiveIn(const State& state) const;

 private:
  static constexpr std::size_t kWordBits = 64;

  // Numbers `var` as one of the variables this analysis works out, unless it
  // already is.
  void Track(StringIndex::Ref var);

  // Marks `var` live in `state`, if it is one of the variables worked out.
  void Insert(State& state, StringIndex::Ref var) const;

  // Marks `var` dead in `state`, if it is one of the variables worked out.
  void Remove(State& state, StringIndex::Ref var) const;

  const SyntaxContext& sctx_;
  const SyntaxControlFlowGraph& scfg_;
  // The variables worked out, by the number of their bit.
  std::vector<StringIndex::Ref> vars_;
  // The number of the bit of each variable worked out.
  HashMap<StringIndex::Ref, std::uint32_t> bits_;
};

}  // namespace lucid
