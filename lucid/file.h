#pragma once

#include <string>
#include <string_view>

namespace lucid {

// Returns the content of the file located at `path`.
std::string ReadFile(std::string_view path);

}  // namespace lucid
