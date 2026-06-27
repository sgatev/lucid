#pragma once

#include <cstdint>
#include <iomanip>
#include <ostream>

namespace lucid {

enum class Color : std::uint8_t {
  Green,
  Blue,
  LightBlue,
  Red,
};

// Changes the color of new text printed to `o`.
inline auto SetColor(Color c) {
  return [c](std::ostream& o) -> std::ostream& {
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
inline std::ostream& ResetColor(std::ostream& o) { return o << "\33[m"; }

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
