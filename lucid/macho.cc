#include "lucid/macho.h"

#include <cstdint>
#include <ostream>
#include <vector>

#include "lucid/arm64.h"

namespace lucid {

void AssembleMachObject(const arm64::Arm64& arm, std::ostream& out) {
  for (auto inst : arm.Encode()) {
    out.write(reinterpret_cast<const char*>(&inst), 4);
  }
}

}  // namespace lucid
