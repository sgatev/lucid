#pragma once

#include <span>
#include <string>
#include <string_view>

namespace lucid {

// Concatenates `parts` by inserting `glue` inbetween every pair.
std::string Concat(std::span<const std::string_view> parts,
                   std::string_view glue);

}  // namespace lucid
