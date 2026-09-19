#pragma once

#include <cstdint>
#include <limits>
#include <vector>

#include "lucid/am/instructions.h"
#include "lucid/core/container/hash_set.h"
#include "lucid/core/string/index.h"

namespace lucid {

// State used in the generation of abstract machine instructions.
struct AbstractMachineState {
  // Strings used in `func`, in the order they were first referenced.
  //
  // A string is identified by its index here, so its identity depends only on
  // the program and not on where the string happens to be stored.
  std::vector<StringIndex::Ref> strings;

  // Integers used in `func`.
  HashSet<std::int64_t> ints;
};

// Returns the instruction that puts `value` in `dst`.
//
// An instruction carries only a small, non-negative value itself. Anything
// else is recorded in `am_state` as a constant the program needs, and loaded
// from there instead.
inline Instruction SetValue(std::int64_t value, Reg dst,
                            AbstractMachineState& am_state) {
  if (value < 0 || value > std::numeric_limits<std::int16_t>::max()) {
    am_state.ints.Insert(value);
    return SetInt{.src_val = value, .dst_reg = dst};
  }
  return SetReg{.src_val = static_cast<int>(value), .dst_reg = dst};
}

}  // namespace lucid
