#pragma once

#include <ostream>
#include <string_view>

#include "lucid/am/cfg.h"

namespace lucid {

// Prints a text representation of the abstract machine control flow graph
// `amcfg` to `out`.
void Print(std::string_view func_name,
           const AbstractMachineControlFlowGraph& amcfg, std::ostream& out);

}  // namespace lucid
