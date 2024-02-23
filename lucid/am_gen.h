#pragma once

#include <cstdint>
#include <string>
#include <unordered_map>
#include <vector>

#include "lucid/am.h"
#include "lucid/arena.h"
#include "lucid/ast.h"
#include "lucid/cfg.h"

namespace lucid {

// State used in the generation of abstract machine instructions.
struct AbstractMachineState {
  // Generated abstract machine function after the last call to
  // `GenerateAbstractMachineFunction` where this state was used.
  Function func;

  // Register allocation data structure that is used to generate abstract
  // machine instructions.
  std::vector<RegId> out_reg;

  // Strings used in `func`.
  std::unordered_map<std::uintptr_t, std::string> strings;
};

// Generates abstract machine instructions for `graph`.
//
// The generated instructions are stored in `state`.
//
// Requirements:
//  * Statements that are reachable from `graph` must be allocated on
//    `stmt_arena`.
//  * Expressions that are reachable from `graph` must be allocated on
//    `expr_arena`.
//  * Types that are reachable from `graph` must be allocated on
//    `type_arena`.
void GenerateAbstractMachineFunction(const Arena<Stmt>& stmt_arena,
                                     const Arena<Expr>& expr_arena,
                                     const Arena<Type>& type_arena,
                                     const ControlFlowGraph& graph,
                                     AbstractMachineState& state);

}  // namespace lucid
