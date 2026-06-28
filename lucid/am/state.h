#pragma once

#include <cstdint>

#include "lucid/core/container/hash_map.h"
#include "lucid/core/container/hash_set.h"
#include "lucid/core/string/index.h"

namespace lucid {

// State used in the generation of abstract machine instructions.
struct AbstractMachineState {
  // Strings used in `func`.
  HashMap<std::uintptr_t, StringIndex::Ref> strings;

  // Integers used in `func`.
  HashSet<std::int64_t> ints;
};

}  // namespace lucid
