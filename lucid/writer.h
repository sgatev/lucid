#pragma once

#include <cstdio>
#include <functional>
#include <string>
#include <string_view>

namespace lucid {

// A writer function that outputs its argument.
using Writer = std::function<void(std::string_view)>;

// Creates a string writer that outputs to `out`.
Writer StringWriter(std::string& out);

// Creates a file writer that outputs to `out`.
Writer FileWriter(std::FILE* out);

}  // namespace lucid
