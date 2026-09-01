#include "lucid/syntax/buffered_lexer.h"

#include <string>
#include <vector>

#include "lucid/core/testing/testing.h"
#include "lucid/syntax/lexer.h"
#include "lucid/syntax/token.h"

namespace lucid {
namespace {

using namespace std::string_literals;

TEST(Test, BufferedLexerProducesTheSameTokensAsLexer) {
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

  BufferedLexer<Lexer> buffered_lexer(Lexer{code});

  std::vector<Token> buffered_lexer_tokens;
  while (true) {
    Token token = buffered_lexer.next();
    buffered_lexer_tokens.push_back(token);
    if (token.kind == Token::Kind::End) break;
  }
  EXPECT_EQ(buffered_lexer_tokens, lexer_tokens);
}

TEST(Test, BufferedLexerCanBeReset) {
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

  BufferedLexer<Lexer> buffered_lexer(Lexer{code});

  while (true) {
    Token token = buffered_lexer.next();
    if (token.kind == Token::Kind::End) break;
  }
  buffered_lexer.Reset();

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
