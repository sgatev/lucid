#include "lucid/core/testing/testing.h"

#include <cstddef>
#include <iostream>
#include <string_view>

namespace lucid {

void Test::Fail(std::string_view reason) {
  std::cout << "Fail: " << reason << "\n";
  failed_ = true;
}

}  // namespace lucid
