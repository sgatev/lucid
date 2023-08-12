#include "lucid/writer.h"

#include <cstdio>
#include <filesystem>
#include <fstream>
#include <string>
#include <string_view>

#include "gmock/gmock.h"
#include "gtest/gtest.h"

namespace lucid {
namespace {

std::string ReadFile(std::string_view path) {
  std::ifstream file(path);
  std::string content;

  file.seekg(0, std::ios::end);
  content.reserve(file.tellg());

  file.seekg(0, std::ios::beg);
  content.assign((std::istreambuf_iterator<char>(file)),
                 std::istreambuf_iterator<char>());

  return content;
}

TEST(StringWriterTest, MultipleWrites) {
  std::string out;

  auto write = StringWriter(out);
  write("foo");
  write("bar");
  write("!");

  EXPECT_EQ(out, "foobar!");
}

TEST(StringWriterTest, PartialWrite) {
  std::string out;

  std::string_view s = "foobar!";
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
  write("!");
  ASSERT_EQ(std::fclose(file), 0);

  EXPECT_EQ(ReadFile(path), "foobar!");
}

TEST(FileWriterTest, PartialWrite) {
  const std::string path = testing::SrcDir() + "test";
  std::FILE* file = std::fopen(path.c_str(), "w+");

  std::string_view s = "foobar!";
  auto write = FileWriter(file);
  write(s.substr(3, 3));
  ASSERT_EQ(std::fclose(file), 0);

  EXPECT_EQ(ReadFile(path), "bar");
}

}  // namespace
}  // namespace lucid
