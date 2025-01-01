#pragma once

#include <ostream>

#include "lucid/arm64.h"

namespace lucid {

// Compiles ARM64 instructions into a Mach-O binary and writes it to `out`.
void WriteCompiledMachObject(const arm64::Assembler& Assembler,
                             std::ostream& out);

}  // namespace lucid
