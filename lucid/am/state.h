#pragma once

#include <cstdint>
#include <list>
#include <string>

#include "lucid/core/container/hash_map.h"
#include "lucid/core/string/index.h"

namespace lucid {

// State used in the generation of abstract machine instructions.
struct AbstractMachineState {
  // Strings used in `func`.
  HashMap<std::uintptr_t, StringIndex::Ref> strings;

  // Strings that represent literals in abstract machine instructions.
  std::list<std::string> literals;
};

}  // namespace lucid
