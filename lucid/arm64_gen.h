#pragma once

#include <string>

#include "lucid/arena.h"
#include "lucid/ast.h"
#include "lucid/cfg.h"

namespace lucid {

// Generates the start sequence for 64-bit ARM assembly source code.
std::string GenerateArmStartSource();

// Generates 64-bit ARM assembly source code for `graph`.
//
// All statements that are reachable from `graph` must be allocated on `arena`.
std::string GenerateArmAssemblySource(const Arena<Stmt>& arena,
                                      const ControlFlowGraph& graph);

}  // namespace lucid
