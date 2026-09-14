#pragma once

#include <concepts>
#include <sstream>
#include <string>
#include <string_view>
#include <type_traits>

namespace lucid::internal {

// A value that can be viewed as a string.
template <typename A>
concept StringLike = std::convertible_to<const A&, std::string_view>;

// A value that can be written to an output stream.
template <typename A>
concept Streamable = requires(std::ostream& out, const A& a) {
  { out << a } -> std::same_as<std::ostream&>;
};

// Returns a string representation of `t`.
//
// Renders the types the matchers meet most often directly, and anything else
// that has an `operator<<` through that, so a type need only be printable to
// read well in a failure message. A type that is none of these falls back to a
// placeholder rather than failing to compile, so that a value of any type can
// still be matched; only the message suffers.
//
// The branches are in order of preference, which is why the first two come
// before `Streamable`: a `bool` streams as `1` and a `char` as an unquoted
// glyph. Integers are rendered without a stream because `std::to_string` is
// both cheaper and, for the single-byte types, numeric rather than glyphic.
//
// A scoped enumeration has no conversion to an integer and so is not
// streamable on its own. It comes out as its numeric value, which at least
// differs between the expected and the actual value. Give the enumeration an
// `operator<<` to see its enumerators named instead -- that branch is checked
// first, so a printable enumeration keeps its names.
//
// Specialize this for a type that is neither printable nor string-like.
template <typename T>
inline std::string ToString(const T& t) {
  if constexpr (std::same_as<T, bool>) {
    return t ? "true" : "false";
  } else if constexpr (std::same_as<T, char>) {
    return "'" + std::string(1, t) + "'";
  } else if constexpr (StringLike<T>) {
    return "\"" + std::string(std::string_view(t)) + "\"";
  } else if constexpr (std::integral<T>) {
    return std::to_string(t);
  } else if constexpr (Streamable<T>) {
    std::ostringstream out;
    out << t;
    return out.str();
  } else if constexpr (std::is_enum_v<T>) {
    return std::to_string(static_cast<std::underlying_type_t<T>>(t));
  } else {
    return "[unstringable]";
  }
}

}  // namespace lucid::internal
