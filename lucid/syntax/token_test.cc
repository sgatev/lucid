#include "lucid/syntax/token.h"

#include <sstream>
#include <string>

#include "gtest/gtest.h"

namespace lucid {
namespace {

TEST(TokenTest, Size) { EXPECT_EQ(sizeof(Token), 12); }

TEST(TokenTest, Default) { EXPECT_EQ(Token(), Token(Token::Kind::End, 0, 0)); }

TEST(TokenTest, Equality) {
  EXPECT_EQ(Token(Token::Kind::Ident, 0, 3), Token(Token::Kind::Ident, 0, 3));
  EXPECT_NE(Token(Token::Kind::Ident, 0, 3), Token(Token::Kind::Number, 0, 3));
  EXPECT_NE(Token(Token::Kind::Ident, 0, 3), Token(Token::Kind::Ident, 1, 3));
  EXPECT_NE(Token(Token::Kind::Ident, 0, 3), Token(Token::Kind::Ident, 0, 5));
}

TEST(TokenTest, OutputStream) {
  std::stringstream out;
  out << Token(Token::Kind::Ident, 0, 3);
  EXPECT_EQ(std::string(out.str()),
            "Token{.kind=10, .start_pos=0, .end_pos=3}");
}

TEST(FindLineTest, Works) {
  EXPECT_EQ(FindLine("", Token(Token::Kind::End, 0, 0)), 1);
  EXPECT_EQ(FindLine("foo", Token(Token::Kind::Ident, 0, 3)), 1);
  EXPECT_EQ(FindLine("foo\nbar", Token(Token::Kind::Ident, 4, 7)), 2);
  EXPECT_EQ(FindLine("foo\nlet = bar", Token(Token::Kind::Ident, 10, 13)), 2);
}

TEST(FindColumnTest, Works) {
  EXPECT_EQ(FindColumn("", Token(Token::Kind::End, 0, 0)), 1);
  EXPECT_EQ(FindColumn("foo", Token(Token::Kind::Ident, 0, 3)), 1);
  EXPECT_EQ(FindColumn("foo\nbar", Token(Token::Kind::Ident, 4, 7)), 1);
  EXPECT_EQ(FindColumn("foo\nlet = bar", Token(Token::Kind::Ident, 10, 13)), 7);
}

}  // namespace
}  // namespace lucid
