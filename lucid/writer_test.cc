#include "lucid/writer.h"

#include <cstdio>
#include <filesystem>
#include <string>
#include <string_view>

#include "gmock/gmock.h"
#include "gtest/gtest.h"
#include "lucid/file.h"

namespace lucid {
namespace {

TEST(StringWriterTest, MultipleWrites) {
  std::string out;

  auto write = StringWriter(out);
  write("foo");
  write("bar");
  write("baz");

  EXPECT_EQ(out, "foobarbaz");
}

TEST(StringWriterTest, PartialWrite) {
  std::string out;

  std::string_view s = "foobarbaz";
  auto write = StringWriter(out);
  write(s.substr(3, 3));

  EXPECT_EQ(out, "bar");
}

TEST(FileWriterTest, MultipleWrites) {
  const std::string path = testing::SrcDir() + "test";
  std::FILE* file = std::fopen(path.c_str(), "w+");

  auto write = FileWriter(file);
  write("foo");
  write("bar");
  write("baz");
  ASSERT_EQ(std::fclose(file), 0);

  EXPECT_EQ(ReadFile(path), "foobarbaz");
}

TEST(FileWriterTest, PartialWrite) {
  const std::string path = testing::SrcDir() + "test";
  std::FILE* file = std::fopen(path.c_str(), "w+");

  std::string_view s = "foobarbaz";
  auto write = FileWriter(file);
  write(s.substr(3, 3));
  ASSERT_EQ(std::fclose(file), 0);

  EXPECT_EQ(ReadFile(path), "bar");
}

}  // namespace
}  // namespace lucid
