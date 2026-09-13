#include "lucid/core/testing/testing.h"

#include <cstddef>
#include <iostream>
#include <source_location>
#include <string_view>

namespace lucid {

void Test::Fail(std::string_view reason, std::source_location location) {
  // Led by "file:line: " so that the message reads like a compiler diagnostic
  // and editors can jump straight to the assertion that failed.
  std::cout << location.file_name() << ":" << location.line()
            << ": Fail: " << reason << "\n";
  failed_ = true;
}

}  // namespace lucid
