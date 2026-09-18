#pragma once

#include <cstdint>
#include <vector>

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

}  // namespace lucid
