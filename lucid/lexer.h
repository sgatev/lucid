#pragma once

#include <algorithm>
#include <array>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <string_view>

#include "lucid/token.h"

namespace lucid {

// Converts a string of Lucid code into a stream of tokens.
class Lexer {
 public:
  // Requirements:
  //   - `buffer` must end in `\0`.
  explicit Lexer(std::string_view buffer)
      : buffer_(buffer.data()), size_(buffer.size()), pos_(0) {
    assert(size_ > 0);
    assert(buffer_[size_ - 1] == '\0');
  }

  // Returns the next token in the buffer.
  //
  // Requirements:
  //   - Must not be called after it returns a `Kind::End` or
  //     `Kind::Error` token.
  inline Token next() {
    const char sym = buffer_[pos_];
    const std::uint8_t sym_class = kClassMap[sym];
    const std::size_t start_pos = pos_++;
    if (sym_class > 3) [[unlikely]] {
      // Ident, number, or whitespace.
      while (kClassMap[buffer_[pos_]] % sym_class < 2) ++pos_;
    } else if (sym == '"' || sym == '#') [[unlikely]] {
      // String or comment.
      const char* pos = std::char_traits<char>::find(
          buffer_ + pos_, size_ - pos_ - 1, sym == '"' ? '"' : '\n');
      if (pos == nullptr) [[unlikely]] {
        pos_ = size_;
        return Token(Token::Kind::Error, start_pos, pos_);
      }
      pos_ = pos - buffer_ + 1;
    }
    return Token(kTokenKindMap[sym], start_pos, pos_);
  }

 private:
  static constexpr std::array<std::uint8_t, 256> kClassMap = []() consteval {
    // The class values have the following properties:
    // - kAlphaClass mod kAlphaClass = 0
    // - kNumClass mod kAlphaClass = 1
    // - kNumClass mod kNumClass = 0
    // - kSpaceClass mod kSpaceClass = 0
    // - X mod Y > 1 for every other pair of classes where Y > 3
    constexpr std::uint8_t kNumClass = 255;
    constexpr std::uint8_t kAlphaClass = 254;
    constexpr std::uint8_t kSpaceClass = 252;
    constexpr std::uint8_t kOtherClass = 3;

    std::array<std::uint8_t, 256> map;
    map.fill(kOtherClass);
    for (char c = 'a'; c <= 'z'; ++c) map[c] = kAlphaClass;
    for (char c = 'A'; c <= 'Z'; ++c) map[c] = kAlphaClass;
    for (char c = '0'; c <= '9'; ++c) map[c] = kNumClass;
    map[' '] = kSpaceClass;
    map['\n'] = kSpaceClass;
    map['\t'] = kSpaceClass;
    return map;
  }();

  static constexpr std::array<Token::Kind, 256> kTokenKindMap = []() consteval {
    std::array<Token::Kind, 256> map = {Token::Kind::End};
    map['='] = Token::Kind::Equal;
    map['('] = Token::Kind::OpenParen;
    map[')'] = Token::Kind::CloseParen;
    map['{'] = Token::Kind::OpenBrace;
    map['}'] = Token::Kind::CloseBrace;
    map['['] = Token::Kind::OpenBracket;
    map[']'] = Token::Kind::CloseBracket;
    map[':'] = Token::Kind::Colon;
    map[','] = Token::Kind::Comma;
    map['+'] = Token::Kind::Plus;
    map['-'] = Token::Kind::Minus;
    map['*'] = Token::Kind::Star;
    map['/'] = Token::Kind::Slash;
    map['>'] = Token::Kind::Greater;
    map['<'] = Token::Kind::Less;
    map['.'] = Token::Kind::Dot;
    map['|'] = Token::Kind::Bar;
    map['!'] = Token::Kind::Exclamation;
    map['#'] = Token::Kind::Comment;
    map['"'] = Token::Kind::String;
    map['%'] = Token::Kind::Percent;
    map[' '] = Token::Kind::Space;
    map['\t'] = Token::Kind::Space;
    map['\n'] = Token::Kind::Space;
    map['\0'] = Token::Kind::End;
    for (char c = 'a'; c <= 'z'; ++c) map[c] = Token::Kind::Ident;
    for (char c = 'A'; c <= 'Z'; ++c) map[c] = Token::Kind::Ident;
    for (char c = '0'; c <= '9'; ++c) map[c] = Token::Kind::Number;
    return map;
  }();

  const char* buffer_;
  const std::size_t size_;
  std::size_t pos_;
};

}  // namespace lucid
