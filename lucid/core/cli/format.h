#pragma once

#include <cstdint>
#include <iomanip>
#include <ios>
#include <ostream>

namespace lucid {

enum class Color : std::uint8_t {
  Green,
  Blue,
  LightBlue,
  Red,
};

namespace internal {

// Returns the slot in a stream's storage that records whether it shows color.
inline int ColorSlot() {
  static const int slot = std::ios_base::xalloc();
  return slot;
}

}  // namespace internal

// Sets whether `SetColor` and `ResetColor` print anything to `o`.
//
// A stream shows no color until this enables it, so that text written
// somewhere other than a terminal, such as a file or a pipe, carries no
// escape sequences.
inline void EnableColor(std::ostream& o, bool enable = true) {
  o.iword(internal::ColorSlot()) = enable ? 1 : 0;
}

// Changes the color of new text printed to `o`.
inline auto SetColor(Color c) {
  return [c](std::ostream& o) -> std::ostream& {
    if (o.iword(internal::ColorSlot()) == 0) return o;
    switch (c) {
      case Color::Green:
        return o << "\33[32m";
      case Color::LightBlue:
        return o << "\33[36m";
      case Color::Blue:
        return o << "\33[34m";
      case Color::Red:
        return o << "\33[31m";
    }
  };
}

// Resets the color of new text printed to `o`.
inline std::ostream& ResetColor(std::ostream& o) {
  if (o.iword(internal::ColorSlot()) == 0) return o;
  return o << "\33[m";
}

// Prints indentation to `o`.
inline auto Indent(int indent) {
  return [indent](std::ostream& o) -> std::ostream& {
    return o << std::setw(indent) << "";
  };
}

}  // namespace lucid

// Enables the use of function-like objects as stream manipulators.
template <class M>
auto operator<<(std::ostream& os, const M& m) -> decltype(m(os)) {
  return m(os);
}
