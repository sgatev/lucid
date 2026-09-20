#pragma once

#include <optional>
#include <variant>

#include "lucid/core/container/hash_map.h"
#include "lucid/core/hash/hash.h"
#include "lucid/core/string/index.h"
#include "lucid/syntax/ast.h"
#include "lucid/syntax/cfg.h"
#include "lucid/syntax/context.h"

namespace lucid {

class SyntaxReachabilityAnalysis {
 public:
  using NamedValueSource =
      std::variant<StmtRef, ParamRef, SyntaxControlFlowGraph::PhiRef>;

  struct State {
    bool operator==(const State&) const = default;

    HashMap<StringIndex::Ref, NamedValueSource> vars_in;
    HashMap<StringIndex::Ref, NamedValueSource> vars_out;
  };

  explicit SyntaxReachabilityAnalysis(SyntaxControlFlowGraph& s_cfg,
                                      SyntaxContext& ctx);

  State Transfer(std::optional<State>&& prior_state,
                 const SyntaxControlFlowGraph::BlockRef& block_ref);

  void Join(State& left, const State& right);

 private:
  SyntaxControlFlowGraph& s_cfg_;
  SyntaxContext& ctx_;
};

inline std::size_t Hash(
    const SyntaxReachabilityAnalysis::NamedValueSource& nvs) {
  return HashCombine(
      Hash(nvs.index()),
      std::visit([](const auto& nvs) { return Hash(nvs); }, nvs));
}

}  // namespace lucid
