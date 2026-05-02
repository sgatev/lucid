#pragma once

#include <string_view>

#include "lucid/am/cfg.h"

namespace lucid {

// Prints the given control flow graph of abstract machine instructions.
void Print(std::string_view func_name,
           const AbstractMachineControlFlowGraph& amcfg);

}  // namespace lucid
