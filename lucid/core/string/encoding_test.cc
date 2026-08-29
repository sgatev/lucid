#include "lucid/core/string/encoding.h"

#include <sstream>

#include "lucid/core/testing/testing.h"

namespace lucid {
namespace {

TEST(Test, EncodedStringLengthCharacters) {
  EXPECT_EQ(EncodedStringLength(R"(foo 21)"), 7);
}

TEST(Test, EncodedStringLengthEscapedCharacter) {
  EXPECT_EQ(EncodedStringLength(R"(\a)"), 2);
  EXPECT_EQ(EncodedStringLength(R"(\b)"), 2);
  EXPECT_EQ(EncodedStringLength(R"(\f)"), 2);
  EXPECT_EQ(EncodedStringLength(R"(\n)"), 2);
  EXPECT_EQ(EncodedStringLength(R"(\r)"), 2);
  EXPECT_EQ(EncodedStringLength(R"(\t)"), 2);
  EXPECT_EQ(EncodedStringLength(R"(\v)"), 2);
}

TEST(Test, EncodedStringLengthEscapedNumber) {
  EXPECT_EQ(EncodedStringLength(R"(\2)"), 2);
  EXPECT_EQ(EncodedStringLength(R"(\33)"), 2);
  EXPECT_EQ(EncodedStringLength(R"(\345)"), 2);
  EXPECT_EQ(EncodedStringLength(R"(\3456)"), 3);
  EXPECT_EQ(EncodedStringLength(R"(\345foo)"), 5);
}

TEST(Test, EncodedStringLengthMix) {
  EXPECT_EQ(EncodedStringLength(R"(foo\215bar\n)"), 9);
}

TEST(Test, WriteEncodedStringCharacters) {
  std::stringstream out;
  EXPECT_EQ(WriteEncodedString(R"(foo 21)", out), 7);
  EXPECT_EQ(std::strcmp(out.str().data(), "foo 21\0"), 0);
}

TEST(Test, WriteEncodedStringEscapedCharacter) {
  {
    std::stringstream out;
    EXPECT_EQ(WriteEncodedString(R"(\a)", out), 2);
    EXPECT_EQ(std::strcmp(out.str().data(), "\a\0"), 0);
  }
  {
    std::stringstream out;
    EXPECT_EQ(WriteEncodedString(R"(\b)", out), 2);
    EXPECT_EQ(std::strcmp(out.str().data(), "\b\0"), 0);
  }
  {
    std::stringstream out;
    EXPECT_EQ(WriteEncodedString(R"(\f)", out), 2);
    EXPECT_EQ(std::strcmp(out.str().data(), "\f\0"), 0);
  }
  {
    std::stringstream out;
    EXPECT_EQ(WriteEncodedString(R"(\n)", out), 2);
    EXPECT_EQ(std::strcmp(out.str().data(), "\n\0"), 0);
  }
  {
    std::stringstream out;
    EXPECT_EQ(WriteEncodedString(R"(\r)", out), 2);
    EXPECT_EQ(std::strcmp(out.str().data(), "\r\0"), 0);
  }
  {
    std::stringstream out;
    EXPECT_EQ(WriteEncodedString(R"(\t)", out), 2);
    EXPECT_EQ(std::strcmp(out.str().data(), "\t\0"), 0);
  }
  {
    std::stringstream out;
    EXPECT_EQ(WriteEncodedString(R"(\v)", out), 2);
    EXPECT_EQ(std::strcmp(out.str().data(), "\v\0"), 0);
  }
}

TEST(Test, WriteEncodedStringEscapedNumber) {
  {
    std::stringstream out;
    EXPECT_EQ(WriteEncodedString(R"(\2)", out), 2);
    EXPECT_EQ(std::strcmp(out.str().data(), "\2\0"), 0);
  }
  {
    std::stringstream out;
    EXPECT_EQ(WriteEncodedString(R"(\33)", out), 2);
    EXPECT_EQ(std::strcmp(out.str().data(), "\33\0"), 0);
  }
  {
    std::stringstream out;
    EXPECT_EQ(WriteEncodedString(R"(\345)", out), 2);
    EXPECT_EQ(std::strcmp(out.str().data(), "\345\0"), 0);
  }
  {
    std::stringstream out;
    EXPECT_EQ(WriteEncodedString(R"(\3456)", out), 3);
    EXPECT_EQ(std::strcmp(out.str().data(), "\3456\0"), 0);
  }
  {
    std::stringstream out;
    EXPECT_EQ(WriteEncodedString(R"(\345foo)", out), 5);
    EXPECT_EQ(std::strcmp(out.str().data(), "\345foo\0"), 0);
  }
}

TEST(Test, WriteEncodedStringMix) {
  std::stringstream out;
  EXPECT_EQ(WriteEncodedString(R"(foo\215bar\n)", out), 9);
  EXPECT_EQ(std::strcmp(out.str().data(), "foo\215bar\n\0"), 0);
}

}  // namespace
}  // namespace lucid
