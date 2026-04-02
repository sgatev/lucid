#pragma once

#include <string_view>

#include "lucid/am/cfg.h"

namespace lucid {

// Prints the abstract machine instructions generated for the given function.
void Print(std::string_view func_name,
           const AbstractMachineControlFlowGraph& am_cfg);

}  // namespace lucid
