#pragma once

#include <cstddef>
#include <cstdint>
#include <string_view>
#include <vector>

namespace lucid {

// Interprets `s` as a Posix-encoded string and returns its length.
std::size_t EncodedStringLength(std::string_view s);

// Interprets `s` as a Posix-encoded string, writes it to the end of `out`, and
// returns its length.
//
// Guarantees:
// - The string written to `out` is null-terminated.
std::size_t WriteEncodedString(std::string_view s,
                               std::vector<std::uint8_t>& out);

}  // namespace lucid
