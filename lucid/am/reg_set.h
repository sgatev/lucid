#pragma once

#include <cstddef>
#include <cstdint>
#include <limits>
#include <vector>

#include "lucid/am/instructions.h"

namespace lucid {

// A dense set of registers.
class RegSet {
 public:
  explicit RegSet(std::size_t reg_count) : indices_(reg_count, kAbsent) {}

  // Returns whether `reg` is in the set.
  bool Contains(Reg reg) const { return indices_[reg.id] != kAbsent; }

  // Inserts `reg` in the set.
  void Insert(Reg reg) {
    if (Contains(reg)) return;

    indices_[reg.id] = static_cast<std::uint32_t>(regs_.size());
    regs_.push_back(reg);
  }

  // Removes `reg` from the set.
  void Remove(Reg reg) {
    const std::uint32_t reg_idx = indices_[reg.id];
    if (reg_idx == kAbsent) return;

    regs_[reg_idx] = regs_.back();
    indices_[regs_[reg_idx].id] = reg_idx;
    regs_.pop_back();
    indices_[reg.id] = kAbsent;
  }

  // Empties the set.
  void Clear() {
    for (Reg reg : regs_) indices_[reg.id] = kAbsent;
    regs_.clear();
  }

  // Returns the number of unique registers inserted so far.
  std::size_t size() const { return regs_.size(); }

  // Returns an iterator referring to the first register in the set or `end()`,
  // if there isn't one.
  auto begin() const { return regs_.begin(); }

  // Returns an iterator past the last register in the set.
  auto end() const { return regs_.end(); }

 private:
  static constexpr std::uint32_t kAbsent =
      std::numeric_limits<std::uint32_t>::max();

  // Dense set of registers.
  std::vector<Reg> regs_;

  // Index of a register (by register id) in `regs_`.
  std::vector<std::uint32_t> indices_;
};

}  // namespace lucid
