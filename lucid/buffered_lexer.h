#pragma once

#include <array>
#include <cstddef>

#include "lucid/token.h"

namespace lucid {

// Maintains a buffer of size `S` of tokens produced by the lexer `L`.
template <typename L, int S>
class BufferedLexer {
 public:
  explicit BufferedLexer(L lexer) : lexer_(std::move(lexer)) {}

  // Returns the next token in the buffer.
  //
  // Requires:
  // - Must not be called after it returns a `Kind::End` or `Kind::Error` token.
  inline Token next() {
    if (pos_ == S) RefillTokens();
    return tokens_[pos_++];
  }

 private:
  // Updates the tokens in the buffer.
  void RefillTokens() {
    for (pos_ = 0; pos_ < S; ++pos_) {
      tokens_[pos_] = lexer_.next();
      if (tokens_[pos_].kind == Token::Kind::End) break;
      if (tokens_[pos_].kind == Token::Kind::Error) break;
    }
    pos_ = 0;
  }

  L lexer_;
  std::array<Token, S> tokens_;
  std::size_t pos_ = S;
};

}  // namespace lucid
