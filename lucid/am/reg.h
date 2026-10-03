#pragma once

#include <cstddef>
#include <optional>
#include <utility>
#include <vector>

#include "lucid/am/cfg.h"
#include "lucid/am/ig.h"
#include "lucid/am/instructions.h"
#include "lucid/am/liveness.h"
#include "lucid/am/state.h"
#include "lucid/core/container/hash_map.h"
#include "lucid/core/container/hash_set.h"

namespace lucid {

// Spills registers until no more than `max_clique_size` of them are live at
// any one point, and returns what the last analysis of the graph settled on.
//
// Deciding what to spill is mostly a matter of working out what is live
// where, and the answer that ends it is the answer for the graph as it is
// left, which is what an interference graph over it is built from.
AbstractMachineLiveness SpillRegisters(AbstractMachineControlFlowGraph& am_cfg,
                                       AbstractMachineState& am_state,
                                       int max_clique_size);

// The colour each register of a function took, held by register id: the ids
// run from zero without gaps, so an index reaches every one of them, and each
// is looked up once for every place a register is named.
class RegisterColors {
 public:
  // The colour of a register that was not coloured.
  static constexpr int kNone = -1;

  explicit RegisterColors(std::vector<int> colors_by_id)
      : colors_by_id_(std::move(colors_by_id)) {}

  // Returns the colour `reg` took, if it took one.
  std::optional<int> Get(Reg reg) const {
    if (reg.id < 0 || std::size_t(reg.id) >= colors_by_id_.size()) {
      return std::nullopt;
    }
    const int color = colors_by_id_[reg.id];
    if (color == kNone) return std::nullopt;
    return color;
  }

 private:
  std::vector<int> colors_by_id_;
};

// Gives every register in `am_ig` one of `colors_count` colours, none shared
// by two registers that interfere.
//
// Requires:
// - `am_cfg` must be in strict single assignment form, which is what lets its
//   registers be coloured in the order its dominator tree has them in. See
//   `CheckStrictSsa`.
RegisterColors ColorInterferenceGraph(
    const AbstractMachineControlFlowGraph& am_cfg,
    const InterferenceGraph& am_ig, int colors_count);

void MergeRegisters(const RegisterColors& reg_colors,
                    AbstractMachineControlFlowGraph& am_cfg);

}  // namespace lucid
