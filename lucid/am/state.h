#pragma once

#include <cstdint>
#include <vector>

#include "lucid/core/container/hash_map.h"
#include "lucid/core/string/index.h"

namespace lucid {

// State used in the generation of abstract machine instructions.
struct AbstractMachineState {
  // Abstract machine stack slots.
  std::vector<std::size_t> stack_slots;

  // Strings used in `func`.
  HashMap<std::uintptr_t, StringIndex::Ref> strings;

  // Next free register ID.
  std::int32_t next_free_reg_id = 1;
};

}  // namespace lucid
