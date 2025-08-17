#include "lucid/buffered_lexer.h"

#include <string>
#include <vector>

#include "gtest/gtest.h"
#include "lucid/lexer.h"
#include "lucid/token.h"

namespace lucid {
namespace {

using namespace std::string_literals;

TEST(BufferedLexer, ProducesTheSameTokensAsLexer) {
  std::string code = R"(
    let main = () -> Void {
      print("Hello, world!")
    }
  )";
  code.append("\0"s);

  Lexer lexer(code);
  std::vector<Token> lexer_tokens;
  while (true) {
    Token token = lexer.next();
    lexer_tokens.push_back(token);
    if (token.kind == Token::Kind::End) break;
  }

  BufferedLexer<Lexer, 10> buffered_lexer(Lexer{code});
  std::vector<Token> buffered_lexer_tokens;
  while (true) {
    Token token = buffered_lexer.next();
    buffered_lexer_tokens.push_back(token);
    if (token.kind == Token::Kind::End) break;
  }

  EXPECT_EQ(buffered_lexer_tokens, lexer_tokens);
}

}  // namespace
}  // namespace lucid
