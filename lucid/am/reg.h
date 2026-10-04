#pragma once

#include <cstddef>
#include <optional>
#include <utility>
#include <vector>

#include "lucid/am/cfg.h"
#include "lucid/am/instructions.h"
#include "lucid/am/liveness.h"
#include "lucid/am/state.h"

namespace lucid {

// Spills registers until no more than `max_clique_size` of them are live at
// any one point, and returns what the last analysis of the graph settled on.
//
// Deciding what to spill is mostly a matter of working out what is live
// where, and the answer that ends it is the answer for the graph as it is
// left, which is what colouring its registers goes by.
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

// Gives every register in `am_cfg` one of `colors_count` colours, none shared
// by two registers live at the same point, given the `liveness` the spilling
// left it with.
//
// The registers are taken in the order the blocks are dominated in, each
// given the lowest colour free where it is written. In strict single
// assignment form that never needs more colours than the most registers live
// at any one point, which the spilling has brought down to `colors_count`,
// so no graph of which registers interfere is needed.
//
// Requires:
// - `am_cfg` must be in strict single assignment form. See `CheckStrictSsa`.
// - `liveness` must be what is live where each block of `am_cfg` starts.
RegisterColors ColorRegisters(const AbstractMachineControlFlowGraph& am_cfg,
                              const AbstractMachineLiveness& liveness,
                              int colors_count);

void MergeRegisters(const RegisterColors& reg_colors,
                    AbstractMachineControlFlowGraph& am_cfg);

}  // namespace lucid
