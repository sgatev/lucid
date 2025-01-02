#include "lucid/string.h"

#include <array>
#include <cmath>
#include <cstdint>
#include <ostream>
#include <string_view>

namespace lucid {
namespace {

static constexpr std::array<char, 256> kEscapeChar = []() consteval {
  std::array<char, 256> map = {0};
  map['\\'] = '\\';
  map['a'] = '\a';
  map['b'] = '\b';
  map['f'] = '\f';
  map['n'] = '\n';
  map['r'] = '\r';
  map['t'] = '\t';
  map['v'] = '\v';
  return map;
}();

static constexpr std::array<std::uint8_t, 256> kEscapeOctet = []() consteval {
  std::array<std::uint8_t, 256> map = {255};
  map['0'] = 0;
  map['1'] = 1;
  map['2'] = 2;
  map['3'] = 3;
  map['4'] = 4;
  map['5'] = 5;
  map['6'] = 6;
  map['7'] = 7;
  return map;
}();

}  // namespace

std::size_t EncodedStringLength(std::string_view s) {
  std::size_t count = 0;

  while (!s.empty()) {
    if (s.front() == '\\') {
      s.remove_prefix(1);

      if (char c = kEscapeChar[s.front()]; c != 0) {
        s.remove_prefix(1);

        ++count;

        continue;
      }

      int i = 0;
      for (; i < 3; ++i) {
        if (s[i] < '0' || s[i] > '7') break;
      }
      --i;

      for (; i >= 0; --i) {
        s.remove_prefix(1);
      }

      ++count;

      continue;
    }

    s.remove_prefix(1);
    ++count;
  }

  ++count;

  return count;
}

std::size_t WriteEncodedString(std::string_view s, std::ostream& out) {
  std::size_t count = 0;

  while (!s.empty()) {
    if (s.front() == '\\') {
      s.remove_prefix(1);

      if (char c = kEscapeChar[s.front()]; c != 0) {
        s.remove_prefix(1);

        out.put(c);
        ++count;

        continue;
      }

      int i = 0;
      for (; i < 3; ++i) {
        if (s[i] < '0' || s[i] > '7') break;
      }
      --i;

      std::uint8_t r = 0;
      for (; i >= 0; --i) {
        r += kEscapeOctet[s.front()] * std::pow(8, i);
        s.remove_prefix(1);
      }

      out.put(r);
      ++count;

      continue;
    }

    out.put(s.front());
    s.remove_prefix(1);
    ++count;
  }

  out.put(0);
  ++count;

  return count;
}

}  // namespace lucid
