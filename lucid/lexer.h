#pragma once

#include <algorithm>
#include <array>
#include <cstddef>
#include <cstdint>
#include <string_view>

#include "lucid/token.h"

namespace lucid {

// Converts a string of Lucid code into a stream of tokens.
class Lexer {
 public:
  // `buffer_` must end in `\n`.
  explicit Lexer(std::string_view buffer)
      : buffer_(buffer.data()), size_(buffer.size()), pos_(0) {}

  // Returns the next token in the buffer.
  inline Token next() {
    if (pos_ == size_) [[unlikely]] {
      return Token(Token::Kind::End, pos_, pos_);
    }

    const char c = buffer_[pos_];
    const std::uint8_t cc = kEquivClassMap[c];
    const std::size_t start_pos = pos_++;
    if (cc > 0xFD) [[likely]] {
      // Ident, number, or whitespace.
      while (pos_ < size_ && kEquivClassMap[buffer_[pos_]] == cc) ++pos_;
    } else if (c == '"' || c == '#') {
      // String or comment.
      const char* pos = std::char_traits<char>::find(
          buffer_ + pos_, size_ - pos_ - 2, c == '"' ? '"' : '\n');
      if (pos == nullptr) {
        pos_ = size_;
        return Token(Token::Kind::IncompleteString, start_pos, pos_);
      }
      pos_ = pos - buffer_ + 1;
    }
    return Token(kTokenKindMap[c], start_pos, pos_);
  }

 private:
  static constexpr std::uint8_t kAlphaNumClass = 0xFF;
  static constexpr std::uint8_t kWhitespaceClass = 0xFE;

  static constexpr std::array<std::uint8_t, 256> kEquivClassMap =
      []() consteval {
        std::array<std::uint8_t, 256> map = {0};
        for (int i = 0; i < 256; ++i) map[i] = i;

        for (char c = 'a'; c <= 'z'; ++c) map[c] = kAlphaNumClass;
        for (char c = 'A'; c <= 'Z'; ++c) map[c] = kAlphaNumClass;
        for (char c = '0'; c <= '9'; ++c) map[c] = kAlphaNumClass;

        map[' '] = kWhitespaceClass;
        map['\n'] = kWhitespaceClass;
        map['\t'] = kWhitespaceClass;
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
    map[' '] = Token::Kind::Whitespace;
    map['\t'] = Token::Kind::Whitespace;
    map['\n'] = Token::Kind::Whitespace;
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
