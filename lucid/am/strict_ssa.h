#pragma once

#include <expected>
#include <string>

#include "lucid/am/cfg.h"

namespace lucid {

// Checks that `am_cfg` is in strict single assignment form: that every
// register is written in one place, and that the write comes before every
// read of it on every way control can take there. A parameter is written
// where the function is entered, a phi function's result where its block
// starts, and a phi function reads each argument where control leaves the
// block that argument comes from.
//
// Returns what is wrong with the first register found that is not, if one
// is, or the first block control cannot reach, if there is one.
//
// A graph in this form is one whose registers can be coloured in the order
// its dominator tree has them in, needing no more colours than are live at
// any one point.
std::expected<void, std::string> CheckStrictSsa(
    const AbstractMachineControlFlowGraph& am_cfg);

}  // namespace lucid
