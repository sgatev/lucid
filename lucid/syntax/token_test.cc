#include "lucid/syntax/token.h"

#include <sstream>
#include <string>

#include "lucid/core/testing/testing.h"

namespace lucid {
namespace {

TEST(Test, TokenSize) { EXPECT_EQ(sizeof(Token), 12); }

TEST(Test, TokenEquality) {
  EXPECT_EQ(Token(Token::Kind::Ident, 0, 3), Token(Token::Kind::Ident, 0, 3));
  EXPECT_NE(Token(Token::Kind::Ident, 0, 3), Token(Token::Kind::Number, 0, 3));
  EXPECT_NE(Token(Token::Kind::Ident, 0, 3), Token(Token::Kind::Ident, 1, 3));
  EXPECT_NE(Token(Token::Kind::Ident, 0, 3), Token(Token::Kind::Ident, 0, 5));
}

TEST(Test, TokenOutputStream) {
  std::stringstream out;
  out << Token(Token::Kind::Ident, 0, 3);
  EXPECT_EQ(std::string(out.str()),
            "Token{.kind=11, .start_pos=0, .end_pos=3}");
}

TEST(Test, FindLineWorks) {
  EXPECT_EQ(FindLine("", Token(Token::Kind::End, 0, 0)), 1);
  EXPECT_EQ(FindLine("foo", Token(Token::Kind::Ident, 0, 3)), 1);
  EXPECT_EQ(FindLine("foo\nbar", Token(Token::Kind::Ident, 4, 7)), 2);
  EXPECT_EQ(FindLine("foo\nlet = bar", Token(Token::Kind::Ident, 10, 13)), 2);
}

TEST(Test, FindColumnWorks) {
  EXPECT_EQ(FindColumn("", Token(Token::Kind::End, 0, 0)), 1);
  EXPECT_EQ(FindColumn("foo", Token(Token::Kind::Ident, 0, 3)), 1);
  EXPECT_EQ(FindColumn("foo\nbar", Token(Token::Kind::Ident, 4, 7)), 1);
  EXPECT_EQ(FindColumn("foo\nlet = bar", Token(Token::Kind::Ident, 10, 13)), 7);
}

}  // namespace
}  // namespace lucid
