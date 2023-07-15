#pragma once

#include <string>

#include "lucid/ast.h"

namespace lucid {

// Generates C++ source code for `stmt`.
std::string GenerateCppSource(const Stmt& stmt);

}  // namespace lucid
