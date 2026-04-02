#pragma once

#include <cstddef>
#include <vector>

#include "lucid/syntax/token.h"

namespace lucid {

// Maintains a buffer of tokens produced by the lexer `L`.
template <typename L>
class BufferedLexer {
 public:
  explicit BufferedLexer(L lexer) {
    while (true) {
      Token token = lexer.next();
      tokens_.push_back(token);
      if (token.kind == Token::Kind::End) break;
      if (token.kind == Token::Kind::Error) break;
    }
  }

  // Returns the next token in the buffer.
  //
  // Requires:
  // - Must not be called after it returns a `Kind::End` or `Kind::Error` token.
  inline Token next() { return tokens_[pos_++]; }

  // Resets the lexer to return tokens from the beginning.
  void Reset() { pos_ = 0; }

 private:
  std::vector<Token> tokens_;
  std::size_t pos_ = 0;
};

}  // namespace lucid
