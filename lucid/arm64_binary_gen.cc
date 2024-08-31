#include "lucid/arm64_binary_gen.h"

#include <cstdint>
#include <ostream>
#include <string>
#include <unordered_map>
#include <vector>

#include "lucid/am.h"
#include "lucid/arm64.h"

namespace lucid {

using namespace ::lucid::arm64;

void GenerateArmStartBinary(std::ostream& out) {}

void GenerateArmEndBinary(
    const std::unordered_map<std::uintptr_t, std::string>& strings,
    std::ostream& out) {}

void GenerateArmAssemblyBinary(const Function& func, std::ostream& out) {
  Arm64 arm;
  arm.Mov(W(1), W(2));
  arm.Adr(X(1), "foo");
  arm.B(Cond::Eq, "foo");
  arm.Sub(SP, SP, X(1));
  arm.Encode();
}

}  // namespace lucid
